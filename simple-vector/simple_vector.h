#pragma once

#include <cassert>
#include <initializer_list>
#include "array_ptr.h"
#include <stdexcept>
#include <algorithm>
#include <utility>

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity_to_reserve)
        : capacity_(capacity_to_reserve)
    {}

    size_t capacity_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) 
        : arr_ptr_(size)
        , size_(size)
        , capacity_(size)
    {
        std::generate(begin(), end(), []() { return Type(); });
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) 
        : SimpleVector(size)
    { 
        std::generate(begin(), end(), [&value]() { return std::move(value); });
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) 
        : SimpleVector(init.size())
    {
        std::copy(init.begin(), init.end(), begin());
    }

    SimpleVector(const SimpleVector& other) 
        : arr_ptr_(other.GetCapacity())
        , size_(other.GetSize())
        , capacity_(other.GetCapacity())
    {
        std::copy(other.begin(), other.end(), begin());
    }

    SimpleVector(SimpleVector&& other) 
        : arr_ptr_(other.GetCapacity())
        , size_(other.GetSize())
        , capacity_(other.GetCapacity())
    {
        std::move(std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()), begin());
        other.Clear();
    }

    SimpleVector(ReserveProxyObj obj) 
        : arr_ptr_(obj.capacity_)
        , size_(0)
        , capacity_(obj.capacity_)
    {
    }   

    SimpleVector& operator=(const SimpleVector& other) {
        if (this == &other) {
            return *this;
        }
        SimpleVector tmp(other);
        swap(tmp);
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& other) {
        if (this == &other) {
            return *this;
        }
        SimpleVector tmp(std::move(other));
        swap(tmp);
        return *this;
    }

    void PushBack(Type&& value) {
        Insert(end(), std::move(value));
    }

    void PushBack(const Type& value) {
        Insert(end(), value);
    }

    void PopBack() noexcept {
        assert(!IsEmpty());
        --size_;
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        Iterator it_to_insert = const_cast<Iterator>(pos);
        Iterator it_to_return;
        if (size_ != capacity_) {
            it_to_return = std::move_backward(it_to_insert, end(), end() + 1) - 1;
            *it_to_return = std::move(value);
            ++size_;
        }
        else {
            size_t new_cap = capacity_ == 0 ? 1 : capacity_ * 2;
            SimpleVector tmp(new_cap); 
            tmp.size_ = size_ + 1;
            it_to_return = std::move(begin(), it_to_insert, tmp.begin());
            *it_to_return = std::move(value);
            std::move_backward(it_to_insert, end(), tmp.end());
            swap(tmp);
        }
        return it_to_return;
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        Iterator it_to_insert = const_cast<Iterator>(pos);
        Iterator it_to_return;
        if (size_ != capacity_) {
            it_to_return = std::copy_backward(it_to_insert, end(), end() + 1) - 1;
            *it_to_return = value;
            ++size_;
        }
        else {
            size_t new_cap = capacity_ == 0 ? 1 : capacity_ * 2;
            SimpleVector tmp(new_cap); 
            tmp.size_ = size_ + 1;
            it_to_return = std::copy(begin(), it_to_insert, tmp.begin());
            *it_to_return = value;
            std::copy_backward(it_to_insert, end(), tmp.end());
            swap(tmp);
        }
        return it_to_return;
    }

    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos <= end() && !IsEmpty());
        Iterator it_to_erase = const_cast<Iterator>(pos);
        std::move(it_to_erase + 1, end(), it_to_erase);
        --size_;
        return it_to_erase;
    }

    void swap(SimpleVector& other) noexcept {
        arr_ptr_.swap(other.arr_ptr_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }
        SimpleVector tmp(new_capacity);
        tmp.size_ = size_;
        std::move(begin(), end(), tmp.begin());
        swap(tmp);
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        // Напишите тело самостоятельно
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        // Напишите тело самостоятельно
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return arr_ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return arr_ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("out of range");
        }
        return arr_ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("out of range");
        }
        return arr_ptr_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        }
        else if (new_size <= capacity_) {
            size_t old_size = size_;
            size_ = new_size;
            std::generate(begin() + old_size, end(), []() { return Type(); });
        }
        else if (new_size > capacity_) {
            size_t new_cap = std::max(new_size, capacity_ * 2);
            SimpleVector tmp(new_cap);
            std::move(begin(), end(), tmp.begin());
            swap(tmp);
        } 
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return arr_ptr_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return arr_ptr_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return arr_ptr_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return arr_ptr_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return arr_ptr_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return arr_ptr_.Get() + size_;
    }

private:
    ArrayPtr<Type> arr_ptr_;

    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
} 