#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>
#include <type_traits>

template <typename T, bool Extendable = false,
          typename Allocator = std::allocator<T>>
class circular_buffer {
 public:
  using value_type = T;
  using reference = T&;
  using pointer = T*;
  using const_reference = const T&;
  using size_type = std::size_t;
  using allocator_type = Allocator;
  using difference_type = std::ptrdiff_t;
  using alloc_traits = std::allocator_traits<Allocator>;

  template <bool IsConst>
  class base_iterator {
   public:
    using value_type = T;
    using raw_pointer = T*;
    using pointer = std::conditional_t<IsConst, const T*, T*>;
    using reference = std::conditional_t<IsConst, const T&, T&>;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;

    base_iterator(const circular_buffer* buff, raw_pointer element)
        : buff_(buff), it_(element) {}

    base_iterator() = default;
    base_iterator(const base_iterator& other) = default;
    base_iterator& operator=(const base_iterator& other) = default;
    ~base_iterator() = default;

    reference operator*() const {
      if (it_ == nullptr) {
        throw std::runtime_error("End iterator dereference\n");
      }
      return *it_;
    }

    pointer operator->() const { return it_; }

    reference operator[](difference_type index) const {
      return *(buff_->pointer_addition(it_, index));
    }

    base_iterator& operator++() {
      buff_->increment(it_);
      if (it_ == buff_->last_) {
        it_ = nullptr;
      }
      return *this;
    }

    base_iterator& operator+=(difference_type n) {
      if (n == 0) {
        return *this;
      }
      if (it_ == nullptr) {
        return *this;
      }
      if (n > 0) {
        difference_type index = logic_index(it_);
        if (index + n > buff_->size_) {
          throw std::runtime_error("Out of range\n");
        }
        it_ = buff_->pointer_addition(it_, n);
        if (it_ == buff_->last_) {
          it_ = nullptr;
        }
      } else {
        *this -= -n;
      }
      return *this;
    }

    base_iterator& operator-=(difference_type n) {
      if (n == 0) {
        return *this;
      }
      if (it_ == nullptr) {
        it_ = buff_->last_;
      }
      if (n > 0) {
        it_ = buff_->pointer_subtraction(it_, n);
      } else {
        *this += -n;
      }
      return *this;
    }

    difference_type logic_index(pointer ptr) const {
      if (ptr == nullptr) {
        return buff_->size_;
      }
      if (ptr >= buff_->first_) {
        return ptr - buff_->first_;
      } else {
        return (ptr - buff_->buff_) + (buff_->buff_end_ - buff_->first_);
      }
    }

    base_iterator operator++(int) {
      base_iterator copy = *this;
      ++(*this);
      return copy;
    }

    base_iterator& operator--() {
      if (it_ == nullptr) {
        it_ = buff_->last_;
      }
      buff_->decrement(it_);
      return *this;
    }

    base_iterator operator--(int) {
      base_iterator copy = *this;
      --(*this);
      return copy;
    }

    base_iterator operator+(difference_type n) const {
      base_iterator copy = *this;
      copy += n;
      return copy;
    }

    base_iterator operator-(difference_type n) const {
      base_iterator copy = *this;
      copy -= n;
      return copy;
    }

    difference_type operator-(const base_iterator& other) const {
      if (buff_ != other.buff_) {
        throw std::runtime_error("Different iterators arithmetics\n");
      }
      return logic_index(it_) - logic_index(other.it_);
    }

    bool operator<(const base_iterator& other) const {
      return *this - other < 0;
    }

    bool operator>(const base_iterator& other) const { return other < *this; }

    bool operator<=(const base_iterator& other) const {
      return !(*this > other);
    }

    bool operator>=(const base_iterator& other) const {
      return !(*this < other);
    }

    bool operator==(const base_iterator& other) const {
      return (it_ == other.it_ && buff_ == other.buff_);
    }
    bool operator!=(const base_iterator& other) const {
      return !(*this == other);
    }

    friend base_iterator operator+(difference_type n, base_iterator it) {
      it += n;
      return it;
    }

    friend base_iterator operator-(difference_type n, base_iterator it) {
      it -= n;
      return it;
    }

   public:
    const circular_buffer* buff_;
    raw_pointer it_;
  };

  using iterator = base_iterator<false>;
  using const_iterator = base_iterator<true>;
  using reverse_iterator = base_iterator<false>;
  using const_reverse_iterator = base_iterator<true>;

  circular_buffer(size_t capacity) {
    buff_ = alloc_traits::allocate(alloc_, capacity);
    buff_end_ = buff_ + capacity;
    first_ = buff_;
    last_ = buff_;
    size_ = 0;
  }

  // Fix
  circular_buffer(const allocator_type& alloc) : alloc_(alloc) {}

  circular_buffer(size_type capacity, allocator_type& alloc) {
    alloc_ = alloc;
    buff_ = alloc_traits::allocate(alloc, capacity);
    buff_end_ = buff_ + capacity;
    first_ = buff_;
    last_ = buff_;
    size_ = 0;
  }

  circular_buffer(const circular_buffer& other) {
    size_type capacity = other.capacity();
    alloc_ = alloc_traits::select_on_container_copy_construction(other.alloc_);
    buff_ = alloc_traits::allocate(alloc_, capacity);
    buff_end_ = buff_ + capacity;
    for (size_type i = 0; i < other.size_; ++i) {
      alloc_traits::construct(alloc_, buff_ + i, other.buff_[i]);
    }
    size_ = other.size_;
    first_ = buff_ + (other.first_ - other.buff_);
    last_ = buff_ + (other.last_ - other.buff_);
  }

  circular_buffer(size_type size, value_type value) {}

  circular_buffer(iterator begin, iterator end) {}

  circular_buffer() {}

  circular_buffer(std::initializer_list<T> list)
      : circular_buffer(list.size()) {
    for (typename std::initializer_list<T>::iterator it = list.begin();
         it != list.end(); ++it) {
      // Should use push_back() ?
      push_back(*it);
    }
  }

  circular_buffer& operator=(const circular_buffer& other) {}

  allocator_type get_allocator() { return alloc_; }

  reverse_iterator rbegin() { return base_iterator(); }

  reverse_iterator rend() { return base_iterator(); }

  const_reverse_iterator rbegin() const { return base_iterator(); }

  const_reverse_iterator rend() const { return base_iterator(); }

  const_reverse_iterator crbegin() const { return base_iterator(); }

  const_reverse_iterator crend() const { return base_iterator(); }

  iterator insert(const_iterator it, value_type value) {
    // Expand
    // After it put value
  }

  iterator insert(iterator it, value_type value) {
    std::cout << "Here\n";
    return iterator();
  }

  iterator insert(iterator it, std::initializer_list<T> list) {
    // After it put everything from list
  }


  iterator insert(const_iterator it, size_type count, value_type value) {
    // After it count times put value
  }

  iterator insert(const_iterator before, iterator range_begin,
                  iterator range_end) {}

  template <class InputIt>
  iterator insert(iterator before, InputIt range_begin, InputIt range_end) {}

  // iterator insert(const_iterator it, std::initializer_list)

  void clear() {}

  void resize(size_type new_size) {
    /*
    Delete all elements that after new_size
    */
  }

  void resize(size_type new_size, value_type value) {
    // It might not be full?
    // Think what happens if not full
    pointer new_buff = alloc_traits::allocate(alloc_, new_size);
    pointer new_buff_end = new_buff + new_size;
    for (size_type i = 0; i < size_; ++i) {
      alloc_traits::construct(alloc_, new_buff + i, buff_[i]);
    }
    auto first_shift = first_ - buff_;
    auto end_shift = last_ - buff_;
    first_ = new_buff + first_shift;
    last_ = new_buff + end_shift;
    
    // Change where first_ and end_
    // push_back(value)
    for (size_type i = 0; i < size_; ++i) {
      alloc_traits::destroy(alloc_, buff_[i]);
    }
    alloc_traits::deallocate(alloc_, buff_, capacity());
    buff_ = new_buff;
    buff_end_ = new_buff_end;
    for()
  }

  template <class U, class Q>
  void assign(U first_arg, Q second_arg) {
    if constexpr (std::is_integral_v<U>) {
      assign_impl_integer(first_arg, second_arg);
    } else {
      assign_impl(first_arg, second_arg);
    }
  }

  pointer pointer_addition(pointer ptr, difference_type n) const {
    difference_type shift = n;
    if (n > capacity()) {
      shift = shift % capacity();
    }
    if (shift < buff_end_ - ptr) {
      return ptr + shift;
    } else {
      return ptr + (shift - capacity());
    }
  }

  pointer pointer_subtraction(pointer ptr, difference_type n) const {
    if (n > ptr - buff_) {
      return ptr - (n - capacity());
    } else {
      return ptr - n;
    }
  }

  void assign_impl_integer(size_type n, value_type value) {
    if (n > capacity()) {
      throw std::runtime_error("Overflow\n");
    }
    size_ = 0;
    pointer ptr = begin().it_;
    last_ = ptr;
    for (size_type i = 0; i < n; ++i) {
      assign_value(ptr, value);
      increment(ptr);
      increment(last_);
      size_++;
    }
  }

  template <class InputIterator>
  void assign_impl(InputIterator begin_pos, InputIterator end_pos) {
    // Exception if end_pos - begin_pos > capacity()
    pointer p = begin().it_;
    last_ = p;
    for (InputIterator it = begin_pos; it != end_pos; ++it) {
      assign_value(p, *it);
      increment(p);
      increment(last_);
    }
  }

  void assign(std::initializer_list<T> list) {
    if (list.size() > capacity()) {
      throw std::runtime_error("Overflow\n");
    }
    size_ = 0;
    pointer ptr = begin().it_;
    last_ = ptr;
    for (auto it = list.begin(); it != list.end(); ++it) {
      assign_value(ptr, *it);
      increment(ptr);
      increment(last_);
      size_++;
    }
  }

  void push_front(const value_type& value) {
    if (is_full()) {
      decrement(first_);
      assign_value(first_, value);
      last_ = first_;
    } else {
      decrement(first_);
      alloc_traits::construct(alloc_, first_, value);
      size_++;
    }
  }

  void push_back(const value_type& value) {
    if (is_full()) {
      assign_value(last_, value);
      increment(last_);
      first_ = last_;
    } else {
      alloc_traits::construct(alloc_, last_, value);
      increment(last_);
      ++size_;
    }
  }

  iterator erase(const_iterator range_begin, const_iterator range_end) {
    if (range_begin.it_ == nullptr) {
      throw std::runtime_error("Non-valid iterator");
    }
    // Check range_begin <= range_end
    // auto it = range_begin;

    /*
    В начало запишем все которые нужны
    Удалим те, которые не нужны
    После удаляемого запишем все неудаляемые
    */
    // pointer p = range_begin.it_;
    // while(it != nullptr) {
    //   assign_value(p, *(range_end));
    //   increment(range_end);
    // }
    return iterator();
  }

  iterator erase(const_iterator deleted_pos) {
    if (deleted_pos.it_ == nullptr) {
      throw std::runtime_error("Non-valid iterator");
    }
    pointer next = deleted_pos.it_;
    increment(next);
    for (pointer it = deleted_pos.it_; next != last_;
         increment(it), increment(next)) {
      assign_value(it, *next);
    }
    decrement(last_);
    size_--;
    if (last_ != deleted_pos.it_) {
      return iterator(this, deleted_pos.it_);
    }
    return end();
  }

  void pop_front() {
    if (size_ == 0) {
      throw std::runtime_error("pop_front on empty buffer");
    }
    increment(first_);
    size_--;
  }

  void pop_back() {
    if (size_ == 0) {
      throw std::runtime_error("pop_back on empty buffer");
    }
    decrement(last_);
    size_--;
  }

  void decrement(pointer& p) const {
    if (p == buff_) p = buff_end_;
    --p;
  }

  reference operator[](size_type index) {
    /*
    Если расстояние до нужного элемента считая от first_ меньше чем расстояние
    до конца : возвращаем просто сдвиг на index от начала
    Иначе происходит wrap_around с самого начала
    */
    if (index < buff_end_ - first_) {
      return *(first_ + index);
    } else {
      return *(first_ + index - capacity());
    }
  }

  const_reference operator[](size_type index) const {
    if (index < buff_end_ - first_) {
      return first_ + index;
    } else {
      return first_ + index - capacity();
    }
  }

  void increment(pointer& p) const {
    ++p;
    if (p == buff_end_) {
      p = buff_;
    }
  }

  ~circular_buffer() {
    for (size_type i = 0; i < size_; ++i) {
      alloc_traits::destroy(alloc_, buff_ + i);
    }
    // alloc_traits::deallocate(alloc_, buff_, capacity());
  }

  reference at(size_type index) {}

  const_reference at(size_type index) const {}

  size_type capacity() const { return buff_end_ - buff_; }

  void assign_value(pointer p, const value_type& value) { *p = value; }

  bool is_full() { return size_ == capacity(); }

  iterator begin() { return iterator(this, first_); }

  const_iterator begin() const { return const_iterator(this, first_); }

  iterator end() { return iterator(this, nullptr); }

  const_iterator end() const { return const_iterator(this, nullptr); }

  const_iterator cbegin() const { return const_iterator(this, first_); }

  const_iterator cend() const { return const_iterator(this, nullptr); }

  bool operator==(const circular_buffer& other) { return true; }

  bool operator!=(const circular_buffer& other) { return !(*this == other); }

  size_type size() const { return size_; }

  size_type max_size() const { return capacity(); }

  bool empty() const { return size_ == 0; }

  reference front() { return *first_; }

  const_reference front() const { return *first_; }

  reference back() {
    pointer ptr = last_;
    decrement(ptr);
    return *ptr;
  }

  const_reference back() const {
    pointer ptr = last_;
    decrement(ptr);
    return *ptr;
  }

  void swap(circular_buffer& other) {}

  friend void swap(circular_buffer& lhs, circular_buffer& rhs) {
    lhs.swap(rhs);
  }

 public:
  pointer buff_;
  pointer buff_end_;
  pointer first_;
  pointer last_;
  size_type size_;
  allocator_type alloc_;
};