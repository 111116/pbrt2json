
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




struct Mesh {
	string file;
	string bsdf;
	mat4f trans;
};
typedef std::vector<Mesh> Obj;

void printobj(std::ostream& out, const Obj& o, mat4f trans = mat4f::unit)
{
	static bool t0 = 0;
	if (o.size()>0) {
		if (!t0) t0=1;
		else out << ",\n";
	}

	bool t = 0;
	for (auto m: o) {
		if (!t) t=1;
		else out << ",";
		out << "{\"type\":\"mesh\",\"file\":\"" << m.file
			<< "\",\"bsdf\":\"" << m.bsdf
			<< "\",\"transform\":" << trans*m.trans << "}";
	}
}

std::unordered_map<string, Obj> objs;

std::vector<mat4f> transforms;

mat4f curtrans() {
	mat4f t = mat4f::unit;
	for (auto a: transforms)
		t = t * a;
	return t;
}



int main(int argc, char* argv[])
{
	if (argc != 3) {
		return 1;
	}
	try
	{
	std::ifstream fin(argv[1]);
	std::ofstream fout(argv[2]);

	string cmd, lastcmd;
	string curobjname;
	Obj curobj;
	int dep = 0;

	fout << "[\n";

	while (lastcmd=cmd, fin >> cmd) {
		// console.log(cmd);
		if (cmd == "") {
			continue;
		}
		if (cmd == "AttributeBegin") {
			dep++;
			transforms.push_back(mat4f::unit);
			continue;
		}
		if (cmd == "AttributeEnd") {
			dep--;
			transforms.pop_back();
			continue;
		}
		if (cmd == "ObjectBegin") {
			curobjname = getString(fin);
			curobj.clear();
			transforms.push_back(mat4f::unit);
			continue;
		}
		if (cmd == "ObjectEnd") {
			objs[curobjname] = curobj;
			transforms.pop_back();
			printobj(fout, curobj);
			continue;
		}
		if (cmd == "Shape") {
			string type = getString(fin);
			string ftype = getString(fin);
			string file = getString(fin);
			// ply to obj
			if (type != "plymesh") throw "s";
			if (ftype != "string filename") throw "s";
			if (file.substr(file.length()-3)!="ply") throw "ply";
			if (file.substr(0,8)!="geometry") throw "geo";
			file = "mesh"+file.substr(8,file.length()-11) + "obj";

			curobj.push_back({file, "default", curtrans()});
			// get attr
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
			printobj(fout, objs[name], curtrans());
			continue;
		}
		if (cmd == "ConcatTransform") {
			mat4f m = getMatrix(fin);
			transforms.back() = m;
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
	fout << "\n]\n";
	}
	catch (const char* err) {
		console.error(err);
		return 1;
	}
	console.info("total",objs.size(), "objs");
}