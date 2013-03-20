#include <string>

/*struct NODE // Start of structure representing a single bone in a skeleton 
{ 
	 char *name; 
	 double length; // Length of segment along the Y-Axis 
	 double offset[3]; // Transitional offset with respect to the end of the parent link 
	 double euler[3]; // Rotation of base position 
	 double colour[3]; // Colour used when displaying wire frame skeleton 
	 int noofchildren; // Number of child nodes 
	 NODE **children; // Array of pointers to child nodes 
	 NODE *parent; // Back pointer to parent node 
	 double **froset; // Array of offsets for each frame 
	 double **freuler; // Array of angles for each frame 
	 double *scale; // Array of scalefactors for each frame 
	 int DOF; 
	 vector<string> channels;
	 //BYTE DOFs; // Used to determine what DOFs the segment has 
	 //OBJECTINFO* object; // Used to point to a 3D object that is the limb 
	 //CONSTRAINT* constraints; // List of constraints - ordered in time 
}; */

class Node 
{
	public:
		char *name;
		double offset[3];
		int noOfChildren;
		Node** children;
		Node* parent;
		string* channels;
		double pos[3];

};