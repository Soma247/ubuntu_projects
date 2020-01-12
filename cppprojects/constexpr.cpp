#include <type_traits>
#include <iostream>
#include <memory>
#include <vector>
#include <functional>
#include <cassert>
#include <exception>
#include <array>
#include <stdexcept>
    //in constexpr calls get error of compilation
    //(throw in constexpr fun), in runtime calls
    //throw this exception

template<typename T>
constexpr inline T condfn(bool condition,T result){
   return condition?result:throw std::logic_error("condition violation");
}

class testcnststruct{
  static constexpr int maxsize=5;
  int sz;
public:
  constexpr explicit testcnststruct(int s)noexcept:sz{s}{}
  constexpr int getsz()noexcept{return sz;}
  constexpr void setsz(int s)noexcept{sz=s;}
  constexpr int getmaxsz()noexcept{return maxsize;}
};
constexpr int getsz(testcnststruct t,int n){t.setsz(n);
   return condfn(t.getsz()<t.getmaxsz(),t.getsz());
                    //in constexpr calls get error of compilation
                    //(throw in constexpr fun), in runtime calls
                    //throw this exception
}
constexpr testcnststruct tstr{4};
constexpr int size=getsz(tstr,4);
//constexpr int size2=getsz(tstr,5); -error, getsz is no constexpr(has throw)
int main(int argc, char* argv[]){
  // TF<decltype(cri)>{};
   std::array<char,getsz(testcnststruct{4},4)> carr;
   //getsz(testcnststruct{4},5);// throw exception, runtime call
   std::cout<<carr.size()<<std::endl;
   return 0;
}
