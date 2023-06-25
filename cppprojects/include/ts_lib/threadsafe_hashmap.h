#ifndef THREADSAFE_HASHMAP_H
#define THREADSAFE_HASHMAP_H
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <forward_list>
#include <shared_mutex>
#include <mutex>
namespace ts_adv{

   template<typename Key, typename T,
      typename Hash=std::hash<Key>,
      typename Key_eq=std::equal_to<Key>,
      class Alloc=
         std::allocator<std::pair<const Key,T>> 
   >
   class ts_hashmap final{
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

      class bucket_type{
      public:
         using bucket_value = std::pair<key_type, mapped_type>;
         using list_type = std::forward_list<value_type,allocator_type>;
         using iterator = typename list_type::iterator;
         using const_iterator = typename list_type::const_iterator;
      private:
         mutable std::shared_mutex m_mut;
         list_type m_data;

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
      public:
         bucket_type(){}
         bucket_type(const bucket_type&)=delete;
         bucket_type(bucket_type&& other){
            std::lock_guard lg{other.m_mut};
            m_data=std::move(other.m_data);
         }
         bucket_type& operator = (const bucket_type&)=delete;
         bucket_type& operator = (bucket_type&& other){
            std::scoped_lock sl(m_mut, other.m_mut);
            m_data=std::move(other.m_data);
         }
         ~bucket_type()=default;
         
         template<typename Oit>
         Oit copy_values(Oit out)const{
            std::shared_lock sl{m_mut};
            return std::copy(std::cbegin(m_data),
                           std::cend(m_data),out);
         }

         [[nodiscard]]
         std::optional<mapped_type> mapped_for(const key_type& k)const{
            std::shared_lock sl{m_mut};
            auto cit=find_entry(k);
            if(cit!=std::cend(m_data))
               return cit->second;
            return {};
         }
         
         template<typename M>
         bool insert_or_assign(const key_type& k, M&& m){
            std::lock_guard lg{m_mut};
            iterator it=find_entry(k);
            if(it!=std::end(m_data)){
               it->second=std::forward<M>(m);
               return false;
            }
            m_data.push_front(bucket_value(k,
                                 std::forward<M>(m)));
            return true;
         }

         template<typename M>
         bool insert_or_assign(key_type&& k, M&& m){
            std::lock_guard lg{m_mut};
            iterator it=find_entry(k);
            if(it!=std::end(m_data)){
               it->second=std::forward<M>(m);
               return false;
            }
            m_data.push_front(bucket_value(std::move(k),
                                 std::forward<M>(m)));
            return true;
         }
         template<typename M>
         bool insert(const key_type& k, M&& m){
            std::lock_guard lg{m_mut};
            iterator it=find_entry(k);
            if(it==std::end(m_data)){
               m_data.push_front(bucket_value(k,
                                 std::forward<M>(m)));
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
               return true;
            }
            return false;
         }

         void remove(const key_type& k){
            std::lock_guard lg{m_mut};
            m_data.remove_if([&k](const bucket_value& val){
                              return key_equal{}(val.first,k);
                              });
         } 
      };//bucket_type

      const size_type m_buckets_count;
      std::vector<bucket_type> m_buckets;
      const hasher m_hasher;

      constexpr const bucket_type& bucket_for(const key_type& key)const{
         return m_buckets[m_hasher(key)%m_buckets_count];
      }
      bucket_type& bucket_for(const key_type& key){
         return m_buckets[m_hasher(key)%m_buckets_count];
      }
   public:
         ts_hashmap(size_type buckets_count):
            m_buckets_count{buckets_count},
            m_buckets(m_buckets_count),
            m_hasher{}{}
         
         size_type buckets_count()const{
            return m_buckets_count;
         }

         template<typename Oit>
         Oit copy(Oit out)const{
            std::for_each(std::cbegin(m_buckets),std::cend(m_buckets),
                  [&out](const bucket_type& b){
                     out=b.copy_values(out);
                  });
            return out;
         }

         template<typename M>
         bool insert_or_assign(const key_type& k, M&& obj){//return is_inserted
            return bucket_for(k).insert_or_assign(k,
                              std::forward<M>(obj));
         }
         template<typename M>
         bool insert_or_assign(key_type&& k, M&& obj){//return is_inserted
            auto& bckt = bucket_for(k);
            return bckt.insert_or_assign(std::move(k),
                              std::forward<M>(obj));
         }

         template<typename M>
         bool insert(const key_type& k, M&& obj){
            return bucket_for(k).insert(k,
                              std::forward<M>(obj));
         }

         template<typename M>
         bool insert(key_type&& k, M&& obj){
            auto& bckt = bucket_for(k);
            return bckt.insert(std::move(k),
                                       std::forward<M>(obj));

         }

         std::optional<mapped_type> mapped_for(const key_type& k)const{
            return bucket_for(k).mapped_for(k);
         }

         void remove(const key_type& k){
            bucket_for(k).remove(k);
         }
   };
}//ts_adv




#endif //THREADSAFE_HASHMAP_H
