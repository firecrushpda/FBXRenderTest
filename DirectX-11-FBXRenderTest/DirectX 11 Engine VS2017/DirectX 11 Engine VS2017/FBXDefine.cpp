//----------------------------------------------------------------
#include "FBXDefine.h"
#include "ErrorLogger.h"
#include "StringConverter.h"
//----------------------------------------------------------------
FBXMeshData::FBXMeshData()
	:pVertexBuff(NULL)
	, nVertexBuffSize(0)
	, nVertexCount(0)
	, pIndexBuff(NULL)
	, nIndexBuffSize(0)
	, nIndexCount(0)
	, pVertexIndex2ControlPointIndex(NULL)
	, nSizeofVertexData(0)
{

}
//----------------------------------------------------------------
FBXMeshData::~FBXMeshData()
{
	if (pVertexBuff)
	{
		free(pVertexBuff);
		pVertexBuff = NULL;
	}
	if (pIndexBuff)
	{
		free(pIndexBuff);
		pIndexBuff = NULL;
	}
	if (pVertexIndex2ControlPointIndex)
	{
		free(pVertexIndex2ControlPointIndex);
		pVertexIndex2ControlPointIndex = NULL;
	}
}
//----------------------------------------------------------------
void FBXMeshData::SetVertexType(const BitFlag& kType)
{
	kVertexType.SetValue(kType.GetValue());
	//计算顶点结构的sizeof值。
	//顶点坐标。
	nSizeofVertexData = FBX_Sizeof_Vector3;
	//法线。
	if (kVertexType.IsFlagExist(FBXElement_Normal))
	{
		nSizeofVertexData += FBX_Sizeof_Vector3;
	}
	//切线。
	if (kVertexType.IsFlagExist(FBXElement_Tangent))
	{
		nSizeofVertexData += FBX_Sizeof_Vector3;
	}
	//顶点色。
	if (kVertexType.IsFlagExist(FBXElement_Color))
	{
		nSizeofVertexData += FBX_Sizeof_Color;
	}
	//UV1。
	if (kVertexType.IsFlagExist(FBXElement_UV1))
	{
		nSizeofVertexData += FBX_Sizeof_UV;
	}
	//UV2。
	if (kVertexType.IsFlagExist(FBXElement_UV2))
	{
		nSizeofVertexData += FBX_Sizeof_UV;
	}
}
//----------------------------------------------------------------
void FBXMeshData::ReserveVertexCount(const int nVertexTotalCount)
{
	if (pVertexBuff)
	{
		free(pVertexBuff);
		pVertexBuff = NULL;
	}
	if (pIndexBuff)
	{
		free(pIndexBuff);
		pIndexBuff = NULL;
	}
	if (pVertexIndex2ControlPointIndex)
	{
		free(pVertexIndex2ControlPointIndex);
		pVertexIndex2ControlPointIndex = NULL;
	}

	nVertexCount = nVertexTotalCount;
	nVertexBuffSize = nVertexCount * nSizeofVertexData;
	nIndexCount = nVertexTotalCount;
	nIndexBuffSize = nIndexCount * sizeof(unsigned int);
	pVertexBuff = (char*)malloc(nVertexBuffSize);
	pIndexBuff = (char*)malloc(nIndexBuffSize);
	pVertexIndex2ControlPointIndex = (int*)malloc(nVertexTotalCount * sizeof(int));
}
//----------------------------------------------------------------
FBXBoneIndexSkinWeight::FBXBoneIndexSkinWeight()
	:nBoneIndex(-1)
	, fSkinWeight(0.0f)
{

}
//----------------------------------------------------------------
FBXControlPoint::FBXControlPoint()
{
	kVertex = XMFLOAT3(0,0,0);
	for (int i = 0; i < FBX_MaxCount_BoneIndexSkinWeightPerControlPoint; ++i)
	{
		kPairList[i].kBoneName.clear();
		kPairList[i].nBoneIndex = -1;
		kPairList[i].fSkinWeight = 0.0f;
	}
}
//----------------------------------------------------------------
void FBXControlPoint::AddBoneNameSkinWeight(const char* szBoneName, float fSkinWeight)
{
	if (szBoneName == NULL || szBoneName[0] == 0)
	{
		ErrorLogger::Log("FBXControlPoint::AddBoneNameSkinWeight : BoneName is Invalid");
		return;
	}

	bool bSuccess = false;
	for (int i = 0; i < FBX_MaxCount_BoneIndexSkinWeightPerControlPoint; ++i)
	{
		if (kPairList[i].kBoneName.length() == 0)
		{
			kPairList[i].kBoneName = szBoneName;
			kPairList[i].fSkinWeight = fSkinWeight;
			bSuccess = true;
			break;
		}
	}
	if (bSuccess == false)
	{
		ErrorLogger::Log("StFBXControlPoint::AddBoneNameSkinWeight : kPairList is full");
	}
}
//----------------------------------------------------------------
FBXControlPointGroup::FBXControlPointGroup()
	:kControlPointArray(NULL)
	, nControlPointValidCount(0)
	, nControlPointMaxCount(0)
{

}
//----------------------------------------------------------------
FBXControlPointGroup::~FBXControlPointGroup()
{
	if (kControlPointArray)
	{
		delete[] kControlPointArray;
		kControlPointArray = NULL;
	}
}
//----------------------------------------------------------------
void FBXControlPointGroup::ReserveControlPointCount(int nCount)
{
	if (kControlPointArray)
	{
		delete[] kControlPointArray;
		kControlPointArray = NULL;
	}

	kControlPointArray = new FBXControlPoint[nCount];
	nControlPointValidCount = 0;
	nControlPointMaxCount = nCount;
}
//----------------------------------------------------------------
FBXControlPoint* FBXControlPointGroup::TakeNew()
{
	if (nControlPointValidCount < nControlPointMaxCount)
	{
		++nControlPointValidCount;
		return kControlPointArray + (nControlPointValidCount - 1);
	}
	else
	{
		ErrorLogger::Log("FBXControlPointGroup::TakeNew : kControlPointArray is full");
		return NULL;
	}
}
//----------------------------------------------------------------
FBXControlPoint* FBXControlPointGroup::GetAt(int nIndex) const
{
	if (nIndex >= 0 && nIndex < nControlPointValidCount)
	{
		return kControlPointArray + nIndex;
	}
	else
	{
		return NULL;
	}
}
//----------------------------------------------------------------
int FBXControlPointGroup::GetSize() const
{
	return nControlPointValidCount;
}
//----------------------------------------------------------------
void FBXControlPointGroup::MakeBoneIndexByBoneName(const FBXBoneGroup* pBoneGroup)
{
	for (int i = 0; i < nControlPointValidCount; ++i)
	{
		FBXBoneIndexSkinWeight* pPairList = kControlPointArray[i].kPairList;
		for (int j = 0; j < FBX_MaxCount_BoneIndexSkinWeightPerControlPoint; ++j)
		{
			if (pPairList[j].kBoneName.length() > 0)
			{
				const char* szBoneName = pPairList[j].kBoneName.c_str();
				int nBoneIndex = pBoneGroup->GetBoneIndexByBoneName(szBoneName);
				pPairList[j].nBoneIndex = nBoneIndex;
				//
				if (nBoneIndex == -1)
				{
					ErrorLogger::Log("FBXControlPointGroup::MakeBoneIndexByBoneName : Can not find bone ");
				}
			}
		}
	}
}
//----------------------------------------------------------------
FBXBone::FBXBone()
	:nParentIndex(-1)
	, kMatFromBoneSpaceToModelSpace(XMMatrixIdentity())
{
	for (int i = 0; i < FBX_MaxCount_ChildBone; ++i)
	{
		kChildIndexList[i] = -1;
	}
}
//----------------------------------------------------------------
FBXBone::~FBXBone()
{

}
//----------------------------------------------------------------
FBXBoneGroup::FBXBoneGroup()
	:kBoneArray(NULL)
	, nBoneValidCount(0)
	, nBoneMaxCount(0)
{

}
//----------------------------------------------------------------
FBXBoneGroup::~FBXBoneGroup()
{
	if (kBoneArray)
	{
		delete[] kBoneArray;
		kBoneArray = NULL;
	}
}
//----------------------------------------------------------------
void FBXBoneGroup::ReserveBoneCount(int nCount)
{
	if (kBoneArray)
	{
		delete[] kBoneArray;
		kBoneArray = NULL;
	}

	kBoneArray = new FBXBone[nCount];
	nBoneValidCount = 0;
	nBoneMaxCount = nCount;
}
//----------------------------------------------------------------
void FBXBoneGroup::GenerateChildren()
{
	for (int i = 0; i < nBoneValidCount; ++i)
	{
		FBXBone* pBone = kBoneArray + i;
		int nChildCount = 0;
		//
		for (int j = 0; j < nBoneValidCount; ++j)
		{
			FBXBone* pChild = kBoneArray + j;
			if (pChild->nParentIndex == i)
			{
				pBone->kChildIndexList[nChildCount] = j;
				++nChildCount;
				if (nChildCount >= FBX_MaxCount_ChildBone)
				{
					break;
				}
			}
		}
	}
}
//----------------------------------------------------------------
int FBXBoneGroup::GetBoneIndexByBoneName(const char* szBoneName) const
{
	int nIndex = -1;
	if (szBoneName != 0 && szBoneName[0] != 0)
	{
		for (int i = 0; i < nBoneValidCount; ++i)
		{
			FBXBone* pBone = kBoneArray + i;
			if (StringConverter::StrCmpNoCase(szBoneName, pBone->kBoneName.c_str()) == 0)
			{
				nIndex = i;
				break;
			}
		}
	}
	return nIndex;
}
//----------------------------------------------------------------
FBXBone* FBXBoneGroup::TakeNew(int* pIndex)
{
	if (nBoneValidCount < nBoneMaxCount)
	{
		if (pIndex)
		{
			*pIndex = nBoneValidCount;
		}
		++nBoneValidCount;
		return kBoneArray + (nBoneValidCount - 1);
	}
	else
	{
		ErrorLogger::Log("FBXBoneGroup::TakeNew : kBoneArray is full");
		return NULL;
	}
}
//----------------------------------------------------------------
FBXBone* FBXBoneGroup::GetAt(int nIndex) const
{
	if (nIndex >= 0 && nIndex < nBoneValidCount)
	{
		return kBoneArray + nIndex;
	}
	else
	{
		return NULL;
	}
}
//----------------------------------------------------------------
int FBXBoneGroup::GetSize() const
{
	return nBoneValidCount;
}
//----------------------------------------------------------------
FBXKeyFrame::FBXKeyFrame()
	:fKeyTime(-1.0f)
	, matKeyTransform(XMMatrixIdentity())
{

}
//----------------------------------------------------------------
FBXBoneAnimation::FBXBoneAnimation()
	:kKeyFrameArray(NULL)
	, nFrameValidCount(0)
	, nFrameMaxCount(0)
	, nBoneIndex(-1)
{

}
//----------------------------------------------------------------
FBXBoneAnimation::~FBXBoneAnimation()
{
	if (kKeyFrameArray)
	{
		delete[] kKeyFrameArray;
		kKeyFrameArray = NULL;
	}
}
//----------------------------------------------------------------
void FBXBoneAnimation::ReserveKeyFrameCount(int nCount)
{
	if (kKeyFrameArray)
	{
		delete[] kKeyFrameArray;
		kKeyFrameArray = NULL;
	}

	kKeyFrameArray = new FBXKeyFrame[nCount];
	nFrameValidCount = 0;
	nFrameMaxCount = nCount;
}
//----------------------------------------------------------------
FBXKeyFrame* FBXBoneAnimation::TakeNew()
{
	if (nFrameValidCount < nFrameMaxCount)
	{
		++nFrameValidCount;
		return kKeyFrameArray + (nFrameValidCount - 1);
	}
	else
	{
		ErrorLogger::Log("StFBXBoneAnimation::TakeNew : kKeyFrameArray is full");
		return NULL;
	}
}
//----------------------------------------------------------------
FBXKeyFrame* FBXBoneAnimation::GetAt(int nIndex) const
{
	if (nIndex >= 0 && nIndex < nFrameValidCount)
	{
		return kKeyFrameArray + nIndex;
	}
	else
	{
		return NULL;
	}
}
//----------------------------------------------------------------
int FBXBoneAnimation::GetSize() const
{
	return nFrameValidCount;
}
//----------------------------------------------------------------
FBXModelAnimation::FBXModelAnimation()
	:kBoneAnimationArray(NULL)
	, nAnimValidCount(0)
	, nAnimMaxCount(0)
	, nFrameCount(0)
	, fAnimLength(0.0f)
{

}
//----------------------------------------------------------------
FBXModelAnimation::~FBXModelAnimation()
{
	if (kBoneAnimationArray)
	{
		delete[] kBoneAnimationArray;
		kBoneAnimationArray = NULL;
	}
}
//----------------------------------------------------------------
void FBXModelAnimation::ReserveBoneCount(int nBoneCount)
{
	if (kBoneAnimationArray)
	{
		delete[] kBoneAnimationArray;
		kBoneAnimationArray = NULL;
	}

	kBoneAnimationArray = new FBXBoneAnimation[nBoneCount];
	nAnimValidCount = 0;
	nAnimMaxCount = nBoneCount;
}
//----------------------------------------------------------------
int FBXModelAnimation::GetSize() const
{
	return nAnimValidCount;
}
//----------------------------------------------------------------
int FBXModelAnimation::GetFrameCount() const
{
	return nFrameCount;
}
//----------------------------------------------------------------
FBXBoneAnimation* FBXModelAnimation::TakeNew()
{
	if (nAnimValidCount < nAnimMaxCount)
	{
		++nAnimValidCount;
		return kBoneAnimationArray + (nAnimValidCount - 1);
	}
	else
	{
		ErrorLogger::Log("FBXModelAnimation::TakeNew : kBoneAnimationArray is full");
		return NULL;
	}
}
//----------------------------------------------------------------
FBXBoneAnimation* FBXModelAnimation::GetAt(int nIndex) const
{
	if (nIndex >= 0 && nIndex < nAnimValidCount)
	{
		return kBoneAnimationArray + nIndex;
	}
	else
	{
		return NULL;
	}
}
//----------------------------------------------------------------
FBXBoneAnimation* FBXModelAnimation::GetBoneAnimation(int nBoneIndex) const
{
	FBXBoneAnimation* pResultBoneAnim = 0;
	if (nBoneIndex >= 0 && nBoneIndex < nAnimValidCount)
	{
		pResultBoneAnim = kBoneAnimationArray + nBoneIndex;
		if (pResultBoneAnim->nBoneIndex == nBoneIndex)
		{
			return pResultBoneAnim;
		}
	}

	pResultBoneAnim = 0;
	for (int i = 0; i < nAnimValidCount; ++i)
	{
		FBXBoneAnimation* pBoneAnim = kBoneAnimationArray + i;
		if (pBoneAnim->nBoneIndex == nBoneIndex)
		{
			pResultBoneAnim = pBoneAnim;
			break;
		}
	}
	return pResultBoneAnim;
}
//----------------------------------------------------------------
int FBXModelAnimation::GetKeyFrameIndexByTime(float fTime)
{
	int nKeyFrameIndex = -1;
	//找出一个帧数大于0的BoneAnimation对象。
	FBXBoneAnimation* pBoneAnim = NULL;
	for (int boneIndex = 0; boneIndex < nAnimValidCount; ++boneIndex)
	{
		if (kBoneAnimationArray[boneIndex].GetSize() > 0)
		{
			pBoneAnim = kBoneAnimationArray + boneIndex;
			break;
		}
	}
	//有了BoneAnimation对象，判断给定的时间落在了哪一帧上。
	if (pBoneAnim)
	{
		for (int i = 0; i < pBoneAnim->nFrameValidCount; ++i)
		{
			FBXKeyFrame* pKeyFrame = pBoneAnim->GetAt(i);
			if (fTime >= pKeyFrame->fKeyTime - 0.001f)
			{
				nKeyFrameIndex = i;
			}
			else
			{
				break;
			}
		}
	}
	return nKeyFrameIndex;
}
//----------------------------------------------------------------
