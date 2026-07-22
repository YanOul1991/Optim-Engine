/**
 * FILE: 
 * Concepts.h
 * 
 * DESC:
 * This file contains concepts to be used for compile time 
 * optimizations (eg. if constexpr(...) statements). 
 * Concepts are also to be used for template intrinsics and compile time
 * assertions.
 */

#pragma once

#include <concepts>
#include <type_traits>

namespace Concepts {

template <typename T>
concept TrivialDestruct = std::is_trivially_destructible_v<T>;

template <typename T>
concept TrivialConstruct = std::is_trivially_constructible_v<T>;

template <typename T>
concept TrivialCopy = std::is_trivially_copyable_v<T>;

template <typename T>
concept IsClassObject = std::is_class_v<T>;

template <typename T>
concept IsFundamental = std::is_fundamental_v<T>;

template <typename T>
concept IsMoveAssignable = std::is_move_assignable_v<T>;

template <typename T>
concept IsMoveConstructible = std::is_move_constructible_v<T>;

template <typename T>
concept IsCopyConstructible = std::is_copy_constructible_v<T>;

template <typename T>
concept IsCopyAssignable = std::is_copy_assignable_v<T>;

}; // namespace Concepts