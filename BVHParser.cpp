#include <stdio.h> 
#include <string.h> 
#include <math.h> 
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <ctime>
#include "BVHParser.h" 

using namespace std;

BVHParser::BVHParser()
{
}

Animation* BVHParser::getData(const char *filename) 
{
	string line;
	ifstream bvhFile (filename);

	Animation* container = new Animation();

	node *root = new node();
	node *currentnode = root;
	node *parent = NULL;
	
	nodeTransform* rootTrans = new nodeTransform();
	nodeTransform* currentTrans = rootTrans;
	nodeTransform* parentTrans = NULL;

	bool buildingHierarchy = false;
	bool buildingRootnode = false;
	bool buildingJointnode = false;
	bool closingnode = false;

	if (!bvhFile.is_open()){

		cout<<"Error Reading File"<<endl;
		exit (EXIT_FAILURE);
	}


	getline(bvhFile,line);
	boost::trim(line);
	unsigned found = line.find("HIERARCHY");

	if(found != string::npos)	//If HIERARCHY we're dealing with a new set of mocap data
	{
		buildingHierarchy = true;

		found = line.find("ROOT");	//Check if ROOT is in the same line as HIERARCHY
		if(found == string::npos)	//If ROOT does not share the same line as HIERARCHY, get the next line
		{
			getline(bvhFile,line);
			boost::trim(line);
			currentnode->isRootNode = false;
		}else{
			buildingRootnode = true;
			currentnode->isRootNode = true;
		}
	}

	while(buildingHierarchy)
	{
		buildingJointnode = (line.find("JOINT") != string::npos);
		closingnode = (line.find("}") != string::npos);

		if(!buildingRootnode)		//if ROOT wasn't found in the same line as HIERARCHY
		{
			buildingRootnode = (line.find("ROOT") != string::npos);
				  
		}else if(buildingJointnode){
			buildingRootnode = false;
		}

		if(buildingJointnode || buildingRootnode)	
		{
			if(buildingRootnode)
			{
				currentnode->parent = NULL;
				currentTrans->parent = NULL;
			}else
			{
				currentnode = new node();
				currentnode->parent = parent;

				currentTrans = new nodeTransform();
				currentTrans->parent = parentTrans;
			}
				  
			currentnode->children = NULL;
			currentnode->numChildren = 0;

			currentTrans->children = NULL;
			currentTrans->numChildren = 0;

			currentnode->name = getName(line);


			//May be able to assume that next line is curly brace, which means we can get rid of this chunk and replace with getline
			getline(bvhFile,line);
			boost::trim(line);

			found = line.find("{");
			if(found != string::npos)
			{
				getline(bvhFile,line);
				boost::trim(line);
					
			}

			//Set offset of node
			found = line.find("OFFSET");
			if(found != string::npos)	
			{
				currentTrans->trans = getOffset(line);
				currentnode->offset = getOffset(line);
			}

			//Set degrees of freedom
			getline(bvhFile,line);
			boost::trim(line);
			found = line.find("CHANNELS");
			if(found != string::npos)	
			{
				vector<string> channels;
				getChannels(line, channels);
				currentnode->channels = new string[ channels.size() ];

				for( unsigned i(0); i < channels.size(); ++i){
						
					currentnode->channels[i] = channels[i];
				}

				currentnode->DOF = channels.size();
			}

			//Checking if we've reached a leaf node
			getline(bvhFile,line);
			boost::trim(line);
			found = line.find("End Site");
			if(found != string::npos)
			{
				getline(bvhFile,line);	//get beginning bracket
				getline(bvhFile,line);	//get offset
				boost::trim(line);
					
				currentnode->isLeafNode = true;
				found = line.find("OFFSET");
				if(found != string::npos)	
				{
					//add endsite to leaf node
					currentnode->endSite = getOffset(line);
				}
				getline(bvhFile,line);	//get closing bracket for End Site
				boost::trim(line);


				if(line.find("}") != string::npos)
				{
					buildingJointnode = false;
					buildingRootnode = false;
					closingnode = true;
					getline(bvhFile,line);
					boost::trim(line);

				}
				currentTrans->figureNode = currentnode;
			}else{
				currentnode->isLeafNode = false;
					
				parent = currentnode;
				parentTrans = currentTrans;
					
			}

		}else if(closingnode)
		{
			currentTrans->figureNode = currentnode;
			if(parent != NULL)
			{
				addChild(parent, currentnode);
				addChildTransform(parentTrans, currentTrans);
			}
			getline(bvhFile,line);
			boost::trim(line);

			if(line.find("}") != string::npos)
			{
				currentnode = parent;
				parent = parent->parent;

				currentTrans = parentTrans;
				parentTrans = parentTrans->parent;

			}else if(line.find("JOINT") != string::npos)
			{
				closingnode = false;
				buildingJointnode = true;
			}else if(line.find("MOTION") != string::npos)
			{
				buildingHierarchy = false;
			}

		}
	}

	while(!bvhFile.eof() && found == string::npos)		//iterate through file until we find motion
	{
		getline(bvhFile,line);
		boost::trim(line);
		found = line.find("MOTION");
	}

	getline(bvhFile,line);
	boost::trim(line);

	if(line.find("Frames") !=string::npos)
	{
		container->numFrames = getFrameCount(line);
		initializeHistory(currentnode, container->numFrames);
		container->motionVector.resize( container->numFrames );

		getline(bvhFile,line);
		boost::trim(line);
	}

	if(line.find("Frame Time") !=string::npos)
	{
		container->frameRate = getFrameTime(line);
	}

	container->figure = currentnode;

	int frameCount = 0;

	container->figure->startingPos = new vec3f[ container->numFrames ];
	while( !bvhFile.eof() && frameCount<container->numFrames){

		float x, y, z;
		bvhFile >> x;
		bvhFile >> y;
		bvhFile >> z;

		container->figure->startingPos[ frameCount ][0] = x;
		container->figure->startingPos[ frameCount ][1] = y;
		container->figure->startingPos[ frameCount ][2] = z;

		currentTrans = new nodeTransform();
		addRotation(currentTrans, container->figure, bvhFile, frameCount);
		container->motionVector[ frameCount ] = currentTrans;

		frameCount++;
	}

	bvhFile.close();

	return container;
}

