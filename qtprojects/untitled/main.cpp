#include <iostream>
#include <thread>
#include <mutex>

std::once_flag flag2;


void may_throw_function(bool do_throw)
{
  if (do_throw) {
    std::cout << "throw: call_once will retry\n"; // this may appear more than once
    throw std::exception();
  }
  std::cout << "Didn't throw, call_once will not attempt again\n"; // guaranteed once
}

void do_once(bool do_throw)
{
  try {
    std::call_once(flag2, may_throw_function, do_throw);
  }
  catch (...) {
  }
}

int main(){
    std::thread t1(do_once, true);
    t1.join();
    std::thread t2(do_once, true);
    std::thread t3(do_once, false);
    t2.join();
    t3.join();
}
