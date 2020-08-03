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
            virtual void call()=0;
            virtual ~impl_base(){}
         };

         //void ret and R ret
         template<typename Callable>
         struct impl:public impl_base{
            Callable m_callable;
            explicit impl(Callable&& c):m_callable{std::move(c)}{}
            void call()override{
               m_callable();
            }
         };

         std::unique_ptr<impl_base> m_pimpl;
      public:
         ptask_wrapper(){}

         template<typename T>
         ptask_wrapper(std::packaged_task<T()>&& pt):
         m_pimpl{std::make_unique<impl<decltype(pt)>>(std::move(pt))}{}
         
         ptask_wrapper(ptask_wrapper&& other)=default;
         ptask_wrapper(const ptask_wrapper&)=delete;

         ptask_wrapper& operator = (ptask_wrapper&& other)=default;
         
         void operator()(){
            m_pimpl->call();
         }
         
         ~ptask_wrapper()=default;
      };
      using allocator_type = AllocTemplate<ptask_wrapper>;
      
      class inner_queue_type{
         std::deque<ptask_wrapper,allocator_type> m_queue;
         mutable std::mutex m_mut;
      public:
         inner_queue_type(){}
         inner_queue_type(inner_queue_type&& other):m_queue{}{
            std::lock_guard lg{other.m_mut};
            m_queue=std::move(other.m_queue);
         }
         inner_queue_type(const inner_queue_type&)=delete;
         inner_queue_type& operator = (const inner_queue_type&)=delete;
         inner_queue_type& operator = (inner_queue_type&&)=delete;

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
            m_queue.pop_back();
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
      thread_local static std::size_t tl_thread_index;
      ts_adv::ts_queue<ptask_wrapper,allocator_type> m_queue;
      std::atomic<bool> m_stopped;
      std::vector<inner_queue_type> m_inner_queues;
      std::vector<thread_adv::jointhread> m_threads;

      void init_thread(std::size_t index){
         tl_thread_index=index;
         tl_p_inner_queue = &m_inner_queues[index];
      }

      bool try_steal_task(ptask_wrapper& task){
         std::size_t sz=m_inner_queues.size();
         for(std::size_t i=0;i<sz;++i){
               if(m_inner_queues[(tl_thread_index+i+1) % sz]
                     .try_steal(task)) 
                  return true;
         }
         return false;
      }
      bool try_take_task(ptask_wrapper& task){
         using status =
            typename ts_adv::ts_queue<
               ptask_wrapper,allocator_type>::pop_status;

         return (tl_p_inner_queue && 
                 tl_p_inner_queue->try_pop(task))||
                 m_queue.pop(task)==status::ready||
                 this->try_steal_task(task); 
      }
      
      void thread_worker(std::size_t index){
         try{
            this->init_thread(index);
            while(!m_stopped.load(std::memory_order_relaxed)){
               this->try_work();
            }
         }
         catch(...){
           stop();
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

      void try_work(){
         ptask_wrapper current_task{};
         if(this->try_take_task(current_task))
            current_task();
         else std::this_thread::yield();
      }

      template<typename Ret>
      std::future<Ret> assign_task(std::packaged_task<Ret()>&& task){
         std::future<Ret> ret=task.get_future();
         if(tl_p_inner_queue){
            tl_p_inner_queue->push(ptask_wrapper{std::move(task)});
         }
         else{
            m_queue.push(ptask_wrapper{std::move(task)});
         }
         return ret;
      }
   private:
     };//basic_thread_pool
   template<template<class>class alloc>
   inline thread_local size_t basic_thread_pool<alloc>::tl_thread_index{};

   template<template<class>class alloc>
   inline thread_local 
   typename basic_thread_pool<alloc>::inner_queue_type* 
         basic_thread_pool<alloc>::tl_p_inner_queue{nullptr}; 

   using thread_pool = basic_thread_pool<std::allocator>;
}//namespace thread_adv
#endif//THREADPOOL_H
