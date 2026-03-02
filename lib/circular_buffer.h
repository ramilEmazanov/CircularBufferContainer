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

    reference operator*() const { return *it_; }

    bool operator==(const base_iterator& other) const {
      return (it_ == other.it_ && buff_ == other.buff_);
    }
    bool operator!=(const base_iterator& other) const {
      return !(*this == other);
    }

    pointer operator->() const { return it_; }

    base_iterator& operator++() {
      buff_->increment(it_);
      if (it_ == buff_->last_) {
        it_ = nullptr;
      }
      return *this;
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

    base_iterator& operator+(difference_type value) {
      it_ += value;
      return *this;
    }

    base_iterator& operator-(difference_type value) {
      it_ -= value;
      return *this;
    }

   public:
    const circular_buffer* buff_;
    raw_pointer it_;
  };

  using iterator = base_iterator<false>;
  using const_iterator = base_iterator<true>;
  using reverse_iterator = base_iterator<false>;
  using const_reverse_iterator = base_iterator<true>;

  // circular_buffer(size_t capacity) : capacity()(capacity) {
  //   // buffer_ = alloc_.allocate(capacity);
  //   // reinterpret_cast<T*>(new char[sizeof(value_type) * capacity]);
  // }

  circular_buffer(size_t capacity) {
    buff_ = reinterpret_cast<T*>(new char[sizeof(value_type) * capacity]);;
    buff_end_ = buff_ + capacity;
    first_ = buff_;
    last_ = buff_;
    size_ = 0;
  }

  circular_buffer(const allocator_type& alloc) : alloc_(alloc) {}

  circular_buffer(circular_buffer&& other, const allocator_type& alloc)
      : alloc_(alloc) {}

  circular_buffer(const circular_buffer& other) {}

  circular_buffer(size_type size, value_type value) {}

  circular_buffer(iterator begin, iterator end) {}

  circular_buffer() {}

  circular_buffer(std::initializer_list<T> list)
      : circular_buffer(list.size()) {
    for (typename std::initializer_list<T>::iterator it = list.begin();
         it != list.end(); ++it) {
      push_back(*it);
    }
  }

  allocator_type get_allocator() { return alloc_; }

  // C(std::from_range, rg)

  // C(il) - initializer list constructor

  // v = il - assignment operator from initializer_list

  // iterator emplace(p, args) {}

  reverse_iterator rbegin() { return base_iterator(); }

  reverse_iterator rend() { return base_iterator(); }

  const_reverse_iterator rbegin() const { return base_iterator(); }

  const_reverse_iterator rend() const { return base_iterator(); }

  const_reverse_iterator crbegin() const { return base_iterator(); }

  const_reverse_iterator crend() const { return base_iterator(); }

  iterator insert(const_iterator it, value_type value) {}

  iterator insert(iterator it, value_type value) {}

  iterator insert(iterator it, std::initializer_list<T> list) {}

  // iterator insert(const_iterator it, rvalue) {}

  iterator insert(const_iterator it, size_type count, value_type value) {}

  iterator insert(const_iterator before, iterator range_begin,
                  iterator range_end) {}

  template <class InputIt>
  iterator insert(iterator before, InputIt range_begin, InputIt range_end) {}

  // iterator insert(const_iterator it, std::initializer_list)

  void clear() {}

  void resize(size_type new_size) {}

  void resize(size_type new_size, value_type value) {}

  template <class U, class Q>
  void assign(U first_arg, Q second_arg) {
    if constexpr (std::is_integral_v<U>) {
      assign_impl_integer(first_arg, second_arg);
    } else {
      assign_impl(first_arg, second_arg);
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

  template<class InputIterator>
  void assign_impl(InputIterator begin_pos, InputIterator end_pos) {
    // Exception if end_pos - begin_pos > capacity()
    pointer p = begin().it_;
    last_ = p;
    for(InputIterator it = begin_pos; it != end_pos; ++it) {
      assign_value(p,*it);
      increment(p);
      increment(last_);
    }
  }

  void assign(std::initializer_list<T> list) {
    if(list.size() > capacity()) {
      throw std::runtime_error("Overflow\n");
    }
    size_ = 0;
    pointer ptr = begin().it_;
    last_ = ptr;
    for(auto it = list.begin(); it != list.end(); ++it) { 
      assign_value(ptr, *it);
      increment(ptr);
      increment(last_);
      size_++;
    }
  }

  reference front() {}

  const_reference front() const {}

  reference back() {}

  const_reference back() const {}

  void push_front(value_type value) {
    decrement(first_);
    assign_value(first_, value);
    if (is_full()) {
      last_ = first_;
    } else {
      size_++;
    }
  }

  void push_back(value_type value) {
    assign_value(last_, value);
    increment(last_);
    if (is_full()) {
      first_ = last_;
    } else {
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

  void assign_value(pointer p, value_type value) { *p = value; }

  bool is_full() { return size_ == capacity(); }

  size_type capacity() { return buff_end_ - buff_; }

  void increment(pointer& p) const {
    ++p;
    if (p == buff_end_) {
      p = buff_;
    }
  }

  reference at(size_type index) {}

  const_reference at(size_type index) const {}

  circular_buffer& operator=(const circular_buffer& other) {}

  ~circular_buffer() {
    // alloc_.deallocate(buff_, capacity());
    for(int i = 0; i < size_; ++i) {
      (buff_ + i)->~T();
    }
    delete[] reinterpret_cast<char*>(buff_);
  }

  iterator begin() { return iterator(this, first_); }

  const_iterator begin() const { return const_iterator(this, first_); }

  iterator end() { return iterator(this, nullptr); }

  const_iterator end() const { return const_iterator(this, nullptr); }

  const_iterator cbegin() const { return const_iterator(this, first_); }

  const_iterator cend() const { return const_iterator(this, nullptr); }

  bool operator==(const circular_buffer& other) { return true; }

  bool operator!=(const circular_buffer& other) { return !(*this == other); }

  size_type size() { return size_; }

  size_type max_size() { return capacity(); }

  bool empty() { return size_ == 0; }

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
