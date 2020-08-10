#ifndef THREADSAFE_QUEUE_H
#define THREADSAFE_QUEUE_H
#include <type_traits>
#include <exception>
#include <stdexcept>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <memory>
namespace ts_adv{
   template<typename T, typename ThreadSafeAlloc=std::allocator<T>>
   class ts_queue final{
   public:
      using value_type=T;
         
      class empty_queue_error:public std::out_of_range{
         template<typename Msg>
         empty_queue_error(Msg&& msg):
            std::out_of_range{std::forward<Msg>(msg)}{}
      };

      enum class pop_status:char{empty, ready,
         try_lock_fail, interrupted, timeout};
   private:
      struct node{
         /*using node_allocator_type = 
            typename alloc_traits::rebind_alloc<node>;
         using node_alloc_traits = 
            typename alloc_traits::rebind_traits<node>;*/
         std::aligned_storage<sizeof(T),alignof(T)> m_storage;
         std::unique_ptr<node> m_next;
         bool initialized;

         node():m_storage{}, m_next{}, initialized{false}{}
         ~node(){
            if(initialized)
               alloc_traits::destroy(
                     s_alloc, get_data_addr());
         }
         void* operator new(std::size_t sz){
            return alloc_traits::allocate(s_alloc, sz);
         }
         void operator delete(void* ptr, std::size_t sz){ 
            alloc_traits::deallocate(s_alloc, static_cast<node*>(ptr), sz);
         }
         T* get_data_addr(){
            return reinterpret_cast<T*>(&m_storage);
         }
         
         template<typename...Args>
         void construct_data(Args...args){
            alloc_traits::construct(s_alloc,
                  get_data_addr(),std::forward<Args>(args)...);
            initialized=true;
         }
         void destruct_data(){
            alloc_traits::destruct(
               s_alloc, get_data_addr());
            initialized=false;
         }
         node& operator=(const node&)=delete;
         node& operator=(node&&)=delete;
      };//node
      using allocator_type=
         typename std::allocator_traits<ThreadSafeAlloc>::
                                            template rebind_alloc<node>;
      using alloc_traits=std::allocator_traits<allocator_type>;

      static allocator_type s_alloc;
      mutable std::mutex m_head_mut;
      mutable std::mutex m_tail_mut;
      std::condition_variable m_cb;
      bool m_interrupt_waiting;
      std::unique_ptr<node> m_head;
      node* m_tail;

      void clear_unprotected(){
         while(m_head.get()!=m_tail){
            m_head=std::move(m_head->m_next);//destruct in reset with data
         }
      }
      node* get_tail_protected()const{
         std::lock_guard lg{m_tail_mut};
         return m_tail;
      }
      
      void pop_unprotected_unchecked(value_type& ret){
         ret=std::move(*m_head->get_data_addr());//move item from head
         m_head=std::move(m_head->m_next);
      }
      
      pop_status pop_unprotected(value_type& ret,
            node* possible_tail)
      {
         if(m_head.get()==possible_tail)
            return pop_status::empty;
         this->pop_unprotected_unchecked(ret);
         return pop_status::ready;
      }


      template<typename Oit, typename difference_type=
         typename std::iterator_traits<Oit>::difference_type>
      Oit pop_n_unprotected(Oit out, 
            difference_type count, node* possible_tail)
      {
         for(; count; ++out, --count){
            if(m_head.get()==possible_tail)//check empty possibility
               return out;
            *out=std::move(*m_head->get_data_addr());
            m_head=std::move(m_head->m_next);
         }
         return out;
      } 

   public:
      ts_queue():m_head_mut{}, m_tail_mut{}, m_cb{},
         m_interrupt_waiting{false},
         m_head{new node{}},
         m_tail{m_head.get()}{}

      ts_queue(const ts_queue&)=delete;
      ts_queue(ts_queue&& other):m_head_mut{}, m_tail_mut{}, m_cb{},
         m_interrupt_waiting{false},
         m_head{}, m_tail{nullptr}
      {    
         std::scoped_lock sl(other.m_head_mut,other.m_tail_mut);
         m_head=std::move(other.m_head);
         m_tail=other.m_tail;
         other.m_head.reset(new node);
         other.m_tail=other.m_head.get();
      }

