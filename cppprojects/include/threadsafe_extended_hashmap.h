#ifndef THREADSAFE_EXTENDED_HASHMAP_H
#define THREADSAFE_EXTENDED_HASHMAP_H
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <forward_list>
#include <shared_mutex>
#include <mutex>
#include <atomic>
namespace ts_adv{

   template<typename Key, typename T,
      typename Hash=std::hash<Key>,
      typename Key_eq=std::equal_to<Key>,
      class Alloc=
         std::allocator<std::pair<const Key,T>> 
   >
   class ts_ex_hashmap final{
   public:
      using key_type = Key;
      using mapped_type = T;
      using key_equal = Key_eq;
      using hasher = Hash;
      using allocator_type = Alloc;
      using value_type = std::pair<const key_type, mapped_type>;
      using reference = value_type&;
      using const_reference = const value_type&;
      using pointer = 
         typename std::allocator_traits<allocator_type>::pointer;
      using const_pointer =
         typename std::allocator_traits<allocator_type>::const_pointer;
      using size_type = std::size_t;
      using difference_type = std::ptrdiff_t;
   private:

      struct bucket_type{
      public:
         using bucket_value = std::pair<key_type, mapped_type>;
         using list_type = std::forward_list<value_type,allocator_type>;
         using iterator = typename list_type::iterator;
         using const_iterator = typename list_type::const_iterator;
         mutable std::shared_mutex m_mut;
         list_type m_data;
         size_type m_size;

         bucket_type():m_mut{},m_data{},m_size{}{}
         bucket_type(const bucket_type&)=delete;
         bucket_type(bucket_type&& other){
            std::lock_guard lg{other.m_mut};
            m_data=std::move(other.m_data);
            m_size=std::move(other.m_size);
         }
         bucket_type& operator = (const bucket_type&)=delete;
         bucket_type& operator = (bucket_type&& other){
            std::scoped_lock sl(m_mut, other.m_mut);
            m_data=std::move(other.m_data);
            m_size=std::move(other.m_size);
         }
         ~bucket_type()=default;
         
         [[nodiscard]]
         iterator find_entry(const key_type& k){
            return std::find_if(std::begin(m_data),std::end(m_data),
                     [&k](const bucket_value& v)->bool{
                        return key_equal{}(v.first, k);
                     }); 
         }

         [[nodiscard]]
         const_iterator find_entry(const key_type& k)const{
            return std::find_if(std::cbegin(m_data),std::cend(m_data),
                     [&k](const bucket_value& v)->bool{
                        return key_equal{}(v.first, k);
                     }); 
         }

         [[nodiscard]]
         iterator find_before_entry(const key_type& k){
            iterator b_it=m_data.before_begin(),
               it=m_data.begin(),
               end=m_data.end();
            for(;it!=end;++b_it,++it){
               if(key_equal{}(it->first,k))
                     return b_it;
            }
            return end; }

         [[nodiscard]]
         const_iterator find_before_entry(const key_type& k)const{
            const_iterator b_cit=m_data.cbefore_begin(),
               cit=m_data.cbegin(),
               cend=m_data.cend();
            for(;cit!=cend;++b_cit,++cit){
               if(key_equal{}(cit->first,k))
                     return b_cit;
            }
            return cend;
         }

         template<typename Oit>
         Oit copy_values(Oit out)const{
            std::shared_lock sl{m_mut};
            return std::copy(std::cbegin(m_data),
                           std::cend(m_data),out);
         }
         [[nodiscard]]
         std::optional<mapped_type> value_for_unprotected(const key_type& k)const{
            auto cit=find_entry(k);
            if(cit!=std::cend(m_data))return cit->second;
         }
 
         [[nodiscard]]
         std::optional<mapped_type> value_for(const key_type& k)const{
            std::shared_lock sl{m_mut};
            return value_for_unprotected(k);
         }
         
         template<typename M>
         void insert_or_assign(const key_type& k, M&& m){
            std::lock_guard lg{m_mut};
            iterator it=find_entry(k);
            if(it!=std::end(m_data)){
               it->second=std::forward<M>(m);
               return;
            }
            ++m_size;
            m_data.push_front(bucket_value(k,
                                 std::forward<M>(m)));
         }

