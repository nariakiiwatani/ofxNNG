#pragma once

#include "nng.h"
#include <map>
#include <vector>
#include <string>
#include <functional>

namespace ofxNNG {
class Pipe
{
public:
	Pipe(nng_socket socket):socket_(socket) {
		nng_pipe_notify(socket, NNG_PIPE_EV_ADD_PRE, event_callback, this);
		nng_pipe_notify(socket, NNG_PIPE_EV_ADD_POST, event_callback, this);
		nng_pipe_notify(socket, NNG_PIPE_EV_REM_POST, event_callback, this);
	}
	virtual ~Pipe() {
		nng_pipe_notify(socket_, NNG_PIPE_EV_ADD_PRE, event_callback_empty, this);
		nng_pipe_notify(socket_, NNG_PIPE_EV_ADD_POST, event_callback_empty, this);
		nng_pipe_notify(socket_, NNG_PIPE_EV_REM_POST, event_callback_empty, this);
	}
	virtual bool create(const std::string &url)=0;
	virtual bool start(int flags=NNG_FLAG_NONBLOCK)=0;
	virtual bool close()=0;
	void setEventCallback(nng_pipe_ev event, std::function<void()> func) {
		event_listener_[event] = func;
	}
protected:
	nng_socket socket_;
private:
	static inline void event_callback(nng_pipe pipe, nng_pipe_ev event, void *data) {
		auto me = (Pipe*)data;
		auto callback = me->event_listener_.find(event);
		if(callback != std::end(me->event_listener_)) {
			callback->second();
		}
	}
	static inline void event_callback_empty(nng_pipe pipe, nng_pipe_ev event, void *data) {}
	std::map<nng_pipe_ev, std::function<void()>> event_listener_;
};
class Dialer : public Pipe
{
public:
	using Pipe::Pipe;
	~Dialer() {
		close();
	}
protected:
	bool create(const std::string &url) {
		auto result = nng_dialer_create(&dialer_, socket_, url.c_str());
		if(result != 0) {
			return false;
		}
		return true;
	}
	bool start(int flags=NNG_FLAG_NONBLOCK) {
		auto result = nng_dialer_start(dialer_, flags);
		if(result != 0) {
			return false;
		}
		return true;
	}
	bool close() {
		auto result = nng_dialer_close(dialer_);
		if(result != 0) {
			return false;
		}
		return true;
	}
private:
	nng_dialer dialer_;
};
class Listener : public Pipe
{
public:
	using Pipe::Pipe;
	~Listener() {
		close();
	}
protected:
	bool create(const std::string &url) {
		auto result = nng_listener_create(&listener_, socket_, url.c_str());
		if(result != 0) {
			return false;
		}
		return true;
	}
	bool start(int flags=NNG_FLAG_NONBLOCK) {
		auto result = nng_listener_start(listener_, flags);
		if(result != 0) {
			return false;
		}
		return true;
	}
	bool close() {
		auto result = nng_listener_close(listener_);
		if(result != 0) {
			return false;
		}
		return true;
	}
private:
	nng_listener listener_;
};

class Node
{
public:
	virtual ~Node() {
		close();
	}
	std::shared_ptr<Pipe> createDialer(const std::string &url) {
		return createPipe<Dialer>(url);
	}
	std::shared_ptr<Pipe> createListener(const std::string &url) {
		return createPipe<Listener>(url);
	}
	void close() {
		nng_close(socket_);
		pipe_.clear();
	}
protected:
	nng_socket socket_;
	std::vector<std::shared_ptr<Pipe>> pipe_;
	template<typename T> std::shared_ptr<Pipe> createPipe(const std::string &url) {
		std::shared_ptr<Pipe> pipe = std::make_shared<T>(socket_);
		if(!pipe->create(url)) { return nullptr; }
		pipe_.emplace_back(pipe);
		return pipe;
	}
};
}
