#include "Model.h"

bool Model::Initialize(const std::string & filePath, ID3D11Device * device, ID3D11DeviceContext * deviceContext, ID3D11ShaderResourceView * texture, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader,bool createP)
{
	this->device = device;
	this->deviceContext = deviceContext;
	this->texture = texture;
	this->cb_vs_vertexshader = &cb_vs_vertexshader;
	this->useDebugMesh = createP;

	try
	{
		fbxmodel->CreateFBXStatus(filePath.c_str(), this->fbxmodel);

		auto pMeshData = fbxmodel->GetMeshData();
		if (pMeshData->pVertexBuff == 0 || pMeshData->nVertexCount == 0)
		{
			return false;
		}
		//create vertex buff
		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.ByteWidth = sizeof(Vertex) * fbxmodel->GetMeshData()->nVertexCount;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = fbxmodel->GetMeshData()->pVertexBuff;//
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;

		HRESULT hr = device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, vertexbuffer.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize vertex buffer.");

		//create index buff
		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(DWORD) * fbxmodel->GetMeshData()->nIndexCount;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexBufferData;
		indexBufferData.pSysMem = fbxmodel->GetMeshData()->pIndexBuff;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;

		hr = device->CreateBuffer(&indexBufferDesc, &indexBufferData, indexbuffer.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer.");

		if (createP)
		{

			//collider
			DWORD s_indices[] =
			{
				0, 1, 2, //FRONT
				0, 2, 3, //FRONT
				4, 7, 6, //BACK 
				4, 6, 5, //BACK
				3, 2, 6, //RIGHT SIDE
				3, 6, 7, //RIGHT SIDE
				4, 5, 1, //LEFT SIDE
				4, 1, 0, //LEFT SIDE
				1, 5, 6, //TOP
				1, 6, 2, //TOP
				0, 3, 7, //BOTTOM
				0, 7, 4, //BOTTOM
			};
			collider->CreateObbByPointppp(pMeshData->nVertexCount, pMeshData->pVertexBuff);
			auto verts = collider->corners;
			InitializeP(verts, s_indices, 8, 36);
		}
	}
	catch (COMException & exception)
	{
		ErrorLogger::Log(exception);
		return false;
	}

	this->scl = XMFLOAT3(1, 1, 1);
	this->sclVector = XMLoadFloat3(&this->scl);
	this->SetPosition(0.0f, 0.0f, 0.0f);
	this->SetRotation(0.0f, 0.0f, 0.0f);
	this->UpdateWorldMatrix();

	
	return true;
}

bool Model::InitializeP(const XMFLOAT3* verts_buff, const DWORD* indies_buff, int vcount, int icount) 
{
	verts_p = verts_buff;
	indies_p = indies_buff;
	vcount_p = vcount;
	icount_p = icount;

	for (size_t i = 0; i < vcount; i++)
	{
		Vertex vert;
		vert.pos = verts_p[i];
		vert.normal = XMFLOAT3(0, 0, 0);
		vert.tangent = XMFLOAT3(0, 0, 0);
		vert.texCoord = XMFLOAT2(0, 0);
		vertex_p.push_back(vert);
	}

	try
	{
		auto pMeshData = verts_buff;
		if (verts_buff == 0 || vcount == 0)
		{
			return false;
		}
		//create vertex buff
		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.ByteWidth = sizeof(Vertex) * vcount;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = (float*)vertex_p.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;

		HRESULT hr = device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, vertexbuffer_p.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize vertex buffer.");

		//create index buff
		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(DWORD) * icount;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexBufferData;
		indexBufferData.pSysMem = indies_buff;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;

		hr = device->CreateBuffer(&indexBufferDesc, &indexBufferData, indexbuffer_p.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer.");
	}
	catch (COMException & exception)
	{
		ErrorLogger::Log(exception);
		return false;
	}
	return true;
}

void Model::SetTexture(ID3D11ShaderResourceView * texture)
{
	this->texture = texture;
}

void Model::Draw(const XMMATRIX & viewProjectionMatrix)
{
	//Update Constant buffer with WVP Matrix
	auto sclMatrix = XMMatrixScalingFromVector(sclVector);
	this->cb_vs_vertexshader->data.mat = this->transfomrMatirx * sclMatrix * this->worldMatrix * viewProjectionMatrix;//
	this->cb_vs_vertexshader->data.mat = XMMatrixTranspose(this->cb_vs_vertexshader->data.mat);
	this->cb_vs_vertexshader->data.mat_wvp = this->transfomrMatirx * sclMatrix * this->worldMatrix;//
	this->cb_vs_vertexshader->data.mat_wvp = XMMatrixTranspose(this->cb_vs_vertexshader->data.mat_wvp);
	this->cb_vs_vertexshader->ApplyChanges();
	this->deviceContext->VSSetConstantBuffers(0, 1, this->cb_vs_vertexshader->GetAddressOf());

	//this->deviceContext->PSSetShaderResources(0, 1, &this->texture); //Set Texture
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	stride = sizeof(Vertex);
	UINT uiOffset = 0;
	this->deviceContext->IASetVertexBuffers(0, 1, this->vertexbuffer.GetAddressOf(), &stride, &uiOffset);
	this->deviceContext->IASetIndexBuffer(this->indexbuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	this->deviceContext->DrawIndexed(fbxmodel->GetMeshData()->nIndexCount, 0, 0);

	//if (useDebugMesh) {
	//	DrawP(viewProjectionMatrix);
	//}
	
}

void Model::DrawP(const XMMATRIX & viewProjectionMatrix)
{
	//update debug mesh coord
	auto sclMatrix = XMMatrixScalingFromVector(sclVector);
	

	D3D11_MAPPED_SUBRESOURCE kMappedResource;
	memset(&kMappedResource, 0, sizeof(kMappedResource));
	HRESULT hr = deviceContext->Map(vertexbuffer_p.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &kMappedResource);
	if (SUCCEEDED(hr))
	{
		memcpy(kMappedResource.pData, vertex_p.data(), 8u * sizeof(Vertex));
		deviceContext->Unmap(vertexbuffer_p.Get(), 0);
	}

	//draw debug mesh
	deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);

	//Update Constant buffer with WVP Matrix
	//	auto sclMatrix = XMMatrixScalingFromVector(sclVector);
	this->cb_vs_vertexshader->data.mat = this->transfomrMatirx * sclMatrix * this->worldMatrix * viewProjectionMatrix;//
	this->cb_vs_vertexshader->data.mat = XMMatrixTranspose(this->cb_vs_vertexshader->data.mat);
	this->cb_vs_vertexshader->data.mat_wvp = this->transfomrMatirx * sclMatrix * this->worldMatrix;//
	this->cb_vs_vertexshader->data.mat_wvp = XMMatrixTranspose(this->cb_vs_vertexshader->data.mat_wvp);
	this->cb_vs_vertexshader->ApplyChanges();
	this->deviceContext->VSSetConstantBuffers(0, 1, this->cb_vs_vertexshader->GetAddressOf());

	//this->deviceContext->PSSetShaderResources(0, 1, &this->texture); //Set Texture

	stride = sizeof(Vertex);
	UINT uiOffset = 0;
	this->deviceContext->IASetVertexBuffers(0, 1, this->vertexbuffer_p.GetAddressOf(), &stride, &uiOffset);
	this->deviceContext->IASetIndexBuffer(this->indexbuffer_p.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	this->deviceContext->DrawIndexed(icount_p, 0, 0);
}


void Model::Update(float fDeltaTime) {

	if (fbxmodel->GetKeyFrameCount() > 0)
	{
		m_fAccTime += fDeltaTime / 1000.0f;
		const FBXMeshData* pMeshData = fbxmodel->GetAnimationMeshData(m_fAccTime);

		ID3D11DeviceContext* pD3DDeviceContext = deviceContext;

		////<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		//把顶点数据从CPU内存拷贝到GPU内存中
		D3D11_MAPPED_SUBRESOURCE kMappedResource;
		memset(&kMappedResource, 0, sizeof(kMappedResource));
		HRESULT hr = pD3DDeviceContext->Map(vertexbuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &kMappedResource);
		if (SUCCEEDED(hr))
		{
			memcpy(kMappedResource.pData, pMeshData->pVertexBuff, pMeshData->nVertexBuffSize);
			pD3DDeviceContext->Unmap(vertexbuffer.Get(), 0);
		}
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

		//update debug corners
		collider->CreateObbByPointppp(pMeshData->nVertexCount, pMeshData->pVertexBuff);
		
		vertex_p.clear();
		auto verts = collider->corners;
		for (size_t i = 0; i < vcount_p; i++)
		{
			Vertex vert;
			vert.pos = verts[i];
			vert.normal = XMFLOAT3(0, 0, 0);
			vert.tangent = XMFLOAT3(0, 0, 0);
			vert.texCoord = XMFLOAT2(0, 0);
			vertex_p.push_back(vert);
		}
	}
}

void Model::UpdateWorldMatrix()
{
	this->worldMatrix = XMMatrixRotationRollPitchYaw(this->rot.x, this->rot.y, this->rot.z) * XMMatrixTranslation(this->pos.x, this->pos.y, this->pos.z);
	XMMATRIX vecRotationMatrix = XMMatrixRotationRollPitchYaw(0.0f, this->rot.y, 0.0f);
	this->vec_forward = XMVector3TransformCoord(this->DEFAULT_FORWARD_VECTOR, vecRotationMatrix);
	this->vec_backward = XMVector3TransformCoord(this->DEFAULT_BACKWARD_VECTOR, vecRotationMatrix);
	this->vec_left = XMVector3TransformCoord(this->DEFAULT_LEFT_VECTOR, vecRotationMatrix);
	this->vec_right = XMVector3TransformCoord(this->DEFAULT_RIGHT_VECTOR, vecRotationMatrix);
}

const XMVECTOR & Model::GetPositionVector() const
{
	return this->posVector;
}

const XMFLOAT3 & Model::GetPositionFloat3() const
{
	return this->pos;
}

const XMVECTOR & Model::GetRotationVector() const
{
	return this->rotVector;
}

const XMFLOAT3 & Model::GetRotationFloat3() const
{
	return this->rot;
}

void Model::SetPosition(const XMVECTOR & pos)
{
	XMStoreFloat3(&this->pos, pos);
	this->posVector = pos;
	this->UpdateWorldMatrix();
}

void Model::SetPosition(const XMFLOAT3 & pos)
{
	this->pos = pos;
	this->posVector = XMLoadFloat3(&this->pos);
	this->UpdateWorldMatrix();
}

void Model::SetPosition(float x, float y, float z)
{
	this->pos = XMFLOAT3(x, y, z);
	this->posVector = XMLoadFloat3(&this->pos);
	this->UpdateWorldMatrix();
}

void Model::AdjustPosition(const XMVECTOR & pos)
{
	this->posVector += pos;
	XMStoreFloat3(&this->pos, this->posVector);
	this->UpdateWorldMatrix();
}

void Model::AdjustPosition(const XMFLOAT3 & pos)
{
	this->pos.x += pos.y;
	this->pos.y += pos.y;
	this->pos.z += pos.z;
	this->posVector = XMLoadFloat3(&this->pos);
	this->UpdateWorldMatrix();
}

void Model::AdjustPosition(float x, float y, float z)
{
	this->pos.x += x;
	this->pos.y += y;
	this->pos.z += z;
	this->posVector = XMLoadFloat3(&this->pos);
	this->UpdateWorldMatrix();
}

void Model::SetRotation(const XMVECTOR & rot)
{
	this->rotVector = rot;
	XMStoreFloat3(&this->rot, rot);
	this->UpdateWorldMatrix();
}

void Model::SetRotation(const XMFLOAT3 & rot)
{
	this->rot = rot;
	this->rotVector = XMLoadFloat3(&this->rot);
	this->UpdateWorldMatrix();
}

void Model::SetRotation(float x, float y, float z)
{
	this->rot = XMFLOAT3(x, y, z);
	this->rotVector = XMLoadFloat3(&this->rot);
	this->UpdateWorldMatrix();
}

void Model::AdjustRotation(const XMVECTOR & rot)
{
	this->rotVector += rot;
	XMStoreFloat3(&this->rot, this->rotVector);
	this->UpdateWorldMatrix();
}

void Model::AdjustRotation(const XMFLOAT3 & rot)
{
	this->rot.x += rot.x;
	this->rot.y += rot.y;
	this->rot.z += rot.z;
	this->rotVector = XMLoadFloat3(&this->rot);
	this->UpdateWorldMatrix();
}

void Model::AdjustRotation(float x, float y, float z)
{
	this->rot.x += x;
	this->rot.y += y;
	this->rot.z += z;
	this->rotVector = XMLoadFloat3(&this->rot);
	this->UpdateWorldMatrix();
}

void Model::SetLookAtPos(XMFLOAT3 lookAtPos)
{
	//Verify that look at pos is not the same as cam pos. They cannot be the same as that wouldn't make sense and would result in undefined behavior.
	if (lookAtPos.x == this->pos.x && lookAtPos.y == this->pos.y && lookAtPos.z == this->pos.z)
		return;

	lookAtPos.x = this->pos.x - lookAtPos.x;
	lookAtPos.y = this->pos.y - lookAtPos.y;
	lookAtPos.z = this->pos.z - lookAtPos.z;

	float pitch = 0.0f;
	if (lookAtPos.y != 0.0f)
	{
		const float distance = sqrt(lookAtPos.x * lookAtPos.x + lookAtPos.z * lookAtPos.z);
		pitch = atan(lookAtPos.y / distance);
	}

	float yaw = 0.0f;
	if (lookAtPos.x != 0.0f)
	{
		yaw = atan(lookAtPos.x / lookAtPos.z);
	}
	if (lookAtPos.z > 0)
		yaw += XM_PI;

	this->SetRotation(pitch, yaw, 0.0f);
}

const XMVECTOR & Model::GetForwardVector()
{
	return this->vec_forward;
}

const XMVECTOR & Model::GetRightVector()
{
	return this->vec_right;
}

const XMVECTOR & Model::GetBackwardVector()
{
	return this->vec_backward;
}

const XMVECTOR & Model::GetLeftVector()
{
	return this->vec_left;
}

FBXModel * Model::GetFBXModel()
{
	return fbxmodel;
}
