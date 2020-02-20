#include "ofApp.h"
#include "ofxNNGMessage.h"

using namespace ofxNNG;

glm::vec2 position;
glm::vec2 position2;
float x,y;

namespace ofxNNG {
	OFX_NNG_ADL_CONVERTER(glm::vec2,x,y);
}

//--------------------------------------------------------------
void ofApp::setup(){
	pub_.setup();
	pub_.createListener("inproc://test")->start();

	sub_.resize(4);
	for(auto &s : sub_) {
		s = std::make_shared<ofxNNG::Sub>();
		s->setup();
		// subscribing specific topic
		// template argument type is which incoming msg will be converted to.
		s->subscribe<std::string>("str", [](const std::string &message) {
			ofLogNotice("str") << message;
		});
		// if you prefer you can get topic as a callback argument.
		// topic argument can be an ofBuffer or a std::string.
		s->subscribe<ofFloatColor>("color", [](const ofFloatColor &color) {
			ofLogNotice("color") << color;
		});
		// topics can be anything other than string.
		// this feature may be used for receiving a kind of raw binary data.
		unsigned char soi[] = {0xFF,0xD8};
		s->subscribe<Message>({soi,2}, [](const Message &msg) {
			ofLogNotice("soi") << "jpg data";
		});
		
		s->subscribe("position", x,y);
		
		// you can subscribe all message by subscribing without topic or with empty topic.
		// but in case a publisher sends a message with non-empty topic, 
		// subscriber can't know how long is the topic the sender expected. 
		// so if you want to separate them you need some external rules.
		//	s->subscribe<ofxNNG::Message>([](const ofxNNG::Message &message) {
		//	});
		
		s->createDialer("inproc://test")->start();
	}
	
	// rough patch for waiting for connection
	ofSleepMillis(300);
	
	pub_.send("str", "string message");
	pub_.send("color", ofFloatColor::red);
	uint8_t binary_topic[] = {0,1,2,3};
	pub_.send(binary_topic, sizeof(binary_topic), "binary topic");
	uint8_t jpeg_data[] = {0xFF, 0xD8, 0,0,0,0,0,0};
	pub_.send(jpeg_data, sizeof(2), jpeg_data);
}

//--------------------------------------------------------------
void ofApp::update(){
	pub_.send("position", glm::vec2{ofGetMouseX(),ofGetMouseY()});
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofDrawCircle(x,y, 5);
	ofDrawCircle(position, 5);
	ofDrawCircle(position2, 5);
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
