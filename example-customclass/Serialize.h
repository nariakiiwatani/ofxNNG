#pragma once

#include "ofxNNGMessage.h"
#include "ofFileUtils.h"

namespace ofxNNG {
	struct Serialize : Message {
		using Message::Message;
		operator ofBuffer() const {
			return ofBuffer(static_cast<const char*>(data()), size());
		}
		void write(const std::filesystem::path &path) {
			ofBufferToFile(path, *this);
		}
	};
	struct Deserialize : Message {
		Deserialize(const ofBuffer &data):Message() {
			appendData(data.getData(), data.size());
		}
		Deserialize(const std::filesystem::path &path):Deserialize(ofBufferFromFile(path)) {}
		
		using Message::to;
		using Message::get;
	};
}
