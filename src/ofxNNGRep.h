#pragma once

#include <stddef.h>
#include "nng.h"
#include "reqrep0/rep.h"
#include "supplemental/util/platform.h"
#include "ofLog.h"
#include "ASyncWork.h"
#include "ofxNNGParseFunctions.h"
#include "ofxNNGConvertFunctions.h"

namespace ofx {
namespace nng {
class Rep
{
public:
	struct Settings {
		std::string url;
		nng_listener *listener=nullptr;
		int flags=0;
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
		work_.initialize(s.max_queue, &Rep::async, this);
		while(auto work = work_.getUnused()) {
			nng_ctx_open(&work->ctx, socket_);
			work->state = aio::RECV;
			nng_ctx_recv(work->ctx, work->aio);
		}
		return true;
	}
	bool hasWaitingRequest() const {
		return !request_.empty();
	}
	template<typename T>
	nng_ctx getNextRequest(T &msg) {
		if(!hasWaitingRequest()) {
			ofLogError("ofxNNGRep") << "no pending message";
			return nng_ctx();
		}
		auto work = request_.front();
		auto nngmsg = nng_aio_get_msg(work->aio);
		if(!util::parse(*nngmsg, msg)) {
			ofLogError("ofxNNGRep") << "failed to parse request";
			return nng_ctx();
		}
		request_.pop_front();
		pending_.push_back(work);
		return work->ctx;
	}
	template<typename T>
	bool reply(nng_ctx ctx, const T &msg) {
		auto found = std::find_if(std::begin(pending_), std::end(pending_), [ctx](aio::Work *work) {
			return nng_ctx_id(ctx) == nng_ctx_id(work->ctx);
		});
		if(found == std::end(pending_)) {
			ofLogError("ofxNNGRep") << "no pending request with the context";
			return false;
		}
		auto work = *found;
		auto nngmsg = nng_aio_get_msg(work->aio);
		if(!util::convert(msg, *nngmsg)) {
			ofLogError("ofxNNGRep") << "failed to convert message";
			return false;
		}
		nng_aio_set_msg(work->aio, nngmsg);
		work->state = aio::SEND;
		nng_ctx_send(work->ctx, work->aio);
		pending_.erase(found);
		return true;
	}
	bool abort(nng_ctx ctx) {
		auto found = std::find_if(std::begin(pending_), std::end(pending_), [ctx](aio::Work *work) {
			return nng_ctx_id(ctx) == nng_ctx_id(work->ctx);
		});
		if(found == std::end(pending_)) {
			ofLogError("ofxNNGRep") << "no pending request with the context";
			return false;
		}
		auto work = *found;
		work->state = aio::RECV;
		nng_ctx_recv(work->ctx, work->aio);
		pending_.erase(found);
		return true;
	}
private:
	nng_socket socket_;
	aio::WorkPool work_;
	std::deque<aio::Work*> request_;
	std::deque<aio::Work*> pending_;
	static void async(void *arg) {
		auto work = (aio::Work*)arg;
		auto me = (Rep*)work->userdata;
		switch(work->state) {
			case aio::RECV: {
				auto result = nng_aio_result(work->aio);
				if(result != 0) {
					ofLogError("ofxNNGRep") << "failed to receive message; " << nng_strerror(result);
					return;
				}
				me->request_.push_back(work);
			}	break;
			case aio::SEND: {
				auto result = nng_aio_result(work->aio);
				if(result != 0) {
					ofLogError("ofxNNGRep") << "failed to send message; " << nng_strerror(result);
					break;
				}
				work->state = aio::RECV;
				nng_ctx_recv(work->ctx, work->aio);
			}	break;
		}
	}
};
}}
