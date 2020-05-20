#ifndef VECTOR_H
#define VECTOR_H

#include <cstdio>
#include <assert.h>
#include <algorithm>


template<typename T>
struct vector {
    typedef T *iterator;
    typedef T const *const_iterator;

    vector();                                   // O(1) nothrow конструктор по умолчанию
    vector(vector const &other);                // O(N) strong  конструктор от другого вектора
    vector &operator=(vector const &other);     // O(N) strong  оператор присваивания

    ~vector();                                  // O(N) nothrow деструктор

    T &operator[](std::size_t i);               // O(1) nothrow обращение к i-ому элементу как a[i] (можно менять)
    T const &operator[](std::size_t i) const;   // O(1) nothrow обращение к i-ому элементу как a[i] (константу вернёт)

    bool empty() const;                         // O(1) nothrow пустой/нет

    T *data();                                  // O(1) nothrow указатель на массив
    T const *data() const;                      // O(1) nothrow указатель на массив (как константа)
    std::size_t size() const;                   // O(1) nothrow размер массива

    T &front();                                 // O(1) nothrow первый элемент массива (можно менять)
    T const &front() const;                     // O(1) nothrow первый элемент массива (как константа)

    T &back();                                  // O(1) nothrow последний элемент массива (можно менять)
    T const &back() const;                      // O(1) nothrow последний элемент массива (как константа)
    void push_back(const T &val);               // O(1)* strong добавление элемента в конец
    void pop_back();                            // O(1) nothrow удаление последнего элемента

    std::size_t capacity() const;               // O(1) nothrow вместимость вектора на данный момент
    void reserve(std::size_t val);              // O(N) strong  выделяет val памяти, если выделено меньше
    void shrink_to_fit();

    void clear();                               // O(N) nothrow отчищает занимаемую память

    void swap(vector &other);                   // O(1) nothrow меняет местами два вектора

    iterator begin();                           // O(1) nothrow итератор начала (можно менять)
    iterator end();                             // O(1) nothrow итератор конца (можно менять)

    const_iterator begin() const;               // O(1) nothrow итератор конца (как константа)
    const_iterator end() const;                 // O(1) nothrow итератор конца (как константа)

    iterator insert(const_iterator pos, T const &val);          // O(N) weak вставляет элемент на нужную позицию

    iterator erase(const_iterator pos);                         // O(N) weak удаляет элемент

    iterator erase(const_iterator first, const_iterator last);  // O(N) weak удаляет элементы в диапазоне

private:
    void push_back_realloc(T const &val);       // O(N) nothrow добавление нового элемента с расширением размера вектора
    void new_buffer(std::size_t new_capacity);  // O(N) weak изменение вместимости вектора

private:
    T *data_;
    std::size_t size_;
    std::size_t capacity_;
};

template<typename T>
vector<T>::vector() {
    capacity_ = 0;
    size_ = 0;
    data_ = nullptr;
}

template<typename T>
bool vector<T>::empty() const {
    return size_ == 0;
}

template<typename T>
vector<T>::vector(const vector &other) {  // strong
    size_t i = 0;
    if (other.size() == 0) {
        data_ = nullptr;
    } else {
        data_ = static_cast<T *>(operator new(other.size() * sizeof(T)));
    }
    try {
        for (; i < other.size(); i++) {
            new(data_ + i) T(other.data()[i]);
        }
    } catch (...) {
        while (i != 0) {
            data_[i - 1].~T();
            --i;
        }
        operator delete(data_);
        throw;
    }
    size_ = other.size();
    capacity_ = size_;
}

template<typename T>
vector<T> &vector<T>::operator=(const vector &other) {
    vector<T> temp(other);
    swap(temp);
    return *this;
}

template<typename T>
vector<T>::~vector() {
    for (size_t i = 0; i != size_; i++) {
        data_[i].~T();
    }
    operator delete(data_);
}

template<typename T>
T &vector<T>::operator[](std::size_t i) {
    assert(i < size_);
    return data_[i];
}

template<typename T>
T const &vector<T>::operator[](std::size_t i) const {
    assert(i < size_);
    return data_[i];
}

