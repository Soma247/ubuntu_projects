#include <condition_variable>
#include <thread>
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
#include <mutex>
#include "include/threadsafe_stack.h"
#include "include/threadsafe_queue.h"
#include "include/threadsafe_hashmap.h"
#include "include/threadsafe_extended_hashmap.h"
#include "include/jointhread.h"
#include "include/threadpool.h"
#include <iterator>
#include <algorithm>
#include <random>
#include <atomic>
#include <sstream>
#include <map>
template<class U>
struct thisTypeIs_;

template<typename T>
decltype(auto) thisTypeIs(T){
  return thisTypeIs_<T>{};
};

template<typename T>
void print(T&& t){
   using namespace std::chrono_literals;
   static std::mutex s_mutcout{};
   std::lock_guard lg{s_mutcout};
   std::cout<<std::forward<T>(t)<<std::endl;
}

std::ostream& operator<< (std::ostream& os, std::pair<int,int> p){
   os<<'{'<<p.first<<','<<p.second<<'}'<<std::endl;
   return os;
}

std::atomic<int> count{0};
std::mutex map_mut{};
std::map<std::string,int> tmap{};

void f1(thread_adv::thread_pool& tp){ 
   using namespace std::chrono_literals;
   if(count.fetch_add(1,std::memory_order_relaxed)>40000){
      return;
   }
   std::stringstream sstr{};
   sstr<<std::this_thread::get_id();
   {std::lock_guard lg{map_mut};
      ++tmap[sstr.str()];
   }
   std::packaged_task<void()> pt([&tp](){f1(tp);});
   auto fut=tp.assign_task(std::move(pt));
   f1(tp);
   while(fut.wait_for(0ms)!=std::future_status::ready)
      tp.try_work();
}

