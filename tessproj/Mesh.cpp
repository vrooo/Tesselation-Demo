#include "Mesh.h"

using namespace mini;
using namespace gk2;

Mesh::Mesh() {}

void Mesh::Initialize(const DxDevice& device,
					  const std::vector<DirectX::XMFLOAT3> vertices,
					  const std::vector<unsigned short> indices)
{
	m_vertices = vertices;
	m_vertexBuffer = device.CreateVertexBuffer(vertices);
	m_indices = indices;
	m_indexBuffer = device.CreateIndexBuffer(indices);
}

void Mesh::Render(const dx_ptr<ID3D11DeviceContext>& context, bool indexed) const
{
	ID3D11Buffer* b = m_vertexBuffer.get();
	context->IASetVertexBuffers(0, 1, &b, &vertexStride, &offset);
	if (indexed)
	{
		context->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
		context->DrawIndexed(m_indices.size(), 0, 0);
	}
	else
	{
		context->Draw(m_vertices.size(), 0);
	}
}
