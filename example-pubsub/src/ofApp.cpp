#include "ofApp.h"
#include "ofxNNGMessage.h"

using namespace ofxNNG;

//--------------------------------------------------------------
void ofApp::setup(){
	pub_.setup();
	pub_.createListener("inproc://test")->start();

	sub_.resize(4);
	for(auto &s : sub_) {
		s = std::make_shared<ofxNNG::Sub>();
		s->setup();
		// subscribe specific topic
		// template argument type is which incoming msg will be converted to.
		s->subscribe<std::string>("str", [](const std::string &message) {
			ofLogNotice("string message") << message;
		});
		// you can also get topic as a callback argument
		s->subscribe<ofFloatColor>("color", [](const std::string &topic, const ofFloatColor &color) {
			ofLogNotice("color message") <<  color;
		});
		// topics could be anything other than string.
		uint8_t topic_ch[] = {0,1,2,3};
		s->subscribe<std::string>(topic_ch, sizeof(topic_ch), [](const ofBuffer &topic, const std::string &message) {
			ofLogNotice("sub integer topic") << "topic:" << topic.getText() << ", message:" << message;
		});
		// you can subscribe all message by passing empty topic
		// but ofxNNG can't separate 'topic' from whole message, so if you want to separate them you need some external rules.
//		s->subscribe<std::string>([](const std::string &message) {
//			ofLogNotice("all topic") << "message:" << message;
//		});
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
