#pragma once

#include "nng.h"
#include <type_traits>
#include <string>
#include <vector>

namespace ofx {
namespace nng {
class Message;
template<typename T, typename SFINAE=void>
struct adl_converter {
	static void from_msg(T &t, const Message &msg, std::size_t offset);
	static Message to_msg(T &&t);
};

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
	
	template<typename T> void to(T &t, std::size_t offset=0) const {
		adl_converter<T>::from_msg(t, *this, offset);
	}
	template<typename T> T get(std::size_t offset=0) const {
		T t;
		to<T>(t, offset);
		return t;
	}
	template<typename T> void set(T &&t) {
		*this = adl_converter<T>::to_msg(std::forward<T>(t));
	}
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

template<>
struct adl_converter<Message> {
	static void from_msg(Message &t, const Message &msg, std::size_t offset) {
		t = msg;
	}
	static Message to_msg(Message &&t) {
		return std::move(t);
	}
	static Message to_msg(const Message &t) {
		return t;
	}
};
template<typename T>
struct adl_converter<T, typename std::enable_if<std::is_trivially_copyable<typename std::remove_reference<T>::type>::value>::type> {
static void from_msg(T &t, const Message &msg, std::size_t offset) {
	memcpy(&t, (const char*)msg.data()+offset, sizeof(T));
}
static Message to_msg(T &&t) {
	Message msg;
	msg.appendData(&t, sizeof(T));
	return msg;
}
};
template<>
struct adl_converter<std::string> {
	static void from_msg(std::string &t, const Message &msg, std::size_t offset) {
		std::size_t size = msg.size()-offset;
		t = std::string((const char*)msg.data()+offset, size);
	}
	static Message to_msg(const std::string &t) {
		Message msg;
		msg.appendData(t.data(), t.size());
		return msg;
	}
};
template<>
struct adl_converter<ofBuffer> {
	static void from_msg(ofBuffer &t, const Message &msg, std::size_t offset) {
		std::size_t size = msg.size()-offset;
		t.set((const char*)msg.data()+offset, size);
	}
	static Message to_msg(const ofBuffer &t) {
		Message msg;
		msg.appendData(t.getData(), t.size());
		return msg;
	}
};

}}