int main(int argc, char* argv[]){
   using namespace std;
   using namespace std::literals;
   using namespace std::chrono;
   using namespace std::chrono_literals;
//   thisTypeIs(der);
try{
   struct abir{
      long long f;
      long long s;
      abir(){}
      abir(int a, int b):f{a},s{b}{}
   };
   std::atomic<abir> abr({1,1});
   abir abr2(1,1),abr3(3,3);
   abr.compare_exchange_strong(abr2,abr3);
   std::cout<<abr.load().f<<'a'<<abr.load().s<<std::endl;
   std::cout<<"is lock_free:"<<boolalpha<<abr.is_lock_free()<<std::endl;
   { 
      std::cout<<"start pool"<<std::endl;
      thread_adv::thread_pool tp{};
      std::packaged_task<void()> pt([&tp](){f1(tp);});
      auto fut= tp.assign_task(std::move(pt));
      fut.wait();
   }
   for(auto&p:tmap)
      std::cout<<p.first<<" - "<<p.second<<std::endl;
   {std::cout<<"end pool"<<std::endl;
      std::vector<int> vint{};
      for(auto i{0};i<1000;++i)
         vint.push_back(i);


      ts_adv::ts_queue<int> queue{};
      auto pusher=[&queue,&vint](int i){
         print("insert from:"+std::to_string(i));
         for(auto i:vint)
            queue.push(i);
      };
      auto pusher2=[&queue,&vint](int i){
         print("insert from:"+std::to_string(i));
         for(auto i:vint)
            queue.push(i);
      };
      auto popper=[&queue,&vint](){
         int ii;
         for(std::size_t i=0;i<vint.size();++i)
               if(queue.pop(ii)==ts_adv::ts_queue<int>::pop_status::ready)
                  print(std::to_string(ii));
      };
      thread_adv::jointhread t1(pusher,1),
            t2(pusher2,2), t3(popper);

   }
   {
      ts_adv::ts_hashmap<int,int> hm(100);
      std::vector<int> vint{};
      for(auto i{0};i<1000;++i)
         vint.push_back(i);

      auto pusher=[&hm,&vint](int i){
         print("insert from:"+std::to_string(i));
         for(auto i:vint)
            hm.insert(i,i);
      };
      auto pusher2=[&hm,&vint](int i){
         print("insert from:"+std::to_string(i));
         for(auto i:vint)
            hm.insert(i,i);
      };
      auto popper=[&hm,&vint](){
         for(auto i:vint)
            if(i%21)
               hm.remove(i);
      };
      {
         thread_adv::jointhread t1(pusher,1),
            t2(pusher2,2), t3(popper);
      }
      std::vector<std::pair<int,int>> vint2;
      hm.copy(std::back_inserter(vint2));
      {
         thread_adv::jointhread t1(popper);
      }
      std::cout<<"_________________"<<std::endl;
      for(auto&p:vint2)
         std::cout<<p.first<<' '<<p.second<<std::endl;
      std::cout<<"***********************"<<std::endl;
      for(int i=0;i<1000;++i){
         auto t=hm.mapped_for(i);
         std::cout<<(t?std::to_string(t.value())+"\n"s:""s);
      }
      std::cout<<std::endl;
   }
   {
      std::cout<<"first"<<std::endl;
      ts_adv::ts_stack<int> tstack{};
      auto range={1,2,3};

      thread_adv::jointhread tr1([&tstack](){
         std::vector<int> vec{};
         tstack.wait_and_pop_n(std::back_inserter(vec),5);
         std::for_each(begin(vec),end(vec),&print<int&>);
      });
      thread_adv::jointhread tr2([&tstack](){
         std::vector<int> vec{};
         tstack.wait_until_and_pop_n(std::back_inserter(vec),5,
               std::chrono::steady_clock::now()+1s);
         std::for_each(begin(vec),end(vec),&print<int&>);
      });

      tstack.push_range_and_notify_all(range.begin(),range.end());
      print("notify!");
      tstack.stop_waitings();
   }

   {
      std::cout<<"second"<<std::endl;
      ts_adv::ts_stack<int> ts{};
      auto range={1,2,3,4,5,6,7};
      thread_adv::jointhread tpush ([&ts,&range](){
            std::random_device rd{};
            std::default_random_engine der{rd()};
            for(auto it=range.begin();it!=range.end();++it){
               std::this_thread::sleep_for(der()%300 *1ms);
               ts.push(*it);
            }
      });
      auto popper=[&ts](int i){
         std::random_device rd{};
         std::default_random_engine der{rd()};
         std::this_thread::sleep_for(1s);
         int val;
         while(ts.pop(val)==decltype(ts)::pop_status::ready){
            print(std::to_string(i)+" "s+std::to_string(val));
            std::this_thread::sleep_for(milliseconds(der()%300));
         }
      };
      thread_adv::jointhread tpop1(popper,1);
      thread_adv::jointhread tpop2(popper,2);
   }
   {
      std::cout<<"third"<<std::endl;
      std::condition_variable cv;
      std::mutex mut;
      bool start{false};
      std::atomic<bool> stop{false};
      ts_adv::ts_stack<int> ts{};
      auto range={1,2,3,4,5,6,7};
      thread_adv::jointhread tpush ([&stop,&cv,&mut,&start,&ts,&range](){
            std::random_device rd{};
            std::default_random_engine der{rd()};
            std::unique_lock ul{mut};
            cv.wait(ul,[&start](){return start;});
            ul.unlock();
            for(auto it=range.begin();it!=range.end();++it){
               std::this_thread::sleep_for(milliseconds(der()%1000));
               ts.push_and_notify_one(*it);
               print("push "s+std::to_string(*it));
            }
            stop=true;
      });
      auto popper=[&stop,&cv,&mut,&start,&ts](int i){
         std::random_device rd{};
         std::default_random_engine der{rd()};
         std::unique_lock ul{mut};
         cv.wait(ul,[&start](){return start;});
         ul.unlock();
         int val;
         //does not wait any, if can't pop now
         do{
            if(ts.wait_and_pop(val)==decltype(ts)::pop_status::ready){
               print("pop"s+std::to_string(i)+" "s+std::to_string(val));
               std::this_thread::sleep_for(milliseconds(der()%30));
            }
            if(ts.empty()&&stop.load()){
               ts.stop_waitings();
               return;
            }
         }while(true);
      };
      thread_adv::jointhread tpop1(popper,1);
      thread_adv::jointhread tpop2(popper,2);
      std::lock_guard lg{mut};
      start=true;
      cv.notify_all();
         }
      struct str{
         int i;
         str(int ii):i{ii}{}
         void* operator new(std::size_t sz){
            print("oper new");
            return ::operator new(sz);
         }
         void operator delete(void* ptr, std::size_t sz){
            print("op del"s+std::to_string(sz));
            ::operator delete(ptr);
         }
      };
   auto ptr{std::make_unique<str>(3)};
}catch(std::exception& e){
   print(e.what());
}
   return 0;

}
