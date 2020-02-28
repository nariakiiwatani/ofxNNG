#include "ofApp.h"
#include "ofxNNGPair.h"
#include "Serialize.h"

// you can use types that you defined (endian mismatch or other unexpected issue may occur).
struct Memcopyable {
	int index;
	glm::vec3 pos;
};

// in case there is any non-memcopyable member(ex; std::string).
// you need to define conversion functions.
// you can use macro for typical conversion.
struct NeedConversion {
	std::string name;
	ofColor color;
	OFX_NNG_MEMBER_CONVERTER(name,color);
};

// if you wanted to define conversion functions for third-party classes, 
// you can do it.
namespace ofxNNG {
	OFX_NNG_ADL_CONVERTER(ofColor, r,g,b);
}

//--------------------------------------------------------------
void ofApp::setup(){
	using namespace ofxNNG;
	// setup socket. no matter which type to use for this example.
	ofxNNG::Pair socket;
	socket.setup();
	auto listener = socket.createListener("inproc://example");
	
	// event callbacks. 
	// these are not necessary.
	listener->setEventCallback(NNG_PIPE_EV_ADD_PRE, [](nng_pipe pipe) {
		// called after connection but before adding to the socket.
		// but you cannot send any messages.
		ofLogNotice("event") << "add pre";
		
		// if you wanted not to communicate with this pipe, you can close the connection explicitly.
		// nng_pipe_close(pipe);
	});
	listener->setEventCallback(NNG_PIPE_EV_ADD_POST, [](nng_pipe pipe) {
		// called after adding to the socket.
		// now you can send messages.
		ofLogNotice("event") << "add post";
	});
	listener->setEventCallback(NNG_PIPE_EV_REM_POST, [](nng_pipe pipe) {
		// called after removal.
		// you no longer can send any messages.
		ofLogNotice("event") << "rem post";
	});
	
	listener->start();
	
	// you can use ofxNNG::Message explicitly
	Message msg;
	msg.append(123);
	msg.append(456,789,"something");
	socket.send(msg);
	socket.send(std::move(msg));	// if you won't use this msg anymore, it'd be better to use std::move.
	
	// or you can send anything directly.
	// they will be converted to ofxNNG::Message internally.
	socket.send(42);
	socket.send(glm::vec3(0,0,1));
	socket.send({123,456,789,"something"});
	// this case it will be memcpy-ed
	Memcopyable copyable;
	copyable.index = 57;
	copyable.pos = {1,1,1};
	socket.send(copyable);
	// this case it will be converted to ofxNNG::Message using user-defined converter.
	NeedConversion needs;
	needs.name = "my name";
	needs.color = ofColor::white;
	socket.send(needs);
	
	// to receive message with a socket, there are 2 options.
	// 1. callback functions
	socket.setCallback<int>([](int){});
	socket.setCallback<std::string, int>([](const std::string&, const int&){});
	socket.setCallback<Memcopyable>([](const Memcopyable&){});
	// 2. reference
	int intval;
	std::string strval;
	NeedConversion ncval;
	socket.setCallback(intval, strval, ncval);
	// you can receive values by any type.
	// but be careful; receiving messages using different type will cause unexpected behaviors.
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
