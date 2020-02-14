#pragma once

#include "nng.h"
#include <type_traits>
#include <string>
#include <vector>
#include "ofFileUtils.h"

namespace ofxNNG {
	namespace {
		using size_type = typename Message::size_type;
	}

namespace basic_converter {
#pragma mark - ofxNNGMessage
	static inline size_type from_msg(Message &t, const Message &msg, size_type offset) {
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
	static inline auto to_msg(T &&t)
	-> enable_if_arithmetic_t<T, Message> {
		size_type size = sizeof(T);
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
	static inline auto to_msg(T &&t)
	-> enable_if_should_memcpy_t<T, Message> {
		Message msg;
		msg.append(sizeof(T));
		msg.appendData(&t, sizeof(T));
		return msg;
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
	static inline Message to_msg(const std::string &t) {
		Message msg;
		msg.append(t.size());
		msg.appendData(t.data(), t.size());
		return msg;
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
	static inline Message to_msg(const ofBuffer &t) {
		Message msg;
		msg.append(t.size());
		msg.appendData(t.getData(), t.size());
		return msg;
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
	static inline Message to_msg(const ofJson &t) {
		return Message{t.dump()};
	}
#pragma mark - vector
	template<typename T>
	static inline size_type from_msg(std::vector<T> &t, const Message &msg, size_type offset) {
		auto pos = offset;
		size_type size;
		pos += msg.to(pos, size);
		t.resize(size);
		for(auto &&val : t) {
			pos += msg.to(pos, val);
		}
		return pos-offset;
	}
	template<typename T>
	static inline Message to_msg(const std::vector<T> &t) {
		Message msg;
		msg.append(t.size());
		for(auto &&val : t) {
			msg.append(const_cast<T&>(val));
		}
		return msg;
	}
#pragma mark - pair
	template<typename T, typename U>
	static inline size_type from_msg(std::pair<T,U> &p, const Message &msg, size_type offset) {
		auto pos = offset;
		pos += msg.to(pos, p.first);
		pos += msg.to(pos, p.second);
		return pos-offset;
	}
	template<typename T, typename U>
	static inline Message to_msg(const std::pair<T,U> &p) {
		Message msg;
		msg.append(const_cast<T&>(p.first), const_cast<U&>(p.second));
		return msg;
	}
#pragma mark - map
	template<typename T, typename U>
	static inline size_type from_msg(std::map<T,U> &m, const Message &msg, size_type offset) {
		auto pos = offset;
		size_type size;
		pos += msg.to(pos, size);
		for(auto i = 0; i < size; ++i) {
			std::pair<T,U> p;
			pos += msg.to(pos, p);
			m.insert(p);
		}
		return pos-offset;
	}
	template<typename T, typename U>
	static inline Message to_msg(const std::map<T,U> &m) {
		Message msg;
		msg.append(m.size());
		for(auto &&p : m) {
			msg.append(p);
		}
		return msg;
	}
}

}

#include "ofxNNGMessageADLConverter.h"
