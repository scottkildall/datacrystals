/*********************************************************
 main.cpp
 runs the dataCrystalsApp itself, standard OF code
 
 Created by Scott Kildall
 July 2, 2017
 
 **********************************************************/

#include "ofMain.h"
#include "dataCrystalsApp.h"

//========================================================================
int main( ){
	ofSetupOpenGL(DEFAULT_SCREEN_WIDTH,DEFAULT_SCREEN_HEIGHT,OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new dataCrystalsApp());
}
