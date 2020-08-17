#include <algorithm>
#include <chrono>
#include <future>
#include <atomic>
#include <vector>
#include <gtest/gtest.h>
#include <gtest/gtest-typed-test.h>
#include "../jointhread.h"
#include "../threadsafe_stack.h"
auto ready=ts_adv::ts_stack<long>::pop_status::ready;
auto empty=ts_adv::ts_stack<long>::pop_status::empty;
auto interrupted=ts_adv::ts_stack<long>::pop_status::interrupted;
auto timeout=ts_adv::ts_stack<long>::pop_status::timeout;

class ts_stk_test_suite:public ::testing::Test{
protected:
   ts_adv::ts_stack<long> m_stk;
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
TEST_F(ts_stk_test_suite, empty_pop_pop_test){
   std::size_t count = 1000;
   while(--count){
      std::promise<void> pop1_rdy, pop2_rdy, start_operation;
      std::shared_future<void> start {start_operation.get_future()};
      m_stk.clear();
      try{

         decltype(m_stk)::pop_status status1{timeout}, status2{timeout};
         auto popper_fun = [&](std::promise<void>& rdy,decltype(m_stk)::pop_status& status){
            rdy.set_value();
            start.wait();
            long l{};
            status = m_stk.pop(l);
         };
      
         thread_adv::jointhread 
               popper1(popper_fun, std::ref(pop1_rdy), std::ref(status1)),
               popper2(popper_fun, std::ref(pop2_rdy), std::ref(status2));
         pop1_rdy.get_future().wait();
         pop2_rdy.get_future().wait();
         start_operation.set_value();
         popper1.join();
         popper2.join();
         ASSERT_TRUE(m_stk.empty());
         ASSERT_EQ(status1, empty);
         ASSERT_EQ(status2,empty);
      }
      catch(...){
         start_operation.set_value();
      }
   }//while
}
TEST_F(ts_stk_test_suite, empty_push_pop_test){
   std::size_t count = 1000;
   while(--count){
      std::promise<void> pusher_rdy, popper_rdy, start_operation;
      std::shared_future<void> start {start_operation.get_future()};
      m_stk.clear();
      try{
         thread_adv::jointhread pusher(
            [&](){
               pusher_rdy.set_value();
               start.wait();
               m_stk.push(3l);
            }
         );
         decltype(m_stk)::pop_status status;
         thread_adv::jointhread popper(
            [&](){
               popper_rdy.set_value();
               start.wait();
               long l{};
               status = m_stk.pop(l);
            }
         );
         pusher_rdy.get_future().wait();
         popper_rdy.get_future().wait();
         start_operation.set_value();
         pusher.join();
         popper.join();
         ASSERT_TRUE(status==ready || !m_stk.empty());
      }
      catch(...){
         start_operation.set_value();
      }
   }//while
}

TEST_F(ts_stk_test_suite, non_waiting_stress_test){
   std::size_t data_count{200'000};
   std::size_t range_size{500};
   std::atomic<size_t> pushed{0};
   std::atomic<size_t> popped{0};
   std::atomic<bool> stop{false};
   m_stk.clear();
   std::vector<long> source{};
   source.reserve(data_count);
   for(size_t i=0; i<data_count; ++i)
      source.push_back(i);
   auto pusher = [&](){
      for(auto& data:source){
         m_stk.push(data);
         pushed.fetch_add(1,std::memory_order_relaxed);
      }
   };
   auto r_pusher = [&](){
      auto it(std::begin(source));
      auto it2(std::next(std::begin(source),range_size));
      while(it!=std::end(source)){
         m_stk.push_range(it,it2);
         pushed.fetch_add(range_size,std::memory_order_relaxed);
         std::advance(it,range_size);
         it2 = std::next(it,range_size);
      }
   };
   auto popper = [&](){
      long tmp{};
      while(true){
         if(m_stk.pop(tmp)==decltype(m_stk)::pop_status::ready)
            popped.fetch_add(1, std::memory_order_relaxed);
         else if(stop.load())return;
         else std::this_thread::yield();
      }
   };
   auto r_popper = [&](){
      std::vector<long> tmp(range_size);
      while(true){
         auto it =  m_stk.pop_n(std::begin(tmp),range_size);
         if(it!=std::begin(tmp))
            popped.fetch_add(
               std::distance(std::begin(tmp),it),
               std::memory_order_relaxed);
         else if(stop.load())return;
         else std::this_thread::yield();
      }
   };
   std::vector<std::thread> push_threads{};
   std::vector<std::thread> pop_threads{};
   push_threads.emplace_back(pusher);
   pop_threads.emplace_back(popper);
   push_threads.emplace_back(pusher);
   pop_threads.emplace_back(popper);
   push_threads.emplace_back(r_pusher);
   pop_threads.emplace_back(r_popper);
   push_threads.emplace_back(r_pusher);
   pop_threads.emplace_back(r_popper);
   for(auto&pt:push_threads)
      pt.join();
   stop.store(true,std::memory_order_release);
   for(auto&popt:pop_threads)
      popt.join();
   ASSERT_EQ(pushed.fetch_sub(0,std::memory_order_relaxed),
         popped.fetch_sub(0,std::memory_order_relaxed));
}
TEST_F(ts_stk_test_suite, waiting_stress_test){
   std::size_t data_count{200'000};
   std::size_t range_size{500};
   std::atomic<size_t> pushed{0};
   std::atomic<size_t> popped{0};
   std::atomic<bool> stop{false};
   m_stk.clear();
   std::vector<long> source{};
   source.reserve(data_count);
   for(size_t i=0; i<data_count; ++i)
      source.push_back(i);
   auto pusher_one = [&](){
      for(auto& data:source){
         m_stk.push_and_notify_one(data);
         pushed.fetch_add(1,std::memory_order_relaxed);
      }
   };
   auto pusher_all = [&](){
      for(auto& data:source){
         m_stk.push_and_notify_all(data);
         pushed.fetch_add(1,std::memory_order_relaxed);
      }
   };
   auto r_pusher_one = [&](){
      auto it(std::begin(source));
      auto it2(std::next(std::begin(source),range_size));
      while(it!=std::end(source)){
         m_stk.push_range_and_notify_one(it,it2);
         pushed.fetch_add(range_size,std::memory_order_relaxed);
         std::advance(it,range_size);
         it2 = std::next(it,range_size);
      }
   };
   auto r_pusher_all = [&](){
      auto it(std::begin(source));
      auto it2(std::next(std::begin(source),range_size));
      while(it!=std::end(source)){
         m_stk.push_range_and_notify_all(it,it2);
         pushed.fetch_add(range_size,std::memory_order_relaxed);
         std::advance(it,range_size);
         it2 = std::next(it,range_size);
      }
   };

   auto popper = [&](){
      long tmp{};
      while(true){
         if(m_stk.wait_and_pop(tmp)==decltype(m_stk)::pop_status::ready)
            popped.fetch_add(1, std::memory_order_relaxed);
         else if(stop.load())return;
         else std::this_thread::yield();
      }
   };
   auto popper_for = [&](){
      using namespace std::chrono_literals;
      long tmp{};
      while(true){
         if(m_stk.wait_for_and_pop(tmp,100ns)==decltype(m_stk)::pop_status::ready)
            popped.fetch_add(1, std::memory_order_relaxed);
         else if(stop.load())return;
         else std::this_thread::yield();
      }
   };
   auto popper_until = [&](){
      using namespace std::chrono_literals;
      long tmp{};
      while(true){
         if(m_stk.wait_until_and_pop(tmp,
                  std::chrono::steady_clock::now()+100ns) ==
                                 decltype(m_stk)::pop_status::ready)
            popped.fetch_add(1, std::memory_order_relaxed);
         else if(stop.load())return;
         else std::this_thread::yield();
      }
   };

   auto r_popper = [&](){
      std::vector<long> tmp(range_size);
      while(true){
         auto it =  m_stk.wait_and_pop_n(std::begin(tmp),range_size);
         if(it!=std::begin(tmp))
            popped.fetch_add(
               std::distance(std::begin(tmp),it),
               std::memory_order_relaxed);
         else if(stop.load())return;
         else std::this_thread::yield();
      }
   };
   auto r_popper_for = [&](){
      using namespace std::chrono_literals;
      std::vector<long> tmp(range_size);
      while(true){
         auto it =  m_stk.wait_for_and_pop_n(std::begin(tmp),range_size,10ns).first;
         if(it!=std::begin(tmp))
            popped.fetch_add(
               std::distance(std::begin(tmp),it),
               std::memory_order_relaxed);
         else if(stop.load())return;
         else std::this_thread::yield();
      }
   };
   auto r_popper_until = [&](){
      using namespace std::chrono_literals;
      std::vector<long> tmp(range_size);
      while(true){
         auto it =  m_stk.wait_until_and_pop_n(
               std::begin(tmp),range_size,
               std::chrono::steady_clock::now()+10ns).first;
         if(it!=std::begin(tmp))
            popped.fetch_add(
               std::distance(std::begin(tmp),it),
               std::memory_order_relaxed);
         else if(stop.load())return;
         else std::this_thread::yield();
      }
   };


   std::vector<std::thread> push_threads{};
   std::vector<std::thread> pop_threads{};
   push_threads.emplace_back(pusher_one);
   pop_threads.emplace_back(popper_for);
   push_threads.emplace_back(pusher_all);
   pop_threads.emplace_back(popper);
   pop_threads.emplace_back(popper_until);
   push_threads.emplace_back(r_pusher_all);
   pop_threads.emplace_back(r_popper_for);
   push_threads.emplace_back(r_pusher_one);
   pop_threads.emplace_back(r_popper_until);
   pop_threads.emplace_back(popper);
   pop_threads.emplace_back(r_popper);
   for(auto&pt:push_threads)
      pt.join();
   stop.store(true,std::memory_order_release);
   while(pushed!=popped)
      m_stk.notify_all();
   m_stk.stop_waitings();
   for(auto&popt:pop_threads)
      popt.join();
   ASSERT_EQ(pushed.fetch_sub(0,std::memory_order_relaxed),
         popped.fetch_sub(0,std::memory_order_relaxed));
}
