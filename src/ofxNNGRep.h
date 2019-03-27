#pragma once

#include <stddef.h>
#include "nng.h"
#include "reqrep0/rep.h"
#include "supplemental/util/platform.h"
#include "ofLog.h"
#include "ASyncWork.h"

namespace ofx {
namespace nng {
class Rep
{
public:
	struct Settings {
		std::string url;
		nng_listener *listener=nullptr;
		int flags=NNG_FLAG_NONBLOCK;
		int max_queue=16;
	};
	bool setup(const Settings &s) {
		int result;
		result = nng_rep0_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGRep") << "failed to open socket;" << nng_strerror(result);
			return false;
		}
		result = nng_listen(socket_, s.url.data(), s.listener, s.flags);
		if(result != 0) {
			ofLogError("ofxNNGRep") << "failed to create listener; " << nng_strerror(result);
			return false;
		}
		work_.initialize(s.max_queue, &Rep::receive, this);
		activateNew();
		return true;
	}
private:
	nng_socket socket_;
	aio::WorkPool work_;
	static void receive(void *arg) {
		auto work = (aio::Work*)arg;
		auto me = (Rep*)work->userdata;
		switch(work->state) {
			case aio::RECV: {
				auto result = nng_aio_result(work->aio);
				if(result != 0) {
					ofLogError("ofxNNGRep") << "failed to receive message; " << nng_strerror(result);
					return;
				}
				me->activateNew();
				auto msg = nng_aio_get_msg(work->aio);
				auto body = nng_msg_body(msg);
				int len = nng_msg_len(msg);
				ofLogNotice("ofxNNGRep") << "receive request: " << std::string((char*)body, len);
				nng_aio_set_msg(work->aio, msg);
				work->state = aio::SEND;
				nng_ctx_send(work->ctx, work->aio);
			}	break;
			case aio::SEND: {
				auto result = nng_aio_result(work->aio);
				if(result != 0) {
					ofLogError("ofxNNGRep") << "failed to send message; " << nng_strerror(result);
					break;
				}
				nng_ctx_close(work->ctx);
				work->release();
			}	break;
		}
	}
	aio::Work* activateNew() {
		auto work = work_.getUnused();
		nng_ctx_open(&work->ctx, socket_);
		work->state = aio::RECV;
		nng_ctx_recv(work->ctx, work->aio);
		return work;
	}
};
}}
