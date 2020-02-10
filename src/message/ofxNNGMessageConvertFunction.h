#pragma once

#include "nng.h"
#include <type_traits>
#include <string>
#include <vector>
#include "ofFileUtils.h"

namespace ofxNNG {

namespace basic_converter {
#pragma mark - ofxNNGMessage
	static inline std::size_t from_msg(Message &t, const Message &msg, std::size_t offset) {
		using size_type = decltype(t.size());
		size_type size = *reinterpret_cast<const size_type*>((const char*)msg.data()+offset);
		t.clear();
		t.append(size);
		auto data = (const char*)msg.data();
		t.appendData(data+offset+sizeof(size_type), size);
		return sizeof(size_type)+size;
	}
	static inline Message to_msg(Message &&t) {
		t.prepend(t.size());
		return std::move(t);
	}
	static inline Message to_msg(const Message &t) {
		Message ret;
		ret.append(t.size());
		ret.appendData(t.data(), t.size());
		return ret;
	}

#pragma mark - arithmetic
	template<typename T, typename Type=void>
	using enable_if_arithmetic_t = typename std::enable_if<std::is_arithmetic<typename std::remove_reference<T>::type>::value, Type>::type;
	template<typename T, std::size_t size=sizeof(T)>
	static inline auto from_msg(T &t, const Message &msg, std::size_t offset)
	-> enable_if_arithmetic_t<T, std::size_t> {
		using size_type = decltype(sizeof(T));
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
	template<typename T, std::size_t size=sizeof(T)>
	static inline auto to_msg(T &&t)
	-> enable_if_arithmetic_t<T, Message> {
		Message msg;
		switch(size) {
			case 1: nng_msg_append(msg, &t, size); break;
			case 2: nng_msg_append_u16(msg, t); break;
			case 4: nng_msg_append_u32(msg, t); break;
			case 8: nng_msg_append_u64(msg, t); break;
		}
		return msg;
	}

	
#pragma mark - trivially_copyable
	template<typename T, typename Type=void>
	using enable_if_should_memcpy_t = typename std::enable_if<
		std::is_trivially_copyable<typename std::remove_reference<T>::type>::value &&
		!std::is_arithmetic<typename std::remove_reference<T>::type>::value
		, Type>::type;
	template<typename T>
	static inline auto from_msg(T &t, const Message &msg, std::size_t offset)
	-> enable_if_should_memcpy_t<T, std::size_t> {
		using size_type = decltype(sizeof(T));
		auto pos = offset;
		size_type size = sizeof(T);
		pos += sizeof(size_type);
		auto data = (const char*)msg.data();
		memcpy(&t, data+pos, size);
		return pos+size-offset;
	}
	template<typename T>
	static inline auto to_msg(T &&t)
	-> enable_if_should_memcpy_t<T, Message> {
		Message msg;
		msg.append(sizeof(T));
		msg.appendData(&t, sizeof(T));
		return msg;
	}
#pragma mark - std::string
	static inline std::size_t from_msg(std::string &t, const Message &msg, std::size_t offset) {
		using size_type = decltype(t.size());
		auto pos = offset;
		size_type size;
		pos += from_msg(size, msg, pos);
		auto data = (const char*)msg.data();
		t = std::string(data+pos, size);
		return pos+size-offset;
	}
	static inline Message to_msg(const std::string &t) {
		Message msg;
		msg.append(t.size());
		msg.appendData(t.data(), t.size());
		return msg;
	}
#pragma mark - ofBuffer
	static inline std::size_t from_msg(ofBuffer &t, const Message &msg, std::size_t offset) {
		using size_type = decltype(t.size());
		auto pos = offset;
		size_type size;
		pos += from_msg(size, msg, pos);
		auto data = (const char*)msg.data();
		t.set(data+pos, size);
		return pos+size-offset;
	}
	static inline Message to_msg(const ofBuffer &t) {
		Message msg;
		msg.append(t.size());
		msg.appendData(t.getData(), t.size());
		return msg;
	}
#pragma mark - ofJson
	static inline std::size_t from_msg(ofJson &t, const Message &msg, std::size_t offset) {
		using size_type = std::size_t;
		auto pos = offset;
		size_type size;
		pos += from_msg(size, msg, pos);
		auto data = (const char*)msg.data();
		t = ofJson::parse(data+pos, data+pos+size);
		return pos+size-offset;
	}
	static inline Message to_msg(const ofJson &t) {
		return to_msg(t.dump());
	}
#pragma mark - vector
	template<typename T>
	static inline std::size_t from_msg(std::vector<T> &t, const Message &msg, std::size_t offset) {
		using size_type = std::size_t;
		auto pos = offset;
		size_type size;
		pos += from_msg(size, msg, pos);
		t.resize(size);
		for(auto &&val : t) {
			pos += msg.to<T>(val, pos);
		}
		return pos-offset;
	}
	template<typename T>
	static inline Message to_msg(const std::vector<T> &t) {
		using rawtype = typename std::remove_const<T>::type;
		Message msg;
		msg.append(t.size());
		for(auto &&val : t) {
			msg.append(std::forward<rawtype>(const_cast<rawtype&>(val)));
		}
		return msg;
	}
}

template<typename T>
struct adl_converter {
	template<typename V>
	static inline auto from_msg(V &v, const Message &msg, std::size_t offset)
	-> decltype(::ofxNNG::basic_converter::from_msg(v,msg,offset)) {
		return ::ofxNNG::basic_converter::from_msg(v,msg,offset);
	}
	template<typename V>
	static inline auto to_msg(V &&v)
	-> decltype(::ofxNNG::basic_converter::to_msg(std::forward<V>(v))) {
		return ::ofxNNG::basic_converter::to_msg(std::forward<V>(v));
	}
};
}
