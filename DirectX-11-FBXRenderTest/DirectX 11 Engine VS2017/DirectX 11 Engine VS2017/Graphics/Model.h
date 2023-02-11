#pragma once
#include "Mesh.h"
#include "../FBXModel.h"
#include "../Collision.h"

using namespace DirectX;

class Model
{
public:
	Model() {	fbxmodel = new FBXModel(); 
				collider = new Collision;
				m_fAccTime = 0;};
	bool Initialize(const std::string & filePath, ID3D11Device * device, ID3D11DeviceContext * deviceContext, ID3D11ShaderResourceView * texture, ConstantBuffer<CB_VS_vertexshader> & cb_vs_vertexshader,bool createP);
	bool InitializeP(const XMFLOAT3* verts,const DWORD* indies,int vcount,int icount);
	void SetTexture(ID3D11ShaderResourceView * texture);
	void Draw(const XMMATRIX & viewProjectionMatrix);
	void DrawP(const XMMATRIX & viewProjectionMatrix);
	void Update(float fDeltaTime);

	const XMVECTOR & GetPositionVector() const;
	const XMFLOAT3 & GetPositionFloat3() const;
	const XMVECTOR & GetRotationVector() const;
	const XMFLOAT3 & GetRotationFloat3() const;

	void SetPosition(const XMVECTOR & pos);
	void SetPosition(const XMFLOAT3 & pos);
	void SetPosition(float x, float y, float z);
	void AdjustPosition(const XMVECTOR & pos);
	void AdjustPosition(const XMFLOAT3 & pos);
	void AdjustPosition(float x, float y, float z);
	void SetRotation(const XMVECTOR & rot);
	void SetRotation(const XMFLOAT3 & rot);
	void SetRotation(float x, float y, float z);
	void AdjustRotation(const XMVECTOR & rot);
	void AdjustRotation(const XMFLOAT3 & rot);
	void AdjustRotation(float x, float y, float z);
	void SetLookAtPos(XMFLOAT3 lookAtPos);
	const XMVECTOR & GetForwardVector();
	const XMVECTOR & GetRightVector();
	const XMVECTOR & GetBackwardVector();
	const XMVECTOR & GetLeftVector();

	FBXModel* GetFBXModel();
	XMMATRIX transfomrMatirx = XMMatrixIdentity();
	/*= XMMatrixSet(
		1, 0, 0, 0,
		0, 0, -1, 0,
		0, 1, 0, 0,
		0, 0, 0, 1
	);*/

	UINT stride;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexbuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexbuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexbuffer_p;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexbuffer_p;

	XMVECTOR sclVector;
	XMFLOAT3 scl;
	XMMATRIX worldMatrix = XMMatrixIdentity();

	Collision* collider = NULL;
	//debug primitive data (if have)
	std::vector<Vertex> vertex_p;
	const XMFLOAT3* verts_p = NULL;
	const DWORD* indies_p = NULL;
	int vcount_p = 0;
	int icount_p = 0;
	bool useDebugMesh = false;

private:

	FBXModel* fbxmodel = NULL;
	void UpdateWorldMatrix();

	ID3D11Device * device = nullptr;
	ID3D11DeviceContext * deviceContext = nullptr;
	ConstantBuffer<CB_VS_vertexshader> * cb_vs_vertexshader = nullptr;
	ID3D11ShaderResourceView * texture = nullptr;
	
	

	XMVECTOR posVector;
	XMVECTOR rotVector;
	
	XMFLOAT3 pos;
	XMFLOAT3 rot;
	

	const XMVECTOR DEFAULT_FORWARD_VECTOR = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	const XMVECTOR DEFAULT_UP_VECTOR = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	const XMVECTOR DEFAULT_BACKWARD_VECTOR = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
	const XMVECTOR DEFAULT_LEFT_VECTOR = XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f);
	const XMVECTOR DEFAULT_RIGHT_VECTOR = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	XMVECTOR vec_forward;
	XMVECTOR vec_left;
	XMVECTOR vec_right;
	XMVECTOR vec_backward;

	float m_fAccTime;
};