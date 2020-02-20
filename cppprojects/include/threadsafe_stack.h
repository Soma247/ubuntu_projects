#include <mutex>
#include <stack>
#include <type_traits>

template <typename T,typename...A>
class threadsafe_stack{
   using stack_type=std::stack<T,A...>; 
   stack_type data_;
   std::mutex mut_;
public:
   threadsafe_stack()noexcept{}

   template <typename...Args,
              std::enable_if_t<
                 !std::is_same_v<
                    threadsafe_stack,
                    std::decay_t<
                       decltype(std::get<0>(
                                std::declval<std::tuple<Args...>>()))
                    >
                 >
              >
            >
      threadsafe_stack(Args&&...args)noexcept(
            noexcept(stack_type(std::declval<Args>()...))):
            data_(std::forward<Args>(args)...),
            mut_{}{}
};
