#ifndef JOINTHREAD_H
#define JOINTHREAD_H
#include <type_traits>
#include <thread>
namespace thread_adv{

   class jointhread{
      std::thread m_thread;
      void check_join()noexcept{
         if(m_thread.joinable())
            m_thread.join();
      }
   public:

      template<typename Callable, typename... Args,
         typename = std::enable_if_t<
            !std::is_same_v<jointhread,
            std::remove_cv_t<std::remove_reference_t<Callable>>>
         >
      >
      explicit jointhread(Callable&& cb, Args...args):
         m_thread(
               std::forward<Callable>(cb),
               std::forward<Args>(args)...){}

      explicit jointhread(const jointhread&)=delete;
      explicit jointhread(jointhread&& rhs)noexcept=default;
      explicit jointhread(const std::thread&)=delete;
      explicit jointhread(std::thread&& rhs)noexcept:
         m_thread{std::move(rhs)}{}
      
      
      jointhread& operator = (const std::thread&)=delete;
      jointhread& operator = (const jointhread&)=delete;
      
      jointhread& operator = (std::thread&& rhs)noexcept{
         check_join();
         m_thread=std::move(rhs);
         return *this;
      }
      jointhread& operator = (jointhread&& rhs)noexcept{
         return this->operator=(std::move(rhs.m_thread));
      }
      ~jointhread(){
         check_join();
      }

      void swap(jointhread& rhs)noexcept{
         m_thread.swap(rhs.m_thread);
      }

      using id=std::thread::id;
      id get_id()const noexcept{
         return m_thread.get_id();
      }

      bool joinable()const noexcept{
         return m_thread.joinable();
      }

      void join(){m_thread.join();}
      void detach(){m_thread.detach();}

      std::thread& to_thread()noexcept{return m_thread;}
      const std::thread& to_thread()const noexcept{return m_thread;}
      
      using native_handle_type=std::thread::native_handle_type;
      native_handle_type native_handle(){
         return m_thread.native_handle();
      }

      static unsigned int hardware_concurrency()noexcept{
         return std::thread::hardware_concurrency();
      }
   };

   }//thread_adv
namespace std{
   template<>
   void swap<thread_adv::jointhread>(
                  thread_adv::jointhread& lhs,
                  thread_adv::jointhread& rhs)noexcept{
      return lhs.swap(rhs);
   }
}
#endif//JOINTHREAD_H
