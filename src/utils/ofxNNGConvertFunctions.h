#pragma once

#include "ofFileUtils.h"
#include "ofJson.h"
#include "nng.h"

namespace ofx {
namespace nng {
namespace util {
	template<typename T>
	inline bool convert(const T &src, nng_msg *dst) {
		return false;
	}
	inline bool convert(const char* src, nng_msg *dst) {
		nng_msg_clear(dst);
		nng_msg_append(dst, src, strlen(src)+1);
		return true;
	}
	template<>
	inline bool convert(const std::string &src, nng_msg *dst) {
		nng_msg_clear(dst);
		nng_msg_append(dst, src.data(), src.length()+1);
		return true;
	}
	template<>
	inline bool convert(const ofBuffer &src, nng_msg *dst) {
		nng_msg_clear(dst);
		nng_msg_append(dst, src.getData(), src.size());
		return true;
	}
	template<>
	inline bool convert(const ofJson &src, nng_msg *dst) {
		return convert(src.dump(), dst);
	}
}
}
}
