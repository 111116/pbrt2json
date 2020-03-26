
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include "consolelog.hpp"
#include "math/matfloat.hpp"

using std::string;


bool isString(std::istream& in) {
	while (isspace(in.peek())) in.get();
	if (in.peek() != '"') return false;
	return true;
}

string getString(std::istream& in) {
	while (isspace(in.peek())) in.get();
	if (in.peek() != '"') throw "strerr";
	in.get();
	string s;
	while (in.peek() != '"')
		s.push_back(in.get());
	in.get();
	return s;
}

void getArray(std::istream& in) {
	while (isspace(in.peek())) in.get();
	if (in.peek() != '[') throw "arr err";
	in.get();
	while (in.peek() != ']') in.get();
	in.get();
}

mat4f getMatrix(std::istream& in) {
	while (isspace(in.peek())) in.get();
	if (in.peek() != '[') throw "mat:wtf1";
	in.get();
	mat4f a;
	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
			in >> a[j][i];
	while (isspace(in.peek())) in.get();
	if (in.peek() != ']') throw "mat:wtf2";
	in.get();
	return a;
}

std::ostream& operator<< (std::ostream& out, const mat4f& m) {
	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
			out << ((i||j)? ",": "[") << m[i][j];
	return out << "]";
}





std::vector<mat4f> transforms;

mat4f curtrans() {
	mat4f t = mat4f::unit;
	for (auto a: transforms)
		t = t * a;
	return t;
}



int main(int argc, char* argv[])
{
	if (argc != 2) {
		return 1;
	}
	try
	{
		std::ifstream fin(argv[1]);

		string cmd;
		fin >> cmd;
		if (cmd != "Transform") throw "fmt";
		mat4f m = getMatrix(fin);
		mat3f R;
		for (int i=0; i<3; ++i)
			for (int j=0; j<3; ++j)
				R[i][j] = m[i][j];
		vec3f T (m[0][3], m[1][3], m[2][3]);

		vec3f pos = transposed(R)*-T;
		vec3f dir = transposed(R)*vec3f(0,0,1);
		vec3f up = normalized(transposed(R)*vec3f(0,1,0));
		vec3f lookat = pos + 1000*dir;

		std::cout << "            \"position\": [" << pos.x << "," << pos.y << "," << pos.z << "],\n";
		std::cout << "            \"look_at\": [" << lookat.x << "," << lookat.y << "," << lookat.z << "],\n";
		std::cout << "            \"up\": [" << up.x << "," << up.y << "," << up.z << "]\n";

	}
	catch (const char* err) {
		console.error(err);
		return 1;
	}
}