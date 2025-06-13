#pragma once

#include <memory>
#include <stdexcept>
#include <utility>

/*
 * Rule of 0/3/6/6
 *
 * std::allocator returns uninitialized memory, need to initialize the memory to a type before assign,
 * meaning use std::uninitialized_copy instead of memcpy and also need to use std::allocator_traits
 * 
 * std::allocator_traits<std::allocator<T>>. The old api of std::allocator used to have
 * everything contained inside the traits. The reason to separate it is
 *
 * add noexcept on anything that cannot throw a c++ exception out of the body
 *
 * edge cases:
 *  - push_back failing with tiny::vector<std::unique_ptr<T>>, unique_ptr doesn't have a copy ctor
 *  - container elements not copyable, cannot use std::unitialized_copy for copy ctor + assign
 *    (this just fails at compile time, which is what it is supposed to do, but clang's vector
 *    throws a different error)
 *
 * TODO
 *  - custom allocator
 *  - implement the other infinite functions for std::vector
 *  - reverse iterator methods
 *  - use concepts "requires copy_constructible" to have std::is_copy_assignable be correct
 *    alternatively use the older SFINAE trick
 *  - inspect assembly better / more
 */

namespace tiny {

template<typename T, typename allocator_type = std::allocator<T>>
class vector {
    using iterator = T*;
    using const_iterator = const T*;

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
    vector(vector &&o) noexcept : 
        m_allocator(std::move(o.m_allocator)),
        begin_(std::exchange(o.begin_, nullptr)),
        end_(std::exchange(o.end_, nullptr)),
        end_cap_(std::exchange(o.end_cap_, nullptr))
    {}

    vector& operator=(vector&& o) noexcept {
        if (this == &o) {
            return *this;
        }

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

        /*
         * Fast path with no resize
         *
         * rdi    = this
         * rsi    = &value
         * r15    = end_
         *
         * LBB11_10:
         * movl	(%rsi), %eax   // value -> eax
         * movl	%eax, (%r15)   // write to end_
         * addq	$4, %r15       // increment end_ by integer size
         * movq	%r15, 16(%rdi) // write it back to this->end_
         * addq	$24, %rsp
         * retq
         *
         * At least one load was necessary for value into eax
         * At least one store is necessary for value at end_
         * Since end_ pointer was already in a register r15 just increment than store
         */

        std::allocator_traits<std::allocator<T>>::construct(m_allocator, end_, value);
        ++end_;
    }

    void push_back(T&& value) {
        if (end_ == end_cap_) {
            reallocate();
        }

        std::allocator_traits<std::allocator<T>>::construct(m_allocator, end_, std::move(value));
        ++end_;
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        if (end_ == end_cap_) {
            reallocate();
        }

        std::allocator_traits<std::allocator<T>>::construct(
            m_allocator, 
            end_, 
            std::forward<Args>(args)...
        );

        ++end_;
    }

    void pop_back() {
        if (end_ == begin_) {
            throw std::runtime_error("end_ == begin_");
        }

        std::allocator_traits<std::allocator<T>>::destroy(m_allocator, end_ - 1);
        --end_;
    }

    size_t size() noexcept {
        return end_ - begin_;
    }

    /* void insert() {} */
    /* void erase() {} */
    void clear() noexcept {
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

    void swap(vector& o) {
        std::swap(m_allocator, o.m_allocator);
        std::swap(begin_, o.begin_);
        std::swap(end_, o.end_);
        std::swap(end_cap_, o.end_cap_);
    }

    // access methods
    T& operator[](size_t index) {
        return begin_[index];
    }

    const T& operator[](size_t index) const {
        return begin_[index];
    }

    T& at(size_t index) {
        if (index >= size()) {
            throw std::out_of_range("index >= size()");
        }
        return begin_[index];
    }

    T& front() {
        return *begin_;
    }

    T& back() {
        return *(end_ - 1);
    }

    // scuffed iterator
    iterator begin() { return begin_; }
    iterator end() { return end_; }

    const_iterator begin() const { return begin_; }
    const_iterator end() const { return end_; }
    const_iterator cbegin() const { return begin_; }
    const_iterator cend() const { return end_; }

private:
    void reallocate() {
        const size_t m_size = end_cap_ - begin_;
        T* new_data = m_allocator.allocate(m_size * 2);
        std::uninitialized_move(begin_, end_, new_data);
        m_allocator.deallocate(begin_, m_size);

        begin_ = new_data;
        end_ = new_data + m_size;
        end_cap_ = new_data + m_size * 2;
    }

    allocator_type m_allocator;

    T* begin_ = nullptr;
    T* end_ = nullptr;
    T* end_cap_ = nullptr;
};

} // namespace tiny
