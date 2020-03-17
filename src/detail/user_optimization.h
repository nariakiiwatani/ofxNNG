#pragma once

#include <type_traits>

namespace ofxNNG {
	template<typename T, typename SFINAE=void> struct is_safe_to_use_memcpy : std::false_type{};
	template<typename T> struct is_safe_to_use_memcpy<T, typename std::enable_if<T::is_safe_to_use_memcpy>::type> : std::true_type{};
}

#define OFX_NNG_NOTIFY_TO_USE_MEMCPY(Type) template<> struct ofxNNG::is_safe_to_use_memcpy<Type> : std::true_type{}
#define OFX_NNG_NOTIFY_TO_USE_MEMCPY_MEMBER static const bool is_safe_to_use_memcpy=true;
