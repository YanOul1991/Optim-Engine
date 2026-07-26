// OptString.h

#pragma once

#include "Core/CoreMinimal.h"

class CORE_API String final
{
 public:
  static String SPrintf(const char* str, ...);
  static int32  GetStringLiteralSize(const char* str);

  String();
  ~String();

  String(const char* other);
  String(const String& other);
  String(String&& other);

  int32 GetLength() const;

  int32 GetCapacity() const;

  const char* GetPointer() const;

  bool IsNullEmpty() const;

  void Reserve(int32 capacity);

  void Clear();

  String& Append(const char* other);
  String& Append(const String& other);
  String& Append(String&& other);

  String& operator=(const char* other);
  String& operator=(const String& other);
  String& operator=(String&& other);
  
  String& operator+=(const char* other);
  String& operator+=(const String& other);
  String& operator+=(String&& other);

  bool operator==(const String& other) const;
  bool operator!=(const String& other) const;

 private:
  char* m_buffer;
  int32 m_count;
  int32 m_capacity;
};