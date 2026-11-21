#pragma once

#include <cstddef>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>

template <typename T>
class Vector {
private:
    template <typename Element>
    class BasicIterator {
    public:
        Element& operator*() const {
            return *position;
        }

        Element* operator->() const {
            return position;
        }

        BasicIterator& operator++() {
            ++position;
            return *this;
        }

        BasicIterator operator++(int) {
            BasicIterator previous = *this;
            ++(*this);
            return previous;
        }

        BasicIterator& operator--() {
            --position;
            return *this;
        }

        BasicIterator operator--(int) {
            BasicIterator previous = *this;
            --(*this);
            return previous;
        }

        bool operator==(const BasicIterator& other) const {
            return position == other.position;
        }

        bool operator!=(const BasicIterator& other) const {
            return !(*this == other);
        }

    private:
        friend class Vector;

        explicit BasicIterator(Element* position) : position(position) {}

        Element* position;
    };

public:
    using Iterator = BasicIterator<T>;
    using ConstIterator = BasicIterator<const T>;

    Vector() = default;

    Vector(const Vector& other) : Vector(other, other.size_) {}

    Vector& operator=(const Vector& other) {
        if (this != &other) {
            Vector copy(other);
            swapStorage(copy);
        }
        return *this;
    }

    ~Vector() {
        clear();
        if (data != nullptr) {
            allocator.deallocate(data, capacity_);
        }
    }

    std::size_t size() const {
        return size_;
    }

    std::size_t capacity() const {
        return capacity_;
    }

    bool empty() const {
        return size_ == 0;
    }

    T& operator[](std::size_t index) {
        return data[index];
    }

    const T& operator[](std::size_t index) const {
        return data[index];
    }

    T& at(std::size_t index) {
        checkIndex(index);
        return data[index];
    }

    const T& at(std::size_t index) const {
        checkIndex(index);
        return data[index];
    }

    void push_back(const T& value) {
        if (size_ < capacity_) {
            append(value);
            return;
        }

        Vector expanded(*this, nextCapacity());
        expanded.append(value);
        swapStorage(expanded);
    }

    void erase(std::size_t index) {
        checkIndex(index);
        for (std::size_t i = index; i + 1 < size_; ++i) {
            data[i] = data[i + 1];
        }
        --size_;
        data[size_].~T();
    }

    void clear() {
        while (size_ > 0) {
            --size_;
            data[size_].~T();
        }
    }

    Iterator begin() {
        return Iterator(data);
    }

    Iterator end() {
        return Iterator(empty() ? data : data + size_);
    }

    ConstIterator begin() const {
        return ConstIterator(data);
    }

    ConstIterator end() const {
        return ConstIterator(empty() ? data : data + size_);
    }

    template <typename Compare>
    void sort(Compare comesBefore) {
        auto current = begin();
        if (current == end()) {
            return;
        }

        // Вставляем очередной элемент в уже отсортированную часть.
        for (++current; current != end(); ++current) {
            const T value = *current;
            auto position = current;

            while (position != begin()) {
                auto previous = position;
                --previous;
                if (!comesBefore(value, *previous)) {
                    break;
                }

                *position = *previous;
                position = previous;
            }
            *position = value;
        }
    }

private:
    using Allocator = std::allocator<T>;
    Allocator allocator;
    T* data = nullptr;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;

    explicit Vector(std::size_t capacity)
        : data(capacity == 0 ? nullptr : allocator.allocate(capacity)), capacity_(capacity) {}

    Vector(const Vector& other, std::size_t capacity) : Vector(capacity) {
        for (const T& value : other) {
            append(value);
        }
    }

    void append(const T& value) {
        // Создаём объект в памяти, заранее выделенной для элемента.
        ::new (static_cast<void*>(data + size_)) T(value);
        ++size_;
    }

    std::size_t nextCapacity() const {
        const std::size_t limit = std::allocator_traits<Allocator>::max_size(allocator);
        if (capacity_ >= limit) {
            throw std::length_error("Vector capacity exceeded");
        }
        if (capacity_ == 0) {
            return 1;
        }
        return capacity_ > limit / 2 ? limit : capacity_ * 2;
    }

    void checkIndex(std::size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Vector index out of range");
        }
    }

    void swapStorage(Vector& other) noexcept {
        std::swap(data, other.data);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
};