         template<typename M>
         void insert_or_assign(key_type&& k, M&& m){
            std::lock_guard lg{m_mut};
            iterator it=find_entry(k);
            if(it!=std::end(m_data)){
               it->second=std::forward<M>(m);
               return;
            }
            ++m_size;
            m_data.push_front(bucket_value(std::move(k),
                                 std::forward<M>(m)));
         }
         template<typename M>
         bool insert(const key_type& k, M&& m){
            std::lock_guard lg{m_mut};
            iterator it=find_entry(k);
            if(it==std::end(m_data)){
               m_data.push_front(bucket_value(k,
                                 std::forward<M>(m)));
               ++m_size;
               return true;
            }
            return false;
         }
         template<typename M>
         bool insert(key_type&& k, M&& m){
            std::lock_guard lg{m_mut};
            iterator it=find_entry(k);
            if(it==std::end(m_data)){
               m_data.push_front(bucket_value(std::move(k),
                                 std::forward<M>(m)));
               ++m_size;
               return true;
            }
            return false;
         }

         void remove(const key_type& k){
            std::lock_guard lg{m_mut};
            iterator b_it=find_before_entry(k);
            if(b_it!=std::end(m_data)){
               m_data.erase_after(b_it);
               --m_size;
            }

         }
         [[nodiscard]]
         size_type size_unprotected()const{
            return m_size;
         }

         [[nodiscard]]
         size_type size()const{
            std::shared_lock sl{m_mut};
            return size_unprotected();
         }
         void swap (bucket_type& other){
            using std::swap;
            swap(m_data,other.m_data);
            swap(m_size,other.m_size);
         }
      };//bucket_type

      mutable std::shared_mutex m_mut;
      const hasher m_hasher;
      size_type m_buckets_count;
      double m_load_factor;
      std::vector<bucket_type> m_buckets;

      constexpr bucket_type& bucket_for(const key_type& key)const{
         return m_buckets[m_hasher(key)%m_buckets_count];
      }
      bucket_type& bucket_for(const key_type& key){
         return m_buckets[m_hasher(key)%m_buckets_count];
      }

   public:
         ts_ex_hashmap(size_type buckets_count=1000, float load_factor=0.7f):
            m_mut{},
            m_hasher{},
            m_buckets_count{buckets_count},
            m_load_factor{load_factor},
            m_buckets(m_buckets_count){}
            
         
         size_type buckets_count()const{
            std::shared_lock sl{m_mut};
            return m_buckets_count;
         }

         size_type size()const{
            size_type sz{0};
            std::shared_lock sl{m_mut};
            for(auto& b:m_buckets)
               sz+=b.size();
            return sz;
         }
         
         template<typename Oit>
         Oit copy(Oit out)const{
            std::shared_lock sl{m_mut};
            std::for_each(std::cbegin(m_buckets),std::cend(m_buckets),
                  [&out](const bucket_type& b){
                     out=b.copy_values(out);
                  });
            return out;
         }

         template<typename M>
         void insert_or_assign(const key_type& k, M&& obj){
            std::shared_lock sl{m_mut};
            bucket_for(k).insert_or_assign(k,
                              std::forward<M>(obj));
         }

         template<typename M>
         void insert_or_assign(key_type&& k, M&& obj){
            std::shared_lock sl{m_mut};
            bucket_for(k).insert_or_assign(std::move(k),
                              std::forward<M>(obj));
         }

         template<typename M>
         bool insert(const key_type& k, M&& obj){
            std::shared_lock sl{m_mut};
            return bucket_for(k).insert(k,
                              std::forward<M>(obj));
         }

         template<typename M>
         bool insert(key_type&& k, M&& obj){
            std::shared_lock sl{m_mut};
            return bucket_for(k).insert(std::move(k),
                                       std::forward<M>(obj));
         }

         std::optional<mapped_type> value_for(const key_type& k)const{
            std::shared_lock sl{m_mut};
            return bucket_for(k).value_for(k);
         }


         void remove(const key_type& k){
            std::shared_lock sl{m_mut};
            bucket_for(k).remove(k);
         }

         void rehash(double load_factor=0.7f){
            size_type sz{0};
            std::lock_guard lg{m_mut};
            //get count of pairs
            for(auto& b:m_buckets)
               sz+=b.size_unprotected();
            sz=static_cast<size_type>(sz/load_factor);
            //create temporary "buckets"
            std::vector<bucket_type> tmp(sz);
            //move all nodes from old buckets to tmp
            for(auto& b : m_buckets){
               auto& old_data=b.m_data;

               while(old_data.begin()!=old_data.end()){
                  size_type bucket_num=m_hasher(old_data.front().first) % sz;
                  auto& new_data=tmp[bucket_num].m_data;
                  //move node from data of bucket b to data in new bucket
                  new_data.splice_after(new_data.cbefore_begin(),
                        old_data,old_data.cbefore_begin());
                  ++new_data.m_size;
               }
            }
            using std::swap;
            //swap old buckets and new buckets
            swap(tmp,m_buckets);
         }
   };
}//ts_adv




#endif //THREADSAFE_EXTENDED_HASHMAP_H
