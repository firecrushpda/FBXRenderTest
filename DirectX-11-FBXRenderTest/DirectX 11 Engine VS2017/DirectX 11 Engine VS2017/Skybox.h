#pragma once
#include "Graphics/Model.h"
class Skybox
{
public:
	Skybox();
	~Skybox();

	bool Initialize( ID3D11Device * device, 
					ID3D11DeviceContext * deviceContext,
					ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader);

	Model* model = NULL;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView1 = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView2 = nullptr;

	HRESULT CreateWICTexture2DCubeFromFile(
		ID3D11Device * d3dDevice,
		ID3D11DeviceContext * d3dDeviceContext,
		const std::wstring & cubeMapFileName,
		ID3D11Texture2D ** textureArray,
		ID3D11ShaderResourceView ** textureCubeView,
		bool generateMips);

	HRESULT CreateWICTexture2DCubeFromFile(
		ID3D11Device * d3dDevice,
		ID3D11DeviceContext * d3dDeviceContext,
		const std::vector<std::wstring>& cubeMapFileNames,
		ID3D11Texture2D ** textureArray,
		ID3D11ShaderResourceView ** textureCubeView,
		bool generateMips);
private:

};



