/*********************************************************
 dataCrystalsApp.cpp
 class for Data Crystals
 
 Created by Scott Kildall
 July 2, 2017
 
 **********************************************************/


#include "dataCrystalsApp.h"
#include "ofxCsv.h"

#define POINT_X_COLUMN_NUM (2)
#define POINT_Y_COLUMN_NUM (3)

#define CLUSTER_DRAW_X  (50)


//--------------------------------------------------------------
void dataCrystalsApp::setup(){
    //-- INSTANCE VARS
    data = NULL;
    numData  = 0;
    bHideGui = false;
    bClustering = false;
    bShowClusterStatus = true;
    bDrawClusterIDs = false;
    numClusterCycles = 0;
    numChildren = 0;
    numParents = 0;
    nextClusterID = 1;

    gravCenter.x = 0;
    gravCenter.y = 0;
    gravCenter.z = 0;
    
    gravRatio = .9;
    
    //-- GUI
    drawFont.loadFont("verdana.ttf",14 );
    initGui();
    
    //-- DATA
    loadCSVFiles();
    
    
}

//--------------------------------------------------------------
void dataCrystalsApp::update(){
    
    ofBackground(0, 0, 0);
    ofShowCursor();
}

//--------------------------------------------------------------
void dataCrystalsApp::draw(){
    ofShowCursor();
    
    ofSetColor(255,255,255);
    cam.begin();
    
    
    if( bClustering) {
        
        makeClusters();
        findGravCenter();
        
        for( unsigned long i = 0; i < numData; i++ ) {
            //-- move unattached and leaders
            if( (data+i)->isChild() == true && (data+i)->getParent() == NULL )
                ;   // cout << "no parent\n";
            else if( (data+i)->isChild() == false )
                (data+i)->jiggle(jigglePct,gravCenter, gravRatio );
        }
        
        numClusterCycles++;
       
        
        //--
        if( numUnattached == 0 && numParents == 1 )
            bClustering =  false;
        
        //-- step-by-step test
        //bClustering = false;
    }
    
    countParentsAndChildren();
    
    
    for( unsigned long i = 0; i < numData; i++ ) {
        (data +i)->draw();
    }

    
    if( bDrawClusterIDs) {
        for( unsigned long i = 0; i < numData; i++ ) {
            if( (data +i)->isTopLevel() ) {
                ofVec3f v;
                (data+i)->getLoc(v);
                sprintf(clusterString, "%lu", (unsigned long)(data+i)->getClusterID());
                
                v.x += 20;
                v.y += 20;
                v.z += 20;
                
                ofDrawBitmapString(clusterString, v );
            }
        }
    }
    
    if( data )
        
    cam.end();
    
    if( !bHideGui )
        gui.draw();
    
    if( bShowClusterStatus )
        drawClusterStatus();
    
    
    ofShowCursor();
}

//-- go through all and check to see if:
//-- (1) any unattached to be added to a cluster
//-- (2) any cluster collision [more complicated]

//-------------------------------------------------------------------------------------------------
//
//  Case scenarios, main loop comparing each datum (i) to another datum (j)
//  (1) is self, i == j, continue loop
//
//  (2) is part of the same cluster
//          (a) is parent (or grandparent)
//          (b) is child (or grandchild)
//          **(c) optimize this by using a cluster id instead of a traversal each time??
//
//
//  (3)
//
//-------------------------------------------------------------------------------------------------
void dataCrystalsApp::makeClusters() {
    for( unsigned long i = 0; i < numData; i++ ) {
        //-- this is unattached, so we can check all
        for( unsigned long j = 0; j < numData; j++ ) {
            if( i == j )
                continue;   // skip self
            
            if( inSameCluster(data+i, data+j) )
                continue;
            
            if( inClusterDistance(data+i, data+j) ) {
                bindClusters( data+i, data+j );
                break;  // done with this one, exit loop
            }
            
            
            //-- end for(j loop)
        }
       
        //-- end main loop (for i loop_
    }
}

//-- when two are in the same cluster distance and have been cross-checked
void dataCrystalsApp::bindClusters( datum *d1, datum *d2) {
    //-- this two countParentsAndChildren() are just in for debugging purposes
   // countParentsAndChildren();
   
    datum * topParent = d1->getTopParent();         // top node of Dd1
    
    attachToCluster(topParent, d2);
    //countParentsAndChildren();
}

//-- attach two clusters, the main cluster will contain the parent and keep its cluster ID
void dataCrystalsApp::attachToCluster(datum *subCluster, datum *mainCluster) {
    //-- unattached, give it a new cluster ID
    if( mainCluster->getClusterID() == 0 ) {
        mainCluster->setClusterID(nextClusterID);
        nextClusterID++;
    }

    subCluster->setClusterID(mainCluster->getClusterID());
    mainCluster->addChild(subCluster);
    subCluster->setParent(mainCluster);
 //   cout << "exit\n";
 

}


//-- only count clustered ones
void dataCrystalsApp::findGravCenter() {
    unsigned long numClustered = 0;
    
    gravCenter.x = 0;
    gravCenter.y = 0;
    gravCenter.z = 0;
    
    ofVec3f tv;
    for( unsigned long i = 0; i < numData; i++ ) {
        if( (data+i)->isUnattached() )
            continue;
        
        (data+i)->getLoc(tv);
        
        gravCenter += tv;
        numClustered++;
        
        //cout << tv << "\n";
    }
    
    if( numClustered > 0 )
        gravCenter /= numClustered;
    
    //cout << "grav center: " << gravCenter << "\n";
}

bool dataCrystalsApp::inSameCluster( datum *d1, datum *d2 ) {
    if( d1->getClusterID() == 0 || d2->getClusterID() == 0)
        return false;
    
    return (d1->getClusterID() == d2->getClusterID());
}

bool dataCrystalsApp::inClusterDistance( datum *d1, datum *d2 ) {
    ofVec3f v1, v2;
    
    d1->getLoc(v1);
    d2->getLoc(v2);
    
    if( v1.distance(v2)  < 10.0 )
        return true;
    else
        return false;
}

void dataCrystalsApp::countParentsAndChildren() {
    numChildren = 0;       // instance var, num children
    numParents = 0;       // instance car, num parents
    
    //cout << "\n-- count --\n";
    
    //-- count parents & children
    for( unsigned long i = 0; i < numData; i++ ) {
        if( (data+i)->isChild() ) {
            numChildren++;
            
      //      cout << "child: " << (data+i)->id << " [" << (data+i)->getParent()->id << "]" << "\n";
            
            
        }
        else if( (data+i)->hasChildren() ) {
            numParents++;
        //     cout << "parent: " << (data+i)->id << "\n";
            
        }
    }
    
    numUnattached = numData - (numParents + numChildren);

}
void dataCrystalsApp::drawClusterStatus() {
    ofSetColor(0,255,0);
    makeClusterDisplayStrings();
    
    int drawY = ofGetScreenHeight() - 200;
    ofDrawBitmapString(numClusterCyclesStr, ofPoint(CLUSTER_DRAW_X, drawY) );
    
    drawY+= 20;
    ofDrawBitmapString(numUnattachedStr, ofPoint(CLUSTER_DRAW_X, drawY) );
    
    drawY+= 20;
    ofDrawBitmapString(numParentsString, ofPoint(CLUSTER_DRAW_X, drawY) );
    
    drawY+= 20;
    ofDrawBitmapString(numChildrenString, ofPoint(CLUSTER_DRAW_X, drawY) );

    drawY+= 20;
    ofDrawBitmapString(numDataString, ofPoint(CLUSTER_DRAW_X, drawY) );
}




void dataCrystalsApp::makeClusterDisplayStrings()
{
    sprintf(numUnattachedStr, "num unattached = %lu", numUnattached);
    sprintf(numClusterCyclesStr, "cycles = %lu", numClusterCycles);
    sprintf(numParentsString, "num parents = %lu", numParents);
    sprintf(numChildrenString, "num children = %lu", numChildren);
    sprintf(numDataString, "num data = %lu", numData);
}


//--------------------------------------------------------------
void dataCrystalsApp::keyPressed(int key){
    if( key == ' ' ) {
        //ofToggleFullscreen();
        bHideGui = !bHideGui;
    }
    else if( key == 's' ) {
        saveMesh();
    }
    
    else if( key == '.' ) {
        // next CSV file
        currentFileIndex++;
        if( currentFileIndex == numCSVFiles )
            currentFileIndex = 0;
        
        loadCSVData(csvFiles[currentFileIndex].getFileName(), NULL );
        
    }
    else if( key == ',' ) {
        // previous CSV file
        currentFileIndex--;
        if( currentFileIndex == -1 )
            currentFileIndex = numCSVFiles - 1;
        
        loadCSVData(csvFiles[currentFileIndex].getFileName(), NULL );
    }
    
    else if( key == 'a' ) {
        loadAllData();
    }
    else if( key == '1' ) {
        bClustering = !bClustering;
        
        // can't cluster if we are done
        if( bClustering && numUnattached == 0 && numParents == 1 )
            bClustering = false;
    }
    else if( key == '9') {
        bDrawClusterIDs = !bDrawClusterIDs;
    }
}


/*
 Load each CSV file here, or cycle to the next one
 */
void dataCrystalsApp::loadCSVFiles() {
    //-- load files into vector array
    ofDirectory dir(ofToDataPath("input"));
    numCSVFiles = dir.listDir();
    dir.sort();
    cout << "num CSV files = " << numCSVFiles << endl;
    cout << "file list:"  << endl;
    
    csvFiles = dir.getFiles();
    
    for(int i=0; i< numCSVFiles; ++i)
        cout <<  "Index Num: " << i << " — Filename: " << csvFiles[i].getFileName() << endl;
    
    //-- load first CSV - will crash if we have no CSV files
    currentFileIndex = 0;
    loadCSVData(csvFiles[currentFileIndex].getFileName(), NULL);
}



unsigned long dataCrystalsApp::loadCSVData(string filename, datum *dataPtr) {
    float latTotal = 0;
    float lngTotal = 0;
    float heightTotal = 0;
    
    loadedFilename = filename;

    // Generate pathname into CSV diretor and load into an ofxCsv object, expects a TAB-delimted file,
    // with LF breaks
    wng::ofxCsv csv;
    string path = ofToDataPath("input/");
    path.append(filename);
    csv.loadFile(path, ",");
    
    //-- skip header
    unsigned long csvDataRows = csv.numRows - 1;

    // this will allocate a new buffer of data from the current CSV file
    if( dataPtr == NULL ) {
        if( data )
            delete [] data;
    
        numData = csvDataRows;
        data = new datum[numData];
        
        dataPtr = data;
        
    }
    else {
        cout << "non-null\n";
    }
    
    // start at i = 0 to skiip header
    
    float pointX, pointY;
    
    // 1st pass: read in CSV, set raw points
    for( unsigned long i = 1; i < csvDataRows+1; i++ ) {
        
        pointX = ofToFloat(csv.data[i][POINT_X_COLUMN_NUM]);
        pointY = ofToFloat(csv.data[i][POINT_Y_COLUMN_NUM]);
      
        
        // index - 1 for data but [i] for CSV array since we are skipping the header
        (dataPtr+i-1)->setValues(   pointX,
                                    pointY,
                                    0,
                                    xScale/20.0f,
                                    xScale/20.0f,
                                    zScale/5);
        
        
    }
    
    csv.clear();
    
    float xTotal = 0;
    float yTotal = 0;
    float zTotal = 0;
    
    // 2nd pass: calculate averages for x, y and z
    for( unsigned long i = 0; i < csvDataRows; i++ ) {
        xTotal += (dataPtr+i)->getX();
        yTotal += (dataPtr+i)->getY();
        zTotal += (dataPtr+i)->getZ();
    }
    
    float avgX = xTotal/csvDataRows;
    float avgY = yTotal/csvDataRows;
    float avgZ = zTotal/csvDataRows;
    
    // This can be cleaned up once we figure out static operators
    
//    datum::latAvg = latTotal/(numData*2);
//    datum::lonAvg = lngTotal/(numData*2);

    cout << "X avg = " << avgX << "\n";
    cout << "Y avg = " << avgY << "\n";
    cout << "Z avg = " << avgZ << "\n";
    
    // 3nd pass: make adjustments averages for x, y and z
    for( unsigned long i = 0; i < csvDataRows; i++ ) {
        (dataPtr+i)->adjustValues( -avgX, -avgY, -avgZ );
        
        //(data+i)->scaleValues( 1.0/20.0, 1.0/20.0, 1.0/20.0);
        
//        (dataPtr+i)->setClusterPct(.5);
        
        // the ID will be a problem...
        (dataPtr+ i)->id = i;
    }

    // display strings
    loadedFilename = filename;
    numDataPointsStr = makePointsStr(csvDataRows);
    
    return csvDataRows;
}


void dataCrystalsApp::loadAllData() {
    
    // Step 1: count all rows in all CSV files
    numData = 0;
    
    wng::ofxCsv csv;
    
    currentFileIndex = 0;
    for( int i = 0; i < numCSVFiles; i++ ) {
        string path = ofToDataPath("input/");
        path.append(csvFiles[i].getFileName());
        csv.loadFile(path, ",");
    
        
        
        //-- skip header
        numData += (csv.numRows - 1);
        
        cout << "num this data = " << (csv.numRows - 1) << "\n";
    }
    
    // Step 2: allocate the data
    
    
    cout << "num total data = " << numData << "\n";
    if( data )
        delete [] data;
    
    
    data = new datum[numData];
    
    datum *dataPtr = data;
    unsigned long dataOffset = 0;
    unsigned long numCSVRows;
    
    for( int i = 0; i < numCSVFiles; i++ ) {
        // Load each file
        currentFileIndex = i;
        numCSVRows = loadCSVData(csvFiles[currentFileIndex].getFileName(), (dataPtr+dataOffset));
        dataOffset += numCSVRows;
        
        
        //-- Make data adjustmetns on each set
        datum *lastDataPtr = dataPtr + (dataOffset - numCSVRows);
        
        for( int j = 0; j < numCSVRows; j++ ) {
            if( currentFileIndex == 0 )
                (lastDataPtr + j)->setColor(255,0,0);
            else if( currentFileIndex == 1 )
                (lastDataPtr + j)->setColor(0,255,0);
            else if( currentFileIndex == 2 )
                (lastDataPtr + j)->setColor(0,0,255);
            else if( currentFileIndex == 3 )
                (lastDataPtr + j)->setColor(0,255,255);
            
            (lastDataPtr + j)->adjustValues(0,0, currentFileIndex * 50 * zScale);
        }
    }
    
    
}

std::string dataCrystalsApp::makePointsStr(unsigned long value) {
    char buffer[64];
    std::sprintf(buffer, "%lu", value);
    return std::string(buffer);
}


void dataCrystalsApp::saveMesh() {
    ofxSTLExporter stlExporter;

    stlExporter.beginModel("dataCrystal");
    
    for( unsigned long i = 0; i < numData; i++ )
        (data+i)->save(stlExporter);
    
    stlExporter.useASCIIFormat(false); //export as binary
    stlExporter.saveModel(ofToDataPath("outputs/dataCrystal.stl"));
}

float dataCrystalsApp::map(float m, float in_min, float in_max, float out_min, float out_max) {
    return (m - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


//--------------------------------------------------------------
void dataCrystalsApp::keyReleased(int key){

}

//--------------------------------------------------------------
void dataCrystalsApp::mouseMoved(int x, int y ){
    ofShowCursor();

}

//--------------------------------------------------------------
void dataCrystalsApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void dataCrystalsApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void dataCrystalsApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void dataCrystalsApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void dataCrystalsApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void dataCrystalsApp::dragEvent(ofDragInfo dragInfo){

}



void dataCrystalsApp::initGui() {
    xScale = 1.0f;
    yScale = 1.0f;
    zScale = 1.0f;
    gravRatio = .9f;
    jigglePct = .5f;
    
    gui.setup(); // most of the time you don't need a name
    
    
    // more sliders here for draw vars?
    gui.add(gravSlider.setup( "gravity", gravRatio, .1, 1.5 ));
    gui.add(jiggleSlider.setup("jiggle", jigglePct,.1,3.0));
   
    //-- SCALING NOT ACTUALLY WORKING
    gui.add(xScaleSlider.setup( "x scale", xScale, .25, 4 ));
//    gui.add(yScaleSlider.setup( "y scale", yScale, .25, 4 ));
    gui.add(zScaleSlider.setup( "z scale", zScale, .25, 4 ));

    //    //gui.add(applyButton.setup("apply scale" ));
    
    // listeners
    applyButton.addListener(this, &::dataCrystalsApp::applyButtonHit);
    
    gravSlider.addListener(this, &::dataCrystalsApp::gravSliderChanged);
    
    jiggleSlider.addListener(this, &::dataCrystalsApp::jiggleSliderChanged);
    
    xScaleSlider.addListener(this, &::dataCrystalsApp::xScaleChanged);
    yScaleSlider.addListener(this, &::dataCrystalsApp::yScaleChanged);
    zScaleSlider.addListener(this, &::dataCrystalsApp::zScaleChanged);
}


void dataCrystalsApp::gravSliderChanged(float & val){
    gravRatio = val;
}

void dataCrystalsApp::jiggleSliderChanged(float & val){
    jigglePct = val;
}


void dataCrystalsApp::xScaleChanged(float & val){
    xScale = val;
    
    // 3nd pass: make adjustments averages for x, y and z
//    for( unsigned long i = 0; i < numData; i++ ) {
//        (data+i)->scaleValues( xScale, yScale, zScale );
//    }

}


void dataCrystalsApp::yScaleChanged(float & val){
    yScale = val;
}

void dataCrystalsApp::zScaleChanged(float & val){
    zScale = val;
}


void dataCrystalsApp::applyButtonHit() {
    // not sure how to get the button values out here
    //cout << "hit\n";
    
}
