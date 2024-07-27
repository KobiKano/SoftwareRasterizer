#include "model.h"
#include "../logger/logger.h"
#include "../window/window.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <sstream>

/**
* Gets filepath of model from user, then parses that file
* @return: pointer to new model object
*/
std::unique_ptr<Model> get_model()
{
	std::string in;
	std::cout << "Input FILENAME of model -(src/Models/{FILENAME})-, or press enter to use default\n";
	in = std::cin.get();
	std::string path = "Models/" + in;
	std::ifstream f(path.c_str());
	if (in[0] == '\n' || !f.good())
	{
		log(WARNING, "Invalid path, defaulting to cube");
		path = "Models/cube.obj";
	}
	else {
		path = path.substr(0, path.size() - 1);
		f.close();
	}
	
	return std::unique_ptr<Model>(new Model(path.c_str()));
}

/**
* Constructor for Model
* Parses wavefront obj file into vertices, faces, and normals
* @param filename: Filepath of obj file
*/
Model::Model(const char* filename)
{
	log(DEBUG2, "parsing: " + std::string(filename));
	//assume vector memory allocated on heap

	//start reading file
	std::string line;
	std::ifstream fstream(filename, std::ifstream::in);
	float largest_vertex_val = -10000;
	float smallest_vertex_val = 10000;
	while (std::getline(fstream, line))
	{
		std::istringstream s(line);
		std::string t;
		s >> t;
		if (!t.compare("v"))
		{
			//vertex coords
			Vec3f v;
			for (int i = 0; i < 3; i++)
			{
				s >> v.raw[i];
				largest_vertex_val = (v.raw[i] > largest_vertex_val) ? v.raw[i] : largest_vertex_val;
				smallest_vertex_val = (v.raw[i] < smallest_vertex_val) ? v.raw[i] : smallest_vertex_val;
			}
			vertices.push_back(v);
			log(DEBUG2, "\n" + v.to_string());
		}
		else if (!t.compare("vt"))
		{
			//vertex texture
			Vec3f v;
			for (int i = 0; i < 2; i++)
			{
				//assume only u-v, default w to zero
				s >> v.raw[i];
			}
			textures.push_back(v);
			log(DEBUG2, "\n" + v.to_string());
		}
		else if (!t.compare("vn"))
		{
			//vertex normal
			Vec3f v;
			for (int i = 0; i < 3; i++)
			{
				s >> v.raw[i];
			}
			normals.push_back(v);
			log(DEBUG2, "\n" + v.to_string());
		}
		else if (!t.compare("f"))
		{
			//faces
			std::vector<Vec3i> f;

			//need to parse based on /'s
			//no / means plain vertex face
			//one / means vertex texture normal
			//two /'s means vertex normal pair
			//lots of nesting to come, I know its ugly 
			//		but I am too lazy to write extra functions that will only be called once :(

			while (s >> t)
			{
				std::string val;
			int start = 0;
			int end = 0;
				Vec3i nf(-1,-1,-1); //-1 to indicate empty
				for (int i = 0; i < 3; i++)
				{
					if (t[start] == '\0')
						break;

					while (t[end] != '\\' && end != t.size())
					{
						//find delim
						end++;
					}
					if (end == 0)
					{
						//no vertex index given, cant form face...
						log(ERR, "invalid file args");
						throw std::invalid_argument("invalid file args");
					}
					if (end != start)
					{
						//add
						val = t.substr(start, end-start);
						nf.raw[i] = atoi(val.c_str()) - 1; //offset by 1 because wavefront indexes from 1 instead of 0
					}
					end = (t[end] == '\0') ? end : end + 1;
					start = end;
				}
				f.push_back(nf);
				log(DEBUG2, "\n" + nf.to_string());
			}
			faces.push_back(f);
		}
		else
		{
			//ignore
			log(DEBUG2, "ignoring: " + t);
		}
	}
	fstream.close();

	//do post processing pass on vertices to normalize all coordinates to -1, 1 range
	this->normalize_verts(largest_vertex_val, smallest_vertex_val);

	//do post processing pass on faces to split simple polygons into triangles
	this->process_faces();

	log(DEBUG1, "model creation complete");
}

/**
* Destructor for model
*/
Model::~Model()
{
	//dont actually need to do anything here
	log(DEBUG2, "destroying model");
}

/**
* Draw this model onto the screen
*/
void Model::draw()
{
	//get screen dims
	int width, height;
	get_dims(width, height);

	//iterate through all faces
	for (int i = 0; i < faces.size(); i++)
	{

	}
}

/***********************************************************************************************************************
* Private functions
***********************************************************************************************************************/

/**
* Translate vertex coordinate in [0, 1] to screen coordinates
* @param in: vertex coordinates
* @param width: width of screen
* @param height: height of screen
*/
inline Vec3f Model::world_to_screen(Vec3f in, int width, int height)
{
	//(0, 0) is the center of the screen
	return Vec3f(in.x * width, in.y * height, in.z);
}

/**
* Normalize all vertices to [0, 1]
* @param largest: largest vertex value
* @param smallest: smallest vertex value
*/
void Model::normalize_verts(float largest, float smallest)
{
	log(DEBUG1, "Normalizing vertices");
	for (int i = 0; i < vertices.size(); i++)
	{
		vertices[i] = (vertices[i] - smallest)/(largest - smallest);
		log(DEBUG1, "\n" + vertices[i].to_string());
	}
}

