#ifndef THREADSAFE_STACK_H
#define THREADSAFE_STACK_H
#include <type_traits>
#include <exception>
#include <stdexcept>
#include <stack>
#include <chrono>
#include <mutex>
namespace ts_adv{
   template<typename T, typename Container>
   class ts_stack{
      using stack_type=std::stack<T,Container>;
      using value_type=typename stack_type::value_type;

      class empty_stack_error:public std::out_of_range{
         template<typename Msg>
         empty_stack_error(Msg&& msg):
            std::out_of_range{std::forward<Msg>(msg)}{}
      };

      mutable std::mutex m_mut;
      std::condition_variable m_cb;
      bool m_iterrupt_waits;
      stack_type m_stack;
   public:
      enum class pop_status:char{empty, ready,
         try_lock_fail, iterrupted, timeout};

      template<typename Arg, typename...Args,
         typename=std::enable_if<!std::is_same_v<ts_stack,Arg>>>
      ts_stack(Arg&& arg, Args&&...args):
         m_mut{},m_cb{},m_iterrupt_waits{false},
         m_stack(std::forward<Arg>(arg),std::forward<Args>(args)...){}

      ts_stack(const ts_stack&)=delete;
      ts_stack(ts_stack&& rhs):m_mut{}, m_cb{},
         m_iterrupt_waits{false}
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
         ret=std::move(m_stack.top());
         m_stack.pop();
      }
      pop_status try_pop_unprotected(value_type& ret){
         if(m_stack.empty())
            return pop_status::empty;
         pop_unprotected(ret);
         return pop_status::ready;
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
      pop_status wait_and_pop(value_type& ret){
         std::unique_lock ul{m_mut};
         if(m_iterrupt_waits)
            return pop_status::iterrupted;
         m_cb.wait(ul,
               [](){
                  return !m_stack.empty()||
                  m_iterrupt_waits;
               }
         );
         if(m_iterrupt_waits)
            return pop_status::iterrupted;
         //stack isn't empty here
         pop_unprotected(ret);
         return pop_status::ready;
      }
      template<typename Rep, typename Period>
      pop_status wait_and_pop_for(value_type& ret,
            const std::chrono::duration<Rep, Period>& dur)
      {
         std::unique_lock ul{m_mut};
         if(m_iterrupt_waits)
            return pop_status::iterrupted;
         bool status = m_cb.wait_for(ul,
                           [](){
                              return !m_stack.empty||
                              m_iterrupt_waits;
                           });
         if(!status)
            return pop_status::timeout;
         if(m_iterrupt_waits)
            return pop_status::iterrupted;
         //stack isn't empty here!!!
         pop_unprotected(ret);
         return pop_status::ready;
      }
      template<typename Clock, typename Dur>
      pop_status wait_and_pop_until(value_type& ret,
            const std::chrono::timepoint<Clock, Dur>& tp)
      {
         std::unique_lock ul{m_mut};
         if(m_iterrupt_waits)
            return pop_status::iterrupted;
         bool status = m_cb.wait_until(ul,
                           [](){
                              return !m_stack.empty||
                              m_iterrupt_waits;
                           });
         if(!status)
            return pop_status::timeout;
         if(m_iterrupt_waits)
            return pop_status::iterrupted;
         //stack isn't empty here!!!
         pop_unprotected(ret);
         return pop_status::ready;
      }

      void stop_waitings(){
         std::lock_guard lg{m_mut};
         m_iterrupt_waits=true;
         m_cb.notify_all();
      }
      template<typename...Args>
      void emplace(Args&&...args){
         std::lock_guard lg{m_mut};
         m_stack.emplace(std::forward<Args>(args)...);
      }
      void push(const value_type& val){
         std::lock_guard lg{m_mut};
         m_stack.push(val);
      }
      void push(value_type&& val){
         std::lock_guard lg{m_mut};
         m_stack.push(std::move(val));
      }
      bool empty()const{
         std::lock_guard lg{m_mut};
         return m_stack.empty();
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
