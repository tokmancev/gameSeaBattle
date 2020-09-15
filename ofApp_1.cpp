#include "ofApp.h"

struct Cell {
	int x_l, y_l, x_r, y_r;
	//ofColor color;
}; 

Cell cells[100];

ofImage img;

static int nCell = -1;
//--------------------------------------------------------------
void ofApp::setup(){
	img.load("net.png");
	int h = img.getHeight() / 10;

	for(int i = 0; i < 10; ++i)
		for (int j = 0; j < 10; ++j){
			cells[i * 10 + j].x_l = j * h;
			cells[i * 10 + j].y_l = (i + 1) * h;
			cells[i * 10 + j].x_r = (j + 1) * h;
			cells[i * 10 + j].y_r = i * h;
		}
	
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	const int width = cells[0].x_r - cells[0].x_l;

	img.draw(10, 10);
	if (nCell != -1) {
		ofSetColor(ofColor::yellow);
		ofDrawRectangle(
			10 + cells[nCell].x_l,
			10 + cells[nCell].y_r,
			width,
			width
		);
		ofSetColor(ofColor::white);
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	int x1 = x - 10;
	int y1 = y - 10;
	for (int i = 0; i < 100; ++i)
	{
		if (x1 >= cells[i].x_l && x1 <= cells[i].x_r
			&& y1 <= cells[i].y_l && y1 >= cells[i].y_r)
		{
			nCell = i;
			return;
		}
	}
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