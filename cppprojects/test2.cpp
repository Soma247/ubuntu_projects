#include <thread>
#include <atomic>
#include <iostream>
#include "../../../../nean/relacy-master/relacy/relacy/relacy.hpp"

std::atomic<int> x{10};
int i{10};
void foo(){
   i=20;
   x.store(20,std::memory_order_release);
}
void bar(){
   int xx=x.load(std::memory_order_acquire);
   if(xx==20)
   assert(i==20);
}

int main(){

   using namespace std::chrono_literals;
   std::thread t1{foo},t2{bar},t3{bar};
   t1.join();
   t2.join();
   t3.join();
   std::cout<<"x="<<std::endl;



}
