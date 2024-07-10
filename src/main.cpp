#include "main.h"
#include "types.h"
#include "json/json.h"

#include <direct.h>
#include <sstream>
#include <cstdio>
#include <fstream>
#include <cstring>

#include <windows.h>

#define UNICODE 0
#define CBB2_VER 1

using namespace std;

bool verbose = false;
bool is_compiling = true;
bool do_pause = true;

void _pause(){
	if(do_pause){
		system("pause");
	}
}

const char* default_project = 
"{\n\
	\"name\":\"Example project\",\n\
	\"base\":{\n\
	},\n\
	\"targets\":{\n\
		\"example_1\":{\n\
			\"compiler\":\"g++\",\n\
			\"from_obj\":0,\n\
			\"type\":\"EXE\",\n\
			\"include\":[\"include\"],\n\
			\"static\":[\"static\"],\n\
			\"dynamic\":[\"lib\"],\n\
			\"out\":\"bin\",\n\
			\"out_libfile\":\"lib\",\n\
			\"src\":[\"src\\\\main.cpp\"],\n\
			\"obj\":\"obj\",\n\
			\"link_static\":[],\n\
			\"link_dynamic\":[],\n\
			\"comp_params\":\"-std=c++17\",\n\
			\"link_params\":\"-static-libstdc++\"\n\
		}\n\
	}\n\
}";

void print_help(){
	cout<<"cbb2 -f <project file> -t <include_this> -e <exclude_this> -r <run this>\n";
	cout<<"-v will print generated GCC console commands\n\n";
	
	cout<<"-generate <name> will create a template JSON project file with the specified name\n";
	cout<<"-addt <jsonfile> <targetname> will add a template target to the existing project file\n";
	cout<<"-rmt <jsonfile> <targetname> will attempt to remove specified target from the project\n\n";
	
	cout<<"-version will print this program`s version\n";
	
	_pause();
	exit(0);
}

string GetClassText(TAR_CLASS cls){
	switch(cls){
		case CLASS_DLL:
			return "DLL";
		case CLASS_EXE:
			return "EXE";
		case CLASS_OBJ:
			return "OBJ";
		case CLASS_STATIC:
			return "STATIC";
	}
	
	return "UNDEF";
}

string GetCompText(TAR_COMPILER cls){
	switch(cls){
		case COMP_C:
			return "gcc";
		case COMP_CPP:
			return "g++";
	}
	
	return "UNDEF";
}

string get_file_text(string file){
	ifstream in(file);
	
	std::ostringstream sstr;
    sstr << in.rdbuf();
	
    return sstr.str();
}

bool is_in_vector(words_t vec, string value){
	bool out = false;
	for(int i = 0; i < vec.size(); i++){
		if(vec[i] == value){
			out = true;
			break;
		}
	}
	
	return out;
}

bool CheckFile(string path){
	FILE* test = fopen(path.c_str(), "r");
	
	if(test != nullptr){
		fclose(test);
		return true;
	}
	
	return false;
}

bool WCheckFile(string path){
	FILE* test = fopen(path.c_str(), "r");
	
	if(test != nullptr){
		fclose(test);
		return true;
	}
	
	return false;
}

vector<string> SplitString(string& str, char delim){
	string tmp;
	vector<string> out;
	
	for(size_t i = 0; i < str.size()+1; i++){
		if(i == str.size()){
			out.push_back(tmp);
			break;
		}

		if(str[i] != delim){
			tmp = tmp + str[i];
		}else{
			out.push_back(tmp);
			tmp.clear();
		}
	}
	
	return out;
}

string StrToWStr(string str){
	return str;
}

Json::Value& GetValue(Json::Value& base_val, Json::Value& val, string target_name){
	if(val.isMember(target_name)){
		return val[target_name];
	}else if(base_val.isMember(target_name)){
		return base_val[target_name];
	}else{
		cout<<"ERROR: Failed to parse value '"<<StrToWStr(target_name)<<"'!\n";
		_pause();
		exit(1);
	}
}

words_t ValueToWords(Json::Value& val){
	words_t out;
	
	for(int i = 0; i < val.size(); i++){
		out.push_back(StrToWStr(val[i].asString()));
	}
	
	return out;
}

target_t ParseTarget(string val_name, Json::Value& val, Json::Value& val_base){
	target_t out;
	
	out.lname = val_name;
	
	string comp_val = GetValue(val_base, val, "compiler").asString();
	
	if(comp_val == "gcc"){
		out.compiler = COMP_C;
	}else if(comp_val == "g++"){
		out.compiler = COMP_CPP;
	}else{
		cout<<"ERROR: undefined compiler name: '"<<StrToWStr(comp_val)<<"'!\n";
		_pause();
		exit(1);
	}
	
	out.from_obj = (uint8_t)( GetValue(val_base, val, "from_obj").asInt() );
	
	string type_val = GetValue(val_base, val, "type").asString();
	if(type_val == "DLL"){
		out._class = CLASS_DLL;
	}else if(type_val == "EXE"){
		out._class = CLASS_EXE;
	}else if(type_val == "STATIC"){
		out._class = CLASS_STATIC;
	}else if(type_val == "OBJ"){
		out._class = CLASS_OBJ;
	}else{
		cout<<"ERROR: undefined target class: '"<<StrToWStr(type_val)<<"'!\n";
		_pause();
		exit(1);
	}
	
	out.include = ValueToWords( GetValue(val_base, val, "include") );
	out._static = ValueToWords( GetValue(val_base, val, "static") );
	out.dynamic = ValueToWords( GetValue(val_base, val, "dynamic") );
	out.src = ValueToWords( GetValue(val_base, val, "src") );
	
	out.link_static = ValueToWords( GetValue(val_base, val, "link_static") );
	out.link_dynamic = ValueToWords( GetValue(val_base, val, "link_dynamic") );
	
	out.out = StrToWStr( GetValue(val_base, val, "out").asString() );
	out.out_libfile = StrToWStr( GetValue(val_base, val, "out_libfile").asString() );
	out.out_obj = StrToWStr( GetValue(val_base, val, "obj").asString() );
	
	out.comp_params = StrToWStr( GetValue(val_base, val, "comp_params").asString() );
	out.link_params = StrToWStr( GetValue(val_base, val, "link_params").asString() );
	
	return out;
}

words_t split_string(string str, wchar_t delim){
	string tmp;
	words_t out;
	
	for(int i = 0; i < str.size()+1; i++){
		if(i == str.size()){
			out.push_back(tmp);
			break;
		}
		
		if(str[i] != delim){
			tmp += str[i];
		}else{
			out.push_back(tmp);
			tmp = string();
			continue;
		}
	}
	
	return out;
}

string get_o_name(string cpp_name){
	auto lvls = split_string(cpp_name, '\\');
	string last = lvls[lvls.size()-1];
	
	auto dot_split = split_string(last, '.');
	string raw_name = dot_split[0];
	
	return raw_name + string(".o");
}

string GetFileName(string cpp_name){
	auto lvls = split_string(cpp_name, '\\');
	string last = lvls[lvls.size()-1];
	
	auto dot_split = split_string(last, '.');
	string raw_name = dot_split[0];
	
	return raw_name;
}

void BuildTarget(string path_to_dir_, target_t& tar){
	cout<<'\t'<<"Type: "<<StrToWStr(GetClassText(tar._class))<<'\n';
	
	words_t includes;
	cout<<'\t'<<"Include: \n";
	for(int i = 0; i < tar.include.size(); i++){
		string pth = path_to_dir_ + "\\" + tar.include[i];
		includes.push_back( pth );
		cout<<"\t\t"<<pth<<'\n';
	}
	
	if(tar.include.size() == 0){
		cout<<"\t\t"<<"No include paths\n";
	}
	
	words_t statics;
	cout<<'\t'<<"Static: \n";
	for(int i = 0; i < tar._static.size(); i++){
		string pth = path_to_dir_ + "\\" + tar._static[i];
		statics.push_back( pth );
		cout<<"\t\t"<<pth<<'\n';
	}
	
	if(tar._static.size() == 0){
		cout<<"\t\t"<<"No static paths\n";
	}
	
	words_t dynamics;
	cout<<'\t'<<"Dynamic: \n";
	for(int i = 0; i < tar.dynamic.size(); i++){
		string pth = path_to_dir_ + "\\" + tar.dynamic[i];
		dynamics.push_back( pth );
		cout<<"\t\t"<<pth<<'\n';
	}
	
	words_t libz;
	cout<<'\n';
	for(int i = 0; i < tar.link_dynamic.size(); i++){
		for(int j = 0; j < dynamics.size(); j++){
			string path = dynamics[j] + '\\' + tar.link_dynamic[i];
			if(WCheckFile(path)){
				cout<<"\tFound libfile: "<<path<<endl;
				libz.push_back(path);
			}
		}
	}
	
	if(tar.dynamic.size() == 0){
		cout<<"\t\t"<<"No dynamic paths\n";
	}
	
	cout<<'\n';
	
	if(tar.src.size() == 0){
		cout<<"\tWARNING: No source files given!\n";
		return;
	}
	
	words_t objfile_list;
	words_t src_list;
	
	cout<<"\n\tCOMPILING...\n";
	
	for(int i = 0; i < tar.src.size(); i++){
		string fname = GetFileName(tar.src[i]);
		string path = path_to_dir_ + '\\' + tar.src[i];
		
		char path_c[MAX_PATH];
		memset(path_c, 0, MAX_PATH);
		memcpy(path_c, path.c_str(), path.length());
		
		WIN32_FIND_DATA find_data;
		HANDLE find_h = FindFirstFile(path_c, &find_data); //handling masks in the filename
		
		string src_dir;
		
		//cout<<"\t\t"<<path_c<<endl;
		
		if(find_h != INVALID_HANDLE_VALUE){
			do{
				src_dir = "";
				auto split_sl = SplitString(tar.src[i], '\\');
				for(int j = 0; j < split_sl.size()-1; j++){
					src_dir = src_dir + split_sl[j] + "\\";
				}
				
				src_list.push_back( src_dir + string(find_data.cFileName) );
				//cout<<src_dir<<'\n';
			}while(FindNextFile(find_h, &find_data));
			
			FindClose(find_h);
		}
	}

	for(int i = 0; i < src_list.size(); i++){ //generating object files
		string path = path_to_dir_ + '\\' + src_list[i];
		string path_o = path_to_dir_ + '\\' + tar.out_obj + '\\' + get_o_name(src_list[i]);
		
		cout<<"\t\tFILE: "<<path;
		if(!WCheckFile(path)){
			cout<<" NOT FOUND\n";
			continue;
		}
		cout<<" EXISTS\n";
		
		if(!tar.from_obj){ //compiling from the source code
			string ogcm = StrToWStr(GetCompText(tar.compiler)) + ' ' + tar.comp_params + ' ';
			ogcm += string("-c ") + path + string(" -o ") + path_o + ' ';
			
			for(int i = 0; i < includes.size(); i++){
				ogcm += string("-I\"") + includes[i] + '\"' + ' ';
			}
			
			for(int i = 0; i < statics.size(); i++){
				ogcm += string("-L\"") + statics[i] + '\"' + ' ';
			}
			
			for(int i = 0; i < tar.link_static.size(); i++){
				ogcm += string("-l") + tar.link_static[i] + ' ';
			}
			
			if(verbose){
				cout<<"\t\t\t"<<ogcm<<'\n';
			}
			system(ogcm.c_str());
			
			objfile_list.push_back(path_o);
		}else{ //building from the pre-compiled object files
			objfile_list.push_back(path);
		}
	}
	
	if(tar._class == CLASS_OBJ){ return; }
	
	cout<<"\n\tLINKAGE...\n";
	
	string comm;
	
	switch(tar._class){
		case CLASS_DLL:
			comm = StrToWStr(GetCompText(tar.compiler)) + ' ' + tar.link_params + ' ';
			comm += "-shared ";
			
			for(int i = 0; i < objfile_list.size(); i++){
				comm += objfile_list[i] + ' ';
			}
			
			for(int i = 0; i < libz.size(); i++){
				comm += libz[i] + ' ';
			}
			
			comm += string("-Wl,--out-implib,") + tar.out_libfile + string("\\") + StrToWStr(tar.lname) + string(".lib ");
			
			for(int i = 0; i < includes.size(); i++){
				comm += string("-I\"") + includes[i] + '\"' + ' ';
			}
			
			for(int i = 0; i < statics.size(); i++){
				comm += string("-L\"") + statics[i] + '\"' + ' ';
			}
			
			for(int i = 0; i < tar.link_static.size(); i++){
				comm += string("-l") + tar.link_static[i] + ' ';
			}
			
			comm += string("-o ") + tar.out + '\\' + StrToWStr(tar.lname) + ".dll ";
			
			if(verbose){
				cout<<"\t\t"<<comm<<'\n';
			}
			
			system(comm.c_str());
			
			break;
			
		case CLASS_EXE:
			comm = StrToWStr(GetCompText(tar.compiler)) + ' ' + tar.link_params + ' ';
			
			for(int i = 0; i < objfile_list.size(); i++){
				comm += objfile_list[i] + ' ';
			}
			
			for(int i = 0; i < libz.size(); i++){
				comm += libz[i] + ' ';
			}
			
			for(int i = 0; i < includes.size(); i++){
				comm += string("-I\"") + includes[i] + '\"' + ' ';
			}
			
			for(int i = 0; i < statics.size(); i++){
				comm += string("-L\"") + statics[i] + '\"' + ' ';
			}
			
			for(int i = 0; i < tar.link_static.size(); i++){
				comm += string("-l") + tar.link_static[i] + ' ';
			}
			
			comm += string("-o ") + tar.out + '\\' + StrToWStr(tar.lname) + ".exe ";
			
			if(verbose){
				cout<<"\t\t"<<comm<<'\n';
			}
			
			system(comm.c_str());
			break;
			
		case CLASS_STATIC:
			comm = string("ar rcs ") + tar.out + '\\' + StrToWStr(tar.lname) + ".a ";
			
			for(int i = 0; i < objfile_list.size(); i++){
				comm += objfile_list[i] + ' ';
			}
			
			if(verbose){
				cout<<"\t\t"<<comm<<endl;
			}
			
			system(comm.c_str());
			break;
	}
}

int main(int argc, char* argv[]){
	//setlocale(LC_ALL, "en-US");
	//SetConsoleCP(437);
	
	if(argc == 1){
		print_help();
	}
	
	char cwd_raw[FILENAME_MAX];
	_getcwd(cwd_raw, sizeof(cwd_raw));
	
	string path_to_dir(cwd_raw);
	
	bool has_project = false, has_base = false, has_targets = false;
	
	words_t targets_nams;
	vector<target_t> targets;
	string json_fname;
	
	words_t targets_names, exclude_names, run_names;

	//parsing arguments
	for(int i = 1; i < argc; i++){
		string arg(argv[i]);
		
		if(arg == "-nopause"){
			do_pause = false;
		}
		
		if(arg == "-f"){
			if(i+1 < argc){
				json_fname = string(argv[i+1]);
				has_project = true;
			}
		}
		
		if(arg == "-t"){
			if(i+1 < argc){
				targets_names.push_back(StrToWStr( string( argv[i+1] ) ));
				has_targets = true;
			}
		}
		
		if(arg == "-e"){
			if(i+1 < argc){
				exclude_names.push_back(StrToWStr( string( argv[i+1] ) ));
			}
		}
		
		if(arg == "-r"){
			if(i+1 < argc){
				run_names.push_back(StrToWStr( string( argv[i+1] ) ));
			}
		}
		
		if(arg == "-v"){
			verbose = true;
		}
		
		if(arg == "-generate"){
			if(i+1 < argc){
				string local_path( StrToWStr(string(argv[i+1])) );
				string gl_path = StrToWStr(path_to_dir) + '\\' + local_path + ".json";
				
				FILE* of = fopen(gl_path.c_str(), "w");
				
				if(of == nullptr){
					cout<<"ERROR: Could not open file "<<gl_path<<endl;
					_pause();
					exit(0);
				}
				
				fputs(default_project, of);
				fclose(of);
				exit(0);
			}
		}
		
		if(arg == "-addt"){
			if(i+2 < argc){
				string local_path( StrToWStr(string(argv[i+1])) );
				string gl_path = StrToWStr(path_to_dir) + '\\' + local_path + ".json";
				
				string tar_name( argv[i+2] );
				
				Json::Value root;
				Json::Reader rd;
				
				std::ifstream ifile(gl_path);
				if(!ifile.is_open()){
					cout<<"ERROR: Could not open file "<<gl_path<<endl;
					_pause();
					exit(0);
				}
				
				if(!rd.parse(ifile, root)) {
					cout<<"ERROR: JSON syntax error: \n";
					cout << rd.getFormattedErrorMessages() << endl;
					_pause();
					exit(1);
				}
				
				ifile.close();
				
				Json::Value nt;
				nt["compiler"] = Json::Value("g++");
				nt["from_obj"] = Json::Value(0);
				nt["type"] = Json::Value("EXE");
				
				nt["include"].append(Json::Value("include"));
				nt["static"].append(Json::Value("static"));
				nt["dynamic"].append(Json::Value("dynamic"));
				
				nt["out"] = Json::Value("bin");
				nt["out_libfile"] = Json::Value("lib");
				nt["src"].append(Json::Value("src\\main.cpp"));
				nt["obj"] = Json::Value("obj");
				
				nt["link_static"].append(Json::Value("example_lib"));
				nt["link_dynamic"].append(Json::Value("example_libfile.lib"));
				
				nt["comp_params"] = Json::Value("-m64");
				nt["link_params"] = Json::Value("-m64");
				
				root["targets"][tar_name] = nt;
				
				std::ofstream ofile(gl_path);
				Json::StyledStreamWriter wrt;
				wrt.write(ofile, root);
				
				ofile.close();

				is_compiling = false;
			}
		}
		
		if(arg == "-rmt"){
			if(i+2 < argc){
				string local_path( StrToWStr(string(argv[i+1])) );
				string gl_path = StrToWStr(path_to_dir) + '\\' + local_path + ".json";
				
				string tar_name( argv[i+2] );
				
				Json::Value root;
				Json::Reader rd;
				
				std::ifstream ifile(gl_path);
				
				if(!ifile.is_open()){
					cout<<"ERROR: Could not open file "<<gl_path<<endl;
					_pause();
					exit(0);
				}
				
				if(!rd.parse(ifile, root)) {
					cout<<"ERROR: JSON syntax error: \n";
					cout << rd.getFormattedErrorMessages() << endl;
					_pause();
					exit(1);
				}

				ifile.close();
				
				if(root["targets"].isMember(tar_name)){
					//root["targets"][tar_name].clear();
					root["targets"].removeMember(tar_name);
					
					std::ofstream ofile(gl_path);
					Json::StyledStreamWriter wrt;
					wrt.write(ofile, root);
					
					ofile.close();
					is_compiling = false;
					
				}else{
					std::cout<<"ERROR: Project file '"<<local_path<<"' has no target called '"<<tar_name<<"'!\n";
				}
			}
		}
		
		if(arg == "-version"){
			cout<<"CBPP Project builder ver. "<<CBB2_VER<<"\n";
			
			if(argc == 2){
				is_compiling = false;
			}
		}
	}
	
	if(!is_compiling){
		exit(0);
	}
	
	string path_to_json = path_to_dir + string("\\") + json_fname + string(".json");
	
	if(has_project){
		cout<<"Input project file: "<<StrToWStr(path_to_json);
		if(CheckFile(path_to_json)){
			cout<<" (EXISTS)\n";
		}else{
			cout<<" (NOT FOUND)\n"<<"ERROR: Project file not found!\n";
			_pause();
			exit(1);
		}
	}else{
		cout<<"ERROR: No project file providen!\n";
		_pause();
		exit(1);
	}
	
	Json::Reader reader;
	Json::Value root, base_lib, targets_v;
	
	cout<<"Parsing JSON...\n";
	if(!reader.parse(get_file_text(path_to_json), root)) {
		cout<<"ERROR: JSON syntax error: \n";
		cout << StrToWStr(reader.getFormattedErrorMessages()) << endl;
		_pause();
		exit(1);
	}
	
	if(root.isMember("base")){
		has_base = true;
		base_lib = root["base"];
		cout<<"Base library specified\n";
	}
	
	if(!root.isMember("targets")){
		cout<<"ERROR: Project file has no targets specified!\n";
		_pause();
		exit(1);
	}
	
	cout<<"JSON OK\n\n";
	
	targets_v = root["targets"];
	
	cout<<"Parsing specified targets...\n";
	
	cout<<"Targets list:\n";
	/* int counter = 0;
	for(auto it = targets_v.begin(); it != targets_v.end(); it++){
		string t_name = it.memberName();
		
		if((!has_targets || is_in_vector(targets_names, StrToWStr(t_name))) && !is_in_vector(exclude_names, StrToWStr(t_name))){
			target_t curr = ParseTarget(t_name, targets_v[t_name], base_lib);
			targets.push_back( curr );
			cout<<'\t'<<StrToWStr(t_name)<<" ("<<StrToWStr( GetClassText(curr._class) )<<")\n";
			counter++;
		}
	} */
	
	int counter = 0;
	for(int i = 0; i < targets_names.size(); i++){
		string t_name = targets_names[i];
		
		if(!is_in_vector(exclude_names, t_name) || (!has_targets) ){
			target_t curr = ParseTarget(t_name, targets_v[t_name], base_lib);
			targets.push_back(curr);
			
			cout<<'\t'<<StrToWStr(t_name)<<" ("<<StrToWStr( GetClassText(curr._class) )<<")\n";
			counter++;
		}
	}
	
	if(counter == 0){
		cout<<"ERROR: No targets!\n";
		_pause();
		exit(0);
	}
	
	cout<<"Parse OK\n\n";
	
	cout<<"Building specified targets...\n";
	
	for(int i = 0; i < targets.size(); i++){
		target_t targ = targets[i];
		cout<<"Target: "<<StrToWStr(targ.lname)<<endl;
		//cout<<"PATH TO DIR: "<<path_to_dir<<endl;
		BuildTarget(path_to_dir, targ);
	}
	cout<<"\n";
	for(int i = 0; i < targets.size(); i++){
		target_t tar = targets[i];
		if(is_in_vector(run_names, tar.lname)){
			cout<<"Running: "<<tar.lname<<"\n";
			
			if(tar._class != CLASS_EXE){
				cout<<"WARNING: Target class is not executeable, skipping\n";
				continue;
			}
			
			string local_path = tar.out + "\\" + tar.lname + string(".exe");
			string gl_path = path_to_dir + "\\" + local_path;
			cout<<"Dedicated EXE path: "<<gl_path<<"\n";
			
			string command = string("start \"") + tar.lname + "\" /wait " + gl_path;
			
			if(verbose){
				cout<<"\t"<<command<<"\n";
			}
			system(command.c_str());
		}
	}
	
	return 0;
}