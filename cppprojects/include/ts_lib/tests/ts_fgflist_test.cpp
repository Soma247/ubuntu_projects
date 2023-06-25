#include <algorithm>
#include <array>
#include <chrono>
#include <gtest/gtest.h>
#include <gtest/gtest-typed-test.h>
#include "../ts_fine_graited_flist.h"
class ts_fg_flist_test_suite:public ::testing::Test{
protected:
   ts_adv::tfg_flist<long> m_list;
   void SetUp(){
      
   }
   void TearDown(){}
};
/*void print(const std::string& str){
   static std::mutex mut;
   std::lock_guard lg{mut};
   std::cout<<str<<std::endl;
}*/

TEST_F(ts_fg_flist_test_suite, push_front_clear_empty){
ASSERT_TRUE(m_list.empty());
m_list.push_front(1);
ASSERT_FALSE(m_list.empty());
m_list.push_front(2);
ASSERT_FALSE(m_list.empty());
m_list.clear();
ASSERT_TRUE(m_list.empty());
}

TEST_F(ts_fg_flist_test_suite, remove_first_if_for_each){
   m_list.push_front(1);
   m_list.push_front(2);
   m_list.push_front(2);
   m_list.push_front(3);
   std::array<long,3> along{};
   int i{};
   m_list.remove_first_if([](auto p_val){return *p_val==2;});
   m_list.for_each([&](auto p_val){along[i]=*p_val;++i;});
   ASSERT_EQ(along[0],3);
   ASSERT_EQ(along[1],2);
   ASSERT_EQ(along[2],1);
   m_list.clear();

}
TEST_F(ts_fg_flist_test_suite, remove_if){
   m_list.push_front(1);
   m_list.push_front(2);
   m_list.push_front(2);
   m_list.push_front(3);
   std::array<long,2> along{};
   int i{};
   m_list.remove_if([](auto p_val){return *p_val==2;});
   m_list.for_each([&](auto p_val){along[i]=*p_val;++i;});
   ASSERT_EQ(along[0],3);
   ASSERT_EQ(along[1],1);
   m_list.clear();
}
#include<vector>
TEST_F(ts_fg_flist_test_suite, for_first_of){
   std::vector<long> v{};
   m_list.push_front(1);
   m_list.push_front(2);
   m_list.push_front(2);
   m_list.push_front(3);
   m_list.for_first_of(
         [&](auto p_val){v.push_back(*p_val);},
         [](auto p_val){
         if(*p_val==2)return true;
         return false;
   });
   ASSERT_EQ(v.size(),1);
   ASSERT_EQ(v.front(),2);
}
