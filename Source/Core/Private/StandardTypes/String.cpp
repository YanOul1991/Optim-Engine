// OptString.cpp

#include "Core/StandardTypes/String.h"

#include "Core/Memory/Allocation.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

String String::SPrintf(const char* format, ...) {
  va_list args;
  va_start(args, format);
  // Make copy of args get needed buffer
  // size to hold output.
  va_list argsCpy;
  va_copy(argsCpy, args);
  int needed = ::vsnprintf(nullptr, 0, format, argsCpy);
  va_end(argsCpy);
  // In case of formatting error
  // return empty string
  if (needed < 0) {
    va_end(args);
    return String();
  }
  // Allocate buffer
  // String class automatically adds null terminator
  std::vector<char> buffer(needed + 1);
  ::vsnprintf(buffer.data(), buffer.size(), format, args);
  va_end(args);
  return String(buffer.data());
}

int32 String::GetStringLiteralSize(const char* str) {
  return static_cast<int32>(strnlen(str, 2048));
}

String::String() {
  m_capacity  = 64; // Default capacity of 64 characters
  m_buffer    = Memory::Alloc<char>(m_capacity);
  m_buffer[0] = '\0';
}

String::~String() {
  Memory::Dealloc<char>(m_buffer, m_count, m_capacity);
  m_buffer = nullptr;
}

String::String(const char* other) {
  int32 otherLength = String::GetStringLiteralSize(other);
  m_count           = otherLength;
  m_capacity        = static_cast<int32>(1.5f * otherLength);
  m_buffer          = Memory::Alloc<char>(m_capacity);

  // Copy other string buffer including the null terminator
  memcpy(m_buffer, other, sizeof(char) * (otherLength + 1));
}

String::String(const String& other) {
  m_capacity = other.m_capacity;
  m_count    = other.m_count;
  m_buffer   = Memory::Alloc<char>(m_capacity);
  memcpy(m_buffer, other.m_buffer, sizeof(char) * m_capacity);
}

String::String(String&& other) {
  m_capacity = other.m_capacity;
  m_count    = other.m_count;
  m_buffer   = other.m_buffer;

  other.m_capacity = 0;
  other.m_count    = 0;
  other.m_buffer   = nullptr;
}

int32 String::GetLength() const {
  return m_count;
}

int32 String::GetCapacity() const {
  return m_capacity;
}

const char* String::GetPointer() const {
  return m_buffer;
}

bool String::IsNullEmpty() const {
  return m_buffer[0] == '\0' || m_count == 0;
};

void String::Reserve(int32 _newCapacity) {
  // If the desired capacity is the same
  // or lower than the current one, then do nothing.
  if (m_capacity >= _newCapacity) {
    return;
  }

  // Allocate new buffer with new capacity
  char* __newBuffer = Memory::Alloc<char>(_newCapacity);

  // Copy from current buffer into new buffer count + 1
  // to include all set chars and null terminator.
  memcpy(__newBuffer, m_buffer, m_count + 1);

  // Free current buffer
  Memory::Dealloc<char>(m_buffer, m_count, m_capacity);

  m_buffer   = __newBuffer;
  m_capacity = _newCapacity;
}

void String::Clear() {
  m_count     = 0;
  m_buffer[0] = '\0';
}

String& String::Append(const char* other) {

  if (other) {
    int32 _otherSize = GetStringLiteralSize(other);

    // If the toal character count will overflow
    // the current capacity, then resize the buffer.
    // Add an extra 3 characters to the combined count.
    if ((m_count + _otherSize + 3) > m_capacity) {
      // Reserve extra space.
      // ! This already updates the m_capacity field
      Reserve(m_capacity + (_otherSize * 2));
    }

    // Append the new string and update count
    memcpy(m_buffer + m_count, other, _otherSize + 1);
    m_count += _otherSize;
  }

  return *this;
}

String& String::Append(const String& other) {

  if ((m_count + other.m_count + 3) > m_capacity) {
    Reserve(static_cast<uint64>(1.5 * m_capacity) + (other.m_count));
  }

  memcpy(m_buffer + m_count, other.m_buffer, other.m_count);
  m_count              += other.m_count;
  m_buffer[m_count + 1] = '\0';

  return *this;
}

String& String::Append(String&& other) {

  if ((m_count + other.m_count + 3) > m_capacity) {
    Reserve(static_cast<uint64>(1.5 * m_capacity) + (other.m_count));
  }

  memcpy(m_buffer + m_count, other.m_buffer, other.m_count);

  m_count += other.m_count;

  m_buffer[m_count + 1] = '\0';

  other.m_count    = 0;
  other.m_capacity = 0;
  other.m_buffer   = nullptr;

  return *this;
}

String& String::operator=(const char* other) {
  const int32 _otherSize = GetStringLiteralSize(other);

  if (_otherSize >= m_capacity) {
    Reserve(static_cast<uint64>(1.5 * _otherSize));
  }

  memcpy(m_buffer, other, _otherSize);
  m_count               = _otherSize;
  m_buffer[m_count + 1] = '\0';

  return *this;
}

String& String::operator=(const String& other) {

  if (this != &other) {
    if (other.m_count >= m_capacity) {
      Reserve(static_cast<uint64>(1.5 * other.m_count));
    }

    Memory::Memcpy<char>(m_buffer, other.m_buffer, other.m_count);

    m_count               = other.m_count;
    m_buffer[m_count + 1] = '\0';
  }

  return *this;
}

String& String::operator=(String&& other) {

  if (this != &other) {
    m_buffer   = other.m_buffer;
    m_count    = other.m_count;
    m_capacity = other.m_capacity;

    other.m_count    = 0;
    other.m_capacity = 0;
    other.m_buffer   = nullptr;
  }

  return *this;
}

String& String::operator+=(const char* other) {
  return Append(other);
}

String& String::operator+=(const String& other) {
  return Append(other);
}

String& String::operator+=(String&& other) {
  return Append(other);
}

bool String::operator==(const String& other) const {
  if (&other == this) {
    return true;
  }

  if (m_count != other.m_count) {
    return false;
  }

  if (m_count == 0 && other.m_count == 0) {
    return true;
  }

  return memcmp(m_buffer, other.m_buffer, m_count) == 0;
}

bool String::operator!=(const String& other) const {
  return !operator==(other);
}