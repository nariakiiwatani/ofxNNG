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
#include "ofxNNGNode.h"

namespace ofxNNG {
class Pair : public Node
{
public:
	struct Settings {
		int version = 1;
		
		bool polyamorous_mode=true;
		bool allow_callback_from_other_thread=false;
	};
	template<typename T>
	bool setup(const Settings &s, const std::function<void(const T&)> &callback) {
		if(!setupInternal(s, callback)) {
			return false;
		}
		callback_ = [callback](nng_msg *msg) {
			callback(util::parse<T>(msg));
		};
		return true;
	}
	template<typename T>
	bool setup(const Settings &s, const std::function<void(const T&, nng_pipe)> &callback) {
		if(!setupInternal(s, callback)) {
			return false;
		}
		callback_ = [callback](nng_msg *msg) {
			callback(util::parse<T>(msg), nng_msg_get_pipe(msg));
		};
		return true;
	}
	template<typename T>
	bool send(const T &data, nng_pipe pipe=NNG_PIPE_INITIALIZER) {
		nng_msg *msg;
		nng_msg_alloc(&msg, 0);
		if(nng_pipe_id(pipe) != -1) {	// nng_pipe_id returns -1 for invalid pipe
			nng_msg_set_pipe(msg, pipe);
		}
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
	nng_aio *aio_;
	std::function<void(nng_msg*)> callback_;
	bool async_;
	ofThreadChannel<nng_msg*> channel_;
	
	static void receive(void *arg) {
		auto me = (Pair*)arg;
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
	template<typename Callback>
	bool setupInternal(const Settings &s, const Callback &callback) {
		int result;
		switch(s.version) {
			case 0:
				result = nng_pair0_open(&socket_);
				break;
			default:
				ofLogWarning("ofxNNGPair") << "version number must be 0 or 1. setting up as version 1.";
			case 1:
				result = nng_pair1_open(&socket_);
				break;
		}
		if(result != 0) {
			ofLogError("ofxNNGPair") << "failed to open socket; " << nng_strerror(result);
			return false;
		}
		nng_setopt_bool(socket_, NNG_OPT_PAIR1_POLY, s.polyamorous_mode);
		async_ = s.allow_callback_from_other_thread;
		if(!async_) {
			ofAddListener(ofEvents().update, this, &Pair::update);
		}
		nng_aio_alloc(&aio_, &Pair::receive, this);
		nng_recv_aio(socket_, aio_);
		return true;	}
};
}
