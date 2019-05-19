#pragma once

#include <stddef.h>
#include "nng.h"
#include "supplemental/util/platform.h"
#include "pubsub0/sub.h"
#include "ASyncWork.h"
#include "ofxNNGParseFunctions.h"

namespace ofx {
namespace nng {
class Sub
{
public:
	struct Settings {
		std::string url;
		nng_dialer *dialer=nullptr;
		bool blocking=false;
		int max_queue=16;
	};
	template<typename T>
	bool setup(const Settings &s, const std::function<void(const T&)> &callback) {
		int result;
		result = nng_sub0_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGSub") << "failed to open socket;" << nng_strerror(result);
			return false;
		}
		int flags = 0;
		if(!s.blocking) flags |= NNG_FLAG_NONBLOCK;
		result = nng_dial(socket_, s.url.data(), s.dialer, flags);
		if(result != 0) {
			ofLogError("ofxNNGSub") << "failed to create dialer; " << nng_strerror(result);
			return false;
		}
		callback_ = [callback](nng_msg *msg) {
			callback(util::parse<T>(msg));
		};
		nng_aio_alloc(&aio_, &Sub::receive, this);
		nng_recv_aio(socket_, aio_);
		return true;
	}
	~Sub() {
		if(aio_) nng_aio_free(aio_);
	}
	bool subscribe(void *topic, std::size_t topic_length) {
		int result = nng_setopt(socket_, NNG_OPT_SUB_SUBSCRIBE, topic, topic_length);
		if(result != 0) {
			ofLogError("ofxNNGSub") << "failed to subscribe topic; " << nng_strerror(result);
			return false;
		}
		return true;
	}
	bool unsubscribe(void *topic, std::size_t topic_length) {
		int result = nng_setopt(socket_, NNG_OPT_SUB_UNSUBSCRIBE, topic, topic_length);
		if(result != 0) {
			ofLogError("ofxNNGSub") << "failed to unsubscribe topic; " << nng_strerror(result);
			return false;
		}
		return true;
	}
private:
	nng_socket socket_;
	nng_aio *aio_;
	std::function<void(nng_msg*)> callback_;
	static void receive(void *arg) {
		auto me = (Sub*)arg;
		auto result = nng_aio_result(me->aio_);
		if(result != 0) {
			ofLogError("ofxNNGRep") << "failed to receive message; " << nng_strerror(result);
			return;
		}
		auto msg = nng_aio_get_msg(me->aio_);
		me->callback_(msg);
		nng_msg_free(msg);
		nng_recv_aio(me->socket_, me->aio_);
	}
};
}}
