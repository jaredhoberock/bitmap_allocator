#include <vector>
#include <algorithm>
#include <cstdint>

template<class T>
class constant_iterator
{
  public:
    using value_type = T;
    using reference = const value_type&;
    using pointer = const value_type*;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;

    constant_iterator() = default;

    constant_iterator(const constant_iterator&) = default;

    constant_iterator(const T& value, size_t position)
      : value_(value), position_(position)
    {}

    constant_iterator(const T& value)
      : constant_iterator(value, 0)
    {}

    // dereference
    reference operator*() const
    {
      return value_;
    }

    // subscript
    reference operator[](difference_type) const
    {
      // note that there is no need to create a temporary iterator
      // e.g. tmp = *this + n
      // because the value returned by *tmp == this->value_
      return value_;
    }

    // not equal
    bool operator!=(const constant_iterator& rhs) const
    {
      return position_ != rhs.position_;
    }

    // pre-increment
    constant_iterator& operator++()
    {
      ++position_;
      return *this;
    }

    // post-increment
    constant_iterator operator++(int)
    {
      constant_iterator result = *this;
      ++position_;
      return result;
    }

    // pre-decrement
    constant_iterator& operator--()
    {
      --position_;
      return *this;
    }

    // post-decrement
    constant_iterator operator--(int)
    {
      constant_iterator result = *this;
      --position_;
      return result;
    }

    // plus-equal
    constant_iterator& operator+=(difference_type n)
    {
      position_ += n;
      return *this;
    }

    // plus
    constant_iterator operator+(difference_type n) const
    {
      constant_iterator result = *this;
      result += n;
      return result;
    }

    // difference
    difference_type operator-(const constant_iterator& rhs)
    {
      return position_ - rhs.position_;
    }

  private:
    T value_;
    size_t position_;
};


class bitmap_memory_resource
{
  private:
    static const std::size_t block_size_in_bytes_ = sizeof(std::size_t);
    std::uint8_t* buffer_;
    std::vector<bool> free_blocks_;

  public:
    bitmap_memory_resource(void* buffer, std::size_t buffer_size_in_bytes)
      : buffer_(buffer),
        free_blocks_(buffer_size_in_bytes / block_size_in_bytes_, true)
    {}

    bitmap_memory_resource(const bitmap_memory_resource&) = delete;

    void* allocate(std::size_t num_bytes)
    {
      // find how many blocks to allocate
      std::size_t num_blocks = (num_bytes + block_size_in_bytes_ - 1) / block_size_in_bytes_;

      // search for num_blocks contiguous free blocks
      auto found = std::search(free_blocks_.begin(), free_blocks_.end(), constant_iterator<bool>(true, 0), constant_iterator<bool>(true, num_blocks));
      if(found == free_blocks_.end())
      {
        return nullptr;
      }

      // mark those blocks as allocated
      std::fill_n(found, num_blocks, false);

      // find the index of the first block allocated
      std::size_t index_of_first_free_block = found - free_blocks_.begin();

      // convert the block index to a pointer
      return buffer_ + (index_of_first_free_block * block_size_in_bytes_);
    }

    void deallocate(void* ptr, std::size_t num_bytes)
    {
      // find the index of the first block to free
      std::size_t index_of_first_byte = reinterpret_cast<std::uint8_t*>(ptr) - buffer_;
      std::size_t index_of_first_block = index_of_first_byte / block_size_in_bytes_;

      // find how many blocks to free
      std::size_t num_blocks = (num_bytes + block_size_in_bytes_ - 1) / block_size_in_bytes_;

      // mark those blocks as free
      std::fill_n(free_blocks_.begin() + index_of_first_block, num_blocks, true);
    }
};

