#include <algorithm>
#include <chrono>
#include <atomic>
#include <vector>
#include <gtest/gtest.h>
#include <gtest/gtest-typed-test.h>
#include "../jointhread.h"
#include "../ts_fine_graited_flist.h"
using namespace std::literals;
std::atomic<size_t> destructed{0};
struct item{
   long m_l{};
   bool m_processed{false};
   item()=default;
   item(long l):m_l{l},m_processed{false}{}
   item(const item& other)=default;
   ~item(){if(m_processed)destructed.fetch_add(1,std::memory_order_relaxed);}
};

class ts_fgflist_stress_test_suite:public ::testing::Test{
protected:
   ts_adv::tfg_flist<item> m_list;
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
TEST_F(ts_fgflist_stress_test_suite, stress_test){
   std::size_t data_count{200'000};
   std::atomic<size_t> pushed{0};
   std::atomic<bool> stop{false};
   auto pusher = [&](){
      for(size_t i=0; i<data_count; ++i){
         m_list.push_front(item{long(i)});
         pushed.fetch_add(1,std::memory_order_relaxed);
      }
   };
  auto popper = [&](){
      do{
         m_list.for_each([](auto sptr){sptr->m_processed=true;});
         m_list.remove_if([](auto sptr){return sptr->m_processed;});
      }while(!(stop.load()&&m_list.empty()));
   };
  auto even_remover = [&](){
      do{
         m_list.for_each([](auto sptr){sptr->m_processed=true;});
         m_list.remove_if([](auto sptr){
               return !(sptr->m_l%2) && sptr->m_processed;});
      }while(!(stop.load()&&m_list.empty()));
   };

  std::vector<std::thread> push_threads{};
   std::vector<std::thread> pop_threads{};
   push_threads.emplace_back(pusher);
   pop_threads.emplace_back(popper);
   push_threads.emplace_back(pusher);
   pop_threads.emplace_back(even_remover);
   push_threads.emplace_back(pusher);
   pop_threads.emplace_back(popper);
   push_threads.emplace_back(pusher);
   pop_threads.emplace_back(even_remover);
   size_t pushers_count{push_threads.size()};
   for(auto&pt:push_threads)
      pt.join();
   stop.store(true,std::memory_order_release);
   for(auto&popt:pop_threads)
      popt.join();
   ASSERT_TRUE(m_list.empty());
   ASSERT_EQ(pushed.fetch_sub(0,std::memory_order_relaxed),
         destructed.fetch_sub(0,std::memory_order_relaxed));
   ASSERT_EQ(pushed.load(),pushers_count*data_count);
}
