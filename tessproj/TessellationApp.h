#pragma once
#include "dxApplication.h"
#include "MeshManager.h"

namespace mini
{
	namespace gk2
	{
		class TessellationApp : public DxApplication
		{
		public:
			explicit TessellationApp(HINSTANCE appInstance);

		protected:
			void Update(const Clock& dt) override;
			void Render() override;

		private:
			static constexpr float MIN_TESS = 1.f;
			static constexpr float MAX_TESS = 50.f;

			void SetShaders(ID3D11VertexShader* vs,
							ID3D11HullShader* hs,
							ID3D11DomainShader* ds,
							ID3D11PixelShader* ps);
			void UpdateCameraCB();
			void UpdateParams();
			bool HandleKeyboardInput(double dt);

			dx_ptr<ID3D11Buffer> m_cbView, m_cbCamPos, m_cbProj, m_cbTex,
				m_cbSurfaceColor, m_cbLightPos,
				m_cbTessCount;
			dx_ptr<ID3D11Buffer> m_vertexBuffer;

			DirectX::XMFLOAT4 m_tessCount;

			MeshManager m_meshManager;
			MeshType m_meshType;
			bool m_renderPolynet, m_fillSurface, m_useLOD, m_useHeightMap;

			dx_ptr<ID3D11InputLayout> m_layout;
			dx_ptr<ID3D11RasterizerState> m_rsWireframe, m_rsFill;

			dx_ptr<ID3D11VertexShader> m_tessVS, m_polynetVS;
			dx_ptr<ID3D11HullShader> m_tessHS, m_lodHS;
			dx_ptr<ID3D11DomainShader> m_tessDS, m_heightDS;
			dx_ptr<ID3D11PixelShader> m_simplePS, m_phongPS;

			dx_ptr<ID3D11ShaderResourceView> m_diffuseMap, m_normalMap, m_heightMap;
			dx_ptr<ID3D11SamplerState> m_sampler;
		};
	}
}