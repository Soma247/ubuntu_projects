#ifndef THREADSAFE_STACK_H
#define THREADSAFE_STACK_H
#include <type_traits>
#include <exception>
#include <stdexcept>
#include <stack>
#include <chrono>
#include <mutex>
#include <condition_variable>
namespace ts_adv{
   template<typename T, typename Container=std::deque<T>>
   class ts_stack final{
      using stack_type = std::stack<T,Container>;
      using value_type = typename stack_type::value_type;
      using size_type = typename stack_type::size_type;
      using reference = typename stack_type::reference;
      using const_reference = typename stack_type::const_reference;
      class empty_stack_error:public std::out_of_range{
         template<typename Msg>
         empty_stack_error(Msg&& msg):
            std::out_of_range{std::forward<Msg>(msg)}{}
      };

      mutable std::mutex m_mut;
      std::condition_variable m_cb;
      bool m_interrupt_waiting;
      stack_type m_stack;
   public:
      enum class pop_status:char{empty, ready,
         try_lock_fail, interrupted, timeout};
      
      ts_stack():m_mut{},m_cb{},
         m_interrupt_waiting{false},m_stack{}{}

      template<typename Arg, typename...Args,
         typename=std::enable_if<!std::is_same_v<ts_stack,Arg>>>
      ts_stack(Arg&& arg, Args&&...args):
         m_mut{},m_cb{},m_interrupt_waiting{false},
         m_stack(std::forward<Arg>(arg),std::forward<Args>(args)...){}

      ts_stack(const ts_stack&)=delete;
      ts_stack(ts_stack&& rhs):m_mut{}, m_cb{},
         m_interrupt_waiting{false}
      {    
         std::lock_guard lg{rhs.m_mut};
         m_stack=std::move(rhs.m_stack);
      }

      ts_stack& operator = (const ts_stack&)=delete;
      ts_stack& operator = (ts_stack&& rhs){
         std::scoped_lock sl(m_mut,rhs.m_mut);
         m_stack=std::move(rhs.stack_);
         return *this;
      }

      void pop_unprotected(value_type& ret){
         ret=std::move(m_stack.top());//if noexcept?
         m_stack.pop();
      }

      pop_status try_pop_unprotected(value_type& ret){
         if(m_stack.empty())
            return pop_status::empty;
         this->pop_unprotected(ret);
         return pop_status::ready;
      }

      template <typename Oit, 
         typename difference_type=
            typename std::iterator_traits<Oit>::difference_type>
      Oit pop_n_unprotected(Oit out, difference_type count){
         while(count){
            if(m_stack.empty())
               return out;
            *out=std::move(m_stack.top());
            m_stack.pop();
            ++out;
            --count;
         }
         return out;
      }

      pop_status try_lock_and_pop(value_type& ret){
         std::unique_lock ul(m_mut, std::try_to_lock);
         if(!ul)
            return pop_status::try_lock_fail;
         return this->try_pop_unprotected(ret);
      }

      pop_status pop(value_type& ret){
         std::lock_guard lg{m_mut};
         return this->try_pop_unprotected(ret);
      }

      template <typename Oit, 
         typename difference_type=
            typename std::iterator_traits<Oit>::difference_type>
      Oit pop_n(Oit out, difference_type count){
         std::lock_guard ld{m_mut};
         return this->pop_n_unprotected(out,count);
      }

      pop_status wait_and_pop(value_type& ret){
         std::unique_lock ul{m_mut};
         if(m_interrupt_waiting)
            return pop_status::interrupted;
         m_cb.wait(ul,
               [this](){
                  return !m_stack.empty()||
                     m_interrupt_waiting;
               }
         );
         if(m_interrupt_waiting)
            return pop_status::interrupted;
         //stack isn't empty here
         this->pop_unprotected(ret);
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
                  return !m_stack.empty() ||
                     m_interrupt_waiting;
               });
         if(m_interrupt_waiting)return out;
         //stack isn't empty here
         return this->pop_n_unprotected(out,count);
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
                              return !m_stack.empty()||
                              m_interrupt_waiting;
                           });
         if(!status)
            return pop_status::timeout;
         if(m_interrupt_waiting)
            return pop_status::interrupted;
         //stack isn't empty here!!!
         this->pop_unprotected(ret);
         return pop_status::ready;
      }

      template <typename Rep, typename Period, typename Oit, 
         typename difference_type=
            typename std::iterator_traits<Oit>::difference_type>
      std::pair<Oit,pop_status> 
      wait_for_and_pop_n(Oit out, difference_type count, 
            const std::chrono::duration<Rep, Period>& dur)
      {
         std::unique_lock ul{m_mut};
         if(m_interrupt_waiting)
            return {out,pop_status::interrupted};
         bool status=m_cb.wait_for(ul,dur,
               [this](){
                  return !m_stack.empty() ||
                     m_interrupt_waiting;
               });
         if(!status)
            return {out,pop_status::timeout};
         if(m_interrupt_waiting)return {out, pop_status::interrupted};
         //stack isn't empty here
         return {this->pop_n_unprotected(out,count),pop_status::ready};
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
                              return !m_stack.empty()||
                              m_interrupt_waiting;
                           });
         if(!status)
            return pop_status::timeout;
         if(m_interrupt_waiting)
            return pop_status::interrupted;
         //stack isn't empty here!!!
         this->pop_unprotected(ret);
         return pop_status::ready;
      }

      template <typename Clock, typename Dur, typename Oit, 
         typename difference_type=
            typename std::iterator_traits<Oit>::difference_type>
      std::pair<Oit,pop_status> 
      wait_until_and_pop_n(Oit out, difference_type count, 
            std::chrono::time_point<Clock,Dur> tp)
      {
         std::unique_lock ul{m_mut};
         if(m_interrupt_waiting)return {out,pop_status::interrupted};
         bool status=m_cb.wait_until(ul,tp,
               [this](){
                  return !m_stack.empty() ||
                     m_interrupt_waiting;
               });
         if(!status)
            return {out,pop_status::timeout};
         if(m_interrupt_waiting)return {out, pop_status::interrupted};
         //stack isn't empty here
         return {this->pop_n_unprotected(out,count),pop_status::ready};
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
         m_stack.emplace(std::forward<Args>(args)...);
      }
      void push_unprotected(const value_type& val){
         m_stack.push(val);
      }
      void push_unprotected(value_type&& val){
         m_stack.push(std::move(val));
      }
      void push(const value_type& val){
         std::lock_guard lg{m_mut};
         this->push_unprotected(val);
      }
      void push(value_type&& val){
         std::lock_guard lg{m_mut};
         this->push_unprotected(std::move(val));
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
      void push_range_unprotected(InIt beg, InIt end){
         for(;beg!=end;++beg)
            m_stack.push(*beg);
      }
      template<typename InIt>
      void push_range(InIt beg, InIt end){
         std::lock_guard lg{m_mut};
         this->push_range_unprotected(beg,end);
      }
      template<typename InIt>
      void push_range_and_notify_one(InIt beg, InIt end){
         this->push_range(beg, end);
         m_cb.notify_one();
      }
      template<typename InIt>
      void push_range_and_notify_all(InIt beg, InIt end){
         this->push_range(beg, end);
         m_cb.notify_all();
      }
      bool empty()const{
         std::lock_guard lg{m_mut};
         return m_stack.empty();
      }
      void clear(){
         std::lock_guard lg{m_mut};
         size_type sz=m_stack.size();
         while(sz){
            m_stack.pop();
            --sz;
         }
      }
      void swap(ts_stack& other){
         using std::swap;
         std::scoped_lock sl(m_mut, other.m_mut);
         swap(m_stack,other.m_stack);
      }
      friend void swap(ts_stack& lhs, ts_stack& rhs){
         lhs.swap(rhs);
      }
   };

}

#endif//THREADSAFE_STACK_H
