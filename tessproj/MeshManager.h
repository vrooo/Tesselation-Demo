#pragma once
#include "dxDevice.h"
#include "Mesh.h"

namespace mini
{
	namespace gk2
	{
		enum class MeshType
		{
			PatchFlat, PatchUp, PatchDown, SurfaceFlat, SurfaceWavy, SurfaceBumpy
		};

		class MeshManager
		{
		public:
			MeshManager(const DxDevice& device);
			void Render(const dx_ptr<ID3D11DeviceContext>& context, MeshType type, bool indexed) const;

		private:
			Mesh m_patchFlat;
			Mesh m_patchUp;
			Mesh m_patchDown;
			Mesh m_surfaceFlat;
			Mesh m_surfaceWavy;
			Mesh m_surfaceBumpy;

			const Mesh& GetMesh(MeshType type) const;
		};
	}
}