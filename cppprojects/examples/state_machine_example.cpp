#include <type_traits>
#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <functional>
#include <cassert>
#include <exception>
#include <stdexcept>
#include <future>
#include <chrono>
#include <limits>

template<class U>
struct TFS;

template<typename T>
decltype(auto) TF(T){
  return TFS<T>{};
};

class state_machine{
   void (state_machine::* state_ptr_)(void);
   bool rdy_to_exit_{false};

   void state1(){
      std::cout<<"state 1:\nto change state press number\n"
                  "to exit press \"q\""<<std::endl;
      char c{};
      std::cin>>c;
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
      switch (c){
         case 'q':
            state_ptr_=&state_machine::exit;
            break;
         case '1': 
            state_ptr_=&state_machine::state1;
            break;
         case '2':
            state_ptr_=&state_machine::state2;
            break;
         case '3':
            state_ptr_=&state_machine::state3;
            break;
         default:
            std::cout<<"undefined state:"<<c<<std::endl;
      }
   }

   void state2(){
      std::cout<<"state 2:\nto change state press number\n"
                  "to exit press \"q\""<<std::endl;
      char c{};
      std::cin>>c;
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
      switch (c){
         case 'q':
            state_ptr_=&state_machine::exit;
            break;
         case '1': 
            state_ptr_=&state_machine::state1;
            break;
         case '2':
            state_ptr_=&state_machine::state2;
            break;
         case '3':
            state_ptr_=&state_machine::state3;
            break;
         default:
            std::cout<<"undefined state:"<<c<<std::endl;
      }
   }

   void state3(){
      std::cout<<"state 3:\nto change state press number\n"
                  "to exit press \"q\""<<std::endl;
      char c{};
      std::cin>>c;
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
      switch (c){
         case 'q':
            state_ptr_=&state_machine::exit;
            break;
         case '1': 
            state_ptr_=&state_machine::state1;
            break;
         case '2':
            state_ptr_=&state_machine::state2;
            break;
         case '3':
            state_ptr_=&state_machine::state3;
            break;
         default:
            std::cout<<"undefined state:"<<c<<std::endl;
      }
   }

   void exit(){
      std::cout<<"exit state:exiting"<<std::endl;
      rdy_to_exit_=true;
   }
public:
   void run(){
      std::cout<<"state_machine is started"<<std::endl;
      state_ptr_=&state_machine::state1;
      while(!rdy_to_exit_){
         (this->*state_ptr_)();
      }
   }
};

int main(int argc, char* argv[]){
   using namespace std::chrono_literals;
   state_machine sm{};
   sm.run();
   return 0;
}
