#pragma once
#include "AdapterReader.h"
#include "Shaders.h"
#include "Vertex.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <WICTextureLoader.h>
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "Camera.h"
#include "..\\Timer.h"
#include "ImGUI\\imgui.h"
#include "ImGUI\\imgui_impl_win32.h"
#include "ImGUI\\imgui_impl_dx11.h"
#include "Model.h"
#include "../Skybox.h"
#include "..//Collision.h"

class Graphics
{
public:
	bool Initialize(HWND hwnd, int width, int height);
	void RenderFrame(float dt);
	Camera camera;
	Model* model = NULL;
	Model* quad = NULL;
	Model* sphere = NULL;
	Model* collider = NULL;

	bool inte = false;

	bool debugswitch = true;
	
private:
	bool InitializeDirectX(HWND hwnd);
	bool InitializeShaders();
	bool InitializeScene();
	bool InitializeIBLStatus();

	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;

	VertexShader vertexshader;
	PixelShader pixelshader;
	PixelShader pixelmatshader;
	
	VertexShader vertexshader_skyBox;
	PixelShader pixelshader_skyBox;

	PixelShader ConvolutionPixelShader;
	PixelShader PrefilterMapPixelShader;
	PixelShader IntegrateBRDFPixelShader;
	PixelShader DisneyBSDFPixelShader;
	ConstantBuffer<CB_VS_vertexshader> cb_vs_vertexshader;
	ConstantBuffer<CB_PS_pixelshader> cb_ps_pixelshader;
	ConstantBuffer<CB_PS_lightData> cb_ps_lightdata;
	ConstantBuffer<CB_PS_IBLSTATUS> cb_ps_iblstatus;
	ConstantBuffer<cb_ps_BSDFData> cb_ps_BSDFData;
	
	

	VertexBuffer<Vertex> vertexBuffer;
	IndexBuffer indicesBuffer;

	Skybox* skybox = NULL;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthUnenableStencilState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> IBLskydepthStencilState;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_CullFront;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> IBLskyrasterizerState;

	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;

	std::unique_ptr<DirectX::SpriteBatch> spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> spriteFont;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pinkTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> grassTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pavementTexture;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> AluminiumInsulator_Albedo;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> AluminiumInsulator_Height;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> AluminiumInsulator_Metallic;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> AluminiumInsulator_Normal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> AluminiumInsulator_Roughness;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Leather_Albedo;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Leather_Height;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Leather_Metallic;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Leather_Normal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Leather_Roughness;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Wood_Albedo;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Wood_Height;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Wood_Metallic;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Wood_Normal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Wood_Roughness;
	//IBL
	ID3D11Texture2D* skyIBLtex;
	//ID3D11RenderTargetView* skyIBLRTV[6];
	ID3D11ShaderResourceView* skyIBLSRV;

	ID3D11Texture2D* envMaptex;
	//ID3D11RenderTargetView* envMapRTV[6];
	ID3D11ShaderResourceView* envMapSRV;

	ID3D11Texture2D* brdfLUTtex;
	//ID3D11RenderTargetView* brdfLUTRTV;
	ID3D11ShaderResourceView* brdfLUTSRV;

	Collision* test;

	int windowWidth = 0;
	int windowHeight = 0;
	//Timer fpsTimer;

};