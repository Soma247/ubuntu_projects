#include <gtest/gtest.h>
#include <gtest/gtest-typed-test.h>
#include <future>
#include "../hierarchy_mutex.h"

class ts_hierarchy_mutex_suite:public ::testing::Test{
protected:
      void SetUp(){
      
   }
   void TearDown(){}
};
   hierarchy_mutex<std::size_t> mut_high{1000};
   hierarchy_mutex<std::size_t> mut_mid{500};
   hierarchy_mutex<std::size_t> mut_low{100};

   int low_fun(){
      std::lock_guard lg{mut_low};
      return 1;
   }
   void high_fun(){
      std::lock_guard lg{mut_high};
      return;
   }
   void other_fun(){
      high_fun();
   }
   void thread1(){
      high_fun();
   }
   void thread2(){
      std::lock_guard lg{mut_mid};
      other_fun();
   }

template<typename T>
   using exception_type=
         typename hierarchy_mutex<T>::hierarchy_violation;
TEST_F(ts_hierarchy_mutex_suite, low_from_high){
   auto fut1{std::async(std::launch::async,thread1)};
   auto fut2{std::async(std::launch::async,thread2)};
   ASSERT_NO_THROW(fut1.get());
   ASSERT_THROW(fut2.get(), exception_type<std::size_t>);
}
