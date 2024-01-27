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

void  load(const std::string filename, PhosObjectVdb &vdb, const VdbLoaderConfig &config) {
  std::string inputFile = filename;
  if (config.useRelativePath)
    inputFile = config.scenePath + filename;

	openvdb::io::File file(inputFile);
	file.open(true);
	if (vdb.grids.size() > 0) {
		for (auto& grid : vdb.grids) {
			auto vdbGrid = file.readGrid(grid.name);
			grid.handle = nanovdb::openToNanoVDB(vdbGrid);
		}
	}
	else {
		auto vdbGridPtrs = file.getGrids();
		for (auto& gridPtr : *vdbGridPtrs) {
			PhosObjectVdb::VdbGrid	grid;
			grid.name = gridPtr->getName();
			std::cout << "grid: (" << grid.name << ")" << std::endl;
			auto vdbGrid = file.readGrid(grid.name);
			vdb.grids.push_back(grid);
			vdb.grids.back().handle = nanovdb::openToNanoVDB(vdbGrid);
		}
	}
	file.close();
}

}