template<typename T>
T *vector<T>::data() {
    return data_;
}

template<typename T>
T const *vector<T>::data() const {
    return data_;
}

template<typename T>
std::size_t vector<T>::size() const {
    return size_;
}

template<typename T>
T &vector<T>::front() {
    return data_[0];
}

template<typename T>
T const &vector<T>::front() const {
    return data_[0];
}

template<typename T>
T &vector<T>::back() {
    return data_[size_ - 1];
}

template<typename T>
T const &vector<T>::back() const {
    return data_[size_ - 1];
}

template<typename T>
void vector<T>::push_back(const T &val) {
    T temp = val;
    if (size_ != capacity_) {
        new(data_ + size_) T(val);
    } else {
        push_back_realloc(val);
    }
    ++size_;
}

template<typename T>
void vector<T>::pop_back() {
    assert(size_ != 0);

    data_[size_ - 1].~T();
    --size_;
}

template<typename T>
std::size_t vector<T>::capacity() const {
    return capacity_;
}

template<typename T>
void vector<T>::reserve(std::size_t val) {
    if (val > capacity_) {
        new_buffer(val);
    }
}

template<typename T>
void vector<T>::shrink_to_fit() {
    if (size_ == capacity_) {
        return;
    }

    vector temp = (*this);
    swap(temp);
}

template<typename T>
void vector<T>::clear() {
    for (int i = 0; i < size_; i++) {
        data_[i].~T();
    }
    size_ = 0;
}

template<typename T>
void vector<T>::swap(vector &other) {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
}

template<typename T>
typename vector<T>::iterator vector<T>::begin() {
    return data_;
}

template<typename T>
typename vector<T>::iterator vector<T>::end() {
    return data_ + size_;
}

template<typename T>
typename vector<T>::const_iterator vector<T>::begin() const {
    return data_;
}

template<typename T>
typename vector<T>::const_iterator vector<T>::end() const {
    return data_ + size_;
}

template<typename T>
typename vector<T>::iterator vector<T>::insert(vector::const_iterator pos, const T &val) {
    int index = pos - data_;

    assert(index >= 0 && index <= size_);

    if (capacity_ == 0) {
        new_buffer(2);
    } else if (capacity_ == size_) {
        new_buffer(capacity_ * 2);
    }

    new(data_ + size_) T();
    for (int i = size_; i > index; i--) {
        data_[i] = data_[i - 1];
    }
    data_[index] = val;
    size_++;
    return data_ + index;
}

template<typename T>
typename vector<T>::iterator vector<T>::erase(vector::const_iterator pos) {
    return erase(pos, pos + 1);
}

template<typename T>
typename vector<T>::iterator vector<T>::erase(vector::const_iterator first, vector::const_iterator last) {
    int from = first - data_;
    int to = last - data_;

    assert (size_ != 0);
    assert (0 <= from && from < size_);   // Не уверен в корректности этой и следующей проверки
    assert (0 <= to && to < size_);
    assert (from <= to);

    int delta = to - from;
    for (int i = from; i < size_ - delta; i++) {
        std::swap(data_[i], data_[i + delta]);
    }

    for (int i = size_ - delta; i < size_; i++) {
        data_[i].~T();
    }
    size_ -= delta;
    return data_ + to;
}

template<typename T>
void vector<T>::push_back_realloc(const T &val) {
    T temp = val;
    if (capacity_ == 0) {
        new_buffer(2);
    } else {
        new_buffer(capacity_ * 2);
    }
    new(data_ + size_) T(temp);
}

template<typename T>
void vector<T>::new_buffer(std::size_t new_capacity) {

    assert(new_capacity >= size_);

    T *new_data = static_cast<T *>(operator new(new_capacity * sizeof(T)));
    size_t i = 0;
    try {
        for (; i < size_; i++) {
            new(new_data + i) T(data_[i]);
        }
    }
    catch (...) {
        while (i != 0) {
            new_data[i - 1].~T();
            --i;
        }
        operator delete(new_data);
        throw;
    }
    this->~vector();
    data_ = new_data;
    capacity_ = new_capacity;
}

#endif