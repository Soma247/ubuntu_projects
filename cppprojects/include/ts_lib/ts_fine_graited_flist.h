#ifndef THREADSAFE_FINE_GRAINED_FORWARD_LIST_H
#define THREADSAFE_FINE_GRAINED_FORWARD_LIST_H
#include <type_traits>
#include <mutex>
#include <memory>

namespace ts_adv{

   template<typename T, class Alloc=std::allocator<T>>
   class tfg_flist{
   public:
      using value_type = T;
      using allocator_type = Alloc;
      using alloc_traits_type=std::allocator_traits<Alloc>;
      using difference_type = typename Alloc::difference_type;
      using size_type = typename Alloc::size_type;
   private:
      struct node_type{
         std::mutex m_mut;
         std::shared_ptr<T> m_pdata;
         std::unique_ptr<node_type> m_next;
         node_type(){}
         explicit node_type(const T& val):
            m_pdata{std::make_shared<T>(val)}
         {
         }
         explicit node_type(T&& val):
            m_pdata{std::make_shared<T>(std::move(val))}
         {
         }
      };

      std::unique_ptr<node_type> m_before_begin;
      allocator_type m_alloc;

      void fill_list_by_default(size_type sz){
         auto current = m_before_begin.get();
         for(;sz;--sz){
            current->m_next=std::make_unique<node_type>();
            current=current->m_next;
         }
      }
      
      void push_node_front(std::unique_ptr<node_type> new_beg){
         std::lock_guard lg{m_before_begin->m_mut};
         new_beg->m_next=std::move(m_before_begin->m_next);
         m_before_begin->m_next=std::move(new_beg);
      }

   public:
      tfg_flist(): 
         m_before_begin(std::make_unique<node_type>()),
         m_alloc{}
      {
      }
      explicit tfg_flist(size_type size):
         m_before_begin(std::make_unique<node_type>()),
         m_alloc{}
      {
         fill_list_by_default(size);
      }
      template<class Allocator>
      explicit tfg_flist(size_type size, Allocator&& alloc):
         m_before_begin(std::make_unique<node_type>()),
         m_alloc{std::forward<Allocator>(alloc)}
      {
         fill_list_by_default(size);
      }

      tfg_flist(const tfg_flist&)=delete;
      tfg_flist(tfg_flist&&)=delete;
      tfg_flist& operator = (const tfg_flist&)=delete;
      tfg_flist& operator = (tfg_flist&&)=delete;

      void push_front(const value_type& val){
         this->push_node_front(std::make_unique<node_type>(val));
      }
      void push_front(value_type&& val){
         this->push_node_front(std::make_unique<node_type>(std::move(val)));
      }

      template<typename UnOp>
      void for_each(UnOp op){// T(op)(std::shared_ptr<T>)
         static_assert(
               std::is_invocable_v<UnOp,std::shared_ptr<T>>,
               "unary operation must expect" 
               "std::shared_ptr<T> type\n");
         //m_before_begin.get() can't change
         node_type* prev_cptr = m_before_begin.get();
         std::unique_lock prev_lock {prev_cptr->m_mut};
         //prev_cptr->m_next under lock, can't change by another thread
         while(prev_cptr->m_next){
            //prev_cptr->m_next.get() under prev mutex lock
            node_type* cur_cptr = prev_cptr->m_next.get();
            std::unique_lock cur_lock{cur_cptr->m_mut};
            prev_lock.unlock();
            op(cur_cptr->m_pdata);
            prev_cptr = cur_cptr;
            prev_lock=std::move(cur_lock);
         }
      }

      template<typename UnOp, typename UnPred> 
      void for_first_of(UnOp op, UnPred p){//bool T()(std::shared_ptr<T>)
         static_assert(
         std::is_invocable_r_v<bool,UnPred,std::shared_ptr<T>>,
               "unary predicate  must expect" 
               "std::shared_ptr<T> type and return bool-convertible\n");
         static_assert(
               std::is_invocable_v<UnOp,std::shared_ptr<T>>,
               "unary operation must expect const " 
               "std::shared_ptr<T> type\n");
         //m_before_begin.get() can't change
         node_type* prev_cptr = m_before_begin.get();
         std::unique_lock prev_lock {prev_cptr->m_mut};
         //prev_cptr->m_next under lock, can't change by another thread
         while(prev_cptr->m_next){
            //prev_cptr->m_next.get() under prev mutex lock
            node_type* cur_cptr = prev_cptr->m_next.get();
            std::unique_lock cur_lock{cur_cptr->m_mut};
            prev_lock.unlock();
            if(p(cur_cptr->m_pdata)){
               op(cur_cptr->m_pdata);
               return;
            }
            prev_cptr = cur_cptr;
            prev_lock=std::move(cur_lock);
         }
      }

      bool empty()const{
         std::lock_guard lg{m_before_begin->m_mut};
         if(m_before_begin->m_next)
            return false;
         return true;
      }
      template<typename UnPred>
      void remove_if(UnPred p){
         static_assert(std::is_invocable_r_v<bool, UnPred,
               std::shared_ptr<T>>, "unary predicate must expect"
               "shared_ptr<T> and return "
               "bool-convertible result\n");
         //m_before_begin.get() can't change
         node_type* prev_cptr = m_before_begin.get();
         std::unique_lock prev_lock {prev_cptr->m_mut};
         while(prev_cptr->m_next){
            //prev_cptr->m_next.get() under prev mutex lock
            node_type* cur_cptr = prev_cptr->m_next.get();
            std::unique_lock cur_lock{cur_cptr->m_mut};
            if(p(cur_cptr->m_pdata)){
               //move current node outside a list, unlock it and delete
               std::unique_ptr<node_type> tmp{std::move(prev_cptr->m_next)};
               prev_cptr->m_next=std::move(tmp->m_next);
               cur_lock.unlock();
            }
            else{
               //go to next node
               prev_lock.unlock();
               prev_cptr = cur_cptr;
               prev_lock=std::move(cur_lock);
            }
         }
      }
      template<typename UnPred>
      bool remove_first_if(UnPred p){
         static_assert(std::is_invocable_r_v<bool, UnPred,
               std::shared_ptr<T>>, "unary predicate must expect"
               "shared_ptr<T> and return "
               "bool-convertible result\n");
         //m_before_begin.get() can't change
         node_type* prev_cptr = m_before_begin.get();
         std::unique_lock prev_lock {prev_cptr->m_mut};
         while(prev_cptr->m_next){
            //prev_cptr->m_next.get() under prev mutex lock
            node_type* cur_cptr = prev_cptr->m_next.get();
            std::unique_lock cur_lock{cur_cptr->m_mut};
            if(p(cur_cptr->m_pdata)){
               //move current node outside a list, unlock it and delete
               std::unique_ptr<node_type> tmp{std::move(prev_cptr->m_next)};
               prev_cptr->m_next=std::move(tmp->m_next);
               cur_lock.unlock();
               return true;
            }
            else{
               //go to next node
               prev_lock.unlock();
               prev_cptr = cur_cptr;
               prev_lock=std::move(cur_lock);
            }
         }
         return false;
      }
      void clear(){
         this->remove_if([](auto){return true;});
      }
      ~tfg_flist(){}

   };
}//ts_adv
#endif//THREADSAFE_FINE_GRAIED_FORWARD_LIST_H
