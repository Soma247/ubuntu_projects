#include <algorithm>
#include <chrono>
#include <atomic>
#include <vector>
#include <gtest/gtest.h>
#include <gtest/gtest-typed-test.h>
#include "../jointhread.h"
#include "../threadsafe_extended_hashmap.h"
using namespace std::literals;
std::atomic<size_t> destructed{0};
class ts_ex_hashmap_stress_test_suite:public ::testing::Test{
protected:
   ts_adv::ts_ex_hashmap<int,size_t> m_map{1000};
   void SetUp(){
      
   }
   void TearDown(){}
};
/*
void print(const std::string& str){
   static std::mutex mut;
   std::lock_guard lg{mut};
   std::cout<<str<<std::endl;
}*/
TEST_F(ts_ex_hashmap_stress_test_suite, stress_test){
   std::size_t data_count{400'000};
   std::atomic<bool> stop{false};
   auto pusher = [&](){
      for(size_t i=0; i<data_count; ++i){
      }
   };
   auto pusher2 = [&](){
      for(size_t i=0; i<data_count; ++i){
         m_map.insert(i,i);
      }
   };

  auto popper = [&](){
      for(int i=0;!stop.load();i=(i+1)%data_count)
         if(auto result = m_map.mapped_for(i);
                              result.has_value())
         {
            m_map.remove(i);
         }
  };
  auto copyer = [&](){
     std::vector<std::pair<int,long>> tmp{};
     tmp.reserve(data_count);
      while(!stop.load()){
         if(m_map.load_factor()>1.0){
            m_map.rehash();
         }
         m_map.copy(std::back_inserter(tmp));
      }
  };

  std::vector<std::thread> push_threads{};
   std::vector<std::thread> pop_threads{};
   push_threads.emplace_back(pusher);
   pop_threads.emplace_back(popper);
   push_threads.emplace_back(pusher2);
   pop_threads.emplace_back(popper);
   pop_threads.emplace_back(copyer);
   push_threads.emplace_back(pusher2);
   pop_threads.emplace_back(popper);
   for(auto&pt:push_threads)
      pt.join();
   stop.store(true,std::memory_order_release);
   for(auto&popt:pop_threads)
      popt.join();
}
