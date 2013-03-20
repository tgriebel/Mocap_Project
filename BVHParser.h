#include <vector>
#include <string.h>
#include <map>
#include <list>
#include <deque>
#include "MVector.h"
#include "Quaternion.h"

using namespace std;

#ifndef ___BVHParser___
#define ___BVHParser___

struct node
{
	unsigned numChildren; // Number of child nodes
	node* parent; // Back pointer to parent node
	node** children; // Array of pointers to child nodes

	unsigned DOF;
	string name;		//for some reason assignment fails
	//char* name;
	//char* name = (char*)malloc(sizeof(char));
	string* channels;

	vec3f pos;
	vec3f offset;			// offset ZYX
	vec3f endSite;
	vec3f* history;			//holds previous pos in order to draw streamers
	vec3f* startingPos;

	bool isLeafNode;
	bool isRootNode;

	node()
	:	numChildren(0),
		parent( NULL ),
		children( NULL ),
		DOF(0),
		name(""),
		channels( NULL ),
		history( NULL ),
		startingPos( NULL ),
		isLeafNode( false ),
		isRootNode( false )
	{}

	~node(){
		delete[] children;
		delete[] channels;
		delete[] history;
		delete[] startingPos;
	}

	node( const node& n ){	//copies anything pertaining to a single note and note the topology
		
		numChildren = n.numChildren; // Number of child nodes
		parent = NULL;
		
		children = new node*[ n.numChildren ];
		for( unsigned i(0); i < n.numChildren; ++i){
			children[i] = NULL;
		}

		DOF = n.DOF;
		name = n.name;	
		channels = new string[ n.DOF ];
		for( unsigned i(0); i < n.DOF; ++i){
			channels[i] = n.channels[i];
		}
		
		pos = n.pos;
		offset = n.offset;			// offset ZYX
		endSite = n.endSite;
		startingPos = NULL;

		isLeafNode = n.isLeafNode;
		isRootNode = n.isRootNode;
	}
};

struct nodeTransform{
	//int DOF;

	nodeTransform()
	:	numChildren(0),
		parent( NULL ),
		children( NULL ), 
		figureNode( NULL ),
		trans(),
		rotation(),
		X( 1.0, 0.0, 0.0),
		Y( 0.0, 1.0, 0.0),
		Z( 0.0, 0.0, 1.0)
	{}
	~nodeTransform(){
	
		delete[] children;
	}

	nodeTransform( const nodeTransform& nT){	//copies anything pertaining to a single note and note the topology
	
		parent = NULL;

		children = new nodeTransform*[ nT.numChildren ];
		for( unsigned i(0); i < nT.numChildren; ++i){
			children[i] = NULL;
		}

		numChildren = nT.numChildren;
		figureNode = nT.figureNode;
		trans = nT.trans;
		rotation = nT.rotation;

		X = nT.X;
		Y = nT.Y;
		Z = nT.Z;
	}

	nodeTransform* parent;
	nodeTransform** children;
	unsigned numChildren; // Number of child nodes

	node* figureNode;

	vec3f trans; // offset ZYX
	vec3f rotation; // Rotation of base position: XYZ

	vec3f X, Y, Z;
};

struct Pose{

	vector< vec3f > pts;
	vector< pair< unsigned, unsigned> > lines;

	vector< pair< vec3f, vec3f > > vlines;
};

struct marker{

	marker( const vec3f& v, marker* p){

		pt = v;
		next = NULL;
		prev = p;
	}

	vec3f pt;

	marker *next, *prev;
};

struct streamer{

	const static unsigned maxSize = 40;
	unsigned size;
	bool on;

	streamer(): size(0), on(false), head(NULL), tail(NULL) {}

	~streamer(){

		marker* p = head;
	
		while( p != NULL ){
			
			marker* tmp = p;
			delete p;
			p = tmp->prev;
		}
	}

	void insert( const vec3f& mark){
		
		if( size == 0){
			tail = new marker( mark, NULL );
			head = tail;
			size++;
		}else if( size == (maxSize - 1) ){
			
			marker* tmp = tail;
			tail = tail->next;
			delete tmp;
			tail->prev = NULL;

			head = new marker( mark, head );
			head->prev->next = head;
		}else{

			head = new marker( mark, head );
			head->prev->next = head;
			size++;
		}
	}

	marker *head, *tail;
};

extern void updateRot( node& n, nodeTransform& nT, float theta, const vec3d& ori, vec3d& axis);
struct Animation
{
	node* figure;
	vector<nodeTransform*> motionVector;

	float frameRate;
	unsigned numFrames;
	unsigned currentFrame;

	bool playing;

public:

	Animation()
	:	currentFrame(0),
		numFrames(0),
		frameRate(0.0f),
		playing(false)
	{}
	~Animation(){

		_destroyPose();

		for( int i(0); i < motionVector.size(); ++i){
		
			destroyFrame( i );
		}
	}

	void destroyFrame( unsigned frame ){

		deque< nodeTransform* > workList;

		workList.push_back( motionVector[ frame ] );

		while( !workList.empty() ){

			nodeTransform* nT = workList.front();
			workList.pop_front();

			for( int i(0); i < nT->numChildren; ++i){
			
				workList.push_back( nT->children[i] );
			}

			delete nT;
		}

		motionVector.erase( motionVector.begin() + frame);
	}

	void play(){
		playing = true;
	}

	void pause(){
		playing = false;
	}

	void getStreamerList( node* n, vector< vec3f >& list)
	{
		marker* m = streamers[n].head;

		while( m != NULL){
		
			list.push_back( m->pt );
			m = m->prev;
		}
	}

	void computePos( Pose& pose, vec3f& trans){

		figure->offset = figure->startingPos[ currentFrame ] + trans;

		_resetPos( *figure, *motionVector[currentFrame] );

		//updateRot( *figure, *motionVector[ currentFrame ], motionVector[ currentFrame ]->rotation[0], figure->offset[0], motionVector[ currentFrame ]->Z);
		//updateRot( *figure, *motionVector[ currentFrame ], motionVector[ currentFrame ]->rotation[1], figure->offset[1], motionVector[ currentFrame ]->X);
		//updateRot( *figure, *motionVector[ currentFrame ], motionVector[ currentFrame ]->rotation[2], figure->offset[2], motionVector[ currentFrame ]->Y);

		//_computePos(pose, *figure, *motionVector[currentFrame], trans, 0);

		Quaternion<float> Q( 0.0, vec3f( 0.0, 0.0, 0.0) );
		_computePos( *figure, *motionVector[ currentFrame] , figure->offset, Q);
	}

	void addStreamers(){
		
		_addStreamers( figure );
	}

	void nextFrame(){

		if(currentFrame < (numFrames) - 1){
			currentFrame++;
		}else{
			currentFrame = 0;
		}
	}

private:

	map< node*, streamer > streamers;

	void _destroyPose()
	{
		deque< node* > workList;

		workList.push_back( figure );

		while( !workList.empty() ){

			node* n = workList.front();
			workList.pop_front();

			for( int i(0); i < n->numChildren; ++i){
			
				workList.push_back( n->children[i] );
			}

			delete n;
		}
	}

	void _resetPos( node& n, nodeTransform& nT)
	{
		nT.X[0] = 1.f;	nT.X[1] = 0.f;	nT.X[2] = 0.f;
		nT.Y[0] = 0.f;	nT.Y[1] = 1.f;	nT.Y[2] = 0.f;
		nT.Z[0] = 0.f;	nT.Z[1] = 0.f;	nT.Z[2] = 1.f;

		n.pos.zero();

		if(n.parent != NULL)
			n.pos = n.offset + n.parent->pos;
		else
			n.pos = n.offset;

		for( unsigned i(0); i < (n.numChildren); ++i){
		
			_resetPos( *n.children[i], *nT.children[i]);
		}
	}

	void _addStreamers( node* n){
	
		pair< map< node*, streamer>::iterator, bool> ret = streamers.insert( pair< node*, streamer >( n, streamer()) );

		ret.first->second.insert( n->pos );

		for( int i(0); i < n->numChildren; ++i){
			_addStreamers( n->children[i] );
		}
	}


	void _computePos( node& n, nodeTransform& nT, vec3f T, const Quaternion<float>& Q){

		if( n.name == "rshin" ){
		
			cout << "har" << endl;
		}

		for(int i(0); i < (n.numChildren); ++i){

			node& child = *n.children[i];

			Quaternion<float> X( nT.rotation[1], vec3d( 1.0, 0.0, 0.0) );
			Quaternion<float> Y( nT.rotation[2], vec3d( 0.0, 1.0, 0.0) );
			Quaternion<float> Z( nT.rotation[0], vec3d( 0.0, 0.0, 1.0) );

			Z = Z.normalize();
			X = X.normalize();
			Y = Y.normalize();

			vec3d p( child.offset[0], child.offset[1], child.offset[2] );

			Quaternion<float> R = Y*X*Z*Q;

			rotate( R, p);

			child.pos = p + T;
	
			_computePos( *n.children[i], *nT.children[i], child.pos, R);
		}
	}

	/*
	void _computePos( Pose& pose, node& n, nodeTransform& nT, vec3f trans, unsigned pt_idx){

		Quaternion<double> X, Y, Z, P, Xp, Yp, Zp;

		vec3f p1( n.pos );

		//pose.pts.push_back( pt );
		//pose.lines.push_back( pair<unsigned, unsigned>( pt_idx, pose.pts.size() - 1) );
		//pose.vlines.push_back( pair< vec3f, vec3f >( trans, pt ) );

		for( unsigned i(0); i < (n.numChildren); ++i){

			node& child					= *n.children[i];
			nodeTransform& transform	= *nT.children[i];
		
			vec3f p2( child.pos );
			updateRot( child, transform, transform.rotation[0], p2, nT.Z);
			updateRot( child, transform, transform.rotation[1], p2, nT.X);
			updateRot( child, transform, transform.rotation[2], p2, nT.Y);

			_computePos( pose, *n.children[i],  *nT.children[i], p2, pose.pts.size() - 1);
		}
	}*/
};

class BVHParser 
{
	public:
		BVHParser();
		//RootNode* getFigure(const char *filename);
		Animation* getData(const char *filename);
		bool ExportData(const char *filename);
		~BVHParser();

	private:
		int xpos, ypos, zpos;
		string getName( const string& s);
		void getChannels( const string& s, vector<string>& channelData);
		//float* getOffset( const string& s);
		vec3f getOffset( const string& s);
		void addChild(node* parent, node* currentNode);
		void addChildTransform(nodeTransform* parentTrans, nodeTransform* currentTrans);

		unsigned getFrameCount( const string& s);
		float getFrameTime( const string& s);
		void getMotions( const string& s, vector<string>& stringData);
		void addRotation(nodeTransform* n, node* figureNode, ifstream& file, int frame);
		void initializeDOF(node* n, int frames);
		void initializeHistory(node* n, int frames);
		void initializeRotationData(nodeTransform** n, int frames);
};

#endif