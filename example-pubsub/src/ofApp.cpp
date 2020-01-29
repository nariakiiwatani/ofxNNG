#include "ofApp.h"

using namespace ofx::nng;

//--------------------------------------------------------------
void ofApp::setup(){
	Pub::Settings pubs;
	pub_.setup(pubs);
	auto listener = pub_.createListener("inproc://test");
	listener->setEventCallback(NNG_PIPE_EV_ADD_POST, [this]() {
		// you can send connection message here.
		// say hello or send topic list, bra bra bra,,,
		pub_.send("connection:", "this is connection message");
	});
	listener->start();

	Sub::Settings subs;
	sub_.resize(1);
	for(auto &s : sub_) {
		s = std::make_shared<ofx::nng::Sub>();
		s->setup(subs);
		// subscribe specific topic
		// template argument type is which incoming msg will be converted to.
		// see ofxNNGConvertFunctions.h and ofxNNGParseFunctions.h
		// or you can add your convert/parse functionalities in ofx::nng::util namespace.
		s->subscribe<ofBuffer>("connection:", [](const std::string &topic, const ofBuffer &message) {
			ofLogNotice("sub connected") << "topic:" << topic << ", message:" << message.getText();
		});
		// you can subscribe all message by passing empty topic
		// but the argument 'topic' also will be empty and it comes with 'message', so if you want to separate them you need some external rules.
		s->subscribe<std::string>("", [](const std::string &topic, const std::string &message) {
			ofLogNotice("sub received") << "topic:" << topic << ", message:" << message;
		});
		// topic don't have to be string.
		char topic_ch = 'a';
		s->subscribe<std::string>(&topic_ch, sizeof(char), [](const void *topic, const std::string &message) {
			ofLogNotice("sub integer topic") << "topic:" << *static_cast<const char*>(topic) << ", message:" << message;
		});
		s->createDialer("inproc://test")->start();
	}
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if(key == 'a') {
		char topic = 'a';
		pub_.send(&topic, sizeof(char), "key a pressed");
	}
	else {
		pub_.send("key pressed:", ofToString((char)key));
	}
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
