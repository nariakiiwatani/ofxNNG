#include "ofApp.h"

using namespace ofxNNG;

//--------------------------------------------------------------
void ofApp::setup(){
	bus_.resize(4);
	Bus::Settings buss;
	for(int i = 0; i < bus_.size(); ++i) {
		auto &b = bus_[i];
		b = std::make_shared<Bus>();
		b->setup<ofBuffer>(buss, [i](const ofBuffer &buffer) {
			cout << "node" << i << ": receive:" << buffer.getText() << endl;
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

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	int index = key-'1';
	if(index >= 0 && index < bus_.size()) {
		bus_[key-'1']->send("message from node"+ofToString(index));
		cout << "node" << index << ": send" << endl;
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
