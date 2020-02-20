#pragma once

#include "nng.h"
#include <type_traits>
#include <string>
#include <vector>
#include "ofFileUtils.h"
#include "ofJson.h"

namespace ofxNNG {
	namespace {
		using size_type = typename Message::size_type;
	}

namespace basic_converter {
#pragma mark - ofxNNGMessage
	static inline size_type from_msg(Message &t, const Message &msg, size_type offset) {
		size_type size = *reinterpret_cast<const size_type*>((const char*)msg.data()+offset);
		t.clear();
		auto data = (const char*)msg.data();
		t.appendData(data+offset+sizeof(size_type), size);
		return sizeof(size_type)+size;
	}
	static inline void append_to_msg(Message &msg, const Message &t) {
		msg.append(t.size());
		msg.appendData(t.data(), t.size());
	}

#pragma mark - arithmetic
	template<typename T, typename Type=void>
	using enable_if_arithmetic_t = typename std::enable_if<std::is_arithmetic<typename std::remove_reference<T>::type>::value, Type>::type;
	template<typename T>
	static inline auto from_msg(T &t, const Message &msg, size_type offset)
	-> enable_if_arithmetic_t<T, size_type> {
		size_type size = sizeof(T);
		Message copy;
		copy.appendData((const char*)msg.data()+offset, size);
		switch(size) {
			case 1: t = *reinterpret_cast<T*>(nng_msg_body(copy)); nng_msg_trim(copy, size); break;
			case 2: nng_msg_trim_u16(copy, (uint16_t*)&t); break;
			case 4: nng_msg_trim_u32(copy, (uint32_t*)&t); break;
			case 8: nng_msg_trim_u64(copy, (uint64_t*)&t); break;
		}
		return size;
	}
	template<typename T>
	static inline auto append_to_msg(Message &msg, T &&t)
	-> enable_if_arithmetic_t<T, void> {
		size_type size = sizeof(T);
		switch(size) {
			case 1: nng_msg_append(msg, &t, size); break;
			case 2: nng_msg_append_u16(msg, *reinterpret_cast<uint16_t*>(&t)); break;
			case 4: nng_msg_append_u32(msg, *reinterpret_cast<uint32_t*>(&t)); break;
			case 8: nng_msg_append_u64(msg, *reinterpret_cast<uint64_t*>(&t)); break;
		}
	}
	
#pragma mark - trivially_copyable
	template<typename T, typename Type=void>
	using enable_if_should_memcpy_t = typename std::enable_if<
		std::is_trivially_copyable<typename std::remove_reference<T>::type>::value &&
		!std::is_arithmetic<typename std::remove_reference<T>::type>::value
		, Type>::type;
	template<typename T>
	static inline auto from_msg(T &t, const Message &msg, size_type offset)
	-> enable_if_should_memcpy_t<T, size_type> {
		auto pos = offset;
		size_type size = sizeof(T);
		pos += sizeof(size_type);
		auto data = (const char*)msg.data();
		memcpy(&t, data+pos, size);
		return pos+size-offset;
	}
	template<typename T>
	static inline auto append_to_msg(Message &msg, T &&t)
	-> enable_if_should_memcpy_t<T, void> {
		msg.append(sizeof(T));
		msg.appendData(&t, sizeof(T));
	}
#pragma mark - ofBuffer
	static inline size_type from_msg(ofBuffer &t, const Message &msg, size_type offset) {
		auto pos = offset;
		size_type size;
		pos += msg.to(pos, size);
		auto data = (const char*)msg.data();
		t.set(data+pos, size);
		return pos+size-offset;
	}
	static inline void append_to_msg(Message &msg, const ofBuffer &t) {
		msg.append(t.size());
		msg.appendData(t.getData(), t.size());
	}
#pragma mark - container
	namespace {
		template<typename T, typename = void>
		struct is_container : std::false_type {};
		
		template<typename... Ts>
		struct is_container_helper {};
		
		template<typename T>
		struct is_container<T,
			typename std::conditional<
				false,
				is_container_helper<
					typename T::value_type,
					typename T::size_type,
					typename T::allocator_type,
					typename T::iterator,
					typename T::const_iterator,
					decltype(std::declval<T>().size()),
					decltype(std::declval<T>().begin()),
					decltype(std::declval<T>().end()),
					decltype(std::declval<T>().cbegin()),
					decltype(std::declval<T>().cend())
				>,
				void
			>::type
		> : public std::true_type {};

		template<typename T, typename U=void>
		using enable_if_container_t = typename std::enable_if<is_container<T>::value, U>::type;
	}
	template<typename T>
	static inline auto from_msg(T &t, const Message &msg, size_type offset) 
	-> enable_if_container_t<T, size_type> {
		auto pos = offset;
		size_type size;
		pos += msg.to(pos, size);
		for(auto i = 0; i < size; ++i) {
			typename T::value_type v;
			pos += msg.to(pos, v);
			t.insert(std::end(t), std::move(v));
		}
		return pos-offset;
	}
	template<typename T>
	static inline auto append_to_msg(Message &msg, const T &t)
	-> enable_if_container_t<T, void> {
		msg.append(t.size());
		for(auto &&val : t) {
			msg.append(const_cast<typename T::value_type&>(val));
		}
	}
#pragma mark - std::string
	static inline size_type from_msg(std::string &t, const Message &msg, size_type offset) {
		auto pos = offset;
		size_type size;
		pos += msg.to(pos, size);
		auto data = (const char*)msg.data();
		t = std::string(data+pos, size);
		return pos+size-offset;
	}
	static inline void append_to_msg(Message &msg, const std::string &t) {
		msg.append(t.size());
		msg.appendData(t.data(), t.size());
	}
#pragma mark - ofJson
	static inline size_type from_msg(ofJson &t, const Message &msg, size_type offset) {
		auto pos = offset;
		size_type size;
		pos += msg.to(pos, size);
		auto data = (const char*)msg.data();
		t = ofJson::parse(data+pos, data+pos+size);
		return pos+size-offset;
	}
	static inline void append_to_msg(Message &msg, const ofJson &t) {
		append_to_msg(msg, t.dump());
	}
#pragma mark - pair
	template<typename T, typename U>
	static inline size_type from_msg(std::pair<T,U> &t, const Message &msg, size_type offset) {
		auto pos = offset;
		pos += msg.to(pos,
					  const_cast<typename std::remove_const<T>::type&>(t.first),
					  const_cast<typename std::remove_const<U>::type&>(t.second)
					  );
		return pos-offset;
	}
	template<typename T, typename U>
	static inline void append_to_msg(Message &msg, const std::pair<T,U> &t) {
		msg.append(
				   const_cast<typename std::remove_const<T>::type&>(t.first),
				   const_cast<typename std::remove_const<U>::type&>(t.second)
				   );
	}
#pragma mark - tuple
	namespace {
		template<typename T, std::size_t ...I>
		auto msg_to_fun(size_type offset, const Message &msg, T &t, nlohmann::detail::index_sequence<I...>)
		-> decltype(msg.to(offset,const_cast<typename std::remove_cv<decltype(std::get<I>(t))>::type>(std::get<I>(t))...)) {
			return msg.to(offset, const_cast<typename std::remove_cv<decltype(std::get<I>(t))>::type>(std::get<I>(t))...);
		}
		template<typename T, std::size_t N=std::tuple_size<T>::value>
		auto msg_to(size_type offset, const Message &msg, T &t)
		-> decltype(msg_to_fun(offset, msg, t, nlohmann::detail::make_index_sequence<N>{})) {
			return msg_to_fun(offset, msg, t, nlohmann::detail::make_index_sequence<N>{});
		}
		template<typename T, std::size_t ...I>
		auto msg_append_fun(Message &msg, const T &t, nlohmann::detail::index_sequence<I...>)
		-> decltype(msg.append(std::get<I>(t)...)) {
			return msg.append(std::get<I>(t)...);
		}
		template<typename T, std::size_t N=std::tuple_size<T>::value>
		auto msg_append(Message &msg, const T &t)
		-> decltype(msg_append_fun(msg, t, nlohmann::detail::make_index_sequence<N>{})) {
			return msg_append_fun(msg, t, nlohmann::detail::make_index_sequence<N>{});
		}
	}
	template<typename ...T>
	inline size_type from_msg(std::tuple<T...> &t, const Message &msg, size_type offset) {
		auto pos = offset;
		pos += msg_to(pos, msg, t);
		return pos-offset;
	}
	template<typename ...T>
	inline void append_to_msg(Message &msg, const std::tuple<T...> &t) {
		msg_append(msg, t);
	}
#pragma mark - array
	template<typename T, size_type N>
	inline size_type from_msg(std::array<T,N> &t, const Message &msg, size_type offset) {
		auto pos = offset;
		pos += msg_to(pos, msg, t);
		return pos-offset;
	}
	template<typename T, size_type N>
	static inline void append_to_msg(Message &msg, const std::array<T,N> &t) {
		msg_append(msg, t);
	}

}
}

#include "ofxNNGMessageADLConverter.h"
