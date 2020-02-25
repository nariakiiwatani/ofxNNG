#include "ofApp.h"

using namespace ofxNNG;

//--------------------------------------------------------------
void ofApp::setup(){
	// 4 buses making all-to-all mesh connection 
	bus_.resize(4);
	for(int i = 0; i < bus_.size(); ++i) {
		auto &b = bus_[i];
		b = std::make_shared<Bus>();
		b->setup();
		b->setCallback<std::string, int>([i](const std::string& str, int index) {
			ofLogNotice("node "+ofToString(i)+" receive from "+ofToString(index)) << str;
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
		bus_[key-'1']->send({"message from node "+ofToString(index), index});
		ofLogNotice("from node "+ofToString(index)+" send");
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
