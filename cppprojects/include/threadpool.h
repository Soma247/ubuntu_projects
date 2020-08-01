#ifndef THREADPOOL_H
#define THREADPOOL_H
#include "jointhread.h"
#include "threadsafe_queue.h"
#include <memory>
#include <future>
#include <deque>
#include <vector>
namespace thread_adv{
   
   template<
      template<typename T> class AllocTemplate=std::allocator
   >
   class basic_thread_pool final{
      class ptask_wrapper{
         struct impl_base{
            virtual void operator()()=0;
            virtual ~impl_base(){}
         };

         //void ret and R ret
         template<typename Callable>
         struct impl:public impl_base{
            Callable m_callable;
            explicit impl(Callable&& c):m_callable{std::move(c)}{}
            void operator()()override{
               return m_callable();
            }
         };

         std::unique_ptr<impl_base> m_pimpl;
      public:
         template<typename T>
         ptask_wrapper(std::packaged_task<T()>&& pt):
            m_pimpl{new std::packaged_task<T()>(std::move(pt))}{}
         
         ptask_wrapper(ptask_wrapper&& other):
            m_pimpl{std::move(other.m_pimpl)}{}
         ptask_wrapper(const ptask_wrapper&)=delete;

         ptask_wrapper& operator = (ptask_wrapper&& other){
            m_pimpl=std::move(other.m_pimpl);
            return *this;
         }
         void operator()(){
            (*m_pimpl)();
         }
         
         ~ptask_wrapper(){}
      };
      using allocator_type = AllocTemplate<ptask_wrapper>;
      
      class inner_queue_type{
         std::deque<ptask_wrapper,allocator_type> m_queue;
         mutable std::mutex m_mut;
      public:
         inner_queue_type(){}
         inner_queue_type(const inner_queue_type&)=delete;
         inner_queue_type(inner_queue_type&&)=delete;
         inner_queue_type& operator = (const inner_queue_type&)=delete;
         inner_queue_type& operator = (inner_queue_type&&)=delete;

         template<typename...A>
            inner_queue_type(A&&...args):m_queue(std::forward<A>(args)...){}

         void push(ptask_wrapper&& task){
            std::lock_guard lg{m_mut};
            m_queue.push_back(std::move(task));
         }
         
         bool try_pop(ptask_wrapper& task){
            std::lock_guard lg{m_mut};
            if(m_queue.empty())
               return false;
            task=std::move(m_queue.front());
            m_queue.pop_front();
            return true;
         }
         
         bool try_steal(ptask_wrapper& task){
            std::lock_guard lg{m_mut};
            if(m_queue.empty())
               return false;
            task=std::move(m_queue.back());
            return true;
         };

         void reserve(std::size_t sz){
            std::lock_guard lg{m_mut};
            m_queue.reserve(sz);
         }
         
         std::size_t size()const{
            std::lock_guard lg{m_mut};
            return m_queue.size();
         }
         bool empty()const{
            std::lock_guard lg{m_mut};
            return m_queue.empty();
         }
      };//inner_queue_type

      thread_local static inner_queue_type* tl_p_inner_queue;
      ts_adv::ts_queue<ptask_wrapper,allocator_type> m_queue;
      std::atomic<bool> m_stopped;
      std::vector<inner_queue_type> m_inner_queues;
      std::vector<thread_adv::jointhread> m_threads;

      void init_p_inner_queue(std::size_t index){
         tl_p_inner_queue = &m_inner_queues[index];
      }

      bool try_steal_task(ptask_wrapper& task, std::size_t index){
         std::size_t sz=m_inner_queues.size();
         for(std::size_t i=0;i<sz;++i){
               if(m_inner_queues[(index+i+1) % sz]
                     .try_steal_task(task)) 
                  return true;
         }
         return false;
      }
      bool try_take_task(ptask_wrapper& task, std::size_t index){
         return (tl_p_inner_queue && 
                 tl_p_inner_queue->try_pop(task))||
                 m_queue.pop(task)||
                 try_steal(task, index); 
      }
      void thread_worker(std::size_t index){
         try{
            this->init_p_inner_queue(index);
            ptask_wrapper current_task{};
            while(!m_stopped.load(std::memory_order_relaxed)){
               if(try_take_task(current_task, index))
                  current_task();
               else std::this_thread::yield();
            }
         }
         catch(...){
            
         }
      }

   public: 
      basic_thread_pool():m_queue{},m_stopped{false},
                           m_inner_queues{},m_threads{}
      {
         std::size_t thread_count=std::thread::hardware_concurrency();
         if(!thread_count)
            thread_count=1;
         m_inner_queues.resize(thread_count);
         m_threads.reserve(thread_count);
         for(std::size_t i{0};i<thread_count;++i)
            m_threads.push_back(
                  jointhread(&basic_thread_pool::thread_worker,this,i));
      }
      basic_thread_pool(const basic_thread_pool&)=delete;
      basic_thread_pool(basic_thread_pool&&)=delete;
      basic_thread_pool& operator = (const basic_thread_pool&)=delete;
      basic_thread_pool& operator = (basic_thread_pool&&)=delete;
      void stop(){m_stopped.store(true,std::memory_order_relaxed);}
      ~basic_thread_pool(){stop();}
     
      template<typename Ret>
      std::future<Ret> assign_task(std::packaged_task<Ret()>&& task){
         std::future<Ret> ret=task.get_future();
         m_queue.push(new ptask_wrapper{std::move(task)});
         return ret;
      }
   private:
     };//basic_thread_pool
   using thread_pool = basic_thread_pool<std::allocator>;
}//namespace thread_adv
#endif//THREADPOOL_H
