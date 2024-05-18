#include "TessellationApp.h"

using namespace std;
using namespace mini;
using namespace gk2;
using namespace DirectX;

TessellationApp::TessellationApp(HINSTANCE appInstance)
	: DxApplication(appInstance, 900, 900, L"Tessellation"),
	m_cbView(m_device.CreateConstantBuffer<XMFLOAT4X4>()),
	m_cbCamPos(m_device.CreateConstantBuffer<XMFLOAT4>()),
	m_cbProj(m_device.CreateConstantBuffer<XMFLOAT4X4>()),
	m_cbTex(m_device.CreateConstantBuffer<XMFLOAT4X4>()),
	m_cbSurfaceColor(m_device.CreateConstantBuffer<XMFLOAT4>()),
	m_cbLightPos(m_device.CreateConstantBuffer<XMFLOAT4>()),
	m_cbTessCount(m_device.CreateConstantBuffer<XMFLOAT4>()),
	//m_diffuseMap(m_device.CreateShaderResourceView(L"textures/diffuse.dds")),
	//m_normalMap(m_device.CreateShaderResourceView(L"textures/normals.dds")),
	//m_heightMap(m_device.CreateShaderResourceView(L"textures/height.dds")),
	m_diffuseMap(m_device.CreateShaderResourceView(L"textures/cliff-diffuse.dds")),
	m_normalMap(m_device.CreateShaderResourceView(L"textures/cliff-normals.dds")),
	m_heightMap(m_device.CreateShaderResourceView(L"textures/cliff-height.dds")),
	m_tessCount(2.f, 4.f, 0.f, 0.f),
	m_meshManager(m_device), m_meshType(MeshType::PatchFlat),
	m_renderPolynet(false), m_fillSurface(false), m_useLOD(false), m_useHeightMap(false)
{
	// constant constant buffers
	auto s = m_window.getClientSize();
	auto ar = static_cast<float>(s.cx) / s.cy;
	XMFLOAT4X4 tmpMtx;
	XMStoreFloat4x4(&tmpMtx, XMMatrixPerspectiveFovLH(XM_PIDIV4, ar, 0.01f, 100.f));
	UpdateBuffer(m_cbProj, tmpMtx);
	float scale = 1.f / 24.f, offset = .5f;
	XMStoreFloat4x4(&tmpMtx, XMMatrixScaling(scale, scale, scale) * XMMatrixTranslation(offset, offset, offset));
	UpdateBuffer(m_cbTex, tmpMtx);
	UpdateBuffer(m_cbSurfaceColor, XMFLOAT4(0.f, 0.f, 1.f, 1.f));
	UpdateBuffer(m_cbLightPos, XMFLOAT4(0.f, 10.f, 0.f, 1.f));
	
	// variable constant buffers
	UpdateParams();
	UpdateCameraCB();

	// rasterizer descriptions
	RasterizerDescription rsDesc;
	rsDesc.CullMode = D3D11_CULL_NONE;
	m_rsFill = m_device.CreateRasterizerState(rsDesc);

	rsDesc.FillMode = D3D11_FILL_WIREFRAME;
	m_rsWireframe = m_device.CreateRasterizerState(rsDesc);

	// sampler states
	SamplerDescription sd;
	sd.Filter = D3D11_FILTER_ANISOTROPIC;
	sd.MaxAnisotropy = 16;
	m_sampler = m_device.CreateSamplerState(sd);

	// shaders
	auto vsCode = m_device.LoadByteCode(L"tessellationVS.cso");
	m_tessVS = m_device.CreateVertexShader(vsCode);
	m_polynetVS = m_device.CreateVertexShader(m_device.LoadByteCode(L"polynetVS.cso"));

	m_tessHS = m_device.CreateHullShader(m_device.LoadByteCode(L"tessellationHS.cso"));
	m_lodHS = m_device.CreateHullShader(m_device.LoadByteCode(L"lodHS.cso"));

	m_tessDS = m_device.CreateDomainShader(m_device.LoadByteCode(L"tessellationDS.cso"));
	m_heightDS = m_device.CreateDomainShader(m_device.LoadByteCode(L"heightDS.cso"));

	m_simplePS = m_device.CreatePixelShader(m_device.LoadByteCode(L"simplePS.cso"));
	m_phongPS = m_device.CreatePixelShader(m_device.LoadByteCode(L"phongPS.cso"));


	const D3D11_INPUT_ELEMENT_DESC layout[1] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	m_layout = m_device.CreateInputLayout(layout, vsCode);
	m_device.context()->IASetInputLayout(m_layout.get());

	// shader buffers
	ID3D11Buffer* vsb[] = { m_cbView.get(), m_cbProj.get() };
	m_device.context()->VSSetConstantBuffers(0, 2, vsb);
	ID3D11Buffer* hsb[] = { m_cbTessCount.get() };
	m_device.context()->HSSetConstantBuffers(0, 1, hsb);
	ID3D11Buffer* dsb[] = { m_cbView.get(), m_cbCamPos.get(), m_cbProj.get(), m_cbTex.get() };
	m_device.context()->DSSetConstantBuffers(0, 4, dsb);
	ID3D11Buffer* psb[] = { m_cbSurfaceColor.get(), m_cbLightPos.get() };
	m_device.context()->PSSetConstantBuffers(0, 2, psb);

	// defaults
	m_device.context()->RSSetState(m_rsWireframe.get());
	m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST);
	
	auto samp = m_sampler.get();
	auto dsTex = { m_heightMap.get() };
	m_device.context()->DSSetShaderResources(0, dsTex.size(), dsTex.begin());
	m_device.context()->DSSetSamplers(0, 1, &samp);
	auto psTex = { m_diffuseMap.get(), m_normalMap.get() };
	m_device.context()->PSSetShaderResources(0, psTex.size(), psTex.begin());
	m_device.context()->PSSetSamplers(0, 1, &samp);
}

void TessellationApp::SetShaders(ID3D11VertexShader* vs,
								 ID3D11HullShader* hs,
								 ID3D11DomainShader* ds,
								 ID3D11PixelShader* ps)
{
	m_device.context()->VSSetShader(vs, nullptr, 0);
	m_device.context()->HSSetShader(hs, nullptr, 0);
	m_device.context()->DSSetShader(ds, nullptr, 0);
	m_device.context()->PSSetShader(ps, nullptr, 0);
}

void TessellationApp::UpdateCameraCB()
{
	XMFLOAT4X4 viewMtx;
	XMStoreFloat4x4(&viewMtx, m_camera.getViewMatrix());
	UpdateBuffer(m_cbView, viewMtx);
	UpdateBuffer(m_cbCamPos, m_camera.getCameraPosition());
}

void TessellationApp::UpdateParams()
{
	UpdateBuffer(m_cbTessCount, m_tessCount);
}

void TessellationApp::Update(const Clock& c)
{
	double dt = c.getFrameTime();
	if (HandleCameraInput(dt))
		UpdateCameraCB();
	if (HandleKeyboardInput(dt))
		UpdateParams();
}

bool TessellationApp::HandleKeyboardInput(double dt)
{
	static KeyboardState prevKeyState;
	static bool hasPrevState = false;

	bool update = false;
	KeyboardState keyState;
	if (!m_keyboard.GetState(keyState))
		return false;
	if (hasPrevState)
	{
		if (prevKeyState.keyPressed(keyState, DIK_UPARROW))
		{
			m_tessCount.x = min(m_tessCount.x + 1.f, MAX_TESS);
			update = true;
		}
		if (prevKeyState.keyPressed(keyState, DIK_DOWNARROW))
		{
			m_tessCount.x = max(m_tessCount.x - 1.f, MIN_TESS);
			update = true;
		}
		if (prevKeyState.keyPressed(keyState, DIK_RIGHTARROW))
		{
			m_tessCount.y = min(m_tessCount.y + 1.f, MAX_TESS);
			update = true;
		}
		if (prevKeyState.keyPressed(keyState, DIK_LEFTARROW))
		{
			m_tessCount.y = max(m_tessCount.y - 1.f, MIN_TESS);
			update = true;
		}

		if (prevKeyState.keyPressed(keyState, DIK_1))
		{
			m_meshType = MeshType::PatchFlat;
			update = true;
		}
		else if (prevKeyState.keyPressed(keyState, DIK_2))
		{
			m_meshType = MeshType::PatchUp;
			update = true;
		}
		else if (prevKeyState.keyPressed(keyState, DIK_3))
		{
			m_meshType = MeshType::PatchDown;
			update = true;
		}
		else if (prevKeyState.keyPressed(keyState, DIK_4))
		{
			m_meshType = MeshType::SurfaceFlat;
			update = true;
		}
		else if (prevKeyState.keyPressed(keyState, DIK_5))
		{
			m_meshType = MeshType::SurfaceWavy;
			update = true;
		}
		else if (prevKeyState.keyPressed(keyState, DIK_6))
		{
			m_meshType = MeshType::SurfaceBumpy;
			update = true;
		}

		if (prevKeyState.keyPressed(keyState, DIK_Q))
		{
			m_useLOD = !m_useLOD;
			update = true;
		}
		if (prevKeyState.keyPressed(keyState, DIK_W))
		{
			m_fillSurface = !m_fillSurface;
			update = true;
		}
		if (prevKeyState.keyPressed(keyState, DIK_E))
		{
			m_renderPolynet = !m_renderPolynet;
			update = true;
		}
		if (prevKeyState.keyPressed(keyState, DIK_R))
		{
			m_useHeightMap = !m_useHeightMap;
			update = true;
		}
	}
	prevKeyState = keyState;
	hasPrevState = true;
	return update;
}

void TessellationApp::Render()
{
	DxApplication::Render();

	UpdateBuffer(m_cbSurfaceColor, XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));

	auto hs = m_tessHS.get();
	auto ds = m_tessDS.get();
	auto ps = m_simplePS.get();
	if (m_useLOD)
	{
		hs = m_lodHS.get();
	}
	if (m_useHeightMap)
	{
		ds = m_heightDS.get();
	}
	if (m_fillSurface)
	{
		ps = m_phongPS.get();
		m_device.context()->RSSetState(m_rsFill.get());
	}
	SetShaders(m_tessVS.get(), hs, ds, ps);
	m_meshManager.Render(m_device.context(), m_meshType, false);
	if (m_fillSurface)
	{
		m_device.context()->RSSetState(m_rsWireframe.get());
	}

	if (m_renderPolynet)
	{
		m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		UpdateBuffer(m_cbSurfaceColor, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
		SetShaders(m_polynetVS.get(), nullptr, nullptr, m_simplePS.get());
		m_meshManager.Render(m_device.context(), m_meshType, true);
		m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST);
	}
}