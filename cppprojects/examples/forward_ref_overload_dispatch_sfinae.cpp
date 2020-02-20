#include <type_traits>
#include <iostream>
#include <memory>
#include <vector>
#include <functional>
#include <cassert>
#include <exception>
#include <array>
#include <stdexcept>
template<class U>
struct TFS;

template<typename T>
decltype(auto) TF(T){
  return TFS<T>{};
};

template<typename T>
   void perf_for_impl(T&& var, std::false_type){
   static_assert(
         std::is_constructible_v<std::string,T>,
         "type isn't valid for std::string ctor"
   );//message printed befor or after many error messages by failed instantiations
   std::cout<<"T&&:"<<std::string(std::forward<T>(var))<<std::endl;
}
void perf_for_impl(int i, std::true_type){
   std::cout<<"int:"<<i<<std::endl;
}

template<typename T>
   void perf_forward(T&& var){
   perf_for_impl(std::forward<T>(var),std::is_integral<std::decay_t<T>>{});
}

class Person{
   std::string name_;
public:
   template<typename T, typename...Args, 
      typename= std::enable_if_t<
               !(std::is_base_of_v<Person,std::decay_t<T>>&&
                 !sizeof...(Args)),double
      >
   >
explicit Person(T&& var,Args&&...args):
   name_(std::forward<T>(var),std::forward<Args>(args)...){
   static_assert(std::is_constructible_v<decltype(name_),T,Args...>);
   }
   Person(const Person& rhs)=default;
   Person(Person&& rhs)=default;
   Person& operator=(const Person& rhs)=default;
   Person& operator=(Person&& rhs)=default;
   virtual ~Person()=default;
   std::string& getname()noexcept{return name_;}
};

class SPerson:public Person{
public:
   explicit SPerson(const std::string& str):Person(str){}
   explicit SPerson(const SPerson& rhs):Person(rhs){}
   explicit SPerson(SPerson&& rhs):Person(std::move(rhs)){}
   ~SPerson()override{}
};

int main(int argc, char* argv[]){
   //TF(&std::vector<int>::cbegin);
   short s{5};
   std::string str{"abir"};
   //perf_forward(10.0);
   perf_forward(10);
   perf_forward(s);
   perf_forward(std::move(str));
   Person p(std::string("123is_name"),3);
   std::cout<<"name1:"<<p.getname()<<std::endl;
   Person pp{std::move(p)};
   std::cout<<"name2:"<<pp.getname()<<std::endl;
   std::cout<<"name1_after_move:"<<p.getname()<<std::endl;
   SPerson sp("is_spname");
   SPerson sp2{std::move(sp)};
   std::cout<<"spname2:"<<sp2.getname()
      <<" spname1_after_move:"<<sp.getname()<<std::endl;
   return 0;
}
