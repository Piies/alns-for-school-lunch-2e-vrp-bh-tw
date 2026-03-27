#include <chrono>
#include <cmath>
#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include <iostream>
#include <process.h>
#include <string>

#include "utils/config.h"
#include "problem/cvrpptpl.h"


static std::string getSourceDir() {
	std::string fileMacro = __FILE__;
	// Find last slash or backslash (cross-platform)
	size_t pos = fileMacro.find_last_of("/\\");
	if (pos == std::string::npos)
		return "."; // fallback: current directory
	return fileMacro.substr(0, pos);
}

int main(int argc, char* argv[])
{
    std::string sourceDir = getSourceDir();
	std::string instanceDir = sourceDir + "/instances";
	Config cfg(argc, argv);
    
	std::string instancePath = instanceDir + "/" + cfg.instanceName;

    return 0;
}