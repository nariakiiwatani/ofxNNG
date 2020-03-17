#pragma once

#include "nng.h"
#include <type_traits>
#include <string>
#include <vector>

namespace ofxNNG {

class Message
{
public:
	using size_type = std::size_t;
	Message() {
		nng_msg_alloc(&msg_, 0);
	}
	Message(Message &msg):Message() {
		*this = msg;
	}
	Message(const Message &msg):Message() {
		*this = msg;
	}
	Message(Message &&msg) {
		is_responsible_to_free_msg_ = false;
		*this = std::move(msg);
	}
	Message(nng_msg *msg):msg_(msg) {}
	~Message() {
		if(is_responsible_to_free_msg_) {
			nng_msg_free(msg_);
		}
	}
	Message& operator=(const Message &msg) {
		nng_msg_dup(&msg_, msg.msg_);
		is_responsible_to_free_msg_ = true;
		return *this;
	}
	Message& operator=(Message &&msg) {
		if(is_responsible_to_free_msg_) {
			nng_msg_free(msg_);
		}
		msg_ = msg.msg_;
		is_responsible_to_free_msg_ = false;
		std::swap(is_responsible_to_free_msg_, msg.is_responsible_to_free_msg_);
		return *this;
	}
	template<typename ...Args>
	Message(Args &&...args):Message() {
		append(std::forward<Args>(args)...);
	}
	Message clone() const {
		Message msg = *this;
		return msg;
	}
	void clear() {
		nng_msg_realloc(msg_, 0);
	}
	operator nng_msg*() { return msg_; }
	void setSentFlag() {
		is_responsible_to_free_msg_ = false;
	}
	
	void appendData(const void *data, size_type size) {
		nng_msg_append(msg_, data, size);
	}
	template<typename Arg, typename ...Rest>
	void append(Arg &&arg, Rest &&...rest) {
		appendTo(*this, std::forward<Arg>(arg), std::forward<Rest>(rest)...);
	}
	void prependData(const void *data, size_type size) {
		nng_msg_insert(msg_, data, size);
	}
	template<typename ...Arg>
	void prepend(Arg &&...arg) {
		Message msg(std::forward<Arg>(arg)...);
		prependData(msg.data(), msg.size());
	}
	
	template<typename Arg, typename ...Rest>
	size_type to(size_type offset, Arg &arg, Rest &...rest) const;
	template<typename Arg, typename ...Rest>
	size_type to(Arg &arg, Rest &...rest) const { return to(0, arg, rest...); }
	template<typename T> T get(size_type offset=0) const {
		T t;
		to<T>(offset, t);
		return t;
	}

	template<typename Arg, typename ...Rest>
	static void appendTo(Message &msg, Arg &&arg, Rest &&...rest);
	template<typename T> void set(T &&t) {
		clear();
		appendTo(*this, std::forward<T>(t));
	}
	
	void* data() { return nng_msg_body(msg_); }
	const void* data() const { return nng_msg_body(msg_); }
	size_type size() const { return nng_msg_len(msg_); }
protected:
	static void appendTo(Message&){}
	size_type to(size_type) const { return 0; }
	
	nng_msg *msg_;
	bool is_responsible_to_free_msg_=true;
};
}

#include "ofxNNGMessageConvertFunction.h"

namespace ofxNNG {
	template<typename Arg, typename ...Rest>
	size_type Message::to(size_type offset, Arg &arg, Rest &...rest) const {
		auto pos = offset;
		pos += adl_converter<Arg>::from_msg(arg, *this, pos);
		pos += to(pos, rest...);
		return pos-offset;
	}
	template<typename Arg, typename ...Rest>
	void Message::appendTo(Message &msg, Arg &&arg, Rest &&...rest) {
		using rawtype = typename std::remove_cv<typename std::remove_reference<Arg>::type>::type;
		adl_converter<rawtype>::append_to_msg(msg, std::forward<Arg>(arg));
		appendTo(msg, std::forward<Rest>(rest)...);
	}
}