      ts_queue& operator = (const ts_queue&)=delete;
      ts_queue& operator = (ts_queue&& other){
         std::scoped_lock sl(m_head_mut,m_tail_mut,
                     other.m_head_mut,other.m_tail_mut);
         this->clear_unprotected();
         m_head=std::move(other.m_head);
         other.m_head.reset(new node);
         m_tail=other.m_tail;
         other.m_tail=other.m_head.get();
         return *this;
      }

      void clear(){
         std::scoped_lock sl(m_head_mut, m_tail_mut);
         this->clear_unprotected();
      }

      pop_status pop(value_type& ret){
         std::lock_guard lg{m_head_mut};
         return pop_unprotected(ret,this->get_tail_protected());
      }

      template <typename Oit, 
         typename difference_type=
            typename std::iterator_traits<Oit>::difference_type>
      Oit pop_n(Oit out, difference_type count){
         std::lock_guard lg{m_head_mut};
         return pop_n_unprotected(out, count,
                     this->get_tail_protected());
      }

      pop_status wait_and_pop(value_type& ret){
         std::unique_lock ul{m_head_mut};
         if(m_interrupt_waiting)
            return pop_status::interrupted;
         m_cb.wait(ul,
               [this](){
               //update tail, check for empty & interrupt
                  return m_head.get() !=
                    this->get_tail_protected() || 
                        m_interrupt_waiting;
               }
         );
         if(m_interrupt_waiting)
            return pop_status::interrupted;
         //queue isn't empty here
         this->pop_unprotected_unchecked(ret);
         return pop_status::ready;
      }
      template <typename Oit, 
         typename difference_type=
            typename std::iterator_traits<Oit>::difference_type>
      Oit wait_and_pop_n(Oit out, difference_type count){
         std::unique_lock ul{m_head_mut};
         if(m_interrupt_waiting)return out;
         node* possible_tail{nullptr};
         m_cb.wait(ul,
               [this,&possible_tail](){
                  return m_head.get()!=
                     (possible_tail=this->get_tail_protected()) ||
                        m_interrupt_waiting;
               });
         if(m_interrupt_waiting)return out;
         //queue isn't empty here
         return pop_n_unprotected(out, count, possible_tail);
      }

      template<typename Rep, typename Period>
      pop_status wait_for_and_pop(value_type& ret,
            const std::chrono::duration<Rep, Period>& dur)
      {
         std::unique_lock ul{m_head_mut};
         if(m_interrupt_waiting)
            return pop_status::interrupted;
         bool status=m_cb.wait_for(ul, dur,
                        [this](){
                        //update tail, check for empty & interrupt
                           return m_head.get() !=
                              this->get_tail_protected() || 
                                 m_interrupt_waiting;
                        }
                     );
         if(!status)return pop_status::timeout;
         if(m_interrupt_waiting)
            return pop_status::interrupted;
         //queue isn't empty here
         this->pop_unprotected_unchecked(ret);
         return pop_status::ready;
      }

      template <typename Rep, typename Period, typename Oit, 
         typename difference_type=
            typename std::iterator_traits<Oit>::difference_type>
      std::pair<Oit,pop_status> 
      wait_for_and_pop_n(Oit out, difference_type count, 
            const std::chrono::duration<Rep, Period>& dur)
      {
         std::unique_lock ul{m_head_mut};
         if(m_interrupt_waiting)return {out, pop_status::interrupted};
         node* possible_tail{nullptr};
         bool status=m_cb.wait_for(ul, dur,
               [this,&possible_tail](){
                  return m_head.get()!=
                     (possible_tail=this->get_tail_protected()) ||
                        m_interrupt_waiting;
               });
         if(!status)return {out, pop_status::timeout};
         if(m_interrupt_waiting)
            return {out, pop_status::interrupted};
         //queue isn't empty here
         return {pop_n_unprotected(out,count, possible_tail),
                  pop_status::ready};
      }
//////////////////////////
      template<typename Clock, typename Dur>
      pop_status wait_until_and_pop(value_type& ret,
            const std::chrono::time_point<Clock, Dur>& tp)
      {
         std::unique_lock ul{m_head_mut};
         if(m_interrupt_waiting)
            return pop_status::interrupted;
         bool status=m_cb.wait_until(ul, tp,
                        [this](){
                        //update tail, check for empty & interrupt
                           return m_head.get() !=
                              this->get_tail_protected() || 
                                 m_interrupt_waiting;
                        }
                     );
         if(!status)return pop_status::timeout;
         if(m_interrupt_waiting)
            return pop_status::interrupted;
         //queue isn't empty here
         this->pop_unprotected_unchecked(ret);
         return pop_status::ready;
      }

