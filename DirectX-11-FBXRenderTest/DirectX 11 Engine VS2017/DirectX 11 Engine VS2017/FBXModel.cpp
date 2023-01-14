#include "FBXModel.h"

FBXModel::FBXModel()
{

}

FBXModel::~FBXModel()
{

}

const FBXMeshData* FBXModel::GetMeshData() const
{
	return &m_kMeshData;
}

const FBXControlPointGroup* FBXModel::GetControlPointGroup() const
{
	return &m_kControlPointGroup;
}

const FBXBoneGroup* FBXModel::GetBoneGroup() const
{
	return &m_kBoneGroup;
}

const FBXModelAnimation* FBXModel::GetAnimation() const
{
	auto m_kAnimation = m_kAnimations.at(m_kcurrentAnimaionIndex);
	return m_kAnimation;
}

int FBXModel::GetKeyFrameCount() const
{
	auto m_kAnimation = m_kAnimations.at(m_kcurrentAnimaionIndex);
	return m_kAnimation->nFrameCount;
}

float FBXModel::GetAnimTimeLength()
{
	auto m_kAnimation = m_kAnimations.at(m_kcurrentAnimaionIndex);
	return m_kAnimation->fAnimLength;
}

void FBXModel::CalculateMeshBoundingBox(XMFLOAT3* pMinPos, XMFLOAT3* pMaxPos)
{
	float fMinX = FLT_MAX;
	float fMinY = FLT_MAX;
	float fMinZ = FLT_MAX;
	float fMaxX = FLT_MIN;
	float fMaxY = FLT_MIN;
	float fMaxZ = FLT_MIN;

	const int nCPCount = m_kControlPointGroup.GetSize();
	for (int cpIndex = 0; cpIndex < nCPCount; ++cpIndex)
	{
		FBXControlPoint* pCP = m_kControlPointGroup.GetAt(cpIndex);
		if (pCP == 0)
		{
			continue;
		}

		if (pCP->kVertex.x < fMinX)
		{
			fMinX = pCP->kVertex.x;
		}
		if (pCP->kVertex.x > fMaxX)
		{
			fMaxX = pCP->kVertex.x;
		}

		if (pCP->kVertex.y < fMinY)
		{
			fMinY = pCP->kVertex.y;
		}
		if (pCP->kVertex.y > fMaxY)
		{
			fMaxY = pCP->kVertex.y;
		}

		if (pCP->kVertex.z < fMinZ)
		{
			fMinZ = pCP->kVertex.z;
		}
		if (pCP->kVertex.z > fMaxZ)
		{
			fMaxZ = pCP->kVertex.z;
		}
	}

	pMinPos->x = fMinX;
	pMinPos->y = fMinY;
	pMinPos->z = fMinZ;
	pMaxPos->x = fMaxX;
	pMaxPos->y = fMaxY;
	pMaxPos->z = fMaxZ;
}

const FBXMeshData* FBXModel::GetAnimationMeshData(float fTime)
{
	auto m_kAnimation = m_kAnimations.at(m_kcurrentAnimaionIndex);
	float fYuShu = fTime / m_kAnimation->fAnimLength;
	fYuShu = fYuShu - floor(fYuShu);
	float curTime = m_kAnimation->fAnimLength * fYuShu;
	if (curTime < 0.0f)
	{
		curTime = 0.0f;
	}

	int nCurKeyFrameIndex = m_kAnimation->GetKeyFrameIndexByTime(curTime);
	if (nCurKeyFrameIndex != -1)
	{
		CalculateMeshDataByKeyFrame(nCurKeyFrameIndex);
	}

	return &m_kMeshData;
}

FBXControlPointGroup* FBXModel::GetControlPointGroup_Modify()
{
	return &m_kControlPointGroup;
}

FBXBoneGroup* FBXModel::GetBoneGroup_Modify()
{
	return &m_kBoneGroup;
}

FBXModelAnimation* FBXModel::GetAnimation_Modify()
{
	auto m_kAnimation = m_kAnimations.at(m_kcurrentAnimaionIndex);
	return m_kAnimation;
}

FBXMeshData* FBXModel::GetMeshData_Modify()
{
	return &m_kMeshData;
}

void FBXModel::CalculateMeshDataByKeyFrame(const int nKeyFrameIndex)
{
	const XMFLOAT3 kVectorZero(0.0f, 0.0f, 0.0f);
	XMFLOAT3 kFinalCPVertex;
	XMVECTOR xmvecFinalCPVertex;

	const int nCPCount = m_kControlPointGroup.GetSize();
	for (int cpIndex = 0; cpIndex < nCPCount; ++cpIndex)
	{
		FBXControlPoint* pCP = m_kControlPointGroup.GetAt(cpIndex);
		if (pCP == NULL)
		{
			continue;
		}
		if (pCP->kPairList[0].nBoneIndex == -1)
		{
			//本控制点不受任何骨骼的影响
			continue;
		}

		const XMFLOAT3* pCPVertex = (XMFLOAT3*)(&(pCP->kVertex));
		const XMVECTOR xmvecCPVertex = XMLoadFloat3(pCPVertex);
		xmvecFinalCPVertex = XMLoadFloat3(&kVectorZero);

		for (int pairIndex = 0; pairIndex < FBX_MaxCount_BoneIndexSkinWeightPerControlPoint; ++pairIndex)
		{
			const int nBoneIndex = pCP->kPairList[pairIndex].nBoneIndex;
			if (nBoneIndex == -1)
			{
				break;
			}
			FBXBone* pBone = m_kBoneGroup.GetAt(nBoneIndex);
			if (pBone == NULL)
			{
				continue;
			}
			auto m_kAnimation = m_kAnimations.at(m_kcurrentAnimaionIndex);
			FBXBoneAnimation* pBoneAnim = m_kAnimation->GetBoneAnimation(nBoneIndex);
			if (pBoneAnim == NULL)
			{
				continue;
			}
			FBXKeyFrame* pKeyFrame = pBoneAnim->GetAt(nKeyFrameIndex);
			if (pKeyFrame == NULL)
			{
				continue;
			}

			XMFLOAT4X4* pMatSpace = (XMFLOAT4X4*)(&(pBone->kMatFromBoneSpaceToModelSpace));
			XMMATRIX xmmatSpace = XMLoadFloat4x4(pMatSpace);

			XMFLOAT4X4* pMatKeyFrame = (XMFLOAT4X4*)(&(pKeyFrame->matKeyTransform));
			XMMATRIX xmmatKeyFrame = XMLoadFloat4x4(pMatKeyFrame);

			XMVECTOR result = XMVector3TransformCoord(xmvecCPVertex, xmmatSpace);
			result = XMVector3TransformCoord(result, xmmatKeyFrame);
			result *= pCP->kPairList[pairIndex].fSkinWeight;
			xmvecFinalCPVertex += result;
		}

		XMStoreFloat3(&kFinalCPVertex, xmvecFinalCPVertex);

		//遍历顶点列表，如果顶点所属的ControlPoint与cpIndex相同，则更新该顶点。
		//遍历过程中不能执行break操作。
		const int nMeshVertexCount = m_kMeshData.nVertexCount;
		const int* pVertexIndex2ControlPointIndex = m_kMeshData.pVertexIndex2ControlPointIndex;
		for (int vertexIndex = 0; vertexIndex < nMeshVertexCount; ++vertexIndex)
		{
			if (pVertexIndex2ControlPointIndex[vertexIndex] == cpIndex)
			{
				float* pVertexData = (float*)(m_kMeshData.pVertexBuff + m_kMeshData.nSizeofVertexData * vertexIndex);
				pVertexData[0] = kFinalCPVertex.x;
				pVertexData[1] = kFinalCPVertex.y;
				pVertexData[2] = kFinalCPVertex.z;
			}
		}
	}
}
//-----------------------------------------------
void FBXModel::CreateFBXStatus(const char* szFileName, FBXModel* pFBXModel) {
	FbxManager* lSdkManager = NULL;
	FbxScene* lScene = NULL;
	bool lResult;

	InitializeSdkObjects(lSdkManager, lScene);

	FbxString lFilePath(szFileName);
	if (lFilePath.IsEmpty())
	{
		lResult = false;
		FBXSDK_printf("\n\nUsage: ImportScene <FBX file name>\n\n");
	}
	else
	{
		FBXSDK_printf("\n\nFile: %s\n\n", lFilePath.Buffer());
		lResult = LoadScene(lSdkManager, lScene, lFilePath.Buffer());
	}

	if (lResult == false)
	{
		FBXSDK_printf("\n\nAn error occurred while loading the scene...");
	}

	//convert scene AxisSystem
	FbxAxisSystem OurAxisSystem(FbxAxisSystem::eMax);
	FbxAxisSystem SceneAxisSystem = lScene->GetGlobalSettings().GetAxisSystem();
	if (SceneAxisSystem != OurAxisSystem)
	{
		OurAxisSystem.ConvertScene(lScene);
	}

	// Convert Unit System to what is used in this example, if needed
	FbxSystemUnit SceneSystemUnit = lScene->GetGlobalSettings().GetSystemUnit();
	if (SceneSystemUnit.GetScaleFactor() != 1.0)
	{
		//The unit in this example is centimeter.
		FbxSystemUnit::cm.ConvertScene(lScene);
	}

	//convert triangle
	FbxGeometryConverter lGeomConverter(lSdkManager);
	lGeomConverter.Triangulate(lScene, true);

	//split material
	lGeomConverter.SplitMeshesPerMaterial(lScene, true);

	//parse
	FbxNode* pSceneRootNode = lScene->GetRootNode();
	FBXMeshData* pMeshData = pFBXModel->GetMeshData_Modify();
	FBXControlPointGroup* pControlPointGroup = pFBXModel->GetControlPointGroup_Modify();
	FBXBoneGroup* pBoneGroup = pFBXModel->GetBoneGroup_Modify();
	//FBXModelAnimation* pModelAnimation = pFBXModel->GetAnimation_Modify();


	BitFlag kVertexType;
	int nMeshVertexTotalCount = 0;
	bool bAllMeshOK = true;
	ParseMeshVertexTotalCountAndVertexType(pSceneRootNode, &kVertexType, &nMeshVertexTotalCount, &bAllMeshOK);
	if (bAllMeshOK == false)
	{
		ErrorLogger::Log("StFBXManager::LoadFBX : ParseMeshVertexTotalCountAndVertexType fail");
	}

	//如果nMeshVertexTotalCount值为0，说明fbx文件内没有Mesh，那么就不需要解析顶点数据，也不需要解析骨骼数据。
		//如果nMeshVertexTotalCount值为0，说明fbx文件可能只包含动画数据。
	if (nMeshVertexTotalCount > 0)
	{
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		//生成Mesh顶点数据。
		//注意，先调用SetVertexType再调用ReserveVertexCount。
		pMeshData->SetVertexType(kVertexType);
		pMeshData->ReserveVertexCount(nMeshVertexTotalCount);
		int nProcessedVertexCount = 0;
		int nProcessedControlPointCount = 0;
		ParseMeshData(pSceneRootNode, pMeshData, &nProcessedVertexCount, &nProcessedControlPointCount);
		//ErrorLogger::Log("FBXManager::LoadFBX : VertexCount VertexType VertexStructSize");
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		//生成控制点数据。
		int nControlPointTotalCount = 0;
		ParseControlPointTotalCount(pSceneRootNode, &nControlPointTotalCount);
		pControlPointGroup->ReserveControlPointCount(nControlPointTotalCount);
		ParseAllControlPoint(pSceneRootNode, pControlPointGroup);
		//ErrorLogger::Log("StFBXManager::LoadFBX : ControlPointTotalCount");
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		//生成控制点的蒙皮数据。
		nProcessedControlPointCount = 0;
		ParseAllControlPointSkinWeight(pSceneRootNode, pControlPointGroup, &nProcessedControlPointCount);
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

		////<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		////生成骨骼数据。
		int nBoneTotalCount = 0;
		ParseBoneTotalCount(pSceneRootNode, &nBoneTotalCount);
		if (nBoneTotalCount > 0)
		{
			pBoneGroup->ReserveBoneCount(nBoneTotalCount);
			ParseBoneHierarchy(pSceneRootNode, pBoneGroup, -1);
			pBoneGroup->GenerateChildren();
			//计算骨骼的变换矩阵。
			ParseAllBoneMatrixFromBoneSpaceToWorldSpace(pSceneRootNode, pSceneRootNode, pBoneGroup);
			//SoLogDebug("StFBXManager::LoadFBX : BoneTotalCount[%d]", pBoneGroup->GetSize());
			//之前，控制点的蒙皮数据中存储的是骨骼名字。
			//现在有了骨骼数据，可以将骨骼名字转换成相应的骨骼序号。
			pControlPointGroup->MakeBoneIndexByBoneName(pBoneGroup);
		}
		////>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	}

	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		//生成动画帧数据。
	int nTheBoneCount = pBoneGroup->GetSize();
	if (nTheBoneCount > 0)
	{
		//pModelAnimation->ReserveBoneCount(nTheBoneCount);
		ParseAllBoneAnimationData(lScene, pBoneGroup);//pModelAnimation
	}
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	if (lScene)
	{
		lScene->Destroy();
		lScene = 0;
	}

	// Destroy all objects created by the FBX SDK.
	DestroySdkObjects(lSdkManager, lResult);

}

void FBXModel::InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
{
	//The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
	pManager = FbxManager::Create();
	if (!pManager)
	{
		FBXSDK_printf("Error: Unable to create FBX Manager!\n");
		exit(1);
	}
	else FBXSDK_printf("Autodesk FBX SDK version %s\n", pManager->GetVersion());

	//Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
	pManager->SetIOSettings(ios);

	//Load plugins from the executable directory (optional)
	FbxString lPath = FbxGetApplicationDirectory();
	pManager->LoadPluginsDirectory(lPath.Buffer());

	//Create an FBX scene. This object holds most objects imported/exported from/to files.
	pScene = FbxScene::Create(pManager, "My Scene");
	if (!pScene)
	{
		FBXSDK_printf("Error: Unable to create FBX scene!\n");
		exit(1);
	}
}

bool FBXModel::LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename)
{
	int lFileMajor, lFileMinor, lFileRevision;
	int lSDKMajor, lSDKMinor, lSDKRevision;
	//int lFileFormat = -1;
	int lAnimStackCount;
	bool lStatus;
	char lPassword[1024];

	// Get the file version number generate by the FBX SDK.
	FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

	// Create an importer.
	FbxImporter* lImporter = FbxImporter::Create(pManager, "");

	// Initialize the importer by providing a filename.
	const bool lImportStatus = lImporter->Initialize(pFilename, -1, pManager->GetIOSettings());
	lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

	if (!lImportStatus)
	{
		FbxString error = lImporter->GetStatus().GetErrorString();
		FBXSDK_printf("Call to FbxImporter::Initialize() failed.\n");
		FBXSDK_printf("Error returned: %s\n\n", error.Buffer());

		if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
		{
			FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
			FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);
		}

		return false;
	}

	FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);

	if (lImporter->IsFBX())
	{
		FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);

		// From this point, it is possible to access animation stack information without
		// the expense of loading the entire file.

		FBXSDK_printf("Animation Stack Information\n");

		lAnimStackCount = lImporter->GetAnimStackCount();

		FBXSDK_printf("    Number of Animation Stacks: %d\n", lAnimStackCount);
		FBXSDK_printf("    Current Animation Stack: \"%s\"\n", lImporter->GetActiveAnimStackName().Buffer());
		FBXSDK_printf("\n");

		for (int i = 0; i < lAnimStackCount; i++)
		{
			FbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i);

			FBXSDK_printf("    Animation Stack %d\n", i);
			FBXSDK_printf("         Name: \"%s\"\n", lTakeInfo->mName.Buffer());
			FBXSDK_printf("         Description: \"%s\"\n", lTakeInfo->mDescription.Buffer());

			// Change the value of the import name if the animation stack should be imported 
			// under a different name.
			FBXSDK_printf("         Import Name: \"%s\"\n", lTakeInfo->mImportName.Buffer());

			// Set the value of the import state to false if the animation stack should be not
			// be imported. 
			FBXSDK_printf("         Import State: %s\n", lTakeInfo->mSelect ? "true" : "false");
			FBXSDK_printf("\n");
		}

		// Set the import states. By default, the import states are always set to 
		// true. The code below shows how to change these states.
		/*IOS_REF.SetBoolProp(IMP_FBX_MATERIAL, true);
		IOS_REF.SetBoolProp(IMP_FBX_TEXTURE, true);
		IOS_REF.SetBoolProp(IMP_FBX_LINK, true);
		IOS_REF.SetBoolProp(IMP_FBX_SHAPE, true);
		IOS_REF.SetBoolProp(IMP_FBX_GOBO, true);
		IOS_REF.SetBoolProp(IMP_FBX_ANIMATION, true);
		IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);*/
	}

	// Import the scene.
	lStatus = lImporter->Import(pScene);
	if (lStatus == true)
	{
		// Check the scene integrity!
		FbxStatus status;
		FbxArray< FbxString*> details;
		FbxSceneCheckUtility sceneCheck(FbxCast<FbxScene>(pScene), &status, &details);
		lStatus = sceneCheck.Validate(FbxSceneCheckUtility::eCkeckData);
		if (lStatus == false)
		{
			if (details.GetCount())
			{
				FBXSDK_printf("Scene integrity verification failed with the following errors:\n");
				for (int i = 0; i < details.GetCount(); i++)
					FBXSDK_printf("   %s\n", details[i]->Buffer());

				FbxArrayDelete<FbxString*>(details);
			}
		}
	}

	if (lStatus == false && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
	{
		FBXSDK_printf("Please enter password: ");

		lPassword[0] = '\0';

		FBXSDK_CRT_SECURE_NO_WARNING_BEGIN
			scanf("%s", lPassword);
		FBXSDK_CRT_SECURE_NO_WARNING_END

			FbxString lString(lPassword);

		/*IOS_REF.SetStringProp(IMP_FBX_PASSWORD, lString);
		IOS_REF.SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);*/

		lStatus = lImporter->Import(pScene);

		if (lStatus == false && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
		{
			FBXSDK_printf("\nPassword is wrong, import aborted.\n");
		}
	}

	// Destroy the importer.
	lImporter->Destroy();

	return lStatus;
}

void FBXModel::ParseMeshVertexTotalCountAndVertexType(FbxNode* pNode, BitFlag* pVertexType, int* pTotalCount, bool* pAllMeshOK) {
	if (pNode == NULL)
	{
		return;
	}
	if (*pAllMeshOK == false)
	{
		return;
	}

	do
	{
		const FbxNodeAttribute* pNodeAttr = pNode->GetNodeAttribute();
		if (pNodeAttr == NULL)
		{
			break;
		}
		if (pNodeAttr->GetAttributeType() != FbxNodeAttribute::eMesh)
		{
			break;
		}
		FbxMesh* pMesh = pNode->GetMesh();
		if (pMesh == NULL)
		{
			break;
		}

		bool bFirstMesh = (*pTotalCount == 0);
		ParseSingleMeshVertexType(pMesh, pVertexType, pAllMeshOK, bFirstMesh);
		const int nPolygonCount = pMesh->GetPolygonCount();
		*pTotalCount += nPolygonCount * 3;

	} while (0);

	const int nChildCount = pNode->GetChildCount();
	for (int i = 0; i < nChildCount; ++i)
	{
		ParseMeshVertexTotalCountAndVertexType(pNode->GetChild(i), pVertexType, pTotalCount, pAllMeshOK);
	}
}

void FBXModel::DestroySdkObjects(FbxManager* pManager, bool pExitStatus)
{
	//Delete the FBX Manager. All the objects that have been allocated using the FBX Manager and that haven't been explicitly destroyed are also automatically destroyed.
	if (pManager) pManager->Destroy();
	if (pExitStatus) FBXSDK_printf("Program Success!\n");

}

void FBXModel::ParseSingleMeshVertexType(FbxMesh* pFbxMesh, BitFlag* pVertexType, bool* pAllMeshOK, bool bFirstMesh)
{
	//肯定有顶点坐标数据
	if (bFirstMesh)
	{
		pVertexType->AddFlag(FBXElement_Position);
	}
	if (pFbxMesh->GetElementMaterialCount() > 0)
	{
		const int nMappingMode = pFbxMesh->GetElementMaterial(0)->GetMappingMode();
		if (nMappingMode != FbxGeometryElement::eAllSame)
		{
			ErrorLogger::Log("ParseSingleMeshVertexType : Material MappingMode Error");
			*pAllMeshOK = false;
			return;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//判断是否存在法线数据
	//获取且仅获取0号位置的法线数据。
	if (FBX_ParseNormalData)
	{
		bool bNormalExist = false;
		if (pFbxMesh->GetElementNormalCount() > 0)
		{
			const int nMappingMode = pFbxMesh->GetElementNormal(0)->GetMappingMode();
			if (nMappingMode == FbxGeometryElement::eByPolygonVertex)
			{
				bNormalExist = true;
			}
			else
			{
				ErrorLogger::Log("ParseSingleMeshVertexType : Normal MappingMode Error");
				*pAllMeshOK = false;
				return;
			}
		}
		if (bFirstMesh)
		{
			if (bNormalExist)
			{
				pVertexType->AddFlag(FBXElement_Normal);
			}
		}
		else
		{
			if (pVertexType->IsFlagExist(FBXElement_Normal) != bNormalExist)
			{
				//本Mesh与其他的Mesh有差异。
				ErrorLogger::Log("StFBXManager::ParseSingleMeshVertexType : Normal Data different");
				*pAllMeshOK = false;
				return;
			}
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////
	//判断是否存在切线数据
	//获取且仅获取0号位置的切线数据。
	if (FBX_ParseTangentData)
	{
		bool bTangentExist = false;
		if (pFbxMesh->GetElementTangentCount() > 0)
		{
			const int nMappingMode = pFbxMesh->GetElementTangent(0)->GetMappingMode();
			if (nMappingMode == FbxGeometryElement::eByPolygonVertex)
			{
				bTangentExist = true;
			}
			else
			{
				ErrorLogger::Log("ParseSingleMeshVertexType : Tangent MappingMode Error");
				*pAllMeshOK = false;
				return;
			}
		}
		if (bFirstMesh)
		{
			if (bTangentExist)
			{
				pVertexType->AddFlag(FBXElement_Tangent);
			}
		}
		else
		{
			if (pVertexType->IsFlagExist(FBXElement_Tangent) != bTangentExist)
			{
				//本Mesh与其他的Mesh有差异。
				ErrorLogger::Log("ParseSingleMeshVertexType : Tangent Data different");
				*pAllMeshOK = false;
				return;
			}
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////
	//判断是否存在顶点颜色
	//获取且仅获取0号位置的顶点色数据。
	if (FBX_ParseColorData)
	{
		bool bColorExist = false;
		if (pFbxMesh->GetElementVertexColorCount() > 0)
		{
			const int nMappingMode = pFbxMesh->GetElementVertexColor(0)->GetMappingMode();
			if (nMappingMode == FbxGeometryElement::eByPolygonVertex)
			{
				bColorExist = true;
			}
			else
			{
				ErrorLogger::Log("ParseSingleMeshVertexType : VertexColor MappingMode Error ");
				*pAllMeshOK = false;
				return;
			}
		}
		if (bFirstMesh)
		{
			if (bColorExist)
			{
				pVertexType->AddFlag(FBXElement_Color);
			}
		}
		else
		{
			if (pVertexType->IsFlagExist(FBXElement_Color) != bColorExist)
			{
				//本Mesh与其他的Mesh有差异。
				ErrorLogger::Log("ParseSingleMeshVertexType : VertexColor Data different");
				*pAllMeshOK = false;
				return;
			}
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////
	//判断是否存在UV
	//获取且仅获取0号和1号位置的UV数据。
	if (FBX_ParseUV1Data)
	{
		bool bUV1Exist = false;
		if (pFbxMesh->GetElementUVCount() > 0)
		{
			const int nMappingMode = pFbxMesh->GetElementUV(0)->GetMappingMode();
			if (nMappingMode == FbxGeometryElement::eByPolygonVertex)
			{
				bUV1Exist = true;
			}
			else
			{
				ErrorLogger::Log("ParseSingleMeshVertexType : UV1 MappingMode Error");
				*pAllMeshOK = false;
				return;
			}
		}
		if (bFirstMesh)
		{
			if (bUV1Exist)
			{
				pVertexType->AddFlag(FBXElement_UV1);
			}
		}
		else
		{
			if (pVertexType->IsFlagExist(FBXElement_UV1) != bUV1Exist)
			{
				//本Mesh与其他的Mesh有差异。
				ErrorLogger::Log("ParseSingleMeshVertexType : UV1 Data different");
				*pAllMeshOK = false;
				return;
			}
		}
	}
	if (FBX_ParseUV2Data)
	{
		bool bUV2Exist = false;
		if (pFbxMesh->GetElementUVCount() > 1)
		{
			const int nMappingMode = pFbxMesh->GetElementUV(1)->GetMappingMode();
			if (nMappingMode == FbxGeometryElement::eByPolygonVertex)
			{
				bUV2Exist = true;
			}
			else
			{
				ErrorLogger::Log("ParseSingleMeshVertexType : UV2 MappingMode Error");
				*pAllMeshOK = false;
				return;
			}
		}
		if (bFirstMesh)
		{
			if (bUV2Exist)
			{
				pVertexType->AddFlag(FBXElement_UV2);
			}
		}
		else
		{
			if (pVertexType->IsFlagExist(FBXElement_UV2) != bUV2Exist)
			{
				//本Mesh与其他的Mesh有差异。
				ErrorLogger::Log("ParseSingleMeshVertexType : UV2 Data different");
				*pAllMeshOK = false;
				return;
			}
		}
	}
}

void FBXModel::ParseMeshData(FbxNode* pNode, FBXMeshData* pMeshData, int* pProcessedVertexCount, int* pProcessedControlPointCount)
{
	if (pNode == NULL)
	{
		return;
	}

	do
	{
		const FbxNodeAttribute* pNodeAttr = pNode->GetNodeAttribute();
		if (pNodeAttr == NULL)
		{
			break;
		}
		if (pNodeAttr->GetAttributeType() != FbxNodeAttribute::eMesh)
		{
			break;
		}
		FbxMesh* pMesh = pNode->GetMesh();
		if (pMesh == NULL)
		{
			break;
		}

		LoadSingleMesh(pMesh, pMeshData, pProcessedVertexCount, pProcessedControlPointCount);

	} while (0);

	const int nChildCount = pNode->GetChildCount();
	for (int i = 0; i < nChildCount; ++i)
	{
		ParseMeshData(pNode->GetChild(i), pMeshData, pProcessedVertexCount, pProcessedControlPointCount);
	}
}

void FBXModel::LoadSingleMesh(FbxMesh* pFbxMesh, FBXMeshData* pMeshData, int* pProcessedVertexCount, int* pProcessedControlPointCount)
{
	const int nFloatCountPerVertexData = pMeshData->nSizeofVertexData / sizeof(float);
	float* theVertexBuff = (float*)(pMeshData->pVertexBuff);
	unsigned int* theIndexBuff = (unsigned int*)(pMeshData->pIndexBuff);
	int* theIndex2ControlPointIndex = pMeshData->pVertexIndex2ControlPointIndex;
	//
	const int nTriangleCount = pFbxMesh->GetPolygonCount();
	int nAccVertexCount = 0;
	FbxVector4 kVector4;
	FbxVector2 kVector2;
	FbxColor kColor;
	for (int i = 0; i < nTriangleCount; ++i)
	{
		//每个三角网格有3个顶点
		for (int j = 0; j < 3; ++j)
		{
			//nVertexIndex表示本顶点在所有顶点中的位置序号。
			const int nVertexIndex = *pProcessedVertexCount + nAccVertexCount;
			//nControlPointIndex表示本顶点受哪个控制点的影响。
			const int nControlPointIndex = pFbxMesh->GetPolygonVertex(i, j);

			//索引
			if (FBX_TriangleIndex_123)
			{
				theIndexBuff[nVertexIndex] = nVertexIndex;
			}
			else
			{
				theIndexBuff[nVertexIndex] = (nVertexIndex - j) + (2 - j);
			}

			//顶点到控制点的映射
			theIndex2ControlPointIndex[nVertexIndex] = *pProcessedControlPointCount + nControlPointIndex;

			//顶点
			float* thisVertex = theVertexBuff + nFloatCountPerVertexData * nVertexIndex;
			int nAccFloat = 0;
			kVector4 = pFbxMesh->GetControlPointAt(nControlPointIndex);
			thisVertex[nAccFloat++] = (float)kVector4[0];
			thisVertex[nAccFloat++] = (float)kVector4[1];
			thisVertex[nAccFloat++] = (float)kVector4[2];

			//获取法线数据
			if (pMeshData->kVertexType.IsFlagExist(FBXElement_Normal))
			{
				LoadSingleMesh_Normal(pFbxMesh, nAccVertexCount, &kVector4);
				thisVertex[nAccFloat++] = (float)kVector4[0];
				thisVertex[nAccFloat++] = (float)kVector4[1];
				thisVertex[nAccFloat++] = (float)kVector4[2];
			}

			//获取切线数据
			if (pMeshData->kVertexType.IsFlagExist(FBXElement_Tangent))
			{
				LoadSingleMesh_Tangent(pFbxMesh, nAccVertexCount, &kVector4);
				thisVertex[nAccFloat++] = (float)kVector4[0];
				thisVertex[nAccFloat++] = (float)kVector4[1];
				thisVertex[nAccFloat++] = (float)kVector4[2];
			}

			//获取顶点色数据
			if (pMeshData->kVertexType.IsFlagExist(FBXElement_Color))
			{
				LoadSingleMesh_Color(pFbxMesh, nAccVertexCount, &kColor);
				thisVertex[nAccFloat++] = (float)kColor.mRed;
				thisVertex[nAccFloat++] = (float)kColor.mGreen;
				thisVertex[nAccFloat++] = (float)kColor.mBlue;
				thisVertex[nAccFloat++] = (float)kColor.mAlpha;
			}

			//获取UV数据
			if (pMeshData->kVertexType.IsFlagExist(FBXElement_UV1))
			{
				LoadSingleMesh_UV(pFbxMesh, 0, nAccVertexCount, &kVector2);
				thisVertex[nAccFloat++] = (float)kVector2[0];
				thisVertex[nAccFloat++] = FBX_UVHeightInverse ? float(1.0 - kVector2[1]) : float(kVector2[1]);
			}
			if (pMeshData->kVertexType.IsFlagExist(FBXElement_UV2))
			{
				LoadSingleMesh_UV(pFbxMesh, 1, nAccVertexCount, &kVector2);
				thisVertex[nAccFloat++] = (float)kVector2[0];
				thisVertex[nAccFloat++] = FBX_UVHeightInverse ? float(1.0 - kVector2[1]) : float(kVector2[1]);
			}

			//
			++nAccVertexCount;
		}
	}

	//
	*pProcessedVertexCount += nAccVertexCount;
	*pProcessedControlPointCount += pFbxMesh->GetControlPointsCount();
}

void FBXModel::LoadSingleMesh_Normal(FbxMesh* pFbxMesh, int nVertexIndex, FbxVector4* pNormal)
{
	FbxGeometryElementNormal* lNormalElement = pFbxMesh->GetElementNormal(0);
	const int nReferenceMode = lNormalElement->GetReferenceMode();
	const FbxLayerElementArrayTemplate<FbxVector4>& kNormalArray = lNormalElement->GetDirectArray();
	//
	int nIndex = nVertexIndex;
	if (nReferenceMode == FbxGeometryElement::eIndexToDirect)
	{
		const FbxLayerElementArrayTemplate<int>& kIndexArray = lNormalElement->GetIndexArray();
		nIndex = kIndexArray.GetAt(nVertexIndex);
	}
	*pNormal = kNormalArray.GetAt(nIndex);
}

void FBXModel::LoadSingleMesh_Tangent(FbxMesh* pFbxMesh, int nVertexIndex, FbxVector4* pTangent)
{
	FbxGeometryElementTangent* leTangent = pFbxMesh->GetElementTangent(0);
	const int nReferenceMode = leTangent->GetReferenceMode();
	const FbxLayerElementArrayTemplate<FbxVector4>& kTangentArray = leTangent->GetDirectArray();
	//
	int nIndex = nVertexIndex;
	if (nReferenceMode == FbxGeometryElement::eIndexToDirect)
	{
		const FbxLayerElementArrayTemplate<int>& kIndexArray = leTangent->GetIndexArray();
		nIndex = kIndexArray.GetAt(nVertexIndex);
	}
	*pTangent = kTangentArray.GetAt(nIndex);
}

void FBXModel::LoadSingleMesh_Color(FbxMesh* pFbxMesh, int nVertexIndex, FbxColor* pColor)
{
	FbxGeometryElementVertexColor* leVtxc = pFbxMesh->GetElementVertexColor(0);
	const int nReferenceMode = leVtxc->GetReferenceMode();
	FbxLayerElementArrayTemplate<FbxColor>& kColorArray = leVtxc->GetDirectArray();
	//
	int nIndex = nVertexIndex;
	if (nReferenceMode == FbxGeometryElement::eIndexToDirect)
	{
		FbxLayerElementArrayTemplate<int>& kIndexArray = leVtxc->GetIndexArray();
		nIndex = kIndexArray.GetAt(nVertexIndex);
	}
	*pColor = kColorArray.GetAt(nIndex);
}

void FBXModel::LoadSingleMesh_UV(FbxMesh* pFbxMesh, int nUVIndex, int nVertexIndex, FbxVector2* pUV)
{
	const FbxGeometryElementUV* leUV = pFbxMesh->GetElementUV(nUVIndex);
	const int nReferenceMode = leUV->GetReferenceMode();
	const FbxLayerElementArrayTemplate<FbxVector2>& kUVArray = leUV->GetDirectArray();
	//
	int nIndex = nVertexIndex;
	if (nReferenceMode == FbxGeometryElement::eIndexToDirect)
	{
		const FbxLayerElementArrayTemplate<int>& kIndexArray = leUV->GetIndexArray();
		nIndex = kIndexArray.GetAt(nVertexIndex);
	}
	*pUV = kUVArray.GetAt(nIndex);
}

void FBXModel::ParseControlPointTotalCount(FbxNode* pNode, int* pTotalCount)
{
	if (pNode == NULL)
	{
		return;
	}

	do
	{
		const FbxNodeAttribute* pNodeAttr = pNode->GetNodeAttribute();
		if (pNodeAttr == NULL)
		{
			break;
		}
		if (pNodeAttr->GetAttributeType() != FbxNodeAttribute::eMesh)
		{
			break;
		}
		FbxMesh* pMesh = pNode->GetMesh();
		if (pMesh == NULL)
		{
			break;
		}

		const int nCount = pMesh->GetControlPointsCount();
		*pTotalCount += nCount;

	} while (0);

	const int nChildCount = pNode->GetChildCount();
	for (int i = 0; i < nChildCount; ++i)
	{
		ParseControlPointTotalCount(pNode->GetChild(i), pTotalCount);
	}
}

void FBXModel::ParseAllControlPoint(FbxNode* pNode, FBXControlPointGroup* pControlPointGroup)
{
	if (pNode == NULL)
	{
		return;
	}

	do
	{
		const FbxNodeAttribute* pNodeAttr = pNode->GetNodeAttribute();
		if (pNodeAttr == NULL)
		{
			break;
		}
		if (pNodeAttr->GetAttributeType() != FbxNodeAttribute::eMesh)
		{
			break;
		}
		FbxMesh* pMesh = pNode->GetMesh();
		if (pMesh == NULL)
		{
			break;
		}

		LoadControlPointFromMesh(pMesh, pControlPointGroup);

	} while (0);

	const int nChildCount = pNode->GetChildCount();
	for (int i = 0; i < nChildCount; ++i)
	{
		ParseAllControlPoint(pNode->GetChild(i), pControlPointGroup);
	}
}

void FBXModel::LoadControlPointFromMesh(FbxMesh* pMesh, FBXControlPointGroup* pControlPointGroup)
{
	FbxVector4 kVector4;
	const int nCount = pMesh->GetControlPointsCount();
	for (int i = 0; i < nCount; ++i)
	{
		FBXControlPoint* pPoint = pControlPointGroup->TakeNew();
		if (pPoint)
		{
			kVector4 = pMesh->GetControlPointAt(i);
			pPoint->kVertex.x = (float)kVector4[0];
			pPoint->kVertex.y = (float)kVector4[1];
			pPoint->kVertex.z = (float)kVector4[2];
		}
	}
}

void FBXModel::ParseAllControlPointSkinWeight(FbxNode* pNode, FBXControlPointGroup* pControlPointGroup, int* pProcessedControlPointCount)
{
	if (pNode == NULL)
	{
		return;
	}

	do
	{
		const FbxNodeAttribute* pNodeAttr = pNode->GetNodeAttribute();
		if (pNodeAttr == NULL)
		{
			break;
		}
		if (pNodeAttr->GetAttributeType() != FbxNodeAttribute::eMesh)
		{
			break;
		}
		FbxMesh* pMesh = pNode->GetMesh();
		if (pMesh == NULL)
		{
			break;
		}

		LoadSingleControlPointSkinWeight(pMesh, pControlPointGroup, pProcessedControlPointCount);

	} while (0);

	const int nChildCount = pNode->GetChildCount();
	for (int i = 0; i < nChildCount; ++i)
	{
		ParseAllControlPointSkinWeight(pNode->GetChild(i), pControlPointGroup, pProcessedControlPointCount);
	}
}

void FBXModel::LoadSingleControlPointSkinWeight(FbxMesh* pMesh, FBXControlPointGroup* pControlPointGroup, int* pProcessedControlPointCount)
{
	const int nStartControlPointIndex = *pProcessedControlPointCount;
	*pProcessedControlPointCount += pMesh->GetControlPointsCount();
	//
	const int nDeformerCount = pMesh->GetDeformerCount();
	for (int deformerIndex = 0; deformerIndex < nDeformerCount; ++deformerIndex)
	{
		// There are many types of deformers in Maya,
		// We are using only skins, so we see if this is a skin
		FbxSkin* pSkin = reinterpret_cast<FbxSkin*>(pMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
		if (pSkin == NULL)
		{
			continue;
		}

		const int nClusterCount = pSkin->GetClusterCount();
		for (int clusterIndex = 0; clusterIndex < nClusterCount; ++clusterIndex)
		{
			FbxCluster* pCluster = pSkin->GetCluster(clusterIndex);
			const char* pBoneName = pCluster->GetLink()->GetName();
			// Associate each joint with the control points it affects
			const int nControlPointCount = pCluster->GetControlPointIndicesCount();
			const int* pCPIndexList = pCluster->GetControlPointIndices();
			const double* pWeightList = pCluster->GetControlPointWeights();
			for (int cpIndex = 0; cpIndex < nControlPointCount; ++cpIndex)
			{
				const int nControlPointIndex = pCPIndexList[cpIndex] + nStartControlPointIndex;
				FBXControlPoint* pControlPoint = pControlPointGroup->GetAt(nControlPointIndex);
				if (pControlPoint)
				{
					float fWeight = (float)(pWeightList[cpIndex]);
					pControlPoint->AddBoneNameSkinWeight(pBoneName, fWeight);
				}
				else
				{
					ErrorLogger::Log("StFBXManager::LoadSingleControlPointSkinWeight : Can not find ControlPoint by index ");
				}
			}
		}
	}
}

void FBXModel::ParseBoneTotalCount(FbxNode* pNode, int* pTotalCount)
{
	if (pNode == NULL)
	{
		return;
	}

	do
	{
		const FbxNodeAttribute* pNodeAttr = pNode->GetNodeAttribute();
		if (pNodeAttr == NULL)
		{
			break;
		}
		if (pNodeAttr->GetAttributeType() != FbxNodeAttribute::eSkeleton)
		{
			break;
		}

		*pTotalCount += 1;

	} while (0);

	const int nChildCount = pNode->GetChildCount();
	for (int i = 0; i < nChildCount; ++i)
	{
		ParseBoneTotalCount(pNode->GetChild(i), pTotalCount);
	}
}

void FBXModel::ParseBoneHierarchy(FbxNode* pNode, FBXBoneGroup* pGroup, int nParentIndex)
{
	if (pNode == NULL)
	{
		return;
	}

	do
	{
		const FbxNodeAttribute* pNodeAttr = pNode->GetNodeAttribute();
		if (pNodeAttr == NULL)
		{
			break;
		}
		if (pNodeAttr->GetAttributeType() != FbxNodeAttribute::eSkeleton)
		{
			break;
		}

		int nMyBoneIndex = -1;
		FBXBone* pMyBone = pGroup->TakeNew(&nMyBoneIndex);
		if (pMyBone)
		{
			pMyBone->kBoneName = pNode->GetName();
			pMyBone->nParentIndex = nParentIndex;
			//FbxAMatrix kGlobalTran = pNode->EvaluateGlobalTransform();
			//kGlobalTran = kGlobalTran.Inverse();
			//ConvertFbxAMatrixToSoMathMatrix4(&kGlobalTran, &(pMyBone->kMatFromBoneSpaceToModelSpace));
			//以本节点做为父节点，接下来要遍历本节点的子节点。
			nParentIndex = nMyBoneIndex;
		}

	} while (0);

	const int nChildCount = pNode->GetChildCount();
	for (int i = 0; i < nChildCount; ++i)
	{
		ParseBoneHierarchy(pNode->GetChild(i), pGroup, nParentIndex);
	}
}

void FBXModel::ParseAllBoneMatrixFromBoneSpaceToWorldSpace(FbxNode* pNode, FbxNode* pRootNode, FBXBoneGroup* pBoneGroup)
{
	if (pNode == NULL)
	{
		return;
	}

	do
	{
		const FbxNodeAttribute* pNodeAttr = pNode->GetNodeAttribute();
		if (pNodeAttr == NULL)
		{
			break;
		}
		if (pNodeAttr->GetAttributeType() != FbxNodeAttribute::eMesh)
		{
			break;
		}
		FbxMesh* pMesh = pNode->GetMesh();
		if (pMesh == NULL)
		{
			break;
		}

		LoadSingleBoneMatrixFromBoneSpaceToWorldSpace(pNode, pMesh, pRootNode, pBoneGroup);

	} while (0);

	const int nChildCount = pNode->GetChildCount();
	for (int i = 0; i < nChildCount; ++i)
	{
		ParseAllBoneMatrixFromBoneSpaceToWorldSpace(pNode->GetChild(i), pRootNode, pBoneGroup);
	}
}

void FBXModel::LoadSingleBoneMatrixFromBoneSpaceToWorldSpace(FbxNode* pNode, FbxMesh* pMesh, FbxNode* pRootNode, FBXBoneGroup* pBoneGroup)
{
	// This geometry transform is something I cannot understand
	// I think it is from MotionBuilder
	// If you are using Maya for your models, 99% this is just an
	// identity matrix
	// But I am taking it into account anyways......
	FbxAMatrix matGeometryTransform;
	matGeometryTransform.SetIdentity();
	GetGeometryTransformation(pNode, &matGeometryTransform);

	const int nDeformerCount = pMesh->GetDeformerCount();
	for (int deformerIndex = 0; deformerIndex < nDeformerCount; ++deformerIndex)
	{
		// There are many types of deformers in Maya,
		// We are using only skins, so we see if this is a skin
		FbxSkin* pSkin = reinterpret_cast<FbxSkin*>(pMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
		if (pSkin == 0)
		{
			continue;
		}

		const int nClusterCount = pSkin->GetClusterCount();
		for (int clusterIndex = 0; clusterIndex < nClusterCount; ++clusterIndex)
		{
			FbxCluster* pCluster = pSkin->GetCluster(clusterIndex);
			const char* pBoneName = pCluster->GetLink()->GetName();
			const int nBoneIndex = pBoneGroup->GetBoneIndexByBoneName(pBoneName);
			if (nBoneIndex == -1)
			{
				//SoLogError("StFBXManager::LoadSingleBoneMatrixFromBoneSpaceToWorldSpace : Can not find bone by BoneName[%s]", pBoneName);
				continue;
			}

			FBXBone* pMyBone = pBoneGroup->GetAt(nBoneIndex);
			FbxAMatrix transformMatrix;
			FbxAMatrix transformLinkMatrix;
			FbxAMatrix globalBindposeInverseMatrix;
			// The transformation of the mesh at binding time
			pCluster->GetTransformMatrix(transformMatrix);
			// The transformation of the cluster(joint) at binding time from joint space to world space
			pCluster->GetTransformLinkMatrix(transformLinkMatrix);
			//transformLinkMatrix = pBoneNode->EvaluateGlobalTransform();
			globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * matGeometryTransform;
			ConvertFbxAMatrixToDirectXMatrix(&globalBindposeInverseMatrix, &(pMyBone->kMatFromBoneSpaceToModelSpace));
		}
	}
}

void FBXModel::GetGeometryTransformation(FbxNode* pNode, FbxAMatrix* pMat)
{
	if (pMat)
	{
		if (pNode)
		{
			const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
			const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
			const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
			*pMat = FbxAMatrix(lT, lR, lS);
		}
		else
		{
			pMat->SetIdentity();
		}
	}
}

void FBXModel::ConvertFbxAMatrixToDirectXMatrix(const FbxAMatrix* pFbxAMatrix, XMMATRIX* pSoMatrix)
{
	for (int row = 0; row < 4; ++row)
	{
		pSoMatrix->r[row] = XMVectorSet((float)pFbxAMatrix->Get(row, 0), (float)pFbxAMatrix->Get(row, 1),
			(float)pFbxAMatrix->Get(row, 2), (float)pFbxAMatrix->Get(row, 3));
	}
}

void FBXModel::ParseAllBoneAnimationData(FbxScene* pSDKScene, FBXBoneGroup* pBoneGroup)//, FBXModelAnimation* pModelAnim
{
	//一般情况下，一个fbx文件中只包含一个动画，也即只包含一个FbxAnimStack对象。
	//把0号FbxAnimStack对象设置为当前要播放（提取）的动画。
	auto pAnimStackCount = pSDKScene->GetSrcObjectCount<FbxAnimStack>();
	for (int i = 0; i < pAnimStackCount; ++i)
	{
		FBXModelAnimation* mNewAnimation = new FBXModelAnimation();
		mNewAnimation->ReserveBoneCount(pBoneGroup->GetSize());
		FbxAnimStack* pAnimStack = pSDKScene->GetSrcObject<FbxAnimStack>(i);//
		if (pAnimStack == NULL)
		{
			ErrorLogger::Log("StFBXManager::ParseAllBoneAnimationData : no FbxAnimStack found");
			return;
		}
		pSDKScene->SetCurrentAnimationStack(pAnimStack);

		const FbxTime::EMode theTimeMode = FbxTime::eFrames24;
		FbxTimeSpan kTimeSpan = pAnimStack->GetLocalTimeSpan();
		FbxTime kAnimDuration = kTimeSpan.GetDuration();
		const float fAnimLength = (float)kAnimDuration.GetSecondDouble();
		const int nFrameCount = (int)kAnimDuration.GetFrameCount(theTimeMode);
		const int nBoneCount = pBoneGroup->GetSize();
		mNewAnimation->nFrameCount = nFrameCount;
		mNewAnimation->fAnimLength = fAnimLength;

		for (int boneIndex = 0; boneIndex < nBoneCount; ++boneIndex)
		{
			const char* szBoneName = pBoneGroup->GetAt(boneIndex)->kBoneName.c_str();
			FbxNode* pBoneNode = NULL;
			int nAccBoneCount = 0;
			FindBoneByIndexAndName(pSDKScene->GetRootNode(), boneIndex, szBoneName, &pBoneNode, &nAccBoneCount);
			if (pBoneNode == NULL)
			{
				ErrorLogger::Log("StFBXManager::ParseAllBoneAnimationData : Can not find Bone ");
				continue;
			}

			FBXBoneAnimation* pBoneAnim = mNewAnimation->TakeNew();
			pBoneAnim->ReserveKeyFrameCount(nFrameCount);
			pBoneAnim->nBoneIndex = boneIndex;
			FbxTime kTime;
			for (int frameIndex = 0; frameIndex < nFrameCount; ++frameIndex)
			{
				FBXKeyFrame* pKeyFrame = pBoneAnim->TakeNew();
				kTime.SetFrame(frameIndex, theTimeMode);
				pKeyFrame->fKeyTime = (float)kTime.GetSecondDouble();
				const FbxAMatrix& kGlobalTran = pBoneNode->EvaluateGlobalTransform(kTime);
				ConvertFbxAMatrixToDirectXMatrix(&kGlobalTran, &(pKeyFrame->matKeyTransform));
			}
		}
		m_kAnimations.push_back(mNewAnimation);
		m_kcurrentAnimaionIndex = i;
	}

	//FbxAnimStack* pAnimStack = pSDKScene->GetSrcObject<FbxAnimStack>(1);//
	//if (pAnimStack == NULL)
	//{
	//	ErrorLogger::Log("StFBXManager::ParseAllBoneAnimationData : no FbxAnimStack found");
	//	return;
	//}
	//pSDKScene->SetCurrentAnimationStack(pAnimStack);

	//const FbxTime::EMode theTimeMode = FbxTime::eFrames24;
	//FbxTimeSpan kTimeSpan = pAnimStack->GetLocalTimeSpan();
	//FbxTime kAnimDuration = kTimeSpan.GetDuration();
	//const float fAnimLength = (float)kAnimDuration.GetSecondDouble();
	//const int nFrameCount = (int)kAnimDuration.GetFrameCount(theTimeMode);
	//const int nBoneCount = pBoneGroup->GetSize();
	//pModelAnim->nFrameCount = nFrameCount;
	//pModelAnim->fAnimLength = fAnimLength;

	//for (int boneIndex = 0; boneIndex < nBoneCount; ++boneIndex)
	//{
	//	const char* szBoneName = pBoneGroup->GetAt(boneIndex)->kBoneName.c_str();
	//	FbxNode* pBoneNode = NULL;
	//	int nAccBoneCount = 0;
	//	FindBoneByIndexAndName(pSDKScene->GetRootNode(), boneIndex, szBoneName, &pBoneNode, &nAccBoneCount);
	//	if (pBoneNode == NULL)
	//	{
	//		ErrorLogger::Log("StFBXManager::ParseAllBoneAnimationData : Can not find Bone ");
	//		continue;
	//	}

	//	FBXBoneAnimation* pBoneAnim = pModelAnim->TakeNew();
	//	pBoneAnim->ReserveKeyFrameCount(nFrameCount);
	//	pBoneAnim->nBoneIndex = boneIndex;
	//	FbxTime kTime;
	//	for (int frameIndex = 0; frameIndex < nFrameCount; ++frameIndex)
	//	{
	//		FBXKeyFrame* pKeyFrame = pBoneAnim->TakeNew();
	//		kTime.SetFrame(frameIndex, theTimeMode);
	//		pKeyFrame->fKeyTime = (float)kTime.GetSecondDouble();
	//		const FbxAMatrix& kGlobalTran = pBoneNode->EvaluateGlobalTransform(kTime);
	//		ConvertFbxAMatrixToDirectXMatrix(&kGlobalTran, &(pKeyFrame->matKeyTransform));
	//	}
	//}
}

void FBXModel::FindBoneByIndexAndName(FbxNode* pNode, int nBoneIndex, const char* szBoneName, FbxNode** ppResultNode, int* pAccBoneCount)
{
	if (pNode == NULL)
	{
		return;
	}
	if (*ppResultNode != NULL)
	{
		//找到了。
		return;
	}

	do
	{
		const FbxNodeAttribute* pNodeAttr = pNode->GetNodeAttribute();
		if (pNodeAttr == NULL)
		{
			break;
		}
		if (pNodeAttr->GetAttributeType() != FbxNodeAttribute::eSkeleton)
		{
			break;
		}

		if (*pAccBoneCount == nBoneIndex)
		{
			if (StringConverter::StrCmpNoCase(pNode->GetName(), szBoneName) == 0)
			{
				*ppResultNode = pNode;
				break;
			}
		}

		*pAccBoneCount += 1;

	} while (0);

	if (*ppResultNode != NULL)
	{
		//找到了。
		return;
	}

	const int nChildCount = pNode->GetChildCount();
	for (int i = 0; i < nChildCount; ++i)
	{
		FindBoneByIndexAndName(pNode->GetChild(i), nBoneIndex, szBoneName, ppResultNode, pAccBoneCount);
	}
}
