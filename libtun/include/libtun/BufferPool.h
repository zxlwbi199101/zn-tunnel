#ifndef LIBTUN_BUFFERPOOL_INCLUDED
#define LIBTUN_BUFFERPOOL_INCLUDED

#include <stdint.h>
#include <list>
#include <stack>
#include <mutex>
#include <boost/asio/buffer.hpp>

namespace libtun {

  using boost::asio::mutable_buffer;

  class BufferPool {
  public:
    BufferPool(uint32_t bufferSize, uint32_t buffersPerChunk = 1000):
      _bufferSize(bufferSize), _buffersPerChunk(buffersPerChunk) {}

    mutable_buffer alloc() {
      std::lock_guard guard(_locker);
      if (_available.empty()) {
        _allocChunk();
      }
      mutable_buffer buffer(_available.top(), _bufferSize);
      _available.pop();
      return buffer;
    }

    void free(void* buffer) {
      std::lock_guard guard(_locker);
      _available.push(buffer);
    }

    ~BufferPool() {
      for (auto buffer : _memoryChunks) {
        delete[] buffer;
      }
    }

  private:
    std::list<void*> _memoryChunks;
    uint32_t _bufferSize;
    uint32_t _buffersPerChunk;
    std::mutex _locker;
    std::stack<void*> _available;

    void _allocChunk() {
      std::lock_guard guard(_locker);
      void* chunk = (void*)(new uint8_t[_bufferSize * _buffersPerChunk]);
      _memoryChunks.push_back(chunk);

      for (int i = 0; i < _buffersPerChunk; i++) {
        _available.push(chunk + i * _bufferSize);
      }
    }
  };

} // namespace libtun

#endif
