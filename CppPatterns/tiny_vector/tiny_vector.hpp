#pragma once

#include <memory>
#include <stdexcept>

namespace tiny {

template<typename T>
class vector {
public:    
    vector() {
        begin_ = m_allocator.allocate(1);
        end_ = begin_;
        end_cap_ = begin_+1;
    }

    ~vector() {
        m_allocator.deallocate(begin_, end_cap_ - begin_);
    }

    // copy rules
    vector(const vector& o) {
        const size_t m_size = o.end_cap_ - o.begin_;
        begin_ = m_allocator.allocate(m_size);
        end_ = begin_ + (o.end_ - o.begin_);
        end_cap_ = begin_ + m_size;
        std::uninitialized_copy(o.begin_, o.end_, begin_);
    }

    vector &operator=(const vector& o) {
        m_allocator.deallocate(begin_, end_cap_ - begin_); 
        const size_t m_size = o.end_cap_ - o.begin_;
        begin_ = m_allocator.allocate(m_size);
        end_ = begin_ + (o.end_ - o.begin_);
        end_cap_ = begin_ + m_size;
        std::uninitialized_copy(o.begin_, o.end_, begin_);
    }

    // move rules
    vector(vector &&o) : 
        m_allocator(std::move(o.m_allocator)),
        begin_(std::exchange(o.begin_, nullptr)),
        end_(std::exchange(o.end_, nullptr)),
        end_cap_(std::exchange(o.end_cap_, nullptr))
    {}

    vector& operator=(vector&& o) {
        m_allocator = std::move(o.m_allocator);
        begin_ = std::exchange(o.begin_, nullptr);
        end_ = std::exchange(o.end_, nullptr);
        end_cap_ = std::exchange(o.end_cap_, nullptr);
        return *this;    
    }

    // main methods
    void push_back(const T& value) {
        if (end_ == end_cap_) {
            reallocate();
        }

        std::allocator_traits<std::allocator<T>>::construct(m_allocator, end_, value);
        ++end_;
    }

    void emplace_back(T&& value) {
        if (end_ == end_cap_) {
            reallocate();
        }

        std::allocator_traits<std::allocator<T>>::construct(m_allocator, end_, std::move(value));
        ++end_;
    }

    /* void insert() {} */
    /* void erase() {} */
    void clear() {
        for (T* i = begin_; i != end_; ++i) {
            std::allocator_traits<std::allocator<T>>::destroy(m_allocator, i);
        }
    }

    void resize(size_t new_size) {
        const size_t m_size = end_cap_ - begin_;
        const size_t n_elements = end_ - begin_;

        if (new_size < n_elements) {
            throw std::out_of_range("new_size < n_elements");
        }

        T* new_data = m_allocator.allocate(new_size);
        std::uninitialized_copy(begin_, end_, new_data);
        m_allocator.deallocate(begin_, m_size);

        begin_ = new_data;
        end_ = new_data + n_elements;
        end_cap_ = new_data + new_size;
    }

    // scuffed iterator
    T* begin() { return begin_; }
    T* end() { return end_; }

    const T* begin() const { return begin_; }
    const T* end() const { return end_; }
    const T* cbegin() const { return begin_; }
    const T* cend() const { return end_; }

private:
    void reallocate() {
        const size_t m_size = end_cap_ - begin_;
        T* new_data = m_allocator.allocate(m_size * 2);
        std::uninitialized_copy(begin_, end_, new_data);
        m_allocator.deallocate(begin_, m_size);

        begin_ = new_data;
        end_ = new_data + m_size;
        end_cap_ = new_data + m_size * 2;
    }

    std::allocator<T> m_allocator;

    T* begin_ = nullptr;
    T* end_ = nullptr;
    T* end_cap_ = nullptr;
};

} // namespace tiny
