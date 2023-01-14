//----------------------------------------------------------------
#ifndef _FBXDefine_h_
#define _FBXDefine_h_
//----------------------------------------------------------------
#include <DirectXMath.h>
#include <string>
#include "BitFlag.h"

using namespace DirectX;
//----------------------------------------------------------------
//1，我们约定，一个fbx文件中，所有的Mesh的顶点数据格式都相同，
//   也即顶点数据中包含的成员（法线，顶点色，UV）都相同。
//   本模块把所有的Mesh的顶点数据存储在同一个VertexBuff中，只需一个
//   DrawCall就把模型渲染出来。
//2，fbx文件中可以包含一个Mesh对象，也可以包含多个Mesh对象，本模块
//   都能够正确解析。
//3，顶点数据中成员的存储顺序必须是 顶点-法线-切线-顶点色-UV1-UV2 。
//   目前，在导出kfm文件时要依赖这个顺序。
//----------------------------------------------------------------
//三角形面片的索引的排列方式，也即绕序是顺时针还是逆时针。
//在fbx文件中不好识别顺时针和逆时针，我们用“123顺序构成三角形”和“321顺序构成三角形”表达。
//下面的值为1，表示使用“123顺序构成三角形”；值为0表示使用“321顺序构成三角形”。
#define FBX_TriangleIndex_123 0
//----------------------------------------------------------------
//UV坐标中的y值是否转换为取反，即取值为(1-y)。
//值为1，表示要将原来的y值转换为(1-y)。
//这是因为，在我的渲染框架中，贴图坐标系的(0,0)点位于左上角，并且贴图像素是正常排列。
//在某些渲染框架中，可能出现“贴图坐标系的(0,0)点位于左下角，并且贴图像素是正常排列”，
//或者“贴图坐标系的(0,0)点位于左上角，但是贴图像素是倒置的，上下颠倒的”。
//在这种情况下，就需要更改下面这个值。
#define FBX_UVHeightInverse 1
//----------------------------------------------------------------
//是否提取fbx文件中的法线数据
#define FBX_ParseNormalData 1
//是否提取fbx文件中的切线数据
#define FBX_ParseTangentData 1
//是否提取fbx文件中的顶点色数据
#define FBX_ParseColorData 0
//是否提取fbx文件中的第一组UV数据
#define FBX_ParseUV1Data 1
//是否提取fbx文件中的第二组UV数据
#define FBX_ParseUV2Data 0
//----------------------------------------------------------------
//每个控制点最多受几根骨骼影响。
#define FBX_MaxCount_BoneIndexSkinWeightPerControlPoint 4
//一个骨骼最多有多少个子骨骼。
//目前，子骨骼列表并没有发挥作用。
#define FBX_MaxCount_ChildBone 10

//----------------------------------------------------------------
//Vector3有3个float元素
#define FBX_Sizeof_Vector3 12
//Color有4个float元素
#define FBX_Sizeof_Color 16
//UV有2个float元素
#define FBX_Sizeof_UV 8
//像素Pixel的Sizeof
#define FBX_Sizeof_Pixel 4
//顶点序号的sizeof值，32位整型。
#define FBX_Sizeof_VertexIndex 4
//----------------------------------------------------------------
enum FBXElementType
{
	FBXElement_Position = 0x00000001,
	FBXElement_Normal = 0x00000002,
	FBXElement_Tangent = 0x00000004,
	FBXElement_Color = 0x00000008,
	FBXElement_UV1 = 0x00000010,
	FBXElement_UV2 = 0x00000020,
};
//----------------------------------------------------------------
//存储fbx文件中所有的Mesh的顶点数据。
//在解析fbx文件完毕后，这里面存储的顶点数据就是模型默认Pose的顶点数据。
//顶点结构体个数与顶点索引个数是相同的。
//这就意味着，如果一个顶点被两个相邻三角形共用的话，这个顶点数据就被制作成了两份顶点结构体。
struct FBXMeshData
{
	char* pVertexBuff;
	int nVertexBuffSize;
	int nVertexCount;
	char* pIndexBuff;
	int nIndexBuffSize;
	int nIndexCount;
	//顶点位置序号到控制点位置序号的映射。
	//该数组的长度就是nVertexCount。
	int* pVertexIndex2ControlPointIndex;
	//顶点结构体中含有哪些成员（法线，贴图坐标等等）。
	BitFlag kVertexType;
	//顶点结构体的字节数。
	//我们约定，一个fbx文件中，所有的Mesh的顶点数据格式都相同。
	//不同的mesh中，顶点结构体的成员相同，顶点结构体的字节数也相同。
	int nSizeofVertexData;

	FBXMeshData();
	~FBXMeshData();
	void SetVertexType(const BitFlag& kType);
	void ReserveVertexCount(const int nVertexCount);
};
//----------------------------------------------------------------
//存储骨骼序号和该骨骼对某个控制点的影响权重。
//BoneIndex就是StFBXBoneGroup中的数组下标。
struct FBXBoneIndexSkinWeight
{
	std::string kBoneName;
	int nBoneIndex;
	float fSkinWeight;

	FBXBoneIndexSkinWeight();
};
//----------------------------------------------------------------
//单个控制点。
struct FBXControlPoint
{
	//顶点坐标。
	XMFLOAT3 kVertex;
	//本控制点受哪些骨骼的影响，以及影响权重。
	FBXBoneIndexSkinWeight kPairList[FBX_MaxCount_BoneIndexSkinWeightPerControlPoint];

	FBXControlPoint();
	void AddBoneNameSkinWeight(const char* szBoneName, float fSkinWeight);
};
//----------------------------------------------------------------
//存储fbx中所有的Mesh的控制点。
struct FBXBoneGroup; //前向声明
struct FBXControlPointGroup
{
private:
	FBXControlPoint* kControlPointArray;
	int nControlPointValidCount;
	int nControlPointMaxCount;

public:
	FBXControlPointGroup();
	~FBXControlPointGroup();
	void ReserveControlPointCount(int nCount);
	//向外界返回一个未使用的 StFBXControlPoint 对象，由外界对其赋值。
	FBXControlPoint* TakeNew();
	FBXControlPoint* GetAt(int nIndex) const;
	int GetSize() const;
	//最开始，StFBXBoneIndexSkinWeight数据中存储的是骨骼名字。
	//当有了骨骼数据后，就将骨骼名字转换成相应的骨骼序号。
	void MakeBoneIndexByBoneName(const FBXBoneGroup* pBoneGroup);

	//检查每个控制点的受骨骼影响权重，保证权重的总和是1。
	//我现在认为不需要保证权重的总和是1，
	//例如，有一个顶点，它只受两根骨骼的轻微的影响，
	//权重值比较小才能体现“轻微”，那么权重的总和必定不是1。
	//void CheckBoneSkinWeight();
};
//----------------------------------------------------------------
//骨骼节点
struct FBXBone
{
	//本骨骼的名字。
	std::string kBoneName;
	//父骨骼在层级列表中的序号位置。
	int nParentIndex;
	//子骨骼在层级列表中的序号位置。每个元素都是一个int值。
	int kChildIndexList[FBX_MaxCount_ChildBone];
	//动画播放过程中，顶点坐标的计算公式：
	//VertexAtTimeT = VertexFromControlPoint * kMatFromBoneSpaceToWorldSpace * KeyFrameMatrixAtTimeT
	XMMATRIX kMatFromBoneSpaceToModelSpace;

	FBXBone();
	~FBXBone();
};
//----------------------------------------------------------------
//存储fbx文件中所有的Bone，骨骼层级结构 Hierarchy。
struct FBXBoneGroup
{
private:
	FBXBone* kBoneArray;
	int nBoneValidCount;
	int nBoneMaxCount;

public:
	FBXBoneGroup();
	~FBXBoneGroup();
	void ReserveBoneCount(int nCount);
	//遍历骨骼列表，分别生成每个骨骼的子骨骼列表。
	void GenerateChildren();
	int GetBoneIndexByBoneName(const char* szBoneName) const;
	//向外界返回一个未使用的 StFBXBone 对象，由外界对其赋值。
	FBXBone* TakeNew(int* pIndex);
	FBXBone* GetAt(int nIndex) const;
	int GetSize() const;
};
//----------------------------------------------------------------
//动画帧
struct FBXKeyFrame
{
	float fKeyTime;
	XMMATRIX matKeyTransform;

	FBXKeyFrame();
};
//----------------------------------------------------------------
//在一个动画中，一个Bone所携带的所有的动画帧。
//BoneIndex就是StFBXBoneGroup中的数组下标。
struct FBXBoneAnimation
{
	FBXKeyFrame* kKeyFrameArray;
	int nFrameValidCount;
	int nFrameMaxCount;
	int nBoneIndex;

	FBXBoneAnimation();
	~FBXBoneAnimation();
	void ReserveKeyFrameCount(int nCount);
	//向外界返回一个未使用的 StFBXKeyFrame 对象，由外界对其赋值。
	FBXKeyFrame* TakeNew();
	FBXKeyFrame* GetAt(int nIndex) const;
	int GetSize() const;
};
//----------------------------------------------------------------
//在一个动画中，一个模型所携带的所有的动画帧。
struct FBXModelAnimation
{
	//所有骨骼的动画帧列表。每个元素都是一个StFBXBoneAnimation对象。
	//一般情况下，本数组的size就是骨骼总个数。
	//有可能出现这种情况，在动画中，只有一部分骨骼发挥了作用，
	//那么本数组的size就会小于骨骼总个数。
	FBXBoneAnimation* kBoneAnimationArray;
	int nAnimValidCount;
	int nAnimMaxCount;
	//动画有多少帧。
	int nFrameCount;
	//动画的持续时长。单位秒。
	float fAnimLength;

	FBXModelAnimation();
	~FBXModelAnimation();
	void ReserveBoneCount(int nBoneCount);
	int GetSize() const;
	int GetFrameCount() const;
	//向外界返回一个未使用的 StFBXBoneAnimation 对象，由外界对其赋值。
	FBXBoneAnimation* TakeNew();
	FBXBoneAnimation* GetAt(int nIndex) const;
	FBXBoneAnimation* GetBoneAnimation(int nBoneIndex) const;
	//根据指定的时间，获取该时间要播放动画的哪一帧。
	int GetKeyFrameIndexByTime(float fTime);
};
//----------------------------------------------------------------
#endif //_FBXDefine_h_
//----------------------------------------------------------------
