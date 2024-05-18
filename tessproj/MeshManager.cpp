#include "MeshManager.h"
#include <algorithm>
#include <DirectXMath.h>
#include <vector>

using namespace mini;
using namespace gk2;
using namespace DirectX;

MeshManager::MeshManager(const DxDevice& device)
{
	/*m_quad = {
		{ XMFLOAT3(-2.f, 0.f, -2.f) },
		{ XMFLOAT3(-2.f, 0.f, +2.f) },
		{ XMFLOAT3(+2.f, 0.f, +2.f) },
		{ XMFLOAT3(+2.f, 0.f, -2.f) }
	};*/

	float bezierStep = 2.f;
	float patchStart = -3.f, patchHeight = 4.f;
	std::vector<DirectX::XMFLOAT3> patchFlat, patchUp, patchDown;
	std::vector<unsigned short> patchIndices;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			XMFLOAT3 point = { patchStart + i * bezierStep, 0.f, patchStart + j * bezierStep };
			patchFlat.push_back(point);

			if (i > 0 && i < 3 && j > 0 && j < 3)
				point.y = patchHeight;
			patchUp.push_back(point);

			point.y = -point.y;
			patchDown.push_back(point);

			if (i < 3)
			{
				patchIndices.push_back(i * 4 + j);
				patchIndices.push_back((i + 1) * 4 + j);
			}
			if (j < 3)
			{
				patchIndices.push_back(i * 4 + j);
				patchIndices.push_back(i * 4 + j + 1);
			}
		}
	}
	m_patchFlat.Initialize(device, patchFlat, patchIndices);
	m_patchUp.Initialize(device, patchUp, patchIndices);
	m_patchDown.Initialize(device, patchDown, patchIndices);

	float surfaceStart = -12.f, surfaceHeight = 2.f;
	std::vector<DirectX::XMFLOAT3> surfaceFlat, surfaceWavy, surfaceBumpy;
	std::vector<unsigned short> surfaceIndices;
	for (int pj = 0; pj < 4; pj++)
	{
		for (int pi = 0; pi < 4; pi++)
		{
			for (int i = 0; i < 4; i++)
			{
				int ii = 3 * pi + i;
				for (int j = 0; j < 4; j++)
				{
					int jj = 3 * pj + j;
					XMFLOAT3 point = { surfaceStart + ii * bezierStep, 0.f, surfaceStart + jj * bezierStep };
					surfaceFlat.push_back(point);

					if (i > 0 && i < 3)
					{
						point.y = surfaceHeight;
						if (pi % 2 == 0)
							point.y = -point.y;
					}
					surfaceWavy.push_back(point);

					point.y = 0.f;
					if (i > 0 && i < 3 && j > 0 && j < 3)
					{
						point.y = surfaceHeight;
						if ((pi % 2 == 0 || pj % 2 == 0) && !(pi % 2 == 0 && pj % 2 == 0))
							point.y = -point.y;
					}
					surfaceBumpy.push_back(point);

					int ind = ((pj * 4 + pi) * 4 + i) * 4 + j;
					if (i < 3)
					{
						surfaceIndices.push_back(ind);
						surfaceIndices.push_back(ind + 4);
					}
					if (j < 3)
					{
						surfaceIndices.push_back(ind);
						surfaceIndices.push_back(ind + 1);
					}
				}
			}
		}
	}
	m_surfaceFlat.Initialize(device, surfaceFlat, surfaceIndices);
	m_surfaceWavy.Initialize(device, surfaceWavy, surfaceIndices);
	m_surfaceBumpy.Initialize(device, surfaceBumpy, surfaceIndices);
}

void mini::gk2::MeshManager::Render(const dx_ptr<ID3D11DeviceContext>& context, MeshType type, bool indexed) const
{
	GetMesh(type).Render(context, indexed);
}

const Mesh& MeshManager::GetMesh(MeshType type) const
{
	switch (type)
	{
		case MeshType::PatchFlat:
			return m_patchFlat;
		case MeshType::PatchUp:
			return m_patchUp;
		case MeshType::PatchDown:
			return m_patchDown;
		case MeshType::SurfaceFlat:
			return m_surfaceFlat;
		case MeshType::SurfaceWavy:
			return m_surfaceWavy;
		case MeshType::SurfaceBumpy:
			return m_surfaceBumpy;
		default:
			return m_patchFlat;
	}
}
