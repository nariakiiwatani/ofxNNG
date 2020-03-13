#pragma once

namespace ofxNNG {
	namespace detail {
		template<bool cond> struct cond_type;
		template<> struct cond_type<true> : std::true_type{};
		template<> struct cond_type<false> : std::false_type{};
		
		template<typename T>
		struct size_of { static constexpr std::size_t value = sizeof(T); };
		template<typename T, std::size_t size>
		struct is_size : cond_type<size_of<T>::value==size>{};
		
		template<typename T>
		struct is_trivially_copyable {
			static constexpr bool value = std::is_trivially_copyable<typename std::decay<T>::type>::value;
		};
	}
	
	template<typename T, typename SFINAE=void>
	struct is_safe_to_use_memcpy {
		static constexpr bool value = detail::is_size<T,1>::value;
	};
	
	namespace detail {
		template<typename T, typename std::enable_if<std::is_array<T>::value>::type* = nullptr>
		using type_of_value_of_array = decltype(*declval<typename std::decay<T>::type>());
	};
	template<typename T>
	struct is_safe_to_use_memcpy<T, typename std::enable_if<std::is_array<T>::value>::type> {
		static constexpr bool value = is_safe_to_use_memcpy<detail::type_of_value_of_array<T>>::value;
	};

	template<typename T>
	struct should_use_memcpy {
		static constexpr bool value = detail::is_trivially_copyable<T>::value && is_safe_to_use_memcpy<T>::value;
	};
}
