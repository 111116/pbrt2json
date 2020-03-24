#include "consolelog.hpp"
#include "math/matfloat.hpp"
#include <fstream>
#include <sstream>
#include <string>

void getString(std::istream& in) {
	if (in.peek() != '"') throw "wtf";
	in.get();
	while (in.peek() != '"') in.get();
	in.get();
}

void getArray(std::istream& in) {
	if (in.peek() != '[') throw "wtf";
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

int main(int argc, char* argv[])
{
	try {
	std::ifstream fin(argv[1]);
	std::string line;
	while (std::getline(fin, line)) {
		std::stringstream in(line);
		while (isspace(in.peek()))
			in.get();
		if (in.peek() == '"') {
			getString(in);
			continue;
		}
		if (in.peek() == '[') {
			getArray(in);
			continue;
		}
		std::string cmd;
		in >> cmd;
		if (cmd == "") {
			continue;
		}
		if (cmd == "AttributeBegin") {
			continue;
		}
		if (cmd == "AttributeEnd") {
			continue;
		}
		if (cmd == "ObjectBegin") {
			continue;
		}
		if (cmd == "ObjectEnd") {
			continue;
		}
		if (cmd == "Shape") {
			continue;
		}
		if (cmd == "Texture") {
			continue;
		}
		if (cmd == "Material") {
			continue;
		}
		if (cmd == "ObjectInstance") {
			continue;
		}
		if (cmd == "ConcatTransform") {
			mat4f m = getMatrix(in);
			continue;
		}
		if (cmd == "Rotate") {
			continue;
		}
		if (cmd == "Scale") {
			continue;
		}
		if (cmd == "Material") {
			continue;
		}
		if (cmd == "MakeNamedMaterial") {
			continue;
		}
		if (cmd == "LightSource") {
			continue;
		}
		console.error("unrecognized cmd",cmd);
	}
	}
	catch (const char* err) {
		console.error(err);
		return 1;
	}
}