/*********************************************************
 datum.cpp
 Data Unit class implementation for Data Crystals
 
 Created by Scott Kildall
 July 2, 2017
 
 
 **********************************************************/

#include "datum.h"

#define DEFAULT_CUBE_SIZE (10)      // edge of cube, in mm


datum::datum() {
    box = NULL;
    parent = NULL;
    id = 0;
    clusterID  = 0;
    
    r = 255;
    g = 255;
    b = 255;
    
    s = DEFAULT_CUBE_SIZE;
    
}

void datum::setSize( float _s  ) {
    s = _s;
}


void datum::setValues( float _x, float _y, float _z, float initialScale  ) {
    x = _x;
    y = _y;
    z = _z;
    
    makeForm(initialScale,initialScale,initialScale);
}



void datum::scaleValues( float xScale, float yScale, float zScale  ) {
    if( xScale == 0 || yScale == 0 || zScale == 0) {
        cout << "ERROR datum::scaleValues() has a zero value\n";
        return;
    }
    
    x *= xScale;
    y *= yScale;
    z *= zScale;
    
    ofMesh *m = this->box->getMeshPtr();
    
    // Apply combined rotations
    for( int i = 0; i < m->getNumVertices(); i++ ) {
        ofVec3f v = m->getVertex(i);
        v.x *= xScale;
        v.y *= yScale;
        v.z *= zScale;
        
        m->setVertex(i, v);
    }
    
}

                        
void datum::adjustValues( float xAdjust, float yAdjust, float zAdjust  ) {
    x += xAdjust;
    y += yAdjust;
    z += zAdjust;
    
    ofMesh *m = this->box->getMeshPtr();
    
    // Apply combined rotations
    for( int i = 0; i < m->getNumVertices(); i++ ) {
        ofVec3f v = m->getVertex(i);
        v.x += xAdjust;
        v.y += yAdjust;
        v.z += zAdjust;
        
        m->setVertex(i, v);
    }
    
    for( int i = 0; i < children.size(); i++ ) {
        datum *d = children.at(i);
        d->adjustValues(xAdjust,yAdjust,zAdjust);
    }
}

void datum::jiggle(float jigglePct, ofVec3f &gravCenter, float gravRatio) {
    //-- move self
    float jiggleAmount = s * jigglePct;

    float rxMin = -jiggleAmount;
    float rxMax = jiggleAmount;
    
    
    if( gravCenter.x + x > 0 )
        rxMax = rxMax * gravRatio;
    else if( gravCenter.x + x  < 0 )
        rxMin = rxMin * gravRatio;
    
    
    float ryMin = -jiggleAmount;
    float ryMax = jiggleAmount;
    
    if( gravCenter.y + y > 0 )
        ryMax = ryMax * gravRatio;
    else if( gravCenter.y + y < 0 )
        ryMin = ryMin * gravRatio;
    
    float rzMin = -jiggleAmount;
    float rzMax = jiggleAmount;
    
    if( gravCenter.z  + z > 0 )
        rzMax = rzMax * gravRatio;
    else if( gravCenter.z + z < 0 )
        rzMin = rzMin * gravRatio;

    
    float mx = ofRandom(rxMin, rxMax);
    float my = ofRandom(ryMin, ryMax);
    float mz = ofRandom(rzMin, rzMax);
   
    //-- this will move children
    adjustValues(mx,my,mz);
}

bool datum::hasChildren() {
    return children.size() > 0;
}

void datum::getLoc( ofVec3f &loc ) {
    loc.x = x;
    loc.y = y;
    loc.z = z;
}

void datum::addChild(datum *newChild) {
    //cout << "addChild() Parent ID# " << id << " adding child, ID #" << newChild->id << "\n";
    
    children.push_back(newChild);
}

void datum::setParent(datum *theParent)
{
//    if( theParent != NULL )
//        cout << "setParent() Child ID# " << id << " parent ID #" << theParent->id << "\n";
//    
    parent = theParent;
}

bool datum::hasChild(datum *d) {
    for( int i = 0; i < children.size(); i++ ) {
        datum *ch = children.at(i);
        
        if( d == children.at(i) )
            return true;
        
        // recursive call
        if( ch->hasChild(d) )
            return true;
    }
    
    return false;
}

void datum::removeChild(datum *d) {
    //iterator iter = children.begin();
    vector<datum *> tempChildren;
    
    for( int i = 0; i < children.size(); i++ ) {
        datum *ch = children.at(i);
        
        if( ch == d )
            continue;
        
        tempChildren.push_back(ch);
    }
    
    children.clear();
    for( int i = 0; i < tempChildren.size(); i++ ) {
        datum *ch = tempChildren.at(i);
        
        children.push_back(ch);
    }
}




// sets for this one and all of its children, recursive
void datum::setClusterID(unsigned short _clusterID) {
    clusterID = _clusterID;
    
    for( int i = 0; i < children.size(); i++ ) {
        datum *d = children.at(i);
        d->setClusterID(_clusterID);
    }

}

//-- recursive upward search
datum* datum::getTopParent() {
    if(parent == NULL)
        return this;
    
    return parent->getTopParent();
}


void datum::draw() {
   
    ofSetColor(r,g,b);
//    if( box == NULL )
//        makeForm(xScale, yScale, zScale );
//    
    box->draw();
}

void datum::setColor(int _r, int _b, int _g) {
    r = _r;
    b = _b;
    g = _g;
}

void datum::save(ofxSTLExporter &stlExporter) {
    if( box )
        box->save(stlExporter);
}


void datum::makeForm(float xScale, float yScale, float zScale ) {
    box = new ofxSTLBoxPrimitive;
    
    x *= xScale;
    y *= yScale;
    z *= zScale;
    
    box->set(s, s, s);
    
    
    float px = x;
    float py = y;
    float pz = z;
    
    box->setPosition( px, py, pz );
    box->setUseVbo(true);
}


