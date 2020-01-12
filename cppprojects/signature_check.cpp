#include <type_traits>
#include <iostream>
#include <memory>
#include <vector>
#include <functional>
#include <cassert>
#include <exception>
#include <array>
#include <stdexcept>
template<class U>
struct TFS;

template<typename T>
decltype(auto) TF(T){
  return TFS<T>{};
};
int main(int argc, char* argv[]){
   TF(&std::vector<int>::cbegin);
   return 0;
}
