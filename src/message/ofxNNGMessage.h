#pragma once

#include "nng.h"
#include <type_traits>
#include <string>
#include <vector>

namespace ofxNNG {

class Message
{
public:
	Message() {
		nng_msg_alloc(&msg_, 0);
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
	operator nng_msg*() { return msg_; }
	void setSentFlag() {
		is_responsible_to_free_msg_ = false;
	}
	
	void appendData(const void *data, std::size_t size) {
		nng_msg_append(msg_, data, size);
	}
	template<typename Arg, typename ...Rest>
	void append(Arg &&arg, Rest &&...rest) {
		Message msg = Message::from(std::forward<Arg>(arg));
		appendData(msg.data(), msg.size());
		append(std::forward<Rest>(rest)...);
	}
	void prependData(const void *data, std::size_t size) {
		nng_msg_insert(msg_, data, size);
	}
	template<typename ...Arg>
	void prepend(Arg &&...arg) {
		Message msg(arg...);
		prependData(msg.data(), msg.size());
	}
	
	template<typename T> void to(T &t, std::size_t offset=0) const;
	template<typename T> T get(std::size_t offset=0) const {
		T t;
		to<T>(t, offset);
		return t;
	}
	template<typename T> void set(T &&t);
	template<typename T> static Message from(T &&t) {
		Message msg;
		msg.set(std::forward<T>(t));
		return msg;
	}
	
	void* data() { return nng_msg_body(msg_); }
	const void* data() const { return nng_msg_body(msg_); }
	std::size_t size() const { return nng_msg_len(msg_); }
private:
	void append(void){}
	nng_msg *msg_;
	bool is_responsible_to_free_msg_=true;
};
}

#include "ofxNNGMessageConvertFunction.h"

namespace ofxNNG {
	template<typename T>
	void Message::to(T &t, std::size_t offset) const {
		adl_converter<T>::from_msg(t, *this, offset);
	}
	template<typename T>
	void Message::set(T &&t) {
		*this = adl_converter<T>::to_msg(std::forward<T>(t));
	}
}

