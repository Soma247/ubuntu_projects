#include <concepts>
#include <iostream>

template<typename T>
concept my_integral=std::integral<T>;

template <my_integral I>
void fn(const I& data){
   std::cout<<data<<" is integral"<<std::endl;
}

int main(int argc, char**argv){
   int i{2};
   fn(i);
   //fn(std::string{});
   return 0;
}
