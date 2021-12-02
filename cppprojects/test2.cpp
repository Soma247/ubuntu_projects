#include <thread>
#include <atomic>
#include <iostream>
#include <cassert>
#include <vector>
std::atomic<bool> x{};
int i{};
void foo(){
   i=20;
   x.store(20,std::memory_order_release);
}
void bar(){
   while(!x.load(std::memory_order_acquire));
   assert(i==20);
}

int main(){

   using namespace std::chrono_literals;
   std::vector<std::thread> vt;
   for(int i=0;i<100;++i)
      vt.emplace_back(bar);

   std::thread t{foo};
   for(auto&rt:vt)
      rt.join();
   t.join();
   std::cout<<"x="<<std::endl;

}
