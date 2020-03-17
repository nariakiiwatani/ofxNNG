#pragma once

#include <type_traits>

namespace ofxNNG {
	template<typename T> struct is_safe_to_use_memcpy : std::false_type{};
}

#define OFX_NNG_NOTIFY_TO_USE_MEMCPY(Type) template<> struct ofxNNG::is_safe_to_use_memcpy<Type> : std::true_type{}
