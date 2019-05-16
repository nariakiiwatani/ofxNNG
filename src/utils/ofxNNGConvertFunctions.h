#pragma once

#include "ofFileUtils.h"
#include "nng.h"

namespace ofx {
namespace nng {
namespace util {
	template<typename T>
	inline bool convert(const T &src, nng_msg &dst) {
		return false;
	}
	template<>
	inline bool convert(const ofBuffer &src, nng_msg &dst) {
		nng_msg_clear(&dst);
		nng_msg_append(&dst, src.getData(), src.size());
		return true;
	}
}
}
}
