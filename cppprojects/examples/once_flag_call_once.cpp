#include <iostream>
#include <thread>
#include <mutex>
#undef _GLIBCXX_HAVE_TLS
std::once_flag flag1, flag2;
void simple_do_once(){
   std::call_once(flag1, [](){ std::cout << "Simple example: called once\n"; });
}
 
void may_throw_function(bool do_throw){
   if (do_throw) {
      std::cout << "throw: call_once will retry\n"; // this may appear more than once
      throw std::exception();
   }
    std::cout << "Didn't throw, call_once will not attempt again\n"; // guaranteed once
}
 
void do_once(bool do_throw){
   try {
      std::call_once(flag2, may_throw_function, do_throw);
   }
   catch (...){}
}

class onceinit{
   std::once_flag of;
   int i;
public:
   void do_init(bool b){
      if(b){
         std::cout<<"onceinit::do_init throw"<<std::endl;
         throw std::logic_error("onceinit::do_init throw");
      }
      else{std::cout<<"onceinit::do_init initted"<<std::endl;}
      i=125;
   }
   void get(bool b){
      try{
         std::call_once(of,&onceinit::do_init,this,b);
      }
      catch(...){}
   }
};
int main(){
   std::thread st1(simple_do_once);
   std::thread st2(simple_do_once);
   std::thread st3(simple_do_once);
   std::thread st4(simple_do_once);
   st1.join();
   st2.join();
   st3.join();
   st4.join();

   //in pthread lib, pthread_once may use mutex.lock && mutex.unlock, gl)))
   std::thread t1(do_once, true);
   std::thread t2(do_once, true);
   std::thread t3(do_once, false);
   std::thread t4(do_once, true);
   t1.join();
   t2.join();
   t3.join();
   t4.join();

   onceinit oi;
   std::thread tt1(&onceinit::get,std::ref(oi),true);
   std::thread tt2(&onceinit::get,&oi,true);
   std::thread tt3(&onceinit::get,std::ref(oi),false);
   std::thread tt4(&onceinit::get,std::ref(oi),true);

   tt1.join();
   tt2.join();
   tt3.join();
   tt4.join();
}
