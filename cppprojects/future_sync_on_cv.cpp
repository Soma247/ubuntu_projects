#include <type_traits>
#include <iostream>
#include <memory>
#include <vector>
#include <functional>
#include <cassert>
#include <exception>
#include <stdexcept>
#include <future>
#include <thread>
#include <chrono>
template<class U>
struct TFS;

template<typename T>
decltype(auto) TF(T){
  return TFS<T>{};
};

thread_local int tlsi{2};//first definition is def for all threads

std::string fn(int i=0){
   using namespace std::chrono;
   std::this_thread::sleep_for(seconds(i));
   time_t tm=system_clock::to_time_t(system_clock::now());
   std::string str=std::ctime(&tm);
   str.resize(str.size()-1);
   return str+=" tlsi="+std::to_string(tlsi);
}


   std::condition_variable cv;
   std::mutex m;
   bool cvflag{false};

void setflag(){
   std::cout<<"setflag_start: "+fn(0)<<std::endl;
   {
      std::lock_guard<std::mutex> lg(m);
      cvflag=true;
   }
   cv.notify_one();
}

void getflag(){
   using namespace std::chrono;
   std::this_thread::sleep_for(seconds(3));
   std::cout<<"getflag_start:"+fn(0)<<std::endl;
   while(true){
      std::unique_lock<std::mutex> ul(m);
      cv.wait(ul,[]{
            std::cout<<(cvflag?"true_try":"false_try")<<std::endl;
            return cvflag;});
      //never use without lambda
      //(random unlock & forewer 
      //wait if notified before wait is called), with lambda while(!lambda())wait();
      std::cout<<"getflag_cycle"<<fn(0)<<std::endl;
      if(cvflag)break;
   }
}

int main(int argc, char* argv[]){
   tlsi=4;
   //TF(&std::vector<int>::cbegin);
   //decltype(&std::vector<int>::cbegin) i;
   auto fut01=std::async(std::launch::async,getflag);
   auto fut02=std::async(std::launch::async,setflag);

   std::future<std::string> fut=std::async(std::launch::async,fn,1);
   std::cout<<fn(3)<<'\n'<<fut.get()<<std::endl;
   int i{};
   auto fut1=std::async(std::launch::async,[&ri=i](){std::cout<<++ri<<std::endl;});
   auto fut2=std::async(std::launch::async,[&ri=i](){std::cout<<++ri<<std::endl;});
   fut1.wait();
   fut2.wait();
   std::cout<<i<<std::endl;
   std::atomic<bool>flag{false};
   std::thread tr1([&flag=flag](){std::cout<<fn(1)<<std::endl;flag=true;});
  // tr1.join();
   tr1.detach();//at end all threads must be a unjoinable
   while(!flag)
      std::this_thread::yield();
   //wait end of tr1, if main tr ret, when tr1 is working, terminate is called
   return 0;
}
