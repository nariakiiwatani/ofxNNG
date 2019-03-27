#pragma once

#include "nng.h"

namespace ofx {
namespace nng {
namespace aio {
enum State {
	SEND, RECV
};
class WorkPool;
struct Work {
	nng_aio *aio=nullptr;
	nng_ctx ctx;
	void *userdata;
	State state;
	WorkPool *parent;
	void release();
	~Work() {
		if(aio) {
			nng_aio_free(aio);
		}
	}
};
	
class WorkPool
{
public:
	void initialize(int num_work, void(*func)(void*), void *userdata) {
		work_.resize(num_work);
		for(auto &w : work_) {
			w.userdata = userdata;
			w.parent = this;
			nng_aio_alloc(&w.aio, func, &w);
			unused_.push_back(&w);
		}
	}
	Work* getUnused() {
		if(unused_.empty()) return nullptr;
		auto ret = unused_.back();
		unused_.pop_back();
		return ret;
	}
	void back(Work *work) {
		unused_.push_back(work);
	}
private:
	std::vector<Work> work_;
	std::vector<Work*> unused_;
};
inline void Work::release() {
	parent->back(this);
}

}}}
