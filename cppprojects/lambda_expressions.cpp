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

class Widg{
   int i_;
public:
   Widg():i_{}{}
   explicit Widg(int i):i_{i}{}
   auto getlambda(){
      return[ci=this->i_](int i)->bool{return i==ci;};
   }//capture copy of datamem i_, if def capt by [=],"this" is captured, this->i_ used
};

auto getfn(int i){
   auto w=std::make_unique<Widg>(i);//temporary widget
   return w->getlambda();
}

struct cpymv{
   int i_;
   explicit cpymv(int i=0)noexcept:i_{i}{}
    cpymv(const cpymv& rhs)noexcept :i_{rhs.i_}{
      std::cout<<"cpy_ctor"<<std::endl;
   }
    cpymv(cpymv&& rhs)noexcept:i_{
      std::move(rhs.i_)}{std::cout<<"mv_ctor"<<std::endl;}
   cpymv& operator=(const cpymv& rhs){
      std::cout<<"cpy_op"<<std::endl;
      i_=rhs.i_;
      return *this;
   }
   cpymv& operator=(cpymv&& rhs){
      std::cout<<"mv_op"<<std::endl;
      i_=std::move(rhs.i_);
      return *this;
   }

};   

int main(int argc, char* argv[]){
   //TF(&std::vector<int>::cbegin);
   //decltype(&std::vector<int>::cbegin) i;
   auto lam=getfn(10);
   auto lam2=getfn(5);
   std::cout<<lam2(10)<<lam2(5)<<std::endl;
   std::cout<<lam(10)<<lam(5)<<std::endl;
   std::string str("mystring");
   auto fun=std::bind(//c++11 style move var to funobj
         [](const std::string&str,int i){
            std::cout<<str<<' '<<i<<std::endl;
         },
         std::move(str),std::placeholders::_1);
   std::cout<<"str afrer fun:"<<str<<std::endl;
   fun(24);
   cpymv igz{38};
   cpymv igz2{43};
   cpymv igz3{56};
   auto lam3=[&captured_reference=igz,//copy ref to igz
               captured_copy=igz2,//copy igz2 to local
               moved_copy=std::move(igz3)](//move igz3 to local
                     auto&& fn, auto&&...args){
      //fwd<&> is &, fwd<&&/noref>is &&
      std::cout<<"capt_ref="<<captured_reference.i_
         <<" captured_copy:"<<captured_copy.i_
         <<" moved_copy:"<<moved_copy.i_<<std::endl;
      captured_reference=cpymv{3};
      return std::forward<decltype(fn)>(fn)(
               std::forward<decltype(args)>(args)...
             );
   };

   std::cout<<lam3(lam2,5)<<std::endl;
   std::cout<<"changed_by_ref:"<<igz.i_<<std::endl;
   return 0;
}