/*
* Private helper to determine whether a point is within a triangle
* Consider a point on the edge of a triangle to not be within the triangle
* Uses barycentric coordinates to find ratios of 3 subtraingles creates by point and vertices of triangle
* Not too concerned with performance since this is done only in pre-processing before rendering
* @param t: triangle(group of 3 vertices)
* @param point: point to check
* @return: true if in triangle, false otherwise
*/
static bool in_traingle(Triangle t, Vec3f point)
{
	//first need to determine if point is on same plane as traingle
	Vec3f u = t.B - t.A;
	Vec3f v = t.C - t.A;
	Vec3f n = u.cross(v);
	Vec3f w = point - t.A;
	//plane normal(n) and vector between point and some vertex of t should be perpendicular if on same plane
	if (w.dot(n) != 0)
		return false;

	//solution by: W. Heidrich, Journal of Graphics, GPU, and Game Tools,Volume 10, Issue 3, 2005
	//finding relative areas of subtriangles made by point and vertices in triangle to determine if they add up to total area
	float gamma = (u.cross(w).dot(n))/(n.dot(n));
	float beta = (w.cross(v).dot(n)) / (n.dot(n));
	float alpha = 1 - gamma - beta;

	//if all alpha, beta, and gamma values are within bounds [0,1], then point is in triangle
	if (alpha >= 0 && alpha <= 1 && beta >= 0 && beta <= 1 && gamma >= 0 && gamma <= 1)
		return true;

	//default return on failiure
	return false;
}

/*
* Checks if point is a valid ear
* I.E it is convex with regard to the overall polygon
* @param v: vertex to check
* @return: true if valid, false otherwise
*/
bool Model::is_valid_ear(Triangle t, int i, int a, int b, int c, Vec3f center)
{
	//check if A is convex with regard to B and C
	Vec3f AB = t.B - t.A;
	Vec3f AC = t.C - t.A;
	Vec3f ACenter = center - t.A;

	//check if angle between center and AB or AC is greater than 90
	float theta_center_B = acos(ACenter.dot(AB) / (ACenter.value() * AB.value()));
	float theta_center_C = acos(ACenter.dot(AC) / (ACenter.value() * AC.value()));
	float error = 0.00000001f; //use error just in case of floating point rounding problems
	if (theta_center_B >= ((PI / 2) - error) || theta_center_C >= ((PI / 2) - error))
		return false; //not convex, can't be valid

	//make sure no other points in face are within this triangle
	for (int j = 0; j < faces[i].size(); j++)
	{
		if (j == a || j == b || j == c)
			continue;
		if (!in_traingle(t, vertices[faces[i][j].i_vert]))
		{
			return true;
		}
	}
	//default return on fail
	return false;
}

/**
* Given a face of n vertices that is a simple polygon, split into multiple triangular faces
* Uses the Ear Clipping method since its a lot more simple even though it is a O(n^2) algo
*/
void Model::process_faces()
{
	log(DEBUG1, "processing faces");
	std::vector<std::vector<Vec3i>> new_faces;
	for (int i = 0; i < faces.size(); i++)
	{
		//make sure only have faces with 3 or above vertices
		if (faces[i].size() < 3)
		{
			log(WARNING, "This is not a face, this is a line...");
			//ignore
			continue;
		}
		//if face only has 3 faces, then we don't need to do anything either
		if (faces[i].size() == 3)
		{
			log(DEBUG1, "Already triangle, skipping");
			new_faces.push_back(faces[i]);
			continue;
		}

		//ear clipping algorithm, we are assuming that we are rendering a simple polygon
		//i.e for each set of vertices we will be assuming that the face is made up of non-overlapping lines on that set
		
		int num_added = 0;
		int init_size = faces[i].size();
		bool invalid = false;
		while (num_added < init_size - 3)
		{
			bool found = false;

			//get center of polygon
			Vec3f center;
			for (int j = 0; j < faces[i].size(); j++)
			{
				center = center + vertices[faces[i][j].i_vert];
			}
			center = center / (float)faces[i].size();

			//start by selecting first point as ear to test, we are going to do this very naively in a greedy style algorithm
			for (int a = 0; a < faces[i].size(); a++)
			{
				Vec3f A, B, C;

				//assume faces vertices populated in order, i.e. vertex A is in index 1, then B is in index 0, and C in index 2
				int b = (a == 0) ? faces[i].size() - 1 : a - 1;
				int c = (a == faces[i].size() - 1) ? 0 : a + 1;

				A = vertices[faces[i][a].i_vert];
				B = vertices[faces[i][b].i_vert];
				C = vertices[faces[i][c].i_vert];

				//make sure A is a valid ear
				found = this->is_valid_ear(Triangle(A, B, C), i, a, b, c, center);

				//if found, add triangle to new faces, then break loop
				if (found)
				{
					//add new face
					std::vector<Vec3i> face;
					face.push_back(faces[i][a]);
					face.push_back(faces[i][b]);
					face.push_back(faces[i][c]);
					new_faces.push_back(face);

					//remove vertex from old faces
					faces[i].erase(std::next(faces[i].begin(), a));

					num_added++;
					break;
				}
			}

			//if nothing found there is an error, skip this
			if (!found)
			{
				log(ERR, "No ear found on face, ignoring");
				invalid = true;
				break;
			}
		}

		//add last 3 vertices as final face if valid
		if (!invalid)
		{
			//add new face
			std::vector<Vec3i> face;
			face.push_back(faces[i][0]);
			face.push_back(faces[i][1]);
			face.push_back(faces[i][2]);
			new_faces.push_back(face);
		}
	}

	//last action is to reassign faces class field, explicitly free all memory associated with original faces
	faces.clear();
	faces.swap(new_faces);
}