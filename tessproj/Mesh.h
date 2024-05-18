#pragma once
#include "dxApplication.h"
#include <DirectXMath.h>
#include <vector>

namespace mini
{
	namespace gk2
	{
		class Mesh
		{
		public:
			Mesh();
			void Initialize(const DxDevice& device,
							const std::vector<DirectX::XMFLOAT3> vertices,
							const std::vector<unsigned short> indices);
			virtual void Render(const dx_ptr<ID3D11DeviceContext>& context, bool indexed) const;

		private:
			static constexpr unsigned int vertexStride = sizeof(DirectX::XMFLOAT3);
			static constexpr unsigned int offset = 0;

			std::vector<DirectX::XMFLOAT3> m_vertices;
			dx_ptr<ID3D11Buffer> m_vertexBuffer;

			std::vector<unsigned short> m_indices;
			dx_ptr<ID3D11Buffer> m_indexBuffer;
		};
	}
}