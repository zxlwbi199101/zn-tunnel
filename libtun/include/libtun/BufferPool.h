#pragma once

#include <stdint.h>
#include <list>
#include <stack>
#include <mutex>
#include <boost/asio/buffer.hpp>
#include <boost/endian/conversion.hpp>

namespace libtun {

  namespace endian = boost::endian;
  using boost::asio::const_buffer;
  using boost::asio::mutable_buffer;

  class Buffer {
  public:
    Buffer() {}
    Buffer(uint8_t* data, uint32_t len):
      _data(data), _size(len), _internal(data), _internalSize(len) {}
    Buffer(const Buffer& other):
      _data(other._data), _size(other._size),
      _internal(other._internal), _internalSize(other._internalSize) {}

    // getters
    uint8_t* data() const { return _data; }
    uint32_t size() const { return _size; }
    uint8_t* internal() const { return _internal; }
    uint32_t internalSize() const { return _internalSize; }
    uint32_t prefixSpace() const { return _data - _internal; }
    uint32_t suffixSpace() const { return internalSize() - prefixSpace() - size(); }

    const_buffer toConstBuffer() const {
      return const_buffer(_data, _size);
    }

    mutable_buffer toMutableBuffer() const {
      return mutable_buffer(_data, _size);
    }

    template <class EndianNumber>
    EndianNumber readNumber(uint32_t from) const {
      return endian::big_to_native(*(EndianNumber*)(_data + from));
    }

    // setters
    Buffer& data(uint8_t* d) {
      _data = d;
      return *this;
    }

    Buffer& size(uint32_t len) {
      if (len <= internalSize() - prefixSpace()) {
        _size = len;
      }
      return *this;
    }

    Buffer& moveFrontBoundary(int distance) {
      if (-distance <= (int32_t)prefixSpace() && distance <= (int32_t)_size) {
        _data += distance;
        _size -= distance;
      }
      return *this;
    }

    Buffer& moveBackBoundary(int distance) {
      if (distance <= (int32_t)suffixSpace() && -distance <= (int32_t)_size) {
        _size += distance;
      }
      return *this;
    }

    template <class EndianNumber>
    Buffer& writeNumber(EndianNumber num, uint32_t from) {
      *((EndianNumber*)(_data + from)) = endian::native_to_big(num);
      return *this;
    }

  private:
    uint8_t* _internal;
    uint32_t _internalSize;
    uint8_t* _data;
    uint32_t _size;

  };


  template<uint32_t bufferSize = 2000>
  class BufferPool {
  public:
    BufferPool(uint32_t buffersPerChunk = 32):
      _buffersPerChunk(buffersPerChunk) {}

    Buffer alloc() {
      std::lock_guard<std::mutex> guard(_locker);
      if (_available.empty()) {
        _allocChunk();
      }
      auto data = _available.top();
      _available.pop();
      return Buffer(data, bufferSize);
    }

    void free(const Buffer& buffer) {
      std::lock_guard<std::mutex> guard(_locker);
      _available.push(buffer.internal());
    }

    uint32_t availableCount() {
      return _available.size();
    }
    uint32_t allCount() {
      return _memoryChunks.size() * _buffersPerChunk;
    }
    uint32_t consumedCount() {
      return allCount() - availableCount();
    }

    ~BufferPool() {
      std::lock_guard<std::mutex> guard(_locker);
      for (auto buffer : _memoryChunks) {
        delete[] buffer;
      }
    }

  private:
    uint32_t _buffersPerChunk;
    std::mutex _locker;
    std::list<uint8_t*> _memoryChunks;
    std::stack<uint8_t*> _available;

    uint8_t* _allocChunk() {
      auto data = new uint8_t[bufferSize * _buffersPerChunk];
      _memoryChunks.push_back(data);
      for (int i = 0; i < _buffersPerChunk; i++) {
        _available.push(data + i * bufferSize);
      }
      return data;
    }
  };

} // namespace libtun
