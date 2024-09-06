#include "model.hpp"
#include "proc.hpp"
#include "../logger/logger.hpp"
#include "../window/window.hpp"
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <sstream>

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
	float largest_vertex_val = 0.f;
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
				largest_vertex_val = (abs(v.raw[i]) > largest_vertex_val) ? abs(v.raw[i]) : largest_vertex_val;			}
			vertices.push_back(v);
			log(DEBUG2, "\n" + v.to_string());
		}
		else if (!t.compare("vn"))
		{
			//vertex texture
			Vec3f v;
			for (int i = 0; i < 2; i++)
			{
				//assume only u-v, default w to zero
				s >> v.raw[i];
			}
			vert_normals.push_back(v);
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

					while (!(t[end] == '\\' || t[end] == '/') && end != t.size())
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
	this->normalize_verts(largest_vertex_val);

	//do post processing pass on faces to split simple polygons into triangles
	this->process_faces();

	//get face normals from vertex normals
	this->add_normals();

	//normalize all normals to 1
	for (int i = 0; i < vert_normals.size(); i++)
	{
		vert_normals[i] = vert_normals[i].norm();
	}
	for (int i = 0; i < face_normals.size(); i++)
	{
		face_normals[i] = face_normals[i].norm();
	}

	//center vertices
	Vec3f center = { 0.f, 0.f, 0.f };
	for (auto v : vertices)
	{
		center = center + v;
	}
	center = center / vertices.size();
	Vec3f diff = Vec3f(0.f, 0.f, 0.f) - center;
	for (int i = 0; i < vertices.size(); i++)
	{
		vertices[i] = vertices[i] + diff;
	}

	//default color to white
	color = WHITE;

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
* Setter for color of model
* @param color: hex value of color to set
*/
void Model::set_color(COLOR color)
{
	this->color = color;
}
/**
* Getter for color of model
* @return: hex value of model color
*/
COLOR Model::get_color() const
{
	return color;
}

/**
* Getter for vertices
* @return vertices of model
*/
std::vector<Vec3f>& Model::get_vertices()
{
	return vertices;
}
/**
* Getter for texture uv coords
* @return texture uv coords
*/
std::vector<Vec3f>& Model::get_textures()
{
	return textures;
}
/**
* Getter for vertex normals
* @return normals of model
*/
std::vector<Vec3f>& Model::get_vert_normals()
{
	return vert_normals;
}
/**
* Getter for face normals
* @return normals of model
*/
std::vector<Vec3f>& Model::get_face_normals()
{
	return face_normals;
}
/**
* Getter for faces
* @return faces of model
*/
std::vector<std::vector<Vec3i>>& Model::get_faces()
{
	return faces;
}
/***********************************************************************************************************************
* Private functions
***********************************************************************************************************************/

/**
* Normalize all vertices to [-1, 1]
* @param largest: largest vertex value
* @param smallest: smallest vertex value
*/
void Model::normalize_verts(float largest)
{
	log(DEBUG1, "Normalizing vertices");
	for (int i = 0; i < vertices.size(); i++)
	{
		vertices[i] = vertices[i]/largest;
		log(DEBUG1, "\n" + vertices[i].to_string());
	}
}

/**
* Add normal vectors to model to allow for lighting
*/
void Model::add_normals()
{
	log(DEBUG1, "adding normals");
	if (vert_normals.size() == 0)
	{
		log(WARNING, "Incorrect mapping of normals to vertices");
		//get center of model
		Vec3f center = Vec3f(0.f, 0.f, 0.f);
		for (auto v : vertices)
			center = center + v;
		center = center / (float)vertices.size();

		//add normals(we assume no concavity, thus we approach this with a very greedy solution)
		//provide normals in obj file if you dont want this problem
		for (int i = 0; i < faces.size(); i++)
		{
			//each normal index is the same as the vertex index
			Vec3f A = vertices[faces[i][0].i_vert];
			Vec3f B = vertices[faces[i][1].i_vert];
			Vec3f C = vertices[faces[i][2].i_vert];

			//get the center of the face
			Vec3f face_center = ((A + B + C)/3.f) - center;
			
			//use the vector from the center to the face as reference point to determine if cross product
			//	on the face is pointing outward from center, if angle greater than 90 degrees, use other cross product
			Vec3f norm = (B - A).cross(C - A);
			norm = (norm.dot(face_center) >= 0.f) ? norm : (C - A).cross(B - A);
			face_normals.push_back(norm.norm());

			//want to add the normal value of the face to each vertex normal to find total normal
			for (int j = 0; j < faces[i].size(); j++)
			{
				faces[i][j].i_norm = vert_normals.size();
				vert_normals.push_back(face_normals[i]);
			}
		}
	}
	else
	{
		//find face normal by adding all normals of each vertex
		face_normals.resize(faces.size());
		for (int i = 0; i < faces.size(); i++)
		{
			Vec3f A = vert_normals[faces[i][0].i_norm];
			Vec3f B = vert_normals[faces[i][1].i_norm];
			Vec3f C = vert_normals[faces[i][2].i_norm];

			face_normals[i] = (A + B + C).norm();
		}
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
		int init_size = (int)faces[i].size();
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
				int b = (a == 0) ? (int)faces[i].size() - 1 : a - 1;
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