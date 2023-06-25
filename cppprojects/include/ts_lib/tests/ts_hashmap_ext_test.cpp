#include <algorithm>
#include <array>
#include <chrono>
#include <gtest/gtest.h>
#include <gtest/gtest-typed-test.h>
#include <iterator>
#include "../threadsafe_extended_hashmap.h"
using hmap_type = ts_adv::ts_ex_hashmap<std::string,long>;
using uptr_hmap_type = std::unique_ptr<hmap_type>; 

class ts_ex_hashmap_test_suite:public ::testing::Test{
protected:
   void SetUp(){
   }
   void TearDown(){}
};
void print(const std::string& str){
   static std::mutex mut;
   std::lock_guard lg{mut};
   std::cout<<str<<std::endl;
}
using namespace std::literals;
TEST_F(ts_ex_hashmap_test_suite, insert_copy){
   uptr_hmap_type m_p_hmap{std::make_unique<hmap_type>(100)};
   std::string str{"two"};
   auto p = std::make_pair<std::string, long>("one"s,1);
   auto p2 = std::make_pair<std::string,long>("two"s,2);
   std::vector<std::pair<std::string,long>> results;
   auto end_it = m_p_hmap->copy(std::end(results));
   //empty hmap, copy returns begin iterator, that eq end of results
   ASSERT_EQ(end_it,std::begin(results));
   results.clear();
   //returns back_inserter
   m_p_hmap->copy(std::back_inserter(results));
   ASSERT_TRUE(results.empty());

   //insert two elements(rval & lval) twice
   ASSERT_TRUE(m_p_hmap->insert("one"s,1));
   ASSERT_FALSE(m_p_hmap->insert("one"s,3));
   ASSERT_TRUE(m_p_hmap->insert(str,2));
   ASSERT_FALSE(m_p_hmap->insert(str,3));
   m_p_hmap->copy(std::back_inserter(results));
   ASSERT_EQ(2,results.size());
   std::sort(std::begin(results), std::end(results));
   ASSERT_EQ(results[0], p);
   ASSERT_EQ(results[1], p2);
   results.clear();
   results.resize(2);
   //copy must return beg+2 iterator
   end_it = m_p_hmap->copy(std::begin(results));
   ASSERT_EQ(end_it,std::end(results));
   ASSERT_EQ(2,results.size());
   std::sort(std::begin(results), std::end(results));
   ASSERT_EQ(results[0], p);
   ASSERT_EQ(results[1], p2);

}

TEST_F(ts_ex_hashmap_test_suite, insert_or_assign){
   uptr_hmap_type m_p_hmap{std::make_unique<hmap_type>(100)};
   auto p = std::make_pair<std::string, long>("one"s,10);
   auto p2 = std::make_pair<std::string,long>("two"s,20);
   std::vector<std::pair<std::string,long>> results;
   std::string str{"two"};
   //insert two elements(rval & lval) twice
   m_p_hmap->insert_or_assign("one"s,1);
   m_p_hmap->insert_or_assign(p.first,p.second);
   m_p_hmap->insert_or_assign("two"s,2);
   m_p_hmap->insert_or_assign(p2.first,p2.second);
   m_p_hmap->copy(std::back_inserter(results));
   std::sort(std::begin(results), std::end(results));
   ASSERT_EQ(results.size(),2);
   ASSERT_EQ(results[0], p);
   ASSERT_EQ(results[1], p2);
}

TEST_F(ts_ex_hashmap_test_suite, mapped_for){
   uptr_hmap_type m_p_hmap{std::make_unique<hmap_type>(100)};
   auto p = std::make_pair<std::string, long>("one"s,10);
   auto p2 = std::make_pair<std::string,long>("two"s,20);
   std::vector<std::pair<std::string,long>> results;
   std::string str{"two"};
   ASSERT_FALSE(m_p_hmap->mapped_for("one").has_value());
   //insert two elements(rval & lval) twice
   m_p_hmap->insert_or_assign(p.first,p.second);
   m_p_hmap->insert_or_assign(p2.first,p2.second);
   ASSERT_EQ(m_p_hmap->mapped_for(p.first).value(),p.second);
   ASSERT_EQ(m_p_hmap->mapped_for(p2.first).value(),p2.second);
   ASSERT_FALSE(m_p_hmap->mapped_for("someint").has_value());
}


TEST_F(ts_ex_hashmap_test_suite, remove){
   uptr_hmap_type m_p_hmap{std::make_unique<hmap_type>(100)};
   auto p = std::make_pair<std::string, long>("one"s,10);
   auto p2 = std::make_pair<std::string,long>("two"s,20);
   std::vector<std::pair<std::string,long>> results;
   std::string str{"two"};
   m_p_hmap->insert(p.first,p.second);
   m_p_hmap->insert(p2.first,p2.second);
   m_p_hmap->remove(p2.first);
   ASSERT_FALSE(m_p_hmap->mapped_for(p2.first).has_value());
   m_p_hmap->insert(p2.first,p2.second);
   m_p_hmap->remove(p.first);
   ASSERT_FALSE(m_p_hmap->mapped_for(p.first).has_value());
   ASSERT_TRUE(m_p_hmap->mapped_for(p2.first).has_value());

}

TEST_F(ts_ex_hashmap_test_suite, rehash){
   uptr_hmap_type m_p_hmap{std::make_unique<hmap_type>(10)};
   for(int i=0;i<100;++i)
      m_p_hmap->insert(std::to_string(i),i);
   ASSERT_GE(m_p_hmap->load_factor(),10);
   m_p_hmap->rehash(0.85);
   ASSERT_LE(m_p_hmap->load_factor(),0.85);
}
