#include "Collision.h"
#include "Graphics/Vertex.h"
#include <vector>

void Collision::CreateObbByPointppp(int vcount,const char* vertex)
{
	//create bounding box
	auto tmep =(Vertex*) (vertex);//(DirectX::XMFLOAT3*) 

	std::vector<DirectX::XMFLOAT3>list;
	for (size_t i = 0; i < vcount; i++)
	{
		list.push_back(XMFLOAT3(tmep[i].pos.x, tmep[i].pos.y, tmep[i].pos.z));
	}
	this->obb.CreateFromPoints(obb, vcount, list.data(), sizeof(DirectX::XMFLOAT3));//
	list.clear();

	corners = NULL;
	corners = (DirectX::XMFLOAT3*)malloc(sizeof(DirectX::XMFLOAT3) * 8u);
	obb.GetCorners((DirectX::XMFLOAT3*)corners);
}