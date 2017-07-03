/*********************************************************
    datum.h
    Data Unit class for Data Crystals

    Created by Scott Kildall
    July 2, 2017

    
    Notes:
    - uses th ofxSTL addons, downloadable from GitHub:
 
    - can be extended to other classes and projects but
        initially built for the Data Crystals app
 
    - currently is a cube
**********************************************************/


#ifndef __datum__
#define __datum__

#include "ofxSTLPrimitive.h"

#define DEFAULT_CUBE_SIZE (10)      // edge of cube, in mm


class datum  {

public:
    datum();
    
    //-- our unique ID number, public var for easier syntax
    unsigned long id;
    
    //-- size of cube (edge)
    void setSize( float _s  );
    float getSize() { return s; }
    
    //-- main draw function
    void draw();
    
    //-- save to STL mesh
    void save(ofxSTLExporter &stlExporter);
    
    //-- accessors for (x,y,z), movement and scale
    void setValues( float _x, float _y, float _z, float xScale, float yScale, float zScale);
    void adjustValues( float xAdjust, float yAdjust, float zAdjust  );
    void scaleValues( float xScale, float yScale, float zScale  );
    
    //-- accessor to change the color (could be moved to the main draw function to optimize)
    void setColor(unsigned short _r, unsigned short _b, unsigned short _g);
    
//-- cluser ID
    unsigned short getClusterID() { return clusterID; }
    void setClusterID(unsigned short _clusterID);        // sets for this one and all of its children
    
    
// calls adjustValues() for random amount on self + followers
    void jiggle(float jigglePct, ofVec3f &gravCenter, float gravRatio);
    
//-- simple accessors
    datum *getParent() { return parent; }
    float getX() { return x; }
    float getY() { return y; }
    float getZ() { return z; }
    void getLoc( ofVec3f &loc );
    
    
//-- child/parent status
    void addChild(datum *newChild);
    void setParent(datum *theParent);
    void removeChild(datum *newChild);
    
    
//-- child/parent accesssor functions
    //-- hasChild is a recursive function, i.e, will check for children and children's children
    bool hasChild(datum *d);
    bool isChild() { return (parent != NULL); }
    bool hasChildren();
    bool isTopLevel() { return (hasChildren() == true && isChild() == false); }
    datum* getTopParent();
    bool isUnattached() { return (isChild() == false && hasChildren() == false); }
    
    
private:
    //-- which cluster group we belong to, for node-traversal optimization
    unsigned short clusterID;
    
    //-- our current (x, y, z)
    float x, y, z;
    
    //-- size of cuve
    float s;
    
    //-- colors (this should be fixed)
    unsigned short r, g, b;
    
    ofxSTLBoxPrimitive *box;
    
    // draws form based on x,y,z variables
    void makeForm(float xScale, float yScale, float zScale );
    
    //-- parent datum, for clustering, we have one parent
    datum *parent;
    
    //-- children, used for clustering
    vector<datum *> children;
};

#endif /* defined(__datum__) */
