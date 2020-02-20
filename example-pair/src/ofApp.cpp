#include "ofApp.h"

std::string node1, node2;
//--------------------------------------------------------------
void ofApp::setup(){
	using std::string;
	using std::cout;
	using std::endl;
	
	ofxNNG::Pair::Settings pairs;
	pairs.polyamorous_mode = true;
	pairs.allow_callback_from_other_thread=false;
	
	// connection diagram; node1 <--> node0 <--> node2
	node0_.setup(pairs);
	node0_.setCallback<string,string>([this](const string &str1, const string &str2, nng_pipe pipe) {
		cout << "node0: got a message from pipe:" << nng_pipe_id(pipe) << endl;
		cout << str1 << endl;
		cout << str2 << endl;
		// node0 is connected to both nodes but it can specify which to send by suggesting a pipe
		node0_.send({"this is a reply" ," from node0"}, false, pipe);
	});
	node1_.setup(pairs);
	node1_.setCallback(node1, node2);
	node2_.setup(pairs);
	node2_.setCallback<string>([](const string &str, nng_pipe pipe) {
		cout << "node2: got a reply from pipe:" << nng_pipe_id(pipe) << endl;
		cout << str << endl;
	});
	string url = "inproc://test";
	node0_.createListener(url)->start();
	node1_.createDialer(url)->start();
	node2_.createDialer(url)->start();
}

//--------------------------------------------------------------
void ofApp::update(){
	cout << node1 << node2 << endl;
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofDrawBitmapString("press left or right and see console to know what happens", 10, 14);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch(key) {
		case OF_KEY_LEFT:
			node1_.send({"this is a message", " from node1"});
			break;
		case OF_KEY_RIGHT:
			node2_.send({"this is a message", " from node2"});
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
