#include "vdbLoader.hpp"
#include <algorithm>
#include <iostream>
#include <string>

#define NANOVDB_USE_OPENVDB
#include "openvdb/openvdb.h"
#include "openvdb/tools/LevelSetSphere.h"
#include "nanovdb/NanoVDB.h"
#include "nanovdb/util/IO.h"
#include "nanovdb/util/CreateNanoGrid.h"

namespace VdbLoader {
void	initialize() {
	openvdb::initialize();
}

void  load(const std::string filename, PhosObjectVdb &mesh, const VdbLoaderConfig &config) {
  std::string inputFile = filename;
  if (config.useRelativePath)
    inputFile = config.scenePath + filename;

	openvdb::io::File file(inputFile);
	file.open(true);
	auto vdbGridPtrs = file.getGrids();
	for (auto& grid : *vdbGridPtrs) {
		std::string name = grid->getName();
		std::cout << "grid: (" << name << ")" << std::endl;
	}
	auto vdbGrid = file.readGrid("density");
	auto handle = nanovdb::openToNanoVDB(vdbGrid);
	file.close();
}

}
