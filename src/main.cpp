
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

void skip(std::istream& in, char c) {
	while (isspace(in.peek())) in.get();
	if (in.peek() != c) throw "skip fail";
	in.get();
}

mat4f getMatrix(std::istream& in) {
	mat4f a;
	skip(in,'[');
	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
			in >> a[j][i];
	skip(in,']');
	return a;
}

std::ostream& operator<< (std::ostream& out, const mat4f& m) {
	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
			out << ((i||j)? ",": "[") << m[i][j];
	return out << "]";
}

// map name to bsdfID
std::unordered_map<string, string> namedMaterial;
std::unordered_map<string, string> textures;


static bool mtlglob_printcomma = 0;

void getMtl(std::istream& in, std::ostream& out, string name, string type="")
{
	if (!mtlglob_printcomma) mtlglob_printcomma = 1;
	else out << ",\n";

	auto passattr = [&](string type, string name) {
		out << ",\"" << name << "\":";
		skip(in,'[');
		if (type == "string") {
			string val = getString(in);
			out << "\"" << val << "\"";
		}
		if (type == "texture") {
			string val = getString(in);
			if (!textures.count(val)) throw "undefined texture";
			out << "\"" << textures[val] << "\"";
		}
		if (type == "namedmaterial") {
			string val = getString(in);
			if (!namedMaterial.count(val)) throw "undefined namedmaterial";
			out << "\"" << namedMaterial[val] << "\"";
		}
		if (type == "float") {
			double val;
			in >> val;
			out << val;
		}
		if (type == "rgb") {
			double a,b,c;
			in >> a >> b >> c;
			out << "[" << a << "," << b << "," << c << "]";
		}
		skip(in,']');
	};

	out << "{\"name\":\"" << name << "\"";
	if (type != "")
		out << ",\"type\":\"" << type << "\"";
	while (isString(in))
	{
		string key = getString(in);
		int spaceindex = key.find(" ");
		if (spaceindex == string::npos) throw "bsdfkey fail";
		string keytype = key.substr(0, spaceindex);
		string keyname = key.substr(spaceindex+1, key.length()-spaceindex-1);

		if (keyname == "type") {
			if (keytype != "string") throw "type mismatch";
			passattr("string","type");
			continue;
		}
		if (keyname == "index") {
			if (keytype != "float") throw "type mismatch";
			passattr("float", "ior");
			continue;
		}
		if (keyname == "roughness") {
			if (keytype != "float") throw "type mismatch";
			passattr("float", "roughness");
			continue;
		}
		if (keyname == "Kd" || keyname == "Ks" || keyname == "Kr" || keyname == "Kt") {
			if (keytype == "rgb")
				passattr("rgb", keyname);
			else if (keytype == "texture")
				passattr("texture", keyname);
			else throw "type mismatch";
			continue;
		}
		if (keyname == "bumpmap") {
			if (keytype != "texture") throw "type mismatch";
			passattr("texture", "bumpmap");
			continue;
		}
		if (keyname == "reflect") {
			if (keytype != "rgb") throw "type mismatch";
			passattr("rgb", "reflect");
			continue;
		}
		if (keyname == "transmit") {
			if (keytype != "rgb") throw "type mismatch";
			passattr("rgb", "transmit");
			continue;
		}
		if (keyname == "namedmaterial1") {
			if (keytype != "string") throw "type mismatch";
			passattr("namedmaterial","material1");
			continue;
		}
		if (keyname == "namedmaterial2") {
			if (keytype != "string") throw "type mismatch";
			passattr("namedmaterial","material2");
			continue;
		}
		if (keyname == "amount") {
			if (keytype != "rgb") throw "type mismatch";
			passattr("rgb", "amount");
			continue;
		}
		console.warn(type,"unrecognized bsdf key", key);
	}
	out << "}";
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
	if (argc != 4) {
		console.info(argv[0], "pbrt out mtlout");
		return 1;
	}
	int mtlcnt = 0;
	try
	{
	std::ifstream fin(argv[1]);
	std::ofstream fout(argv[2]);
	std::ofstream mout(argv[3]);

	string cmd, lastcmd;
	string curobjname;
	Obj curobj;
	string curmtl = "undefined";

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
			printobj(fout, curobj);
			curobj.clear();
			transforms.push_back(mat4f::unit);
			continue;
		}
		if (cmd == "ObjectEnd") {
			if (objs.count(curobjname))
				objs[curobjname].insert(objs[curobjname].end(), curobj.begin(), curobj.end());
			else
				objs[curobjname] = curobj;
			transforms.pop_back();
			curobj.clear();
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

			curobj.push_back({file, curmtl, curtrans()});
			// get attr
			string alphaname;
			while (isString(fin)) {
				string key = getString(fin);
				string attr = getString(fin);
				if (key == "texture alpha") {
					if (!textures.count(attr)) throw "undef alphatex";
					alphaname = attr;
					++mtlcnt;
					string basemtl = curmtl;
					curmtl = std::to_string(1000+mtlcnt);
					curmtl[0] = 'm';
					curobj.back().bsdf = curmtl;
					if (!mtlglob_printcomma) mtlglob_printcomma = 1;
					else mout << ",\n";
					mout << "{\"name\":\"" << curmtl
						<< "\",\"type\":\"transparency\",\"base\":\""
						<< basemtl << "\",\"alpha\":\""
						<< textures[attr] << "\"}";
				}
				else if (key == "texture shadowalpha") {
					if (alphaname != attr) throw "unmatch shadowalpha";
				}
				else throw "unrecognized shape attr";
			}
			continue;
		}
		if (cmd == "Texture") {
			string name = getString(fin);
			string valtype = getString(fin);
			string textype = getString(fin);
			if (textype != "imagemap") throw "unrecognized texture type";
			if (valtype != "spectrum" && valtype != "float") throw "unrecognized texture valtype";
			while (isString(fin)) {
				string key = getString(fin);
				if (key != "string filename") throw "unrecognized texture attr";
				skip(fin, '[');
				std::string file = getString(fin);
				skip(fin, ']');
				textures[name] = file;
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
			++mtlcnt;
			curmtl = std::to_string(1000+mtlcnt);
			curmtl[0] = 'm';
			getMtl(fin, mout, curmtl, type);
			continue;
		}
		if (cmd == "MakeNamedMaterial") {
			string name = getString(fin);
			++mtlcnt;
			curmtl = std::to_string(1000+mtlcnt);
			curmtl[0] = 'm';
			namedMaterial[name] = curmtl;
			getMtl(fin, mout, curmtl);
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
	printobj(fout, curobj);
	curobj.clear();
	fout << "\n]\n";
	}
	catch (const char* err) {
		console.error(err);
		return 1;
	}
	console.info("total",objs.size(), "objs");
	console.info("total",mtlcnt, "mtls");
}