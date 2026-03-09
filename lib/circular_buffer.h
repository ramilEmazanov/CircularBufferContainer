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

    template <bool B = IsConst, typename = std::enable_if_t<B>>
    base_iterator(const base_iterator<false>& other)
        : buff_(other.buff_), it_(other.it_) {}

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
      if (n == 0 || it_ == nullptr) {
        return *this;
      }

      if (n > 0) {
        difference_type index = buff_->logic_index(it_);
        if (index + n > buff_->size_) {
          throw std::runtime_error("Out of range\n");
        }
        it_ = buff_->pointer_addition(it_, n);
        if (it_ == buff_->last_) {
          it_ = nullptr;
        }
        return *this;
      }

      *this -= -n;
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
      return buff_->logic_index(it_) - buff_->logic_index(other.it_);
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

  circular_buffer(const Allocator& alloc)
      : alloc_(alloc),
        buff_(nullptr),
        buff_end_(nullptr),
        first_(nullptr),
        last_(nullptr),
        size_(0) {}

  circular_buffer(size_type capacity,
                  const allocator_type& alloc = allocator_type())
      : alloc_(alloc),
        buff_(alloc_traits::allocate(alloc_, capacity)),
        buff_end_(buff_ + capacity),
        first_(buff_),
        last_(buff_),
        size_(0) {}

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

  iterator insert(const_iterator add, std::initializer_list<T> list) {
    size_type new_capacity = capacity() * 2;
    pointer new_buff = alloc_traits::allocate(alloc_, new_capacity);
    size_type i = 0;
    const_iterator it = begin();

    for (it; it != add; ++it) {
      alloc_traits::construct(alloc_, new_buff + i, *it);
      ++i;
    }
    for (auto init_it = list.begin(); init_it != list.end(); ++init_it) {
      alloc_traits::construct(alloc_, new_buff + i, *init_it);
      i++;
    }
    i--;
    while (it != end()) {
      i++;
      alloc_traits::construct(alloc_, new_buff + i, *it);
      it++;
    }

    for (i = 0; i < size_; ++i) {
      alloc_traits::destroy(alloc_, buff_ + i);
    }
    alloc_traits::deallocate(alloc_, buff_, capacity());

    size_type new_size = size_ + list.size();

    buff_ = new_buff;
    buff_end_ = buff_ + new_capacity;
    first_ = buff_;
    size_ = new_size;
    if (size_ == new_capacity) {
      last_ = first_;
    } else {
      last_ = buff_ + size_;
    }

    return iterator();
  }

  iterator insert(const_iterator add, const value_type& value) {
    size_type new_capacity = capacity() * 2;
    pointer new_buff = alloc_traits::allocate(alloc_, new_capacity);
    size_type i = 0;
    const_iterator it = begin();

    for (it = begin(); it != add; ++it) {
      alloc_traits::construct(alloc_, new_buff + i, *it);
      ++i;
    }

    alloc_traits::construct(alloc_, new_buff + i, value);
    while (it != end()) {
      i++;
      alloc_traits::construct(alloc_, new_buff + i, *it);
      it++;
    }

    for (i = 0; i < size_; ++i) {
      alloc_traits::destroy(alloc_, buff_ + i);
    }
    alloc_traits::deallocate(alloc_, buff_, capacity());

    size_type new_size = size_ + 1;

    buff_ = new_buff;
    buff_end_ = buff_ + new_capacity;
    first_ = buff_;
    size_ = new_size;
    if (size_ == new_capacity) {
      last_ = first_;
    } else {
      last_ = buff_ + size_;
    }

    return iterator();
  }

  iterator insert(const_iterator add, size_type count,
                  const value_type& value) {
    size_type new_capacity = capacity() * 2;
    pointer new_buff = alloc_traits::allocate(alloc_, new_capacity);
    size_type i = 0;
    const_iterator it = begin();

    for (it; it != add; ++it) {
      alloc_traits::construct(alloc_, new_buff + i, *it);
      ++i;
    }

    for (int j = 0; j < count; ++j) {
      alloc_traits::construct(alloc_, new_buff + i, value);
      ++i;
    }

    while (it != end()) {
      alloc_traits::construct(alloc_, new_buff + i, *it);
      i++;
      it++;
    }

    for (i = 0; i < size_; ++i) {
      alloc_traits::destroy(alloc_, buff_ + i);
    }
    alloc_traits::deallocate(alloc_, buff_, capacity());

    size_type new_size = size_ + count;

    buff_ = new_buff;
    buff_end_ = buff_ + new_capacity;
    first_ = buff_;
    size_ = new_size;
    if (size_ == new_capacity) {
      last_ = first_;
    } else {
      last_ = buff_ + size_;
    }

    return iterator();
  }

  // SeqContainer requirement. Inserts copies of elements in [i, j) before p.
  iterator insert(const_iterator before, iterator range_begin,
                  iterator range_end) {}

  template <class InputIt, typename std::iterator_traits<
                               InputIt>::iterator_category* = nullptr>
  iterator insert(const_iterator before, InputIt range_begin,
                  InputIt range_end) {
    size_type count = range_end - range_begin;
    size_type new_capacity = 2 * capacity();
    pointer new_buff = alloc_traits::allocate(alloc_, new_capacity);
    size_type i = 0;
    const_iterator it = begin();

    for (it; it != before; ++it) {
      alloc_traits::construct(alloc_, new_buff + i, *it);
      ++i;
    }

    for (range_begin; range_begin != range_end; ++range_begin) {
      alloc_traits::construct(alloc_, new_buff + i, *range_begin);
      ++i;
    }

    while (it != end()) {
      alloc_traits::construct(alloc_, new_buff + i, *it);
      i++;
      it++;
    }

    for (i = 0; i < size_; ++i) {
      alloc_traits::destroy(alloc_, buff_ + i);
    }
    alloc_traits::deallocate(alloc_, buff_, capacity());

    size_type new_size = size_ + count;

    buff_ = new_buff;
    buff_end_ = buff_ + new_capacity;
    first_ = buff_;
    size_ = new_size;
    if (size_ == new_capacity) {
      last_ = first_;
    } else {
      last_ = buff_ + size_;
    }

    return iterator();
  }

  // Inserts
  iterator insert_range() {}

  void clear() {}

  void resize(size_type new_capacity) {
    /*
    1. new_capacity > capacity()
    2. new_capacity == capacity()
    3. new_capacity < capacity()
    Only for shrinking?
     */
    if (new_capacity == capacity()) {
      return;
    }
    if (new_capacity < capacity()) {
      // 1. size_ > new_capacity
      // 2. size_ < new_capacity
      // 3. size_ == new_capacity
      pointer new_buff = alloc_traits::allocate(alloc_, new_capacity);
      auto it = begin();
      for (size_type i = 0; i < new_capacity; ++i) {
        alloc_traits::construct(alloc_, new_buff + i, *it);
        ++it;
      }
      for (int i = 0; i < size_; ++i) {
        alloc_traits::destroy(alloc_, buff_ + i);
      }
      alloc_traits::deallocate(alloc_, buff_, capacity());
      buff_ = new_buff;
      buff_end_ = buff_ + new_capacity;
      first_ = buff_;
      // last_ = ?????;
      size_ = new_capacity;
    } else {
      // new_capacity > capacity()
      // move to a longer buff_
      // copy elements from beginning
      // the same as with resize(new_capacity, value)
    }
  }

  void resize(size_type new_capacity, const value_type& value) {
    /*
    1. new_capacity > capacity() - done
    2. new_capacity == capacity() - done
    3. new_capacity < capacity()
    4. new_capacity > size_ > capacity
    5. new_capacity == size_ == capacity()
    6. new_capacity < size_ <= capacity()

    1. Buffer is full. new_capacity > capacity()
    2. Buffer is not full. new_capacity > capacity()
    */
    if (new_capacity <= capacity()) return;

    pointer new_buff = alloc_traits::allocate(alloc_, new_capacity);
    size_type i = 0;
    for (auto it = begin(); it != end(); ++it) {
      alloc_traits::construct(alloc_, new_buff + i, *it);
      ++i;
    }

    for (i = 0; i < size_; ++i) {
      alloc_traits::destroy(alloc_, buff_ + i);
    }
    alloc_traits::deallocate(alloc_, buff_, capacity());

    size_type old_size = size_;

    buff_ = new_buff;
    buff_end_ = buff_ + new_capacity;

    first_ = buff_;
    last_ = buff_ + old_size;
    for (i = 0; i < new_capacity - old_size; ++i) {
      push_back(value);
    }
    size_ = new_capacity;
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

  difference_type logic_index(pointer ptr) const {
    if (ptr == nullptr) {
      return size_;
    }
    if (ptr >= first_) {
      return ptr - first_;
    } else {
      return (ptr - buff_) + (buff_end_ - first_);
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
    if (Extendable && is_full()) {
      insert(begin(), value);
      return;
    }
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
    if (Extendable && is_full()) {
      insert(end(), value);
      return;
    }
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
    difference_type begin_index = logic_index(range_begin.it_);
    difference_type end_index = logic_index(range_end.it_);

    if (begin_index == end_index) {
      if (range_begin.it_ == nullptr) {
        return end();
      }
      return iterator(this, range_begin.it_);
    }
    size_type count = static_cast<size_type>(end_index - begin_index);

    pointer to = range_begin.it_;
    pointer from = range_end.it_;
    while (from != last_) {
      assign_value(to, *from);
      increment(to);
      increment(from);
    }

    for (size_type i = 0; i < count; ++i) {
      decrement(last_);
    }
    size_ -= count;

    return iterator(this, range_begin.it_);
  }

  iterator erase(const_iterator deleted_pos) {
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
    if (index < buff_end_ - first_) {
      return *(first_ + index);
    } else {
      return *(first_ + index - capacity());
    }
  }

  const_reference operator[](size_type index) const {
    if (index < buff_end_ - first_) {
      return *(first_ + index);
    } else {
      return *(first_ + index - capacity());
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
    alloc_traits::deallocate(alloc_, buff_, capacity());
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
  allocator_type alloc_;
  pointer buff_;
  pointer buff_end_;
  pointer first_;
  pointer last_;
  size_type size_;
};