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
#include "include/jointhread.h"
#include <iterator>
#include <algorithm>
#include <random>
template<class U>
struct thisTypeIs_;

template<typename T>
decltype(auto) thisTypeIs(T){
  return thisTypeIs_<T>{};
};
std::mutex s_mutcout;
template<typename T>
void print(T&& t){
   using namespace std::chrono_literals;
   std::lock_guard lg{s_mutcout};
   std::cout<<std::forward<T>(t)<<std::endl;
}



int main(int argc, char* argv[]){
   using namespace std;
   using namespace std::literals;
   using namespace std::chrono;
   using namespace std::chrono_literals;
//   thisTypeIs(der);
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
         tstack.wait_until_and_pop_n(std::back_inserter(vec),std::chrono::steady_clock::now()+1s,5);
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

      ts_adv::ts_stack<int> ts{};
      auto range={1,2,3,4,5,6,7};
      thread_adv::jointhread tpush ([&cv,&mut,&start,&ts,&range](){
            std::random_device rd{};
            std::default_random_engine der{rd()};
            std::unique_lock ul{mut};
            cv.wait(ul,[&start](){return start;});
            for(auto it=range.begin();it!=range.end();++it){
               std::this_thread::sleep_for(der()%1000 *1ms);
               ts.push_and_notify_one(*it);
            }
      });
      auto popper=[&cv,&mut,&start,&ts](int i){
         std::random_device rd{};
         std::default_random_engine der{rd()};
         std::unique_lock ul{mut};
         cv.wait(ul,[&start](){return start;});
         int val;
         //does not wait any, if can't pop now
         while(ts.wait_and_pop(val)==decltype(ts)::pop_status::ready){
            print(std::to_string(i)+" "s+std::to_string(val));
            std::this_thread::sleep_for(milliseconds(der()%30));
         }
      };
      thread_adv::jointhread tpop1(popper,1);
      thread_adv::jointhread tpop2(popper,2);
      ts.stop_waitings();
      std::lock_guard lg{mut};
      start=true;
      cv.notify_all();
   }

   return 0;

}
