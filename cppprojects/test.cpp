#include <type_traits>
#include <iostream>
#include <memory>
#include <vector>
#include <functional>
#include <cassert>
template<typename T>
struct TF;


void fun()noexcept{
   assert(false);
}
int main(int argc, char* argv[]){
   const int i{3};
   auto& cri{i};
  // TF<decltype(cri)>{};
   std::cout<<"hi!"<<std::endl;
   auto ppp=&std::vector<int>::cbegin;
   try{fun();}catch(...){};
   return 0;
}
