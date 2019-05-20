#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofx::nng::Pair::Settings pairs;
	pairs.url = "inproc://test";
	pairs.allow_callback_from_other_thread=true;
	node0_.setupAsDialer(pairs, std::function<void(const ofBuffer&)>([](const ofBuffer &buffer) {
		cout << buffer.getText() << endl;
	}));
	node1_.setupAsListener(pairs, std::function<void(const ofBuffer&)>([](const ofBuffer &buffer) {
		cout << buffer.getText() << endl;
	}));
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch(key) {
		case OF_KEY_LEFT:
			node0_.send("message from left");
			break;
		case OF_KEY_RIGHT:
			node1_.send("message from right");
			break;
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
