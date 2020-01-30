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
	static Message to_msg(const T &t);
};

class Message
{
public:
	Message() {
		nng_msg_alloc(&msg_, 0);
	}
	Message(const Message &msg) {
		nng_msg_alloc(&msg_, 0);
		nng_msg_dup(&msg_, msg.msg_);
	}
	Message(Message &&msg) {
		is_responsible_to_free_msg_ = false;
		*this = std::forward<Message>(msg);
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
		return std::move(msg);
	}
	operator nng_msg*() { return msg_; }
	void setSentFlag() {
		is_responsible_to_free_msg_ = false;
	}
	
	template<typename T>
	void add(const T &t) {
		Message msg = Message::from(t);
		add(msg.data(), msg.size());
	}
	void add(const void *data, std::size_t size) {
		nng_msg_append(msg_, data, size);
	}
	
	template<typename T> void to(T &t, std::size_t offset=0) const {
		adl_converter<T>::from_msg(t, *this, offset);
	}
	template<typename T> T get(std::size_t offset=0) const {
		T t;
		to<T>(t, offset);
		return std::move(t);
	}
	template<typename T> void set(const T &t) {
		*this = adl_converter<T>::to_msg(t);
	}
	template<typename T> static Message from(const T &t) {
		Message msg;
		msg.set(t);
		return std::move(msg);
	}
	
	void* data() { return nng_msg_body(msg_); }
	const void* data() const { return nng_msg_body(msg_); }
	std::size_t size() const { return nng_msg_len(msg_); }
private:
	nng_msg *msg_;
	bool is_responsible_to_free_msg_=true;
};
template<typename T>
struct adl_converter<T, typename std::enable_if<std::is_trivially_copyable<T>::value>::type> {
	static void from_msg(T &t, const Message &msg, std::size_t offset) {
		memcpy(&t, (const char*)msg.data()+offset, sizeof(T));
	}
	static Message to_msg(const T &t) {
		Message msg;
		msg.add(&t, sizeof(T));
		return std::move(msg);
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
		msg.add(t.data(), t.size());
		return std::move(msg);
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
		msg.add(t.getData(), t.size());
		return std::move(msg);
	}
};

}}
