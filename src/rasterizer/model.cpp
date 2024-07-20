#include "model.h"
#include "../logger/logger.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <sstream>

//helper function to get model
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

//constructor for model object
Model::Model(const char* filename)
{
	log(DEBUG2, "parsing: " + std::string(filename));
	//assume vector memory allocated on heap

	//start reading file
	std::string line;
	std::ifstream fstream(filename, std::ifstream::in);
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
						nf.raw[i] = atoi(val.c_str());
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
}

//destructor for model object
Model::~Model()
{
	//dont actually need to do anything here
	log(DEBUG2, "destroying model");
}

//gets number of vertices in model
int Model::num_verts()
{
	return (int)vertices.size();
}

//gets number of faces in model
int Model::num_faces()
{
	return (int)faces.size();
}

//gets vertex at index
Vec3f Model::get_vert(int index)
{
	return vertices[index];
}

//gets face at index
std::vector<Vec3i> Model::get_face(int index)
{
	return faces[index];
}