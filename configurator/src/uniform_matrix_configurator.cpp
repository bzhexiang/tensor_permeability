#include "uniform_matrix_configurator.h"
#include "bpreds.h"

#include "Model.h"

namespace csmp {
	namespace tperm {


		UniformMatrixConfigurator::UniformMatrixConfigurator(const csmp::TensorVariable<3>& perm)
			: Configurator(), perm_(perm)
		{
		}


		UniformMatrixConfigurator::UniformMatrixConfigurator(double perm)
			: Configurator(), perm_(csmp::TensorVariable<3>(PLAIN, perm, 0., 0., 0., perm, 0., 0., 0., perm))
		{
		}


		/**
		Assigns permeability to all equi-dimensional elements in `Model`
		*/
		bool UniformMatrixConfigurator::configure(Model& model) const
		{
			auto melmts = model.ElementsFrom(MatrixElement<3>(false));
			const char* vname = "permeability";
			Index pKey(model.Database().StorageKey(vname));
			for (const auto& it: melmts)
				it->Store(pKey, perm_);			
			return true;
		}


	} // !tperm
} // ! csmp