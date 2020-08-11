#include <algorithm>
#include <chrono>
#include <gtest/gtest.h>
#include <gtest/gtest-typed-test.h>
#include "../threadsafe_stack.h"
auto ready=ts_adv::ts_stack<long>::pop_status::ready;
auto empty=ts_adv::ts_stack<long>::pop_status::empty;
auto interrupted=ts_adv::ts_stack<long>::pop_status::interrupted;
auto timeout=ts_adv::ts_stack<long>::pop_status::timeout;
auto tri_lock_fail=ts_adv::ts_stack<long>::pop_status::try_lock_fail;

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

TEST_F(ts_stk_test_suite, push_unprotected_pop_unprotected_test){
   ASSERT_TRUE(m_stk.empty());
   m_stk.push_unprotected(1);
   m_stk.push_unprotected(2);
   ASSERT_FALSE(m_stk.empty());
   long tmp{};
   m_stk.pop_unprotected(tmp);
   ASSERT_EQ(2,tmp);
   ASSERT_FALSE(m_stk.empty());
   m_stk.pop_unprotected(tmp);
   ASSERT_EQ(1,tmp);
   ASSERT_TRUE(m_stk.empty());
}

TEST_F(ts_stk_test_suite, push_unprotected_try_pop_unprotected){
   long tmp{};
   ASSERT_EQ(empty,m_stk.try_pop_unprotected(tmp));
   m_stk.push_unprotected(1);
   m_stk.push_unprotected(2);
   ASSERT_EQ(m_stk.try_pop_unprotected(tmp),ready);
   ASSERT_EQ(2,tmp);
   ASSERT_EQ(m_stk.try_pop_unprotected(tmp),ready);
   ASSERT_EQ(1,tmp);
   ASSERT_EQ(empty,m_stk.try_pop_unprotected(tmp));
}

TEST_F(ts_stk_test_suite, emplace_pop_unprotected_test){
   ASSERT_TRUE(m_stk.empty());
   m_stk.emplace(1);
   m_stk.emplace(2);
   ASSERT_FALSE(m_stk.empty());
   long tmp{};
   m_stk.pop_unprotected(tmp);
   ASSERT_EQ(2,tmp);
   ASSERT_FALSE(m_stk.empty());
   m_stk.pop_unprotected(tmp);
   ASSERT_EQ(1,tmp);
   ASSERT_TRUE(m_stk.empty());
}

TEST_F(ts_stk_test_suite, clear){
   ASSERT_TRUE(m_stk.empty());
   m_stk.push_unprotected(1);
   m_stk.push_unprotected(2);
   m_stk.clear();
   ASSERT_TRUE(m_stk.empty());
}

#include <array>

const std::size_t lower_dist{3}, higher_dist{9}; 

TEST_F(ts_stk_test_suite, push_range_unprotected_pop_n_unprotected){
   std::array<long,5> source_arr{0,1,2,3,4}, dest_arr{};
   //push 5, pop 3
   m_stk.push_range_unprotected(
         std::begin(source_arr),std::end(source_arr));
   auto dest_end = m_stk.pop_n_unprotected(
                     std::begin(dest_arr),lower_dist);
   ASSERT_EQ(std::distance(std::begin(dest_arr),dest_end),lower_dist);
   //pop all
   m_stk.clear();
   
   //push 5, pop 0
   m_stk.push_range_unprotected(
         std::begin(source_arr),std::end(source_arr));
   dest_end = m_stk.pop_n_unprotected(std::begin(dest_arr),0);
   ASSERT_EQ(dest_end, std::begin(dest_arr));

   // pop 9
   dest_end = m_stk.pop_n_unprotected(
                   std::begin(dest_arr),higher_dist);
   ASSERT_TRUE(std::equal(
                  std::begin(source_arr),std::end(source_arr),
                  std::rbegin(dest_arr),std::rend(dest_arr)));
   ASSERT_EQ(dest_end,std::end(dest_arr));
   ASSERT_TRUE(m_stk.empty());

   //push 0, pop 9
   dest_end = m_stk.pop_n_unprotected(std::begin(dest_arr),higher_dist);
   ASSERT_EQ(dest_end,std::begin(dest_arr));
   ASSERT_TRUE(m_stk.empty()); 
}
#include <atomic>
#include <thread>
using namespace std::chrono_literals;

TEST_F(ts_stk_test_suite, push_and_notify_wait_and_pop){
   std::atomic<bool> start_push{false};
   std::atomic<bool> pop_started{false};
   std::atomic<bool> pop_exited{false};
   auto wait_push=[&start_push](){
      while(!start_push.load())
         std::this_thread::yield();
   };  
   auto wait_pop_started = [&pop_started](){
      while(!pop_started.load())
         std::this_thread::yield();
   };  

   std::thread pusher([&](){
      wait_push();
      start_push.store(false);
      m_stk.push_and_notify_one(1);
      wait_push();
      m_stk.push_and_notify_one(2);
   });

   std::thread poper([&](){
      long tmp{};
      pop_started.store(true);
      auto status = m_stk.wait_and_pop(tmp);
      ASSERT_EQ(status,ready);
      ASSERT_EQ(tmp,1);
      start_push.store(true);
      status = m_stk.wait_and_pop(tmp);
      ASSERT_EQ(status,ready);
      ASSERT_EQ(tmp,2);
      status = m_stk.wait_and_pop(tmp);
      ASSERT_EQ(status,interrupted);
      ASSERT_EQ(tmp,2);
      pop_exited.store(true);
   });
   wait_pop_started();
   start_push.store(true);
   std::this_thread::sleep_for(1ms);
   pusher.join();
   m_stk.stop_waitings();
   while(!pop_exited.load()){
      m_stk.notify_one();
      std::this_thread::yield();
   }
   poper.join();
   ASSERT_TRUE(m_stk.empty());
}
TEST_F(ts_stk_test_suite, push_and_notify_wait_for_and_pop){
   std::atomic<bool> start_push{false};
   std::atomic<bool> pop_started{false};
   const auto dur = 1ns;
   auto wait_push=[&start_push](){
      while(!start_push.load())
         std::this_thread::yield();
   };
   auto wait_pop_started = [&pop_started](){
      while(!pop_started.load())
         std::this_thread::yield();
   };  
   std::thread pusher([&](){
      wait_push();
      start_push.store(false);
      m_stk.push_and_notify_one(1);
      std::this_thread::sleep_for(1ms);
      wait_push();
      m_stk.push_and_notify_one(2);
   });

   std::thread poper([&](){
      long tmp{};
      pop_started.store(true);
      auto status = empty;
      while(timeout == (status=m_stk.wait_for_and_pop(tmp,dur)))
         std::this_thread::yield();
      ASSERT_EQ(status,ready);
      ASSERT_EQ(tmp,1);
      start_push.store(true);
      std::this_thread::sleep_for(1ms);
      while(timeout == (status=m_stk.wait_for_and_pop(tmp,dur)))
         std::this_thread::yield();
      ASSERT_EQ(status,ready);
      ASSERT_EQ(tmp,2);
      status = m_stk.wait_for_and_pop(tmp,dur);
      ASSERT_EQ(status,timeout);
      ASSERT_EQ(tmp,2);
   });
   wait_pop_started();
   start_push.store(true);
   pusher.join();
   poper.join();
   ASSERT_TRUE(m_stk.empty());
}

TEST_F(ts_stk_test_suite, push_and_notify_wait_until_and_pop){
   std::atomic<bool> start_push{false};
   std::atomic<bool> pop_started{false};
   const auto dur = 1ns;
   auto wait_push=[&start_push](){
      while(!start_push.load())
         std::this_thread::yield();
   };
   auto wait_pop_started = [&pop_started](){
      while(!pop_started.load())
         std::this_thread::yield();
   };  
   std::thread pusher([&](){
      wait_push();
      start_push.store(false);
      m_stk.push_and_notify_one(1);
      wait_push();
      m_stk.push_and_notify_one(2);
   });

   std::thread poper([&](){
      using clock = std::chrono::steady_clock;
      long tmp{};
      auto status = empty;
      pop_started.store(true);
      while(timeout == (status =
                  m_stk.wait_until_and_pop(tmp, clock::now()+dur)))
         std::this_thread::yield();
      ASSERT_EQ(status, ready);
      ASSERT_EQ(tmp, 1);
      start_push.store(true);
      while(timeout == (status =
               m_stk.wait_until_and_pop(tmp, clock::now()+dur)))
         std::this_thread::yield();
      ASSERT_EQ(status, ready);
      ASSERT_EQ(tmp, 2);
      status = m_stk.wait_until_and_pop(tmp, clock::now()+dur);
      ASSERT_EQ(status, timeout);
      ASSERT_EQ(tmp, 2);
   });
   wait_pop_started();
   start_push.store(true);
   pusher.join();
   poper.join();
   ASSERT_TRUE(m_stk.empty());
}

TEST_F(ts_stk_test_suite, push_range_and_notify_one_wait_and_pop_n){
   std::array<long,5> source_arr{0,1,2,3,4}, dest_arr{};
   std::atomic<bool> start_push{false};
   std::atomic<bool> pop_started{false};
   std::atomic<bool> pop_exited{false};

   auto wait_push=[&start_push](){
      while(!start_push.load())
         std::this_thread::yield();
   };
   auto wait_pop_started = [&pop_started](){
      while(!pop_started.load())
         std::this_thread::yield();
   };  
   std::thread pusher([&](){;
      auto end {std::next(std::begin(source_arr),lower_dist)};
      wait_push();
      start_push.store(false);
      m_stk.push_range_and_notify_one(std::begin(source_arr),end);
      wait_push();
      m_stk.push_range_and_notify_one(end,std::end(source_arr));
   });

   std::thread poper([&](){
      pop_started.store(true);
      //pushed 3, pop 3
      auto dest_end = m_stk.wait_and_pop_n(
                        std::begin(dest_arr),lower_dist);
      ASSERT_EQ(dest_end,std::next(std::begin(dest_arr),lower_dist));
      start_push.store(true);
      //pushed 2, pop 9
      dest_end = m_stk.wait_and_pop_n(dest_end,higher_dist);
      ASSERT_EQ(dest_end,std::end(dest_arr));
      ASSERT_TRUE(m_stk.empty());
      std::sort(std::begin(dest_arr),std::end(dest_arr));
      ASSERT_TRUE(std::equal(
                  std::begin(source_arr),std::end(source_arr),
                  std::begin(dest_arr),std::end(dest_arr)));
      //push 0, wait to interrupt
      dest_end = m_stk.wait_and_pop_n(std::begin(dest_arr),higher_dist);
      ASSERT_EQ(dest_end,std::begin(dest_arr));
      ASSERT_TRUE(m_stk.empty());
      pop_exited.store(true);
   });
   wait_pop_started();
   start_push.store(true);
   std::this_thread::sleep_for(1ms);
   pusher.join();
   m_stk.stop_waitings();
   while(!pop_exited.load()){
      m_stk.notify_one();
      std::this_thread::yield();
   }
   poper.join();
}

TEST_F(ts_stk_test_suite, push_range_and_notify_one_wait_for_and_pop_n){
   std::array<long,5> source_arr{0,1,2,3,4}, dest_arr{};
   std::atomic<bool> start_push{false};
   std::atomic<bool> pop_started{false};
   const auto dur = 1ns;
   auto wait_push=[&start_push](){
      while(!start_push.load())
         std::this_thread::yield();
   };
   auto wait_pop_started = [&pop_started](){
      while(!pop_started.load())
         std::this_thread::yield();
   };  
   std::thread pusher([&](){;
      auto end {std::next(std::begin(source_arr),lower_dist)};
      wait_push();
      start_push.store(false);
      m_stk.push_range_and_notify_one(std::begin(source_arr),end);
      wait_push();
      m_stk.push_range_and_notify_one(end,std::end(source_arr));
   });

   std::thread poper([&](){
      using iterator = typename std::array<long,5>::iterator;
      using pop_status = typename ts_adv::ts_stack<long>::pop_status;
      pop_started.store(true);
      //pushed 3, pop 3
      iterator dest_end{};
      pop_status status{empty};
      do{
         std::tie(dest_end,status) = m_stk.wait_for_and_pop_n(
                              std::begin(dest_arr),lower_dist,dur);
         ASSERT_TRUE(status==timeout || status == ready);
         std::this_thread::yield();
      }while(status!=ready); 
      ASSERT_EQ(status,ready);
      ASSERT_EQ(dest_end,std::next(std::begin(dest_arr),lower_dist));
      start_push.store(true);
      //pushed 2, pop 9
      status=empty;
      do{
         std::tie(dest_end,status) = m_stk.wait_for_and_pop_n(
                        dest_end,higher_dist,dur);
         std::this_thread::yield();
      }while(status!=ready); 
      ASSERT_EQ(status,ready);
      ASSERT_EQ(dest_end,std::end(dest_arr));
      ASSERT_TRUE(m_stk.empty());
      std::sort(std::begin(dest_arr), std::end(dest_arr));
      ASSERT_TRUE(std::equal(
                  std::begin(source_arr),std::end(source_arr),
                  std::begin(dest_arr),std::end(dest_arr)));
      //push 0, wait to timeout
      status = empty;
      std::tie(dest_end,status) = m_stk.wait_for_and_pop_n(
                           std::begin(dest_arr), higher_dist,dur);
      ASSERT_EQ(dest_end,std::begin(dest_arr));
      ASSERT_TRUE(m_stk.empty());
      ASSERT_EQ(status,timeout);
   });
   wait_pop_started();
   start_push.store(true);
   pusher.join();
   poper.join();
}
TEST_F(ts_stk_test_suite, push_range_and_notify_one_wait_until_and_pop_n){
   std::array<long,5> source_arr{0,1,2,3,4}, dest_arr{};
   std::atomic<bool> start_push{false};
   std::atomic<bool> pop_started{false};
   const auto dur = 1ns;
   using clock = std::chrono::steady_clock;
   auto wait_push=[&start_push](){
      while(!start_push.load())
         std::this_thread::yield();
   };
   auto wait_pop_started = [&pop_started](){
      while(!pop_started.load())
         std::this_thread::yield();
   };  
   std::thread pusher([&](){;
      auto end {std::next(std::begin(source_arr),lower_dist)};
      wait_push();
      start_push.store(false);
      m_stk.push_range_and_notify_one(std::begin(source_arr),end);
      wait_push();
      m_stk.push_range_and_notify_one(end,std::end(source_arr));
   });

   std::thread poper([&](){
      using iterator = typename std::array<long,5>::iterator;
      using pop_status = typename ts_adv::ts_stack<long>::pop_status;
      pop_started.store(true);
      //pushed 3, pop 3
      iterator dest_end{};
      pop_status status{empty};
      do{
         std::tie(dest_end,status) = m_stk.wait_until_and_pop_n(
                     std::begin(dest_arr),lower_dist,clock::now()+dur);
         std::this_thread::yield();
         ASSERT_TRUE(status==timeout || status == ready);
      }while(status!=ready); 
      ASSERT_EQ(status,ready);
      ASSERT_EQ(dest_end,std::next(std::begin(dest_arr),lower_dist));
      start_push.store(true);
      //pushed 2, pop 9
      status=interrupted;
      do{
         std::tie(dest_end,status) = m_stk.wait_until_and_pop_n(
                        dest_end,higher_dist,clock::now()+dur);
         std::this_thread::yield();
      }while(status!=ready); 
      ASSERT_EQ(status,ready);
      ASSERT_EQ(dest_end,std::end(dest_arr));
      ASSERT_TRUE(m_stk.empty());
      std::sort(std::begin(dest_arr), std::end(dest_arr));
      ASSERT_TRUE(std::equal(
                  std::begin(source_arr),std::end(source_arr),
                  std::begin(dest_arr),std::end(dest_arr)));
      //push 0, wait to timeout
      status = empty;
      std::tie(dest_end,status) = m_stk.wait_until_and_pop_n(
                  std::begin(dest_arr), higher_dist,clock::now()+dur);
      ASSERT_EQ(dest_end,std::begin(dest_arr));
      ASSERT_TRUE(m_stk.empty());
      ASSERT_EQ(status,timeout);
   });
   wait_pop_started();
   start_push.store(true);
   pusher.join();
   poper.join();
}
