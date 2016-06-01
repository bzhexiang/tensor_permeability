#include <fstream>

#include "catch.hpp"
#include "settings.h"
#include "json.hpp"
#include "model_io.h"
#include "matrix_configurator_factory.h"
#include "fracture_configurator_factory.h"
#include "configurator.h"
#include "sort_boundaries.h"
#include "pressure_solver.h"
#include "omega_configurator_factory.h"
#include "fetch.h"
#include "report.h"
#include "Model.h"
#include "TensorVariable.h"

using namespace csmp;
using namespace csmp::tperm;
using namespace std;


#define TP_EXTENDED_TESTS 1


TEST_CASE("reading base configuration file") {
	std::ifstream f("config.json");
	Settings s;
	REQUIRE_NOTHROW(s.json << f);
	f.close();
	auto jconfig = s.json["configuration"];
	REQUIRE(jconfig["matrix"]["configuration"].get<string>() == string("uniform"));
}


TEST_CASE("model factory nullptr") {
	Settings ms;
	ms.json = R"({
                     "file name": "debug",
					 "format": "none"
				})"_json;
	auto nomodel = load_model(ms);
	REQUIRE(nomodel == nullptr);
}


TEST_CASE("tensor extraction") {
	Settings cs;
	cs.json = R"({
					"tensor": 1.0,
					"tensor diagonal": [1.0, 2.0, 3.0],
					"tensor full": [1.0, 0.1, 0.1, 0.2, 2.0, 0.2, 0.3, 0.3, 3.0]
				})"_json;
	// Spherical tensor
	REQUIRE(tensor("tensor", cs) == TensorVariable<3>(PLAIN, 1.0, 0., 0., 0., 1.0, 0., 0., 0., 1.0));
	// Diagonal tensor
	cs.json["tensor"] = {1.0, 2.0, 3.0};
	TensorVariable<3> td(PLAIN, 1.0, 0., 0., 0., 2.0, 0., 0., 0., 3.0);
	REQUIRE(tensor("tensor", cs) == td);
	REQUIRE(tensor("tensor diagonal", cs) == td);
	// Full tensor
	cs.json["tensor"] = { 1.0, 0.1, 0.1, 0.2, 2.0, 0.2, 0.3, 0.3, 3.0 };
	TensorVariable<3> tf(PLAIN, 1.0, 0.1, 0.1, 0.2, 2.0, 0.2, 0.3, 0.3, 3.0);
    REQUIRE(tensor("tensor full", cs) == tf);
	REQUIRE(tensor("tensor", cs) == tf);
}


TEST_CASE("flow tdd minimal configuration") {
	// generate settings
	Settings s;
	s.json = R"({
				 "model": {
                     "file name": "debug",
					 "format": "icem"
                 },
				 "configuration": {
				     "matrix":{
				         "configuration": "uniform",
						 "permeability": 1.0E-15
					 },
					 "fractures":{
						 "configuration": "uniform",
						 "mechanical aperture": 0.0001, 
						 "hydraulic aperture": 0.0001
					 }
				 },
				"analysis":{
					"configuration": "uniform boundary distance",
					"distance": 2.0
				}
				})"_json;

	// get matrix configurator
	Settings mcs(Settings(s, "configuration"), "matrix");
	MatrixConfiguratorFactory mcf;
	auto mconf = mcf.configurator(mcs);
	// get fracture configurator
	Settings fcs(Settings(s, "configuration"), "fractures");
	FractureConfiguratorFactory fcf;
	auto fconf = fcf.configurator(fcs);
	// get omega generator
	Settings acs(s, "analysis");
	OmegaConfiguratorFactory ocf;
	auto oconf = ocf.configurator(acs);

	if (0) { 
		// load model...		
		Settings ms(s, "model");
		auto model = load_model(ms);
		// configure material properties
		mconf->configure(*model);
		fconf->configure(*model);
		// sort boundaries
		auto bds = sort_boundaries(*model, s);
		// ready to solve
		solve(bds, *model);
		// generate omegas
		oconf->configure(*model);
		// get upscaled tensors
		auto omega_tensors = fetch(*model);
		// report
		report(omega_tensors, *model);
	}
}


TEST_CASE("flow tdd extended configuration") {
	// generate settings
	Settings s;
	s.json = R"({
				 "model": {
                     "file name": "debug",
					 "format": "icem",
					 "regions": ["CREATED_MATERIAL_9", "FRACTURES", "BOUNDARY1", "BOUNDARY2", "BOUNDARY3", "BOUNDARY4", "BOUNDARY5", "BOUNDARY6"],
					 "save final binary as": ""
                 },
				 "configuration": {
				     "matrix":{
				         "configuration": "uniform",
						 "permeability": 1.0E-15
					 },
					 "fractures":{
						 "configuration": "uniform",
						 "mechanical aperture": 0.0001, 
						 "hydraulic aperture": 0.0001
					 }
				 },
				"analysis":{
					"configuration": "bounding box",
					"corner points": [[[-3.0,-3.0,2.0],[3.0,3.0,8.0]], [[-2.0,-2.0,3.0],[2.0,2.0,7.0]]]
				}
				})"_json;

	// get matrix configurator
	Settings mcs(Settings(s, "configuration"), "matrix");
	MatrixConfiguratorFactory mcf;
	auto mconf = mcf.configurator(mcs);
	// get fracture configurator
	Settings fcs(Settings(s, "configuration"), "fractures");
	FractureConfiguratorFactory fcf;
	auto fconf = fcf.configurator(fcs);
	// get omega generator
	Settings acs(s, "analysis");
	OmegaConfiguratorFactory ocf;
	auto oconf = ocf.configurator(acs);

	if (1) {
		// load model...		
		Settings ms(s, "model");
		auto model = load_model(ms);
		// configure material properties
		mconf->configure(*model);
		fconf->configure(*model);
		// sort boundaries
		auto bds = sort_boundaries(*model, s);
		// ready to solve
		solve(bds, *model);
		// generate omegas
		oconf->configure(*model);
		// get upscaled tensors
		auto omega_tensors = fetch(*model);
		// report results
		report(omega_tensors, *model);
		// output model
		if (acs.json.count("save final binary"))
			if (acs.json["save final binary"].get<string>() != "")
				save_model(*model, acs.json["save final binary"].get<string>().c_str());
	}
}

