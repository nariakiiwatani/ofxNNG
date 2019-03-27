#pragma once

#include <stddef.h>
#include "nng.h"
#include "reqrep0/req.h"
#include "supplemental/util/platform.h"
#include "ofLog.h"
#include "ASyncWork.h"

namespace ofx {
namespace nng {
class Req
{
public:
	struct Settings {
		std::string url;
		nng_dialer *dialer=nullptr;
		int flags=0;
		int max_queue=16;
	};
	bool setup(const Settings &s) {
		int result;
		result = nng_req0_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGReq") << "failed to open socket; " << nng_strerror(result);
			return false;
		}
		result = nng_dial(socket_, s.url.data(), s.dialer, s.flags);
		if(result != 0) {
			ofLogError("ofxNNGReq") << "failed to create dialer; " << nng_strerror(result);
			return false;
		}
		work_.initialize(s.max_queue, &Req::receive, this);
		return true;
	}
	void send(void *data, size_t len, int flags) {
		auto work = work_.getUnused();
		if(!work) {
			ofLogWarning("ofxNNGReq") << "no unused work";
			return;
		}
		nng_ctx_open(&work->ctx, socket_);
		nng_msg *msg;
		nng_msg_alloc(&msg, len);
		nng_msg_append(msg, data, len);
		nng_aio_set_msg(work->aio, msg);
		work->state = aio::SEND;
		nng_ctx_send(work->ctx, work->aio);
	}
private:
	nng_socket socket_;
	aio::WorkPool work_;
	static void receive(void *arg) {
		auto work = (aio::Work*)arg;
		auto me = (Req*)work->userdata;
		switch(work->state) {
			case aio::SEND: {
				auto result = nng_aio_result(work->aio);
				if(result != 0) {
					ofLogError("ofxNNGReq") << "failed to send message; " << nng_strerror(result);
					break;
				}
				work->state = aio::RECV;
				nng_ctx_recv(work->ctx, work->aio);
			}	break;
			case aio::RECV: {
				auto result = nng_aio_result(work->aio);
				if(result != 0) {
					ofLogError("ofxNNGReq") << "failed to receive message; " << nng_strerror(result);
					return;
				}
				auto msg = nng_aio_get_msg(work->aio);
				auto body = nng_msg_body(msg);
				int len = nng_msg_len(msg);
				ofLogNotice("ofxNNGReq") << "receive response: " << std::string((char*)body, len);
				nng_ctx_close(work->ctx);
				work->release();
			}	break;
		}
	}
};
}}
