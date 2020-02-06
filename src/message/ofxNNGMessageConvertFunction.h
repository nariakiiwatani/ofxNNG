#pragma once

#include "nng.h"
#include <type_traits>
#include <string>
#include <vector>

namespace ofxNNG {

namespace basic_converter {
#pragma mark - ofxNNGMessage
	static inline void from_msg(Message &t, const Message &msg, std::size_t offset) {
		t = msg;
	}
	static inline Message to_msg(Message &&t) {
		return std::move(t);
	}
	static inline Message to_msg(const Message &t) {
		return t;
	}

#pragma mark - trivially_copyable
	template<typename T, typename Type=void>
	using enable_if_trivially_copyable_t = typename std::enable_if<std::is_trivially_copyable<typename std::remove_reference<T>::type>::value, Type>::type;
	template<typename T>
	static inline auto from_msg(T &t, const Message &msg, std::size_t offset)
	-> enable_if_trivially_copyable_t<T> {
		memcpy(&t, (const char*)msg.data()+offset, sizeof(T));
	}
	template<typename T>
	static inline auto to_msg(T &&t)
	-> enable_if_trivially_copyable_t<T, Message> {
		Message msg;
		msg.appendData(&t, sizeof(T));
		return msg;
	}
#pragma mark - std::string
	static inline void from_msg(std::string &t, const Message &msg, std::size_t offset) {
		std::size_t size = msg.size()-offset;
		t = std::string((const char*)msg.data()+offset, size);
	}
	static inline Message to_msg(const std::string &t) {
		Message msg;
		msg.appendData(t.data(), t.size());
		return msg;
	}
#pragma mark - ofBuffer
	static inline void from_msg(ofBuffer &t, const Message &msg, std::size_t offset) {
		std::size_t size = msg.size()-offset;
		t.set((const char*)msg.data()+offset, size);
	}
	static inline Message to_msg(const ofBuffer &t) {
		Message msg;
		msg.appendData(t.getData(), t.size());
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
