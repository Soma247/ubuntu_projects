#include <thread>
#include <cassert>
class thread_guard{
public:
   //construction
   explicit thread_guard(std::thread&& tr)noexcept;
   explicit thread_guard(thread_guard&& tg)noexcept;
   //join/detach
   void join();
   //get
   std::thread& get()noexcept;
   //destruction
   ~thread_guard();
   //move assigment
   thread_guard& operator = (thread_guard&& tg)noexcept;
   //deleted copy ctors and assigment
   thread_guard(const std::thread&)=delete;
   thread_guard(const thread_guard&)=delete;
   thread_guard& operator=(const std::thread&)=delete;
   thread_guard& operator=(const thread_guard&)=delete;
private:
   std::thread tr_;
};

//construction
inline thread_guard::thread_guard(std::thread&& tr)noexcept:
   tr_{std::move(tr)}{assert(tr_.joinable());}
inline thread_guard::thread_guard(thread_guard&& tg)noexcept:
   tr_{std::move(tg.tr_)}{assert(tr_.joinable());}
//join/detach
inline void thread_guard::join(){assert(tr_.joinable());tr_.join();}
//get
inline std::thread& thread_guard::get()noexcept{return tr_;}
//destruction
inline thread_guard::~thread_guard(){
   if(tr_.joinable())
      tr_.join();
}
//move assigment
inline thread_guard& thread_guard::operator = (thread_guard&& tg)noexcept{
   tr_=std::move(tg.tr_);
   return *this;
}
