#ifndef CBB2_TYPES_H
#define CBB2_TYPES_H

#include "main.h"
#include <cstdint>
#include <string>

enum TAR_COMPILER : uint8_t {
	COMP_C,
	COMP_CPP
};

enum TAR_CLASS : uint8_t {
	CLASS_DLL,
	CLASS_EXE,
	CLASS_STATIC,
	CLASS_OBJ
};

struct target_t {
	std::string lname;
	TAR_COMPILER compiler = COMP_CPP;
	uint8_t from_obj = 0;
	
	TAR_CLASS _class = CLASS_EXE;
	
	words_t include, _static, dynamic, src;
	words_t link_static, link_dynamic;
	
	std::string out, out_libfile, out_obj;
	std::string comp_params, link_params;
};

#endif