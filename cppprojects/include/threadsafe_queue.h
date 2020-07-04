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
   template<typename T, typename Allocator=std::allocator<T>>
   class ts_queue final{
      using value_type=T;
      using allocator_type=Allocator;

      class empty_queue_error:public std::out_of_range{
         template<typename Msg>
         empty_queue_error(Msg&& msg):
            std::out_of_range{std::forward<Msg>(msg)}{}
      };

      struct node{
         static allocator_type s_alloc{};
         std::aligned_storage<sizeof(T),alignof(T)> m_storage;
         std::unique_ptr<node> m_next;
            
         T& get_data_ref(){
            return static_cast<T*>(&m_storage);
         }
         template<typename...Args>
         void construct_data(Args...args){
            std::allocator_traits<allocator_type>::construct(s_alloc,
                  &get_data_ref(),std::forward<Args>(args)...);
         }
         void destruct_data(){
            std::allocator_traits<allocator_type>::destruct(
                  s_alloc,&get_data_ref());
         }

         node& operator=(const node&)=delete;
         node& operator=(node&&)=delete;
      };

      mutable std::mutex m_head_mut;
      mutable std::mutex m_tail_mut;
      std::condition_variable m_cb;
      bool m_interrupt_waiting;
      std::unique_ptr<node> m_head;
      node* m_tail;
      void clear_unprotected(){
         while(m_head.get()!=m_tail){
            auto tmp=std::move(m_head);
            tmp->destruct_data();
            m_head=std::move(tmp->next);
         }
      }
      void clear(){
         std::scoped_lock sl(m_head_mut, m_tail_mut);
         this->clear_unprotected();
      }
   public:
      enum class pop_status:char{empty, ready,
         try_lock_fail, interrupted, timeout};
      
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
////////////////////////////////////////////////
      void pop_unprotected(value_type& ret){
         ret=std::move(m_queue.top());
         m_queue.pop();
      }

      pop_status try_pop_unprotected(value_type& ret){
         if(m_queue.empty())
            return pop_status::empty;
         pop_unprotected(ret);
         return pop_status::ready;
      }

      template <typename Oit, 
         typename difference_type=
            typename std::iterator_traits<Oit>::difference_type>
      Oit pop_n_unprotected(Oit out, difference_type count){
         while(count){
            if(m_queue.empty())
               return out;
            *out=std::move(m_queue.top());
            m_queue.pop();
            ++out;
            --count;
         }
         return out;
      }

      pop_status try_lock_and_pop(value_type& ret){
         std::unique_lock ul(m_mut, std::try_to_lock);
         if(!ul)
            return pop_status::try_lock_fail;
         return try_pop_unprotected(ret);
      }

      pop_status pop(value_type& ret){
         std::lock_guard lg{m_mut};
         return try_pop_unprotected(ret);
      }

      template <typename Oit, 
         typename difference_type=
            typename std::iterator_traits<Oit>::difference_type>
      Oit pop_n(Oit out, difference_type count){
         std::lock_guard ld{m_mut};
         return pop_n_unprotected(out,count);
      }

      pop_status wait_and_pop(value_type& ret){
         std::unique_lock ul{m_mut};
         if(m_interrupt_waiting)
            return pop_status::interrupted;
         m_cb.wait(ul,
               [this](){
                  return !m_queue.empty()||
                     m_interrupt_waiting;
               }
         );
         if(m_interrupt_waiting)
            return pop_status::interrupted;
         //queue isn't empty here
         pop_unprotected(ret);
         return pop_status::ready;
      }

      template <typename Oit, 
         typename difference_type=
            typename std::iterator_traits<Oit>::difference_type>
      Oit wait_and_pop_n(Oit out, difference_type count){
         std::unique_lock ul{m_mut};
         if(m_interrupt_waiting)return out;
         m_cb.wait(ul,
               [this](){
                  return !m_queue.empty() ||
                     m_interrupt_waiting;
               });
         if(m_interrupt_waiting)return out;
         //queue isn't empty here
         return pop_n_unprotected(out,count);
      }


      template<typename Rep, typename Period>
      pop_status wait_for_and_pop(value_type& ret,
            const std::chrono::duration<Rep, Period>& dur)
      {
         std::unique_lock ul{m_mut};
         if(m_interrupt_waiting)
            return pop_status::interrupted;
         bool status = m_cb.wait_for(ul, dur,
                           [this](){
                              return !m_queue.empty||
                              m_interrupt_waiting;
                           });
         if(!status)
            return pop_status::timeout;
         if(m_interrupt_waiting)
            return pop_status::interrupted;
         //queue isn't empty here!!!
         pop_unprotected(ret);
         return pop_status::ready;
      }

      template <typename Rep, typename Period, typename Oit, 
         typename difference_type=
            typename std::iterator_traits<Oit>::difference_type>
      std::pair<Oit,pop_status> 
      wait_for_and_pop_n(Oit out, 
            const std::chrono::duration<Rep, Period>& dur,
            difference_type count)
      {
         std::unique_lock ul{m_mut};
         if(m_interrupt_waiting)return {out,pop_status::interrupted};
         bool status=m_cb.wait_for(ul,dur,
               [this](){
                  return !m_queue.empty() ||
                     m_interrupt_waiting;
               });
         if(!status)
            return {out,pop_status::timeout};
         if(m_interrupt_waiting)return {out, pop_status::interrupted};
         //queue isn't empty here
         return {pop_n_unprotected(out,count),pop_status::ready};
      }

      template<typename Clock, typename Dur>
      pop_status wait_until_and_pop(value_type& ret,
            const std::chrono::time_point<Clock, Dur>& tp)
      {
         std::unique_lock ul{m_mut};
         if(m_interrupt_waiting)
            return pop_status::interrupted;
         bool status = m_cb.wait_until(ul, tp,
                           [this](){
                              return !m_queue.empty||
                              m_interrupt_waiting;
                           });
         if(!status)
            return pop_status::timeout;
         if(m_interrupt_waiting)
            return pop_status::interrupted;
         //queue isn't empty here!!!
         pop_unprotected(ret);
         return pop_status::ready;
      }

      template <typename Clock, typename Dur, typename Oit, 
         typename difference_type=
            typename std::iterator_traits<Oit>::difference_type>
      std::pair<Oit,pop_status> 
      wait_until_and_pop_n(Oit out, 
            std::chrono::time_point<Clock,Dur> tp,
                             difference_type count)
      {
         std::unique_lock ul{m_mut};
         if(m_interrupt_waiting)return {out,pop_status::interrupted};
         bool status=m_cb.wait_until(ul,tp,
               [this](){
                  return !m_queue.empty() ||
                     m_interrupt_waiting;
               });
         if(!status)
            return {out,pop_status::timeout};
         if(m_interrupt_waiting)return {out, pop_status::interrupted};
         //queue isn't empty here
         return {pop_n_unprotected(out,count),pop_status::ready};
      }



      void notify_one(){
         m_cb.notify_one();
      }
      void notify_all(){
         m_cb.notify_all();
      }
      void stop_waitings(){
         std::lock_guard lg{m_mut};
         m_interrupt_waiting=true;
         m_cb.notify_all();
      }
      template<typename...Args>
      void emplace(Args&&...args){
         std::lock_guard lg{m_mut};
         m_queue.emplace(std::forward<Args>(args)...);
      }
      void push(const value_type& val){
         std::lock_guard lg{m_mut};
         m_queue.push(val);
      }
      void push(value_type&& val){
         std::lock_guard lg{m_mut};
         m_queue.push(std::move(val));
      }
      template<typename...Args>
      void emplace_and_notify_one(Args&&...args){
         emplace(std::forward<Args>(args)...);
         m_cb.notify_one();
      }
      void push_and_notify_one(const value_type& val){
         push(val);
         m_cb.notify_one();
      }
      void push_and_notify_one(value_type&& val){
         push(std::move(val));
         m_cb.notify_one();
      }
      template<typename...Args>
      void emplace_and_notify_all(Args&&...args){
         emplace(std::forward<Args>(args)...);
         m_cb.notify_all();
      }
      void push_and_notify_all(const value_type& val){
         push(val);
         m_cb.notify_all();
      }
      void push_and_notify_all(value_type&& val){
         push(std::move(val));
         m_cb.notify_all();
      }
      template<typename InIt>
      void push_range_unprotected(InIt beg, InIt end){
         for(;beg!=end;++beg)
            m_queue.push(*beg);
      }
      template<typename InIt>
      void push_range(InIt beg, InIt end){
         std::lock_guard lg{m_mut};
         push_range_unprotected(beg,end);
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
         std::lock_guard lg{m_mut};
         return m_queue.empty();
      }
      void swap(ts_queue& other){
         using std::swap;
         std::scoped_lock sl(m_mut, other.m_mut);
         swap(m_queue,other.m_queue);
      }
      friend void swap(ts_queue& lhs, ts_queue& rhs){
         lhs.swap(rhs);
      }
   };

}

#endif//THREADSAFE_QUEUE_H
