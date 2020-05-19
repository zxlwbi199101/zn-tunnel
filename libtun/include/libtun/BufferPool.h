#ifndef LIBTUN_BUFFER_POOL_INCLUDED
#define LIBTUN_BUFFER_POOL_INCLUDED

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

    // mutates
    uint32_t size(uint32_t len) {
      if (len <= internalSize() - prefixSpace()) {
        _size = len;
      }
      return _size;
    }

    uint32_t shift(int32_t distance) {
      if (distance <= (int32_t)suffixSpace() && -distance <= (int32_t)prefixSpace()) {
        _data += distance;
      }
      return _size;
    }

    uint32_t moveFrontBoundary(int distance) {
      if (-distance <= (int32_t)prefixSpace() && distance <= (int32_t)_size) {
        _data += distance;
        _size -= distance;
      }
      return _size;
    }

    uint32_t moveBackBoundary(int distance) {
      if (distance <= (int32_t)suffixSpace() && -distance <= (int32_t)_size) {
        _size += distance;
      }
      return _size;
    }

    // read content
    mutable_buffer readBuffer(uint32_t from) const {
      if (size() < from + 2) {
        return mutable_buffer();
      }

      uint16_t len = endian::big_to_native(*((uint16_t*)(_data + from)));
      if (size() < from + 2 + len) {
        return mutable_buffer();
      }
      return mutable_buffer(_data + from + 2, len);
    }
    mutable_buffer readBufferFromFront() {
      auto buf = readBuffer(0);
      if (buf.size() > 0) {
        moveFrontBoundary(2 + buf.size());
      }
      return buf;
    }

    std::string readString(uint32_t from) const {
      auto buf = readBuffer(from);
      return std::string((const char*)buf.data(), buf.size());
    }
    std::string readStringFromFront() {
      auto buf = readBufferFromFront();
      return std::string((const char*)buf.data(), buf.size());
    }
    template <class EndianNumber>
    EndianNumber readNumber(uint32_t from) const {
      return endian::big_to_native(*(EndianNumber)(_data + from));
    }

    // write content
    bool writeBuffer(const void* buf, uint16_t len, uint32_t from) {
      if (from + len + 2 >= size() + suffixSpace()) {
        return false;
      }
      if (from + len + 2 > size()) {
        size(from + len + 2);
      }
      *((uint16_t*)(_data + from)) = endian::native_to_big(len);
      std::memcpy(_data + from + 2, buf, len);
      return true;
    }
    bool writeBufferToBack(const void* buf, uint16_t len) {
      return writeBuffer(buf, len, size());
    }

    bool writeString(const std::string& str, uint32_t from) {
      return writeBuffer(str.data(), str.size(), from);
    }
    bool writeStringToBack(const std::string& str) {
      return writeBufferToBack(str.data(), str.size());
    }
    template <class EndianNumber>
    EndianNumber writeNumber(EndianNumber num, uint32_t from) {
      *((EndianNumer*)(_data + from)) = endian::native_to_big(num);
      return true;
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

#endif
