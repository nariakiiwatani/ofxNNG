#include "ofApp.h"
#include "ofxNNGMessage.h"

using namespace ofxNNG;

// you can send/receive any class you define.
struct UserType {
	int ch;
};
// if your class is not memcpy-able, you can define your converter like this.
//namespace ofx { namespace nng {
//	template<>
//	struct adl_converter<UserType> {
//		static inline void from_msg(UserType &t, const ofx::nng::Message &msg, std::size_t offset=0) {
//		}
//		static inline ofx::nng::Message to_msg(const UserType &t) {
//			return Message();
//		}
//	};
//}}
//--------------------------------------------------------------
void ofApp::setup(){
	Pub::Settings pubs;
	pub_.setup(pubs);
	auto listener = pub_.createListener("inproc://test");
	listener->setEventCallback(NNG_PIPE_EV_ADD_POST, [this]() {
		// you can send connection message here.
		// say hello or send topic list, bra bra bra,,,
		ofxNNG::Message message;
		message.prepend('H','e','l','l','o','!');
		message.prepend("this is connection message");
		pub_.send("connection:", message);
	});
	listener->start();

	Sub::Settings subs;
	sub_.resize(1);
	for(auto &s : sub_) {
		s = std::make_shared<ofxNNG::Sub>();
		s->setup(subs);
		// subscribe specific topic
		// template argument type is which incoming msg will be converted to.
		// see ofxNNGConvertFunctions.h and ofxNNGParseFunctions.h
		// or you can add your convert/parse functionalities in ofx::nng::util namespace.
		s->subscribe<std::string>("connection:", [](const std::string &topic, const std::string &message) {
			ofLogNotice("sub connected") << "topic:" << topic << ", message:" << message;
		});
		// you can subscribe all message by passing empty topic
		// but the argument 'topic' also will be empty and it comes with 'message', so if you want to separate them you need some external rules.
		s->subscribe<UserType>("user", [](const std::string &topic, const UserType &message) {
			ofLogNotice("sub received") << "message:" << message.ch;
		});
		s->subscribe<uint64_t>("frame", [](const std::string &topic, const uint64_t &message) {
			ofLogNotice("sub received") << "topic:" << topic << "message:" << message;
		});
		// topic don't have to be string.
		char topic_ch = 'a';
		s->subscribe<std::string>(&topic_ch, sizeof(char), [](const ofBuffer &topic, const std::string &message) {
			ofLogNotice("sub integer topic") << "topic:" << topic.getText() << ", message:" << message;
		});
		s->createDialer("inproc://test")->start();
	}
}

//--------------------------------------------------------------
void ofApp::update(){
//	pub_.send("frame", ofGetFrameNum());

}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	pub_.send("user", UserType{key});
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
