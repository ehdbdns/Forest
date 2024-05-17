#pragma once
#include"d3dUtil.h"
#include"rayTracer.h"
struct subset {
	UINT ID;
	UINT startv;
	UINT numv;
	UINT startface;
	UINT numface;
};

class modelReader {//不用读空行
public:
	void readMaterials(std::ifstream& fin) {
		std::string ignore;
		float a;

		for (int i = 0;i < numMaterials;i++) {
			fin >> ignore >> ignore;
			fin >> ignore >> ignore >> ignore >> ignore;
			fin >> ignore >> ignore >> ignore >> ignore;
			fin >> ignore >> ignore;
			fin >> ignore >> ignore;
			fin >> ignore >> ignore;
			fin >> ignore >> ignore;
			fin >> ignore >> ignore;

		}
	}
	void readSubset(std::ifstream& fin) {
		subsets.resize(5);
		std::string ignore;
		fin >> ignore;
		for (int i = 0;i < 5;i++) {
			fin >> ignore >> subsets[i].ID >> ignore >> subsets[i].startv >> ignore >> subsets[i].numv >> ignore >> subsets[i].startface >> ignore >> subsets[i].numface;
		}

	}
	void readVertex(std::ifstream& fin) {
		std::string ignore;
		mvertices.resize(numVertex);
		fin >> ignore;
		for (int i = 0;i < numVertex;i++) {
			fin >> ignore;
			fin >> mvertices[i].position.x >> mvertices[i].position.y >> mvertices[i].position.z;
			mvertices[i].position.w = 1.0f;
			fin >> ignore;
			fin >> mvertices[i].tangent.x >> mvertices[i].tangent.y >> mvertices[i].tangent.z >> mvertices[i].tangent.w;
			fin >> ignore;
			fin >> mvertices[i].normal.x >> mvertices[i].normal.y >> mvertices[i].normal.z;
			fin >> ignore;
			fin >> mvertices[i].texuv.x >> mvertices[i].texuv.y;
			fin >> ignore;
			fin >> mvertices[i].blendW.x >> mvertices[i].blendW.y >> mvertices[i].blendW.z >> mvertices[i].blendW.w;
			fin >> ignore;
			fin >> mvertices[i].blendIndex.x >> mvertices[i].blendIndex.y >> mvertices[i].blendIndex.z >> mvertices[i].blendIndex.w;
			mvertices[i].color = XMFLOAT3{ 0,0,0.001f };
		}
	}
	void readIndex(std::ifstream& fin) {
		std::string ignore;
		mindices.resize(numTriangles * 3);
		fin >> ignore;
		for (int i = 0;i < numTriangles * 3;i++) {
			fin >> mindices[i];
		}
	}
	void readOffsetMatrix(std::ifstream& fin) {
		std::string ignore;
		fin >> ignore;
		offsetMatrix.resize(numBones);
		for (int i = 0;i < numBones;i++) {
			fin >> ignore;
			fin >> offsetMatrix[i]._11 >> offsetMatrix[i]._12 >> offsetMatrix[i]._13 >> offsetMatrix[i]._14
				>> offsetMatrix[i]._21 >> offsetMatrix[i]._22 >> offsetMatrix[i]._23 >> offsetMatrix[i]._24
				>> offsetMatrix[i]._31 >> offsetMatrix[i]._32 >> offsetMatrix[i]._33 >> offsetMatrix[i]._34
				>> offsetMatrix[i]._41 >> offsetMatrix[i]._42 >> offsetMatrix[i]._43 >> offsetMatrix[i]._44;
		}
	}
	void readBoneFamily(std::ifstream& fin) {
		boneFamily.resize(numBones);
		std::string ignore;
		fin >> ignore;
		for (int i = 0;i < numBones;i++) {
			fin >> ignore;
			fin >> boneFamily[i];
		}
	}
	void readAnimationClips(std::ifstream& fin) {
		animationclips.resize(numAnimationClips);
		std::string ignore;
		fin >> ignore;
		for (int i = 0;i < numAnimationClips;i++) {
			fin >> ignore;
			fin >> animationclips[i].clipname;
			fin >> ignore;
			animationclips[i].boneAnimations.resize(numBones);
			for (int j = 0;j < numBones;j++) {
				int numkf = 0;
				fin >> ignore >> ignore >> numkf;
				animationclips[i].boneAnimations[j].numkeyframe = numkf;
				animationclips[i].boneAnimations[j].keyframes.resize(numkf);
				fin >> ignore;
				for (int k = 0;k < numkf;k++) {
					fin >> ignore;
					fin >> animationclips[i].boneAnimations[j].keyframes[k].starttime;
					fin >> ignore;
					fin >> animationclips[i].boneAnimations[j].keyframes[k].translation.x >>
						animationclips[i].boneAnimations[j].keyframes[k].translation.y >>
						animationclips[i].boneAnimations[j].keyframes[k].translation.z;
					fin >> ignore;
					fin >> animationclips[i].boneAnimations[j].keyframes[k].scale.x >>
						animationclips[i].boneAnimations[j].keyframes[k].scale.y >>
						animationclips[i].boneAnimations[j].keyframes[k].scale.z;
					fin >> ignore;
					fin >> animationclips[i].boneAnimations[j].keyframes[k].quat.x >>
						animationclips[i].boneAnimations[j].keyframes[k].quat.y >>
						animationclips[i].boneAnimations[j].keyframes[k].quat.z >> animationclips[i].boneAnimations[j].keyframes[k].quat.w;
				}
				fin >> ignore;
			}
			fin >> ignore;
		}
	}
	void readFileHeader(std::string filename) {
		std::ifstream fin(filename);
		int flag = 0;
		if (fin) {
			flag = 1;
		}
		std::string ignore;
		fin >> ignore;
		fin >> ignore >> numMaterials;
		fin >> ignore >> numVertex;
		fin >> ignore >> numTriangles;
		fin >> ignore >> numBones;
		fin >> ignore >> numAnimationClips;
		fin >> ignore;
		readMaterials(fin);
		readSubset(fin);
		readVertex(fin);
		readIndex(fin);
		readOffsetMatrix(fin);
		readBoneFamily(fin);
		readAnimationClips(fin);

	}

	UINT numMaterials;
	UINT numVertex;
	UINT numTriangles;
	UINT numBones;
	UINT numAnimationClips;
	EST::vector<subset>subsets;
	EST::vector<modelVertex>mvertices;
	EST::vector<std::uint16_t>mindices;
	EST::vector<XMFLOAT4X4>offsetMatrix;
	EST::vector<UINT>boneFamily;
	EST::vector<animationClips> animationclips;
};