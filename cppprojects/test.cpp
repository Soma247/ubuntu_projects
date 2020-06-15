#include <type_traits>
#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <functional>
#include <cassert>
#include <exception>
#include <stdexcept>
#include <future>
#include <chrono>
#include <limits>
#include "include/threadsafe_stack.h"
#include "include/jointhread.h"
template<class U>
struct thisTypeIs_;

template<typename T>
decltype(auto) thisTypeIs(T){
  return thisTypeIs_<T>{};
};

template<typename T>
void print(T&& t){
   using namespace std::chrono_literals;
   std::this_thread::sleep_for(300ms);
   std::cout<<std::forward<T>(t)<<std::endl;
}

int main(int argc, char* argv[]){
   using namespace std;
   using namespace std::literals;
   using namespace std::chrono_literals;
//   thisTypeIs(der);
   thread_adv::jointhread tr(&print<std::string>,"abir"s);
   thread_adv::jointhread t2{std::move(tr)};
   return 0;
}
