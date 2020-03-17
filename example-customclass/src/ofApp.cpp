#include "ofApp.h"
#include "ofxNNGPair.h"
#include "Serialize.h"

// you can use any types which you define
struct Memcopyable {
	int index;
	glm::vec3 pos;
	// if you are sure it's safe to use memcpy to pack this type into message,
	// you can notify that to ofxNNG by putting this macro in its scope.
	// typically, this means at least 1 condition of 2 below.
	// 1. your host is using big endian(nng uses big endian inside)
	// 2. both sending and receiving hosts uses same endian
	OFX_NNG_NOTIFY_TO_USE_MEMCPY_MEMBER
};
// or you can notify same thing by this macro outside the scope.
// this is useful for third-party struct.
struct ThirdPartys{};
OFX_NNG_NOTIFY_TO_USE_MEMCPY(ThirdPartys);

// in case there is any non-memcopyable member(ex; std::string),
// you need to define conversion functions.
// you can use macro for typical conversion.
struct NeedConversion {
	std::string name;
	ofColor color;
	ofMesh mesh;
	ofMeshFace face;
	ofPoint point;
	ofPixels pixels;
	ofRectangle rectangle;
	ofBuffer buffer;
	OFX_NNG_MEMBER_CONVERTER(name,color,mesh,face,point,pixels,rectangle,buffer);
};

struct ThirdParty2 {int i;};
namespace ofxNNG {
	// for third-party classes
	OFX_NNG_ADL_CONVERTER(ThirdParty2,i);
	// actually ofxNNG already defined converters for many of glm and of types(see ofxNNGMessageConvertFunctions.h).
	// but still you can override them.
	OFX_NNG_ADL_CONVERTER(ofColor, r,g,b);
	OFX_NNG_ADL_CONVERTER(glm::vec3, x,y,z);
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
	ofPixels pix;
	pix.allocate(1,1,OF_PIXELS_RGB);
	Message msg;
	msg.append(123);
	msg.append(456,789,"something");
	msg.append(pix);
	msg.append(ofVec3f());
	msg.append(ofPixels());
	msg.append(ofMatrix4x4());
	msg.append(glm::mat4());
	msg.append(glm::quat());
	msg.append(ofColor());
	socket.send(msg);
	socket.send(std::move(msg));	// if you won't use this msg anymore, it'd be better to use std::move.
	
	// or you can send everything directly.
	// they will be converted to ofxNNG::Message internally.
	socket.send(42);
	socket.send(glm::vec3(0,0,1));
	socket.send({123,456,789,"something"});
	// this case it will be memcpy-ed.
	// remember there was OFX_NNG_NOTIFY_TO_USE_MEMCPY_MEMBER avobe.
	Memcopyable copyable;
	copyable.index = 57;
	copyable.pos = {1,1,1};
	socket.send(copyable);
	// this case it will be converted to ofxNNG::Message using user-defined converter.
	// defined by OFX_NNG_MEMBER_CONVERTER macro
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
