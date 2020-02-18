#pragma once

#include <stddef.h>
#include "nng.h"
#include "pair0/pair.h"
#include "pair1/pair.h"
#include "ofLog.h"
#include "ASyncWork.h"
#include "ofxNNGMessage.h"
#include "ofThreadChannel.h"
#include "ofxNNGNode.h"

namespace ofxNNG {
class Pair : public Node
{
public:
	struct Settings {
		Settings(){};
		
		int version = 1;
		bool polyamorous_mode=true;
		bool allow_callback_from_other_thread=false;
	};
	bool setup(const Settings &s=Settings()) {
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
		setEnabledAutoUpdate(!s.allow_callback_from_other_thread);
		nng_aio_alloc(&aio_, &Pair::receive, this);
		nng_recv_aio(socket_, aio_);
		return true;
	}
	template<typename T>
	void setCallback(const std::function<void(T&&)> &callback) {
		callback_ = [callback](Message msg) {
			callback(msg.get<T>());
		};
	}
	template<typename T>
	void setCallback(const std::function<void(T&&, nng_pipe)> &callback) {
		callback_ = [callback](Message msg) {
			callback(msg.get<T>());
		};
	}
	bool send(Message msg, bool blocking = false, nng_pipe pipe=NNG_PIPE_INITIALIZER) {
		if(nng_pipe_id(pipe) != -1) {	// nng_pipe_id returns -1 for invalid pipe
			nng_msg_set_pipe(msg, pipe);
		}
		int result;
		int flags = 0;
		if(!blocking) flags |= NNG_FLAG_NONBLOCK;
		result = nng_sendmsg(socket_, msg, flags);
		if(result != 0) {
			ofLogError("ofxNNGPair") << "failed to send message; " << nng_strerror(result);
			return false;
		}
		msg.setSentFlag();
		return true;
	}
private:
	nng_aio *aio_;
	std::function<void(Message)> callback_;
	ofThreadChannel<Message> channel_;
	
	static void receive(void *arg) {
		auto me = (Pair*)arg;
		auto result = nng_aio_result(me->aio_);
		if(result != 0) {
			ofLogError("ofxNNGPair") << "failed to receive message; " << nng_strerror(result);
			return;
		}
		Message msg(nng_aio_get_msg(me->aio_));
		if(me->isEnabledAutoUpdate()) {
			me->channel_.send(std::move(msg));
		}
		else {
			me->callback_(std::move(msg));
		}
		nng_recv_aio(me->socket_, me->aio_);
	}
	void update() {
		Message msg;
		while(channel_.tryReceive(msg)) {
			callback_(std::move(msg));
		}
	}
};
}
