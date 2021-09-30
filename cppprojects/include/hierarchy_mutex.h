#include <functional>
#include <limits>
#include <mutex>
#include <string>
template<typename size_type=std::size_t>
class hierarchy_mutex{
   mutable std::mutex mut_;
   size_type prev_h_;
   size_type const hierarchy_;
   static thread_local size_type this_thread_h_;
   void check()const{
      using namespace std::string_literals;
      if(hierarchy_>=this_thread_h_)
         throw hierarchy_violation{"mutex hierarchy violation"s};
   }
   void update_hierarchy(){
      prev_h_=this_thread_h_;
      this_thread_h_=hierarchy_;
   }
public:
   class hierarchy_violation: public std::logic_error{
   public:
      hierarchy_violation(const std::string& str):
         std::logic_error(str){}
   };
   explicit hierarchy_mutex(size_type hierarchy):
      prev_h_{},hierarchy_{hierarchy}{}
   hierarchy_mutex(const hierarchy_mutex&)=delete;
   hierarchy_mutex(hierarchy_mutex&&)=delete;
   hierarchy_mutex& operator()(const hierarchy_mutex&)=delete;
   hierarchy_mutex& operator()(hierarchy_mutex&&)=delete;
   void lock(){
      this->check();
      mut_.lock();
      this->update_hierarchy();
   }
   void unlock(){
      if(this_thread_h_!=hierarchy_)
         throw hierarchy_violation{"mutex hierarchy violated on unlock"};
      this_thread_h_=prev_h_;
      mut_.unlock();
   }
   bool try_lock(){
      this->check();
      if(!mut_.try_lock()) return false;
      update_hierarchy();
      return true;
   }
};
template<typename T>
thread_local T hierarchy_mutex<T>::this_thread_h_{std::numeric_limits<std::size_t>::max()};
