#pragma once

#include <stddef.h>
#include "nng.h"
#include "pair0/pair.h"
#include "pair1/pair.h"
#include "ofLog.h"
#include "ASyncWork.h"
#include "ofxNNGParseFunctions.h"
#include "ofxNNGConvertFunctions.h"
#include "ofThreadChannel.h"

namespace ofx {
namespace nng {
class Pair0
{
};

class Pair1
{
public:
	struct Settings {
		std::string url;
		union {
			nng_dialer *dialer;
			nng_listener *listener=nullptr;
		};
		bool blocking=false;
		
		bool polyamorous_mode=true;
		bool allow_callback_from_other_thread=false;
	};
	template<typename T>
	bool setupAsDialer(const Settings &s, const std::function<void(const T&)> &callback) {
		int result;
		result = nng_pair1_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGPair") << "failed to open socket; " << nng_strerror(result);
			return false;
		}
		int flags = 0;
		if(!s.blocking) flags |= NNG_FLAG_NONBLOCK;
		result = nng_dial(socket_, s.url.data(), s.dialer, flags);
		if(result != 0) {
			ofLogError("ofxNNGPair") << "failed to create dialer; " << nng_strerror(result);
			return false;
		}
		callback_ = [callback](nng_msg *msg) {
			callback(util::parse<T>(msg));
		};
		nng_setopt_bool(socket_, NNG_OPT_PAIR1_POLY, s.polyamorous_mode);
		async_ = s.allow_callback_from_other_thread;
		if(!async_) {
			ofAddListener(ofEvents().update, this, &Pair1::update);
		}
		nng_aio_alloc(&aio_, &Pair1::receive, this);
		nng_recv_aio(socket_, aio_);
		return true;
	}
	template<typename T>
	void setupAsListener(const Settings &s, const std::function<void(const T&)> &callback) {
		int result;
		result = nng_pair1_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGPair") << "failed to open socket; " << nng_strerror(result);
			return false;
		}
		int flags = 0;
		if(!s.blocking) flags |= NNG_FLAG_NONBLOCK;
		result = nng_listen(socket_, s.url.data(), s.listener, flags);
		if(result != 0) {
			ofLogError("ofxNNGPair") << "failed to create listener; " << nng_strerror(result);
			return false;
		}
		callback_ = [callback](nng_msg *msg) {
			callback(util::parse<T>(msg));
		};
		nng_setopt_bool(socket_, NNG_OPT_PAIR1_POLY, s.polyamorous_mode);
		async_ = s.allow_callback_from_other_thread;
		if(!async_) {
			ofAddListener(ofEvents().update, this, &Pair1::update);
		}
		nng_aio_alloc(&aio_, &Pair1::receive, this);
		nng_recv_aio(socket_, aio_);
		return true;
	}
	template<typename T>
	bool setupAsDialer(const Settings &s, const std::function<void(const T&, nng_pipe)> &callback) {
		int result;
		result = nng_pair1_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGPair") << "failed to open socket; " << nng_strerror(result);
			return false;
		}
		int flags = 0;
		if(!s.blocking) flags |= NNG_FLAG_NONBLOCK;
		result = nng_dial(socket_, s.url.data(), s.dialer, flags);
		if(result != 0) {
			ofLogError("ofxNNGPair") << "failed to create dialer; " << nng_strerror(result);
			return false;
		}
		callback_ = [callback](nng_msg *msg) {
			callback(util::parse<T>(msg), nng_msg_get_pipe(msg));
		};
		nng_setopt_bool(socket_, NNG_OPT_PAIR1_POLY, s.polyamorous_mode);
		async_ = s.allow_callback_from_other_thread;
		if(!async_) {
			ofAddListener(ofEvents().update, this, &Pair1::update);
		}
		nng_aio_alloc(&aio_, &Pair1::receive, this);
		nng_recv_aio(socket_, aio_);
		return true;
	}
	template<typename T>
	void setupAsListener(const Settings &s, const std::function<void(const T&, nng_pipe)> &callback) {
		int result;
		result = nng_pair1_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGPair") << "failed to open socket; " << nng_strerror(result);
			return false;
		}
		int flags = 0;
		if(!s.blocking) flags |= NNG_FLAG_NONBLOCK;
		result = nng_listen(socket_, s.url.data(), s.listener, flags);
		if(result != 0) {
			ofLogError("ofxNNGPair") << "failed to create listener; " << nng_strerror(result);
			return false;
		}
		callback_ = [callback](nng_msg *msg) {
			callback(util::parse<T>(msg), nng_msg_get_pipe(msg));
		};
		nng_setopt_bool(socket_, NNG_OPT_PAIR1_POLY, s.polyamorous_mode);
		async_ = s.allow_callback_from_other_thread;
		if(!async_) {
			ofAddListener(ofEvents().update, this, &Pair1::update);
		}
		nng_aio_alloc(&aio_, &Pair1::receive, this);
		nng_recv_aio(socket_, aio_);
		return true;
	}
	template<typename T>
	bool send(const T &data) {
		nng_msg *msg;
		nng_msg_alloc(&msg, 0);
		if(!util::convert(data, msg)) {
			ofLogError("ofxNNGPair") << "failed to convert message";
			return false;
		}
		int result;
		result = nng_sendmsg(socket_, msg, NNG_FLAG_NONBLOCK);
		if(result != 0) {
			ofLogError("ofxNNGPair") << "failed to send message; " << nng_strerror(result);
			nng_msg_free(msg);
			return false;
		}
		return true;
	}
	template<typename T>
	bool send(const T &data, nng_pipe pipe) {
		nng_msg *msg;
		nng_msg_alloc(&msg, 0);
		nng_msg_set_pipe(msg, pipe);
		if(!util::convert(data, msg)) {
			ofLogError("ofxNNGPair") << "failed to convert message";
			return false;
		}
		int result;
		result = nng_sendmsg(socket_, msg, NNG_FLAG_NONBLOCK);
		if(result != 0) {
			ofLogError("ofxNNGPair") << "failed to send message; " << nng_strerror(result);
			nng_msg_free(msg);
			return false;
		}
		return true;
	}
private:
	nng_socket socket_;
	nng_aio *aio_;
	std::function<void(nng_msg*)> callback_;
	bool async_;
	ofThreadChannel<nng_msg*> channel_;
	
	static void receive(void *arg) {
		auto me = (Pair1*)arg;
		auto result = nng_aio_result(me->aio_);
		if(result != 0) {
			ofLogError("ofxNNGPair") << "failed to receive message; " << nng_strerror(result);
			return;
		}
		auto msg = nng_aio_get_msg(me->aio_);
		if(me->async_) {
			me->callback_(msg);
			nng_msg_free(msg);
		}
		else {
			me->channel_.send(msg);
		}
		nng_recv_aio(me->socket_, me->aio_);
	}
	void update(ofEventArgs&) {
		nng_msg *msg;
		while(channel_.tryReceive(msg)) {
			callback_(msg);
			nng_msg_free(msg);
		}
	}
};
	
using Pair = Pair1;
}}
