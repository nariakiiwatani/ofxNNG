#include "ofApp.h"

using namespace ofxNNG;

//--------------------------------------------------------------
void ofApp::setup(){
	// 4 buses make all-to-all mesh connection 
	bus_.resize(4);
	Bus::Settings buss;
	for(int i = 0; i < bus_.size(); ++i) {
		auto &b = bus_[i];
		b = std::make_shared<Bus>();
		b->setup<std::string>(buss, [i](const std::string &message) {
			ofLogNotice("bus "+ofToString(i)+" receive") << message;
		});
		std::string recv_url = "inproc://bus"+ofToString(i);
		b->createListener(recv_url)->start();
		for(int j = i+1; j < bus_.size(); ++j) {
			std::string send_url = "inproc://bus"+ofToString(j);
			b->createDialer(send_url)->start();
		}
	}
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	ofDrawBitmapString("press 1,2,3,4 and see console to know what happens", 10, 14);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	int index = key-'1';
	if(index >= 0 && index < bus_.size()) {
		std::string message = "message from node"+ofToString(index);
		bus_[key-'1']->send(message);
		ofLogNotice("bus "+ofToString(index)+" send") << message;
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
