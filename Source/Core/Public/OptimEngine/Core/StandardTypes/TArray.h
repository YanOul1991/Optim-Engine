#pragma once

template <typename T, int N> class TArray final
{
  static_assert((N > 0));

  TArray() {
  }
  
  ~TArray() {
  }
  
 private:
  T m_buffer[N];
};