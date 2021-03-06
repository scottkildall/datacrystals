/*********************************************************
 dataCrystalsApp.cpp
 class for Data Crystals
 
 Created by Scott Kildall
 July 2, 2017
 
 **********************************************************/


#include "dataCrystalsApp.h"
#include "ofxCsv.h"

#define CATEGORY_TYPE_COLUMN_NUM (1)
#define POINT_X_COLUMN_NUM (2)
#define POINT_Y_COLUMN_NUM (3)
#define SIZE_COLUMN_NUM (4)


#define CLUSTER_DRAW_X  (20)            // offset from left of screen
#define CLUSTER_DRAW_Y (200)            // offset from bottom of screen
#define CLUSTER_DRAW_Y_INCREMENT (18)   // amount between each line

//--------------------------------------------------------------
void dataCrystalsApp::setup(){
    //-- INSTANCE VARS
    data = NULL;
    numData  = 0;
    bHideGui = false;
    bClustering = false;
    bShowClusterStatus = true;
    bDrawClusterIDs = false;
    bUseColor = true;
    bAllLoaded = false;
    bUseSizeColumn = false;
    numClusterCycles = 0;
    numChildren = 0;
    numParents = 0;
    nextClusterID = 1;

    gravCenter.x = 0;
    gravCenter.y = 0;
    gravCenter.z = 0;
    
    gravRatio = .9;
    
    maxUnattachedSize = DEFAULT_CUBE_SIZE;
    
    //-- GUI
    drawFont.loadFont("verdana.ttf",14 );
    initGui();
    
    //-- DATA
    minDataCategory = 1;
    maxDataCategory = 10;
    dataCategory = minDataCategory;
    generateTreeString();
    
    loadCSVFiles();
    
    //-- display strings
    formGUIStrings();
    
    //-- go full screen
    ofToggleFullscreen();
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
                (data+i)->jiggle(jigglePct, maxUnattachedSize, gravCenter, gravRatio );
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
        if( (data+i)->visible )
            (data+i)->draw();
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
        if( (data+i)->visible == false )
            continue;
        
        //-- this is unattached, so we can check all
        for( unsigned long j = 0; j < numData; j++ ) {
            if( (data+j)->visible == false)
                continue;
        
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
    
    /*
    float totalSize = d1->getSize() + d2->getSize();
    float minClusterDist = (totalSize/2) * clusterPct;
    
    float shortestSize = (d1->getSize() < d2->getSize()) ? d1->getSize() : d2->getSize();
    if( shortestSize < 3 && (d1->getSize() != d2->getSize()) )
        cout << "\n";
    
    minClusterDist = shortestSize * clusterPct;
     */
    
    float minClusterDist = DEFAULT_CUBE_SIZE * clusterPct;
    
    
    d1->getLoc(v1);
    d2->getLoc(v2);
    
    if( v1.distance(v2)  < minClusterDist)
        return true;
    else
        return false;
}

void dataCrystalsApp::countParentsAndChildren() {
    numChildren = 0;       // instance var, num children
    numParents = 0;       // instance car, num parents
    
    maxUnattachedSize = 0;
    
    if( maxUnattachedSize == 0 )
        maxUnattachedSize = 2;
    
    //cout << "\n-- count --\n";
    
    //-- count parents & children
    for( unsigned long i = 0; i < numData; i++ ) {
        if( (data+i)->visible == false )
            continue;
        
        if( (data+i)->isUnattached() )
            maxUnattachedSize = ((data+i)->getSize() > maxUnattachedSize ) ? (data+i)->getSize() : maxUnattachedSize;
        
        if( (data+i)->isChild() ) {
            numChildren++;
            
      //      cout << "child: " << (data+i)->id << " [" << (data+i)->getParent()->id << "]" << "\n";
            
            
        }
        else if( (data+i)->hasChildren() ) {
            numParents++;
        //     cout << "parent: " << (data+i)->id << "\n";
            
        }
    
    }
    
    
    numUnattached = numVisible - (numParents + numChildren);

    // OLD
    //numUnattached = numData - (numParents + numChildren);

}
void dataCrystalsApp::drawClusterStatus() {
    ofSetColor(0,255,0);
    makeClusterDisplayStrings();
    
    int drawY = ofGetScreenHeight() - CLUSTER_DRAW_Y;
    ofDrawBitmapString(numClusterCyclesStr, ofPoint(CLUSTER_DRAW_X, drawY) );
    
    drawY+= CLUSTER_DRAW_Y_INCREMENT;
    ofDrawBitmapString(numVisibleString, ofPoint(CLUSTER_DRAW_X, drawY) );
    
    drawY+= CLUSTER_DRAW_Y_INCREMENT;
    ofDrawBitmapString(numUnattachedStr, ofPoint(CLUSTER_DRAW_X, drawY) );
    
    drawY+= CLUSTER_DRAW_Y_INCREMENT;
    ofDrawBitmapString(numParentsString, ofPoint(CLUSTER_DRAW_X, drawY) );
    
    drawY+= CLUSTER_DRAW_Y_INCREMENT;
    ofDrawBitmapString(numChildrenString, ofPoint(CLUSTER_DRAW_X, drawY) );

    drawY+= CLUSTER_DRAW_Y_INCREMENT;
    ofDrawBitmapString(numDataString, ofPoint(CLUSTER_DRAW_X, drawY) );
    
    drawY += CLUSTER_DRAW_Y_INCREMENT;
    ofDrawBitmapString(maxUnattachedSizeString, ofPoint(CLUSTER_DRAW_X, drawY) );

    drawY+= CLUSTER_DRAW_Y_INCREMENT;
    ofDrawBitmapString(sizeOnString, ofPoint(CLUSTER_DRAW_X, drawY) );
    
    ofSetColor(255,255,255);
    drawY+= CLUSTER_DRAW_Y_INCREMENT;
    ofDrawBitmapString(treeDisplayStr, ofPoint(CLUSTER_DRAW_X, drawY) );
}




void dataCrystalsApp::makeClusterDisplayStrings()
{
    sprintf(numVisibleString, "num visible = %lu", numVisible);
    sprintf(numUnattachedStr, "num unattached = %lu", numUnattached);
    sprintf(numClusterCyclesStr, "cycles = %lu", numClusterCycles);
    sprintf(numParentsString, "num parents = %lu", numParents);
    sprintf(numChildrenString, "num children = %lu", numChildren);
    sprintf(numDataString, "num data = %lu", numData);
    sprintf(maxUnattachedSizeString, "max unnatached size = %d", maxUnattachedSize);
}

void dataCrystalsApp::formGUIStrings() {
    if( bUseSizeColumn)
        strcpy(sizeOnString, "use size column = true");
    else
        strcpy(sizeOnString, "use size column = false");
    
    strcpy( fileDisplayStr, "\n");
    //(char *)csvFiles[currentFileIndex].getFileName() );
    
}

void dataCrystalsApp::generateTreeString() {
    if( bAllLoaded ) {
        strcpy( treeDisplayStr, "All" );
        return;
    }
    
    switch(dataCategory) {
        case 1:
            strcpy( treeDisplayStr, "Apple" );
            break;
        case 2:
            strcpy( treeDisplayStr, "Birch" );
            break;
        case 3:
            strcpy( treeDisplayStr, "Cherry" );
            break;
        case 4:
            strcpy( treeDisplayStr, "Cypress" );
            break;
        case 5:
            strcpy( treeDisplayStr, "Elm" );
            break;
        case 6:
            strcpy( treeDisplayStr, "Maple" );
            break;
        case 7:
            strcpy( treeDisplayStr, "Oak" );
            break;
        case 8:
            strcpy( treeDisplayStr, "Pine" );
            break;
        case 9:
            strcpy( treeDisplayStr, "Poplar" );
            break;
        case 10:
            strcpy( treeDisplayStr, "Willow" );
            break;
        default:
            strcpy( treeDisplayStr, "Other" );
            break;
    }
}


//--------------------------------------------------------------
void dataCrystalsApp::keyPressed(int key){
    if( key == 'g' ) {
        bHideGui = !bHideGui;
        bShowClusterStatus = !bShowClusterStatus;
    }
    else if( key == 'f') {
        ofToggleFullscreen();
    }
    else if( key == 's' ) {
        saveMesh();
    }
    
    else if( key == 'c' ) {
        bUseColor = !bUseColor;
        
        if( bAllLoaded && bUseColor )
            applyColorToAll();
        else
            applyColor();
    }
    
    //-- scroll data by data
    if( key == '1' ) {
        bClustering = false;
        
        bAllLoaded = false;
        
        // previous set of trees
        dataCategory--;
        if(dataCategory < minDataCategory)
            dataCategory = maxDataCategory;
        
        generateTreeString();
        
        // this will reload the entire data file, resetting the positions of everythng
        loadCSVFiles();
        
    }
    else if( key == '2' ) {
        bClustering = false;
        
        bAllLoaded = false;
        
        // next set of trees
        dataCategory++;
        if(dataCategory > maxDataCategory)
            dataCategory = minDataCategory;
        
        generateTreeString();
        
        // this will reload the entire data file, resetting the positions of everything
        loadCSVFiles();
    }
    
    else if( key == 'a' ) {
        bClustering = false;
        bAllLoaded = true;
        
        // this will reload the entire data file, resetting the positions of everything
        loadCSVFiles();
        
        generateTreeString();
    }

    
    //-- old file-indexing code
    /*
    else if( key == '2' ) {
        // next CSV file
        currentFileIndex++;
        if( currentFileIndex == numCSVFiles )
            currentFileIndex = 0;
        
        loadCSVData(csvFiles[currentFileIndex].getFileName(), NULL, currentFileIndex );
        
        applyColor();
        bAllLoaded = false;
        
    }
    else if( key == '1' ) {
        // previous CSV file
        currentFileIndex--;
        if( currentFileIndex == -1 )
            currentFileIndex = numCSVFiles - 1;
        
        loadCSVData(csvFiles[currentFileIndex].getFileName(), NULL, currentFileIndex );
        
        applyColor();
        bAllLoaded = false;
    }
    
    //-- not supported now
    
    else if( key == 'a' ) {
        loadAllData();
        bAllLoaded = true;
    }
    */
    
    else if( key == 'r' ) {
        if( bAllLoaded )
            loadAllData();
        else
            loadCSVData(csvFiles[currentFileIndex].getFileName(), NULL, currentFileIndex );
        
        if( bAllLoaded && bUseColor )
            applyColorToAll();
        else
            applyColor();
    }
    
    //-- not supported now
    /*
    else if( key == 'z' ) {
        bUseSizeColumn = !bUseSizeColumn;
        
        if( bAllLoaded )
            loadAllData();
        else
            loadCSVData(csvFiles[currentFileIndex].getFileName(), NULL, currentFileIndex );
        
        if( bAllLoaded && bUseColor )
            applyColorToAll();
        else
            applyColor();
        
        formGUIStrings();
    }
    */
    
    else if( key == ' ' ) {
        bClustering = !bClustering;
        
        // can't cluster if we are done
        if( bClustering && numUnattached == 0 && numParents == 1 )
            bClustering = false;
    }
//    else if( key == '9') {
//        bDrawClusterIDs = !bDrawClusterIDs;
//    }
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
    loadCSVData(csvFiles[currentFileIndex].getFileName(), NULL, currentFileIndex);
    //applyColor();
}



unsigned long dataCrystalsApp::loadCSVData(string filename, datum *dataPtr, int fileIndex) {
    float latTotal = 0;
    float lngTotal = 0;
    float heightTotal = 0;
    numVisible = 0;
    
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
    
    // start at i = 0 to skip header

    float pointX, pointY, pointZ, s;
    int categoryID;
    unsigned short r,g,b;
    
    // 1st pass: read in CSV, set raw points
    for( unsigned long i = 1; i < csvDataRows+1; i++ ) {
        
        categoryID = ofToInt(csv.data[i][CATEGORY_TYPE_COLUMN_NUM]);
        pointX = ofToFloat(csv.data[i][POINT_X_COLUMN_NUM]);
        pointY = ofToFloat(csv.data[i][POINT_Y_COLUMN_NUM]);
        
        /*
        if( bUseSizeColumn ) {
            s = ofToFloat(csv.data[i][SIZE_COLUMN_NUM]);
            if( s == 0 )
                s = DEFAULT_CUBE_SIZE;
            
            (dataPtr+i-1)->setSize(s);
        }
        */
        
        (dataPtr+i-1)->setCategoryType(categoryID);
        
        //cout << "category id = " << categoryID << "\n";
        
        if( bAllLoaded )
            pointZ = categoryID * 1000;
        else
            pointZ = 0;
        
        // index - 1 for data but [i] for CSV array since we are skipping the header
        (dataPtr+i-1)->setValues(   pointX,
                                    pointY,
                                    pointZ,
                                    xScale/20.0f,
                                    xScale/20.0f,
                                    zScale/20.f);
        
        //-- use categoryIDs instead of colors
        getColorFromFileIndex(categoryID,r,g,b);
        (dataPtr+i-1)->setColor(r,g,b);
        
        //-- turn off visibilty of those not in category
        if( bAllLoaded == false  ) {
            if( categoryID == dataCategory ) {
                (dataPtr+i-1)->visible = true;
                numVisible++;
            }
            else {
                (dataPtr+i-1)->visible = false;
            }
        }
        else {
            (dataPtr+i-1)->visible = true;
            numVisible++;
        }
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
        numCSVRows = loadCSVData(csvFiles[currentFileIndex].getFileName(), (dataPtr+dataOffset), currentFileIndex);
        dataOffset += numCSVRows;
        
        
        //-- Make data adjustmetns on each set
        datum *lastDataPtr = dataPtr + (dataOffset - numCSVRows);
        
        unsigned short r, g, b;
        
        getColorFromFileIndex(currentFileIndex,r,g,b);
        
        for( int j = 0; j < numCSVRows; j++ ) {
            (lastDataPtr + j)->setColor(r,g,b);
            
            (lastDataPtr + j)->adjustValues(0,0, currentFileIndex * 50 * zScale);
        }
    }
}

void dataCrystalsApp::applyColorToAll() {
    // Step 1: count all rows in all CSV files
    unsigned long startIndex = 0;
    unsigned long endIndex = 0;
    unsigned long numCSVRows;
    datum *dataPtr = data;
    
    wng::ofxCsv csv;
    
    unsigned short r, g, b;
    
    currentFileIndex = 0;
    for( int i = 0; i < numCSVFiles; i++ ) {
        string path = ofToDataPath("input/");
        path.append(csvFiles[i].getFileName());
        csv.loadFile(path, ",");
        
        //-- skip header
        numCSVRows = (csv.numRows - 1);

        endIndex = startIndex + numCSVRows;
        
        getColorFromFileIndex(i,r,g,b);
        
        for( int j = startIndex; j < endIndex; j++ ) {
            (dataPtr + j)->setColor(r,g,b);
        }
        
        startIndex = endIndex;
    }
}

//-- more complicated, apply color to each section
void dataCrystalsApp::getColorFromFileIndex(int fileIndex, unsigned short &r, unsigned short &b, unsigned short &g) {
    //-- default all to white
    r = 255; g = 255; b = 255;
    
    if( fileIndex == 0 ) {
        // Arenas
        r = 255; g = 0; b = 0;
    }
    else if( fileIndex == 1 ) {
        // Tennis Courts
        r = 255; g = 255; b = 0;
    }
    else if( fileIndex == 2 ) {
        // Community Gardens
        r = 0; g = 255; b = 0;
    }
    else if( fileIndex == 3 ) {
        // Washrooms
        r = 255; g = 255; b = 255;
    }
    else if( fileIndex == 4 ) {
        // Pools
        r = 0; g = 0; b= 255;
    }
    else if( fileIndex == 5) {
        // Spray & Wading Pools
        r = 255; g = 0; b= 128;
    }
    else if( fileIndex == 6) {
        // Play Strucures
        r = 128; g = 255; b= 255;
    }
    else if( fileIndex == 7) {
        // Swing Swets
        r = 255; g = 128; b= 128;
    }
    else if( fileIndex == 8) {
        // Community Centers
        r = 128; g = 0; b= 160;
    }
    else if( fileIndex == 9) {
        // Skateboard parks
        r = 45; g = 250; b= 170;
    }
    else if( fileIndex == 10) {
        // Football fields
        r = 71; g = 128; b= 241;
    }
    else {
        r = 255; g = 255; b = 255;
    }
}


//-- goes through all data points, applies color (or unapplies) from the current index
void dataCrystalsApp::applyColor() {
    unsigned short r = 255;
    unsigned short g = 255;
    unsigned short b = 255;
    
    if( bUseColor )
        getColorFromFileIndex(currentFileIndex,r,g,b);
    
    for( int i = 0; i < numData; i++ ) {
        (data + i)->setColor(r,g,b);
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
    
    for( unsigned long i = 0; i < numData; i++ ) {
        if( (data+i)->visible )
            (data+i)->save(stlExporter);
    }
    
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
    clusterPct = .8;
    
    gui.setup(); // most of the time you don't need a name
    
    
    // more sliders here for draw vars?
    gui.add(gravSlider.setup( "gravity", gravRatio, .001, 1.5 ));
    gui.add(jiggleSlider.setup("jiggle", jigglePct,.1,3.0));
   
    //-- SCALING NOT ACTUALLY WORKING
    gui.add(xScaleSlider.setup( "xy scale", xScale, .25, 4 ));
//    gui.add(yScaleSlider.setup( "y scale", yScale, .25, 4 ));
    gui.add(zScaleSlider.setup( "z scale", zScale, .25, 4 ));
    gui.add(clusterPctSlider.setup( "cluster %", clusterPct, .1, .9 ));

    //    //gui.add(applyButton.setup("apply scale" ));
    
    // listeners
    applyButton.addListener(this, &::dataCrystalsApp::applyButtonHit);
    
    gravSlider.addListener(this, &::dataCrystalsApp::gravSliderChanged);
    
    jiggleSlider.addListener(this, &::dataCrystalsApp::jiggleSliderChanged);
    clusterPctSlider.addListener(this, &::dataCrystalsApp::clusterPctChanged);
    
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

void dataCrystalsApp::clusterPctChanged(float & val){
    clusterPct = val;
}


void dataCrystalsApp::applyButtonHit() {
    // not sure how to get the button values out here
    //cout << "hit\n";
    
}
