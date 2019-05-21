#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofx::nng::Pair::Settings pairs;
	pairs.polyamorous_mode = true;
	pairs.allow_callback_from_other_thread=false;
	node0_.setup(pairs, std::function<void(const ofBuffer&, nng_pipe)>([this](const ofBuffer &buffer, nng_pipe pipe) {
		cout << "node0: got message from pipe:" << nng_pipe_id(pipe) << endl;
		cout << buffer.getText() << endl;
		node0_.send("this is reply from node0", pipe);
	}));
	node1_.setup(pairs, std::function<void(const ofBuffer&, nng_pipe)>([](const ofBuffer &buffer, nng_pipe pipe) {
		cout << "node1: got reply from pipe:" << nng_pipe_id(pipe) << endl;
		cout << buffer.getText() << endl;
	}));
	node2_.setup(pairs, std::function<void(const ofBuffer&, nng_pipe)>([](const ofBuffer &buffer, nng_pipe pipe) {
		cout << "node2: got reply from pipe:" << nng_pipe_id(pipe) << endl;
		cout << buffer.getText() << endl;
	}));
	std::string url = "inproc://test";
	node0_.listen(url, false);
	node1_.dial(url, false);
	node2_.dial(url, false);
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
			node1_.send("message from node1");
			break;
		case OF_KEY_RIGHT:
			node2_.send("message from node2");
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
