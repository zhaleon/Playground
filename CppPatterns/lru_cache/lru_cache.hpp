#pragma once
#include <concepts>
#include <memory>
#include <unordered_map>
#include <list>

template<typename T>
concept Hashable =
    requires(T t) {
        { std::hash<T>{}(t) } -> std::convertible_to<std::size_t>;
    };

template<typename T>
concept MovableValue = std::movable<T>;

template<
    typename K,
    typename V,
    typename Alloc = std::allocator<std::byte>
> requires(Hashable<K> && MovableValue<V>)
class LRUCache {
    using LinkedListNodeIterator = std::list<std::pair<K, V>>::iterator;

public:
    LRUCache(size_t capacity) : capacity_(capacity), cache_hits_(0), cache_misses_(0) {
        map_.reserve(capacity_);
    }

    void put(K key, V value) {
        auto map_it = map_.find(key); // map_it = &{key, list<K,V>::iterator}
        if (map_it != map_.end()) {
            auto it = map_it->second;
            it->second = std::move(value);
            list_.splice(list_.begin(), list_, it); // bubble new access to the front
            return;
        }

        list_.emplace_back(key, std::move(value));
        auto it = --list_.end();
        map_.emplace(key, it);

        if (list_.size() > capacity_) {
            auto old_node = list_.back();
            map_.erase(old_node.first);
            list_.pop_back();
        }
    }

    V* get(const K &key) noexcept {
        auto it = map_.find(key);
        if (it == map_.end()) {
            ++cache_misses_;
            return nullptr;
        }

        list_.splice(list_.begin(), list_, it->second);
        ++cache_hits_;

        return &it->second->second;
    }

private:
    size_t capacity_;
    std::unordered_map<K, LinkedListNodeIterator> map_;
    std::list<std::pair<K, V>> list_;

    size_t cache_hits_ = 0;
    size_t cache_misses_ = 0;
};
