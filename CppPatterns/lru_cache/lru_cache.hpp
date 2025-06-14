#pragma once
#include <concepts>
#include <memory>
#include <unordered_map>

template<typename T>
class LinkedList {
public:
    struct Node {
        T data;
        Node* next;
        Node* prev;

        Node() : next(nullptr), prev(nullptr) {}
        Node(T data) : data(std::move(data)), next(nullptr), prev(nullptr) {}

        T get() {
            return data;
        }
    };

public:
    // TODO make the other constructors
    LinkedList() {
        head_ = new Node;
        head_->next = nullptr;
        head_->prev = nullptr;
        tail_ = head_;
    }

    ~LinkedList() {
        // TODO ensure the list nodes are deleted
        delete head_;
        tail_ = nullptr;
    }

    Node* front() {
        return head_->next;
    }

    void push_front(Node *node) {
        // head_ -> node -> next
        Node *next = head_->next;
        next->prev = node;
        head_->next = node;

        node->next = next;
        node->prev = head_;
        ++size_;
    }

    void push_back(Node *node) {
        // [tail -> nullptr] -> [tail -> node}
        tail_->next = node;
        node->prev = tail_;
        tail_ = node;
        ++size_;
    }

    void bubble_front(Node *node) {
        Node *next = node->next;
        Node *prev = node->prev;

        if (next) {
            next->prev = prev;
        }

        if (prev) {
            prev->next = next;
        }

        push_front(node);
    }

    void pop_front() {
        // head_ -> a -> b
        Node *a = head_->next;
        if (a) {
            Node *b = a->next;
            if (b) {
                b->prev = head_;
                head_->next = b;
            }
            // TODO delete a?
            --size_;
        }
    }

    size_t size() {
        return size_;
    }

private:
    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    size_t size_ = 0;
};

template<typename T>
concept Hashable =
    requires(T t) {
        { std::hash<T>{}(t) } -> std::convertible_to<std::size_t>;
    };

template<typename T>
concept MovableValue = std::movable<T>;

template<
    typename HashableT,
    typename MovableT,
    typename Alloc = std::allocator<std::byte>
>
class LRUCache {
    using K = HashableT;
    using V = MovableT;
    using LinkedListNode = typename LinkedList<std::pair<K, V>>::Node;

public:
    LRUCache(size_t capacity) : capacity_(capacity) {}

    void put(K key, V value) {
        if (map_.find(key) != map_.end()) {
            map_.at(key)->data.second = std::move(value);
            list_.bubble_front(map_.at(key));
            return;
        }

        auto kv_pair = std::make_pair(key, std::move(value));
        LinkedListNode* node = new LinkedListNode(kv_pair);
        map_.emplace(key, node);

        if (list_.size() > capacity_) {
            auto old_node = list_.front();
            map_.erase(old_node->data.first);
            list_.pop_front();
        }
    }

    V* get(const K &key) noexcept {
        auto it = map_.find(key);
        if (it == map_.end()) {
            return nullptr;
        }

        LinkedListNode *node = it->second;
        list_.bubble_front(node);

        ++cache_hits_;
        return &node->data.second;
    }

private:
    size_t capacity_;
    std::unordered_map<K, LinkedListNode*> map_;
    LinkedList<std::pair<K, V>> list_;

    size_t cache_hits_ = 0;
    size_t cache_misses_ = 0;
};