      template <typename Clock, typename Dur, typename Oit, 
         typename difference_type=
            typename std::iterator_traits<Oit>::difference_type>
      std::pair<Oit,pop_status> 
      wait_until_and_pop_n(Oit out, difference_type count,
            std::chrono::time_point<Clock,Dur> tp)
      {
         std::unique_lock ul{m_head_mut};
         if(m_interrupt_waiting)return {out,pop_status::interrupted};
         node* possible_tail{nullptr};
         bool status=m_cb.wait_until(ul, tp,
               [this,&possible_tail](){
                  return m_head.get()!=
                     (possible_tail=this->get_tail_protected()) ||
                        m_interrupt_waiting;
               });
         if(!status)return {out, pop_status::timeout};
         if(m_interrupt_waiting)
            return {out, pop_status::interrupted};
         //queue isn't empty here
         return {pop_n_unprotected(out,count, possible_tail),
                  pop_status::ready};
      }

      void notify_one(){
         m_cb.notify_one();
      }
      void notify_all(){
         m_cb.notify_all();
      }
      void stop_waitings(){
         {
            std::lock_guard lg{m_head_mut};
            m_interrupt_waiting=true;
         }
         m_cb.notify_all();
      }
      template<typename...Args>
      void emplace(Args&&...args){
         auto pnew_tail=std::make_unique<node>();
         std::lock_guard lg{m_tail_mut};
         m_tail->construct_data(std::forward<Args>(args)...);
         m_tail->m_next=std::move(pnew_tail);
         m_tail=m_tail->m_next.get();
      }
      
      void push(const value_type& val){
        this->emplace(val); 
      }
      void push(value_type&& val){
         this->emplace(std::move(val));
      }
      template<typename...Args>
      void emplace_and_notify_one(Args&&...args){
         this->emplace(std::forward<Args>(args)...);
         m_cb.notify_one();
      }
      void push_and_notify_one(const value_type& val){
         this->push(val);
         m_cb.notify_one();
      }
      void push_and_notify_one(value_type&& val){
         this->push(std::move(val));
         m_cb.notify_one();
      }
      template<typename...Args>
      void emplace_and_notify_all(Args&&...args){
         this->emplace(std::forward<Args>(args)...);
         m_cb.notify_all();
      }
      void push_and_notify_all(const value_type& val){
         this->push(val);
         m_cb.notify_all();
      }
      void push_and_notify_all(value_type&& val){
         this->push(std::move(val));
         m_cb.notify_all();
      }
      template<typename InIt>
      void push_range(InIt beg, InIt end){
         if(beg==end)return;
         auto it=std::next(beg);
         
         //create subqueue with pole tail node
         auto head_sub{std::make_unique<node>()};
         auto tail_sub{head_sub.get()};

         for(; it!=end; ++it){
            tail_sub->construct_data(*it);
            tail_sub->m_next=std::make_unique<node>();
            tail_sub=tail_sub->m_next.get();
         }
         
         std::lock_guard lg{m_tail_mut};
         
         //append subqueue to list of nodes
         m_tail->construct_data(*beg);//insert first elem to tail_node
         m_tail->m_next=std::move(head_sub);//append subseq
         m_tail=tail_sub;//set tail ptr to tail of subseq
      }
      template<typename InIt>
      void push_range_and_notify_one(InIt beg, InIt end){
         push_range(beg, end);
         m_cb.notify_one();
      }
      template<typename InIt>
      void push_range_and_notify_all(InIt beg, InIt end){
         push_range(beg, end);
         m_cb.notify_all();
      }
      bool empty()const{
         std::scoped_lock sl(m_head_mut, m_tail_mut);
         return m_head.get()==m_tail;
      }
   };
   template<typename T, typename A>
   inline typename ts_queue<T,A>::allocator_type ts_queue<T,A>::s_alloc{};

}

#endif//THREADSAFE_QUEUE_H
