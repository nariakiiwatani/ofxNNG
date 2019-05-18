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
		bool blocking=false;
		int max_queue=16;
	};
	template<typename Request, typename Response>
	bool setup(const Settings &s, const std::function<Response(const Request&)> &make_reply) {
		int result;
		result = nng_rep0_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGRep") << "failed to open socket;" << nng_strerror(result);
			return false;
		}
		int flags = 0;
		if(!s.blocking) flags |= NNG_FLAG_NONBLOCK;
		result = nng_listen(socket_, s.url.data(), s.listener, flags);
		if(result != 0) {
			ofLogError("ofxNNGRep") << "failed to create listener; " << nng_strerror(result);
			return false;
		}
		result = nng_mtx_alloc(&mtx_);
		if(result != 0) {
			ofLogError("ofxNNGReq") << "failed to create mutex; " << nng_strerror(result);
			return false;
		}
		make_reply_ = [make_reply](nng_msg *msg) {
			Request req = util::parse<Request>(*msg);
			Response res = make_reply(req);
			if(!util::convert(res, *msg)) {
				ofLogError("ofxNNGRep") << "failed to convert message";
			}
			return msg;
		};
		work_.initialize(s.max_queue, &Rep::async, this);
		activateNewReceiver();
		return true;
	}
private:
	nng_socket socket_;
	aio::WorkPool work_;
	std::function<nng_msg*(nng_msg*)> make_reply_;
	nng_mtx *mtx_;
	
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
				me->activateNewReceiver();
				auto msg = nng_aio_get_msg(work->aio);
				msg = me->make_reply_(msg);
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
				nng_mtx_lock(me->mtx_);
				work->release();
				nng_mtx_unlock(me->mtx_);
			}	break;
		}
	}
	bool activateNewReceiver() {
		aio::Work *work = nullptr;
		nng_mtx_lock(mtx_);
		work = work_.getUnused();
		nng_mtx_unlock(mtx_);
		if(!work) {
			ofLogWarning("ofxNNGRep") << "no unused work";
			return false;
		}
		nng_ctx_open(&work->ctx, socket_);
		work->state = aio::RECV;
		nng_ctx_recv(work->ctx, work->aio);
		return true;
	}
};
}}
