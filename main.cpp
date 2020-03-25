#include "consolelog.hpp"
#include "math/matfloat.hpp"
#include <fstream>
#include <sstream>
#include <string>

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

int main(int argc, char* argv[])
{
	try {
	std::ifstream fin(argv[1]);
	string cmd, lastcmd;
	while (lastcmd=cmd, fin >> cmd) {
		// console.log(cmd);
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
			string type = getString(fin);
			continue;
		}
		if (cmd == "ObjectEnd") {
			continue;
		}
		if (cmd == "Shape") {
			string a = getString(fin);
			string b = getString(fin);
			string c = getString(fin);
			while (isString(fin)) {
				string key = getString(fin);
				getString(fin);
			}
			continue;
		}
		if (cmd == "Texture") {
			string a = getString(fin);
			string b = getString(fin);
			string c = getString(fin);
			while (isString(fin)) {
				string key = getString(fin);
				getArray(fin);
			}
			continue;
		}
		if (cmd == "ObjectInstance") {
			string name = getString(fin);
			continue;
		}
		if (cmd == "ConcatTransform") {
			mat4f m = getMatrix(fin);
			continue;
		}
		if (cmd == "Rotate") {
			float a,b,c,d;
			fin >> a >> b >> c >> d;
			continue;
		}
		if (cmd == "Scale") {
			float a,b,c,d;
			fin >> a >> b >> c;
			continue;
		}
		if (cmd == "Material") {
			string type = getString(fin);
			while (isString(fin)) {
				string key = getString(fin);
				getArray(fin);
			}
			continue;
		}
		if (cmd == "MakeNamedMaterial") {
			string type = getString(fin);
			while (isString(fin)) {
				string key = getString(fin);
				getArray(fin);
			}
			continue;
		}
		if (cmd == "LightSource") {
			string type = getString(fin);
			while (isString(fin)) {
				string key = getString(fin);
				getArray(fin);
			}
			continue;
		}
		console.error("unrecognized cmd", cmd);
		return 1;
	}
	}
	catch (const char* err) {
		console.error(err);
		return 1;
	}
}