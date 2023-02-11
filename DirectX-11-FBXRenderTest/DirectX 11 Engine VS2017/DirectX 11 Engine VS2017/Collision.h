#pragma once

#include <map>
#include <DirectXCollision.h>
using namespace DirectX;


class Collision
{
public:
	/*Collision();
	~Collision();*/
	//BoundingOrientedBox
	BoundingBox obb;
	DirectX::XMFLOAT3* corners = NULL;

	void CreateObbByPointppp(int vcount, const char* vertex);

private:

};

