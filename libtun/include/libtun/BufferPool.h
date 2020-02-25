#ifndef LIBTUN_BUFFER_POOL_INCLUDED
#define LIBTUN_BUFFER_POOL_INCLUDED

#include <stdint.h>
#include <list>
#include <stack>
#include <mutex>

namespace libtun {

  class Buffer {
  public:
    Buffer() {}
    Buffer(uint8_t* data, uint32_t len):
      _data(data), _size(len), _internal(data), _internalSize(len) {}
    Buffer(const Buffer& other):
      _data(other._data), _size(other._size),
      _internal(other._internal), _internalSize(other._internalSize) {}
    Buffer(const Buffer& other, int shiftLen):
      Buffer(other) {
      shift(shiftLen);
    }

    // getters
    uint8_t* data() const { return _data; }
    uint32_t size() const { return _size; }
    uint8_t* internal() const { return _internal; }
    uint32_t internalSize() const { return _internalSize; }
    uint32_t prefixSpace() const { return _data - _internal; }
    uint32_t suffixSpace() const { return internalSize() - prefixSpace() - size(); }

    // mutates
    uint32_t size(uint32_t len) {
      if (len <= internalSize() - prefixSpace()) {
        _size = len;
      }
      return _size;
    }

    uint32_t shift(int len) {
      int diff = _data - _internal;
      if (len >= -diff && len <= (int64_t)_size) {
        _data += len;
        _size -= len;
      }
      return _size;
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
    uint32_t _bufferSize;
    uint32_t _buffersPerChunk;
    std::mutex _locker;
    std::list<uint8_t*> _memoryChunks;
    std::stack<uint8_t*> _available;

    uint8_t* _allocChunk() {
      auto data = new uint8_t[_bufferSize * _buffersPerChunk];
      _memoryChunks.push_back(data);
      for (int i = 0; i < _buffersPerChunk; i++) {
        _available.push(data + i * _bufferSize);
      }
      return data;
    }
  };

} // namespace libtun

#endif
