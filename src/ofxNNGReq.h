#pragma once

#include <stddef.h>
#include "nng.h"
#include "reqrep0/req.h"
#include "supplemental/util/platform.h"
#include "ofLog.h"
#include "ASyncWork.h"
#include "ofxNNGConvertFunctions.h"
#include "ofxNNGParseFunctions.h"

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
		work_.initialize(s.max_queue, &Req::async, this);
		return true;
	}
	template<typename T>
	nng_ctx send(const T &data) {
		auto work = work_.getUnused();
		if(!work) {
			ofLogWarning("ofxNNGReq") << "no unused work";
			return nng_ctx();
		}
		nng_ctx_open(&work->ctx, socket_);
		nng_msg *msg;
		nng_msg_alloc(&msg, 0);
		if(!util::convert(data, *msg)) {
			ofLogError("ofxNNGReq") << "failed to convert data";
			return nng_ctx();
		}
		nng_aio_set_msg(work->aio, msg);
		work->state = aio::SEND;
		nng_ctx_send(work->ctx, work->aio);
		return work->ctx;
	}
	bool hasWaitingResponse() const {
		return !pending_.empty();
	}
	bool hasWaitingResponse(nng_ctx ctx) const {
		return std::find_if(std::begin(pending_), std::end(pending_), [ctx](aio::Work *work) {
			return nng_ctx_id(ctx) == nng_ctx_id(work->ctx);
		}) != std::end(pending_);
	}
	template<typename T>
	bool getNextResponse(T &dst) {
		return hasWaitingResponse() && getResponse(pending_.front()->ctx, dst);
	}
	template<typename T>
	bool getResponse(nng_ctx ctx, T &dst) {
		auto found = std::find_if(std::begin(pending_), std::end(pending_), [ctx](aio::Work *work) {
			return nng_ctx_id(ctx) == nng_ctx_id(work->ctx);
		});
		if(found == std::end(pending_)) {
			ofLogError("ofxNNGReq") << "no pending request with the context";
			return false;
		}
		auto work = *found;
		auto msg = nng_aio_get_msg(work->aio);
		if(!util::parse(*msg, dst)) {
			ofLogError("ofxNNGReq") << "failed to convert message";
			return false;
		}
		nng_msg_free(msg);
		work->release();
		nng_ctx_close(work->ctx);
		pending_.erase(found);
		return true;
	}
	bool abort(nng_ctx ctx) {
		auto found = std::find_if(std::begin(pending_), std::end(pending_), [ctx](aio::Work *work) {
			return nng_ctx_id(ctx) == nng_ctx_id(work->ctx);
		});
		if(found == std::end(pending_)) {
			ofLogError("ofxNNGReq") << "no pending response with the context";
			return false;
		}
		auto work = *found;
		auto msg = nng_aio_get_msg(work->aio);
		nng_msg_free(msg);
		work->release();
		nng_ctx_close(work->ctx);
		pending_.erase(found);
		return true;
	}
private:
	nng_socket socket_;
	aio::WorkPool work_;
	std::deque<aio::Work*> pending_;
	
	static void async(void *arg) {
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
				me->pending_.push_back(work);
			}	break;
		}
	}
};
}}
