#include "ofApp.h"

using namespace ofxNNG;
//--------------------------------------------------------------
void ofApp::setup(){
	// setup with a bool function for replying.
	// if it returns true the output arg will be sent to the peer else nothing will be sent.
	// the types of input and output args can be anything that can convert from/to ofxNNG::Message.
	rep_.setup();
	// callback that receives int as request and return std::string as response 
	rep_.setCallback<int, std::string>([](int index, const std::string &message) {
		//response = ofToString(request);
		return std::make_pair(index==' ', ofToString(index)+" "+message);
	});
	// easiest way to start listener
	rep_.createListener("tcp://127.0.0.1:9000")->start();

	Req::Settings reqs;
	reqs.timeout_milliseconds = 1000;
	req_.setup(reqs);
	req_.createDialer("tcp://127.0.0.1:9000")->start();
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	// sending int and receiving string.
	// the types can be anything that can be converted from/to ofxNNG::Message.
	req_.send<std::string>({key, "pressed"}, [](const std::string &response) {
		ofLogNotice("got response") << response;
	});
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
