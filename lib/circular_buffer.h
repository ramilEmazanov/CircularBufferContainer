#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>

template <typename T, bool Extendable = false,
          typename Allocator = std::allocator<T>>
class circular_buffer {
 public:
  using value_type = T;
  using reference = T&;
  using const_reference = const T&;
  using size_type = std::size_t;
  using allocator_type = Allocator;
  using difference_type = std::ptrdiff_t;

  template <bool IsConst>
  class base_iterator {
   public:
    using value_type = T;
    using pointer = std::conditional_t<IsConst, const T*, T*>;
    using reference = std::conditional_t<IsConst, const T&, T&>;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;

    base_iterator(size_type capacity, size_type head, size_type size,
                  pointer ptr, size_type index)
        : cap_(capacity),
          head_(head),
          size_(size),
          begin_(ptr),
          index_(index) {}

    base_iterator() = default;
    base_iterator(const base_iterator& other) = default;
    base_iterator& operator=(const base_iterator& other) = default;
    ~base_iterator() = default;

    reference operator*() const { return begin_[(head_ + index_) % cap_]; }

    bool operator==(const base_iterator& other) const {
      return (index_ == other.index_ && begin_ == other.begin_);
    }
    bool operator!=(const base_iterator& other) const {
      return !(*this == other);
    }

    pointer operator->() const { return begin_ + index_; }

    base_iterator& operator++() {
      index_++;
      return *this;
    }
    base_iterator operator++(int) {
      base_iterator copy = *this;
      ++(*this);
      return copy;
    }

    base_iterator& operator--() {
      index_--;
      return *this;
    }
    base_iterator operator--(int) {
      base_iterator copy = *this;
      --(*this);
      return copy;
    }

    base_iterator& operator+(difference_type value) {
      index_ += value;
      return *this;
    }

    base_iterator& operator-(difference_type value) {
      index_ -= value;

      return *this;
    }

   private:
    // Swappable
    size_type cap_ = 0;
    size_type head_ = 0;
    size_type size_ = 0;
    pointer begin_ = nullptr;
    size_type index_ = 0;
  };

  using iterator = base_iterator<false>;
  using const_iterator = base_iterator<true>;
  using reverse_iterator = base_iterator<false>;
  using const_reverse_iterator = base_iterator<true>;

  circular_buffer(size_t capacity) : cap_(capacity) {
    buffer_ = alloc_.allocate(capacity);
    // reinterpret_cast<T*>(new char[sizeof(value_type) * capacity]);
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
      this->push_back(*it);
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

  // O(N)
  iterator erase(const_iterator it) {
    // But what if head wasn't in 0?
    // Different cases with head_
    const_iterator forward = this->cbegin();
    while (forward != this->cend()) {
      if (forward != it) {
        this->push_back(*it);
      }
    }
    size_--;
    return iterator();
  }

  iterator erase(const_iterator range_begin, const_iterator range_end) {}

  void clear() {}

  void resize(size_type new_size) {}

  void resize(size_type new_size, value_type value) {}

  template <typename InputIterator>
  void assign(InputIterator begin, InputIterator end) {}

  void assign(size_type n, value_type value) {}

  void assign(std::initializer_list<T> list) {}

  reference front() {}

  const_reference front() const {}

  reference back() {}

  const_reference back() const {}
  //   or const value_type ref?
  // new(newarr + index) T(arr_[index]);
  void push_front(value_type value) {
    head_ = (head_ - 1 + cap_) % cap_;
    new (buffer_ + head_) value_type(value);
    if (size_ != cap_) {
      size_++;
    }
  }

  void push_back(value_type value) {
    new (buffer_ + (head_ + size_) % cap_) value_type(value);
    if (size_ != cap_) {
      size_++;
    } else {
      head_++;
      head_ = head_ % cap_;
    }
  }

  void pop_front() {
    if (size_ == 0) {
      throw std::runtime_error("pop_front on empty buffer");
    }
    head_ = (head_ + 1) % cap_;
    size_--;
  }

  void pop_back() {
    if (size_ == 0) {
      throw std::runtime_error("pop_back on empty buffer");
    }
    size_--;
  }

  void printPhysical() {
    std::cout << "PrintPhysical : ";
    for (int i = 0; i < size_; ++i) {
      std::cout << buffer_[i] << ' ';
    }
    std::cout << '\n';
  }

  reference operator[](size_type index) {
    return buffer_[(head_ + index) % cap_];
  }

  const_reference operator[](size_type index) const {
    return buffer_[(head_ + index) % cap_];
  }

  reference at(size_type index) {}

  const_reference at(size_type index) const {}

  circular_buffer& operator=(const circular_buffer& other) {}

  ~circular_buffer() {
    alloc_.deallocate(buffer_, cap_);
    // for(int i = 0; i < size_; ++i) {
    //   (buffer_ + i)->~T();
    // }
    // delete[] reinterpret_cast<char*>(buffer_);
  }

  iterator begin() { return iterator(cap_, head_, size_, buffer_, 0); }

  const_iterator begin() const {
    return const_iterator(cap_, head_, size_, buffer_, 0);
  }

  iterator end() { return iterator(cap_, head_, size_, buffer_, size_); }

  const_iterator end() const {
    return const_iterator(cap_, head_, size_, buffer_, size_);
  }

  const_iterator cbegin() const {
    return const_iterator(cap_, head_, size_, buffer_, size_);
  }

  const_iterator cend() const {
    return const_iterator(cap_, head_, size_, buffer_, size_);
  }

  bool operator==(const circular_buffer& other) { return true; }

  bool operator!=(const circular_buffer& other) { return !(*this == other); }

  size_type size() { return size_; }

  size_type max_size() { return cap_; }

  bool empty() { return size_ == 0; }

  void swap(circular_buffer& other) {}

  friend void swap(circular_buffer& lhs, circular_buffer& rhs) {
    lhs.swap(rhs);
  }

 public:
  size_type cap_ = 0;
  size_type head_ = 0;
  size_type size_ = 0;
  T* buffer_ = nullptr;
  allocator_type alloc_;
};
