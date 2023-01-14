#pragma once
#include <fbxsdk.h>
#include "BitFlag.h"
#include "FBXDefine.h"
#include "ErrorLogger.h"
#include <vector>

class FBXModel
{
public:
	FBXModel();
	~FBXModel();

	const FBXMeshData* GetMeshData() const;
	const FBXControlPointGroup* GetControlPointGroup() const;
	const FBXBoneGroup* GetBoneGroup() const;
	const FBXModelAnimation* GetAnimation() const;

	unsigned int m_kcurrentAnimaionIndex = 0;

	//
	const FBXMeshData* GetAnimationMeshData(float fTime);
	//目前模型中只有一个动画。获取动画中有多少个关键帧。
	int GetKeyFrameCount() const;
	float GetAnimTimeLength();
	//计算Mesh的包围盒。
	void CalculateMeshBoundingBox(XMFLOAT3* pMinPos, XMFLOAT3* pMaxPos);

	void CreateFBXStatus(const char* szFileName, FBXModel* pFBXModel);
	void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene);
	bool LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename);
	void DestroySdkObjects(FbxManager* pManager, bool pExitStatus);
	void ParseMeshVertexTotalCountAndVertexType(FbxNode* pNode, BitFlag* pVertexType, int* pTotalCount, bool* pAllMeshOK);
	void ParseSingleMeshVertexType(FbxMesh* pFbxMesh, BitFlag* pVertexType, bool* pAllMeshOK, bool bFirstMesh);
	void ParseMeshData(FbxNode* pNode, FBXMeshData* pMeshData, int* pProcessedVertexCount, int* pProcessedControlPointCount);
	void LoadSingleMesh(FbxMesh* pFbxMesh, FBXMeshData* pMeshData, int* pProcessedVertexCount, int* pProcessedControlPointCount);
	void LoadSingleMesh_Normal(FbxMesh* pFbxMesh, int nVertexIndex, FbxVector4* pNormal);
	void LoadSingleMesh_Tangent(FbxMesh* pFbxMesh, int nVertexIndex, FbxVector4* pTangent);
	void LoadSingleMesh_Color(FbxMesh* pFbxMesh, int nVertexIndex, FbxColor* pColor);
	void LoadSingleMesh_UV(FbxMesh* pFbxMesh, int nUVIndex, int nVertexIndex, FbxVector2* pUV);
	void ParseControlPointTotalCount(FbxNode* pNode, int* pTotalCount);
	void ParseAllControlPoint(FbxNode* pNode, FBXControlPointGroup* pControlPointGroup);
	void LoadControlPointFromMesh(FbxMesh* pMesh, FBXControlPointGroup* pControlPointGroup);
	void ParseAllControlPointSkinWeight(FbxNode* pNode, FBXControlPointGroup* pControlPointGroup, int* pProcessedControlPointCount);
	void LoadSingleControlPointSkinWeight(FbxMesh* pMesh, FBXControlPointGroup* pControlPointGroup, int* pProcessedControlPointCount);
	void ParseBoneTotalCount(FbxNode* pNode, int* pTotalCount);
	void ParseBoneHierarchy(FbxNode* pNode, FBXBoneGroup* pGroup, int nParentIndex);
	void ParseAllBoneMatrixFromBoneSpaceToWorldSpace(FbxNode* pNode, FbxNode* pRootNode, FBXBoneGroup* pBoneGroup);
	void GetGeometryTransformation(FbxNode* pNode, FbxAMatrix* pMat);
	void LoadSingleBoneMatrixFromBoneSpaceToWorldSpace(FbxNode* pNode, FbxMesh* pMesh, FbxNode* pRootNode, FBXBoneGroup* pBoneGroup);
	void FindBoneByIndexAndName(FbxNode* pNode, int nBoneIndex, const char* szBoneName, FbxNode** ppResultNode, int* pAccBoneCount);
	void ConvertFbxAMatrixToDirectXMatrix(const FbxAMatrix* pFbxAMatrix, XMMATRIX* pSoMatrix);
	void ParseAllBoneAnimationData(FbxScene* pSDKScene, FBXBoneGroup* pBoneGroup);//, FBXModelAnimation* pModelAnim

protected:
	FBXMeshData* GetMeshData_Modify();
	FBXControlPointGroup* GetControlPointGroup_Modify();
	FBXBoneGroup* GetBoneGroup_Modify();
	FBXModelAnimation* GetAnimation_Modify();
	void CalculateMeshDataByKeyFrame(const int nKeyFrameIndex);

private:
	//载入数据后，这里的Mesh数据渲染出来就是默认的Pose；
	//当执行CalculateMeshDataByKeyFrame()后，这里的Mesh数据就变成了指定帧的Pose。
	FBXMeshData m_kMeshData;
	FBXControlPointGroup m_kControlPointGroup;
	FBXBoneGroup m_kBoneGroup;
	//FBXModelAnimation m_kAnimation;
	std::vector<FBXModelAnimation*> m_kAnimations;

	
};