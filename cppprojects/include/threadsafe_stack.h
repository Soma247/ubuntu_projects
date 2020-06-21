#ifndef THREADSAFE_STACK_H
#define THREADSAFE_STACK_H
#include <type_traits>
#include <stack>
#include <mutex>
namespace ts_adv{
  template<typename T, typename Container>
     class ts_stack{
        using stack_type=std::stack<T,Container>;
        using value_type=typename stack_type::value_type;
        std::mutex mut_;
        stack_type stack_;

     public:
        template<typename Arg, typename...Args,
           typename=std::enable_if<!std::is_same_v<ts_stack,Arg>>>
        ts_stack(Arg&& arg, Args&&...args):
          mut_{},
          stack_(std::forward<Arg>(arg),std::forward<Args>(args)...){}

        ts_stack(const ts_stack&)=delete;
        ts_stack(ts_stack&& rhs)=default;
        ts_stack& operator = (const ts_stack&)=delete;

        void pop(value_type& ret){//ret=move_if_noexcept(top)
           std::lock_guard lg{mut_};
           ret=std::move_if_noexcept(stack_.top());
           stack_.pop();
        }
     };












}

#endif//THREADSAFE_STACK_H