string BVHParser::getName( const string& s)
{
	vector <string> nameData;
	boost::split(nameData, s, boost::is_any_of("\t "), boost::token_compress_on);

	while(*nameData.begin() == "")
	{
		nameData.erase(nameData.begin());		//any empty space in vector
	}

	nameData.erase(nameData.begin());		//Removes "JOINT" or "ROOT"
	if(nameData[0].length()==0)
	{
		cout<<"Error: No name found"<<endl;
		exit (EXIT_FAILURE);
	}
	return nameData[0];
}

void BVHParser::getChannels( const string& s, vector<string>& channelData)
{
	boost::split(channelData,s, boost::is_any_of("\t "),boost::token_compress_on);

	while(*channelData.begin() == "")
	{
		channelData.erase(channelData.begin());		//any empty space in vector
	}

	channelData.erase(channelData.begin());		//Removes "CHANNEL"
	channelData.erase(channelData.begin());		//Removes Dof
}

vec3f BVHParser::getOffset( const string& s)
{
	vector <string> offsetData;
	boost::split(offsetData, s, boost::is_any_of("\t "), boost::token_compress_on);
	
	while(*offsetData.begin() == "")
	{
		offsetData.erase(offsetData.begin());		//any empty space in vector
	}

	offsetData.erase(offsetData.begin());		//Removes "OFFSET"

	//float offset[3];						//change to float?  delete later?
	vec3f offset((float) atof(offsetData[0].c_str()), (float) atof(offsetData[1].c_str()), (float) atof(offsetData[2].c_str()));
	int ctr = 0;
	string tmp;
	
	bool offsetError = (offsetData[0].length()==0) || (offsetData[1].length()==0) || (offsetData[2].length()==0);

	if(offsetError)
	{
		cout << "Error: Parsing offset" << endl;
		exit (EXIT_FAILURE);
	}

	return offset;
}

void BVHParser::addChild(node* parent, node* currentnode)
{
	parent->numChildren++;
	if(parent->children == NULL)
	{
		parent->children = new node*[1];
		parent->children[0] = currentnode;
		
	}else{

		node** temp = new node*[ parent->numChildren ];

		for( unsigned i(0); i < ( parent->numChildren - 1); ++i){
			temp[i] = parent->children[i];
		}
		temp[ parent->numChildren - 1 ] = currentnode;

		delete[] parent->children;
		parent->children = temp;
	}
	
}

void BVHParser::addChildTransform(nodeTransform* parentTrans, nodeTransform* currentTrans)
{
	parentTrans->numChildren++;
	if(parentTrans->children == NULL)
	{
		parentTrans->children = new nodeTransform*[1];
		parentTrans->children[0] = currentTrans;
		
	}else{

		nodeTransform** temp = new nodeTransform*[ parentTrans->numChildren ];

		for( unsigned i(0); i < ( parentTrans->numChildren - 1); ++i){
			temp[i] = parentTrans->children[i];
		}
		temp[ parentTrans->numChildren - 1 ] = currentTrans;

		delete[] parentTrans->children;
		parentTrans->children = temp;
	}
	
}

unsigned BVHParser::getFrameCount( const string& s)
{
	vector <string> frameData;
	boost::split(frameData,s, boost::is_any_of("\t "),boost::token_compress_on);
	unsigned frames = 0;

	frameData.erase(frameData.begin());		//Removes "Frame:"

	if(frameData[0].length()==0)
	{
		cout << "Error: Parsing # of frames" << endl;
		exit (EXIT_FAILURE);
	}


	frames = (unsigned) atof((*frameData.begin()).c_str());
	return frames;
}

float BVHParser::getFrameTime( const string& s)
{
	vector <string> frameTimeData;
	boost::split(frameTimeData,s, boost::is_any_of("\t "),boost::token_compress_on);
	float frameTime = 0;

	frameTimeData.erase(frameTimeData.begin());		//Removes "Frame"
	frameTimeData.erase(frameTimeData.begin());		//Removes "Time:"

	if(frameTimeData[0].length()==0)
	{
		cout << "Error: Parsing frame time" << endl;
		exit (EXIT_FAILURE);
	}

	frameTime = (float) atof((*frameTimeData.begin()).c_str());

	return frameTime;
}

void BVHParser::getMotions( const string& s, vector<string>& stringData)
{
	boost::split(stringData, s, boost::is_any_of( " \t" ), boost::token_compress_on);
	while(*stringData.begin() == "")
	{
		stringData.erase(stringData.begin());		//any empty space in the beginning of the vector
	}
}

void BVHParser::initializeDOF(node* n, int frames)
{
}

void BVHParser::initializeHistory(node* n, int frames)
{
	vec3f* hist = new vec3f[ frames ];
	n->history = hist;
	for( unsigned i = 0; i<n->numChildren; i++)
	{
		initializeHistory(n->children[i], frames);
	}
}

void BVHParser::initializeRotationData(nodeTransform** n, int frames)
{
	n = new nodeTransform*[ frames ];
	for(int i = 0; i<frames; i++)
	{
		n[i] = new nodeTransform();
	}
}

void BVHParser::addRotation(nodeTransform* n, node* figureNode, ifstream& file, int frame){
	float x, y, z;
	file >> x;
	file >> y;
	file >> z;

	n->rotation		= vec3f(x,y,z);
	n->figureNode	= figureNode;
	n->numChildren	= figureNode->numChildren;
	n->children		= new nodeTransform*[ n->numChildren ];
		
	for( unsigned i = 0; i < figureNode->numChildren; i++)
	{
		n->children[i] = new nodeTransform();
		n->children[i]->parent = n;
		addRotation(n->children[i], figureNode->children[i], file, frame);	//hopefully this won't blow up
	}
}

BVHParser::~BVHParser()
{
}