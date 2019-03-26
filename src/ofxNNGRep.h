#pragma once

#include <stddef.h>
#include "nng.h"
#include "reqrep0/rep.h"
#include "supplemental/util/platform.h"
#include "ofLog.h"

namespace ofx {
namespace nng {
class Rep
{
public:
	struct Settings {
		std::string url;
		nng_listener *listener=nullptr;
		int flags=NNG_FLAG_NONBLOCK;
	};
	virtual ~Rep() {
		if(thread_) {
			nng_mtx_lock(lock_run_);
			is_thread_running_ = false;
			nng_mtx_unlock(lock_run_);
			nng_thread_destroy(thread_);
		}
		if(lock_run_) {
			nng_mtx_free(lock_run_);
		}
	};
	bool setup(const Settings &s) {
		int result;
		result = nng_rep0_open(&socket_);
		if(result != 0) {
			ofLogError("ofxNNGRep") << "failed to open socket";
			return false;
		}
		result = nng_listen(socket_, s.url.data(), s.listener, s.flags);
		if(result != 0) {
			ofLogError("ofxNNGRep") << "failed to create listener";
			return false;
		}
		nng_mtx_alloc(&lock_run_);
		is_thread_running_ = true;
		nng_thread_create(&thread_, Rep::threadedFunction, this);
		return true;
	}
	void send(void *msg, size_t len, int flags) {
		int result = nng_send(socket_, msg, len, flags);
		if(result != 0) {
			ofLogError("ofxNNGRep") << "failed to send message";
			return;
		}
	}
private:
	nng_socket socket_;
	nng_thread *thread_=nullptr;
	nng_mtx *lock_run_;
	bool is_thread_running_=false;
	static void fatal(const char *func, int rv)
	{
		fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
		exit(1);
	}
	static void threadedFunction(void *arg) {
		auto me = (Rep*)arg;
		void *msg;
		int flags = NNG_FLAG_NONBLOCK|NNG_FLAG_ALLOC;
		for(;;) {
			nng_mtx_lock(me->lock_run_);
			if(!me->is_thread_running_) {
				nng_mtx_unlock(me->lock_run_);
				return;
			}
			nng_mtx_unlock(me->lock_run_);
			size_t len=256;
			int result = nng_recv(me->socket_, &msg, &len, flags);
			if(result != 0) {
				if(result == NNG_EAGAIN) {
					continue;
				}
				ofLogError("ofxNNGRep") << "failed to receive message";
				fatal("nng_recv", result);
				return false;
			}
			ofLogNotice("ofxNNGRep") << "receive request: " << std::string((char*)msg, len);
			me->send(msg, len, flags);
		}
	}
};
}}
