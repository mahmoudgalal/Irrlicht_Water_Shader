#pragma once

#include <d3d9.h>
#include <D3dx9.h>

class WaterMesh
{
public:
	WaterMesh(LPDIRECT3DDEVICE9 device);
	~WaterMesh(void);
	HRESULT loadMesh(char* fpath);
	void drawWater(D3DXVECTOR3& vEyePos, unsigned int time = 0);
	void rotateMesh(float x, float y, float z);
	void setWaterType(int wType);
	void switchWaterType() {
		setWaterType((waterType == WATER_TYPE_RM) ? WATER_TYPE_NV : WATER_TYPE_RM);
	}
	static const int WATER_TYPE_RM = 1; // RenderMonkey Water Shader
	static const int WATER_TYPE_NV = 2; // NVIDIA Water Shader
private:
	HRESULT loadEffects();
	HRESULT loadRMEffect();
	HRESULT loadNVEffect();
	LPDIRECT3DDEVICE9   g_pd3dDevice; // Our rendering device
	LPD3DXMESH mesh;
	D3DMATERIAL9* g_pMeshMaterials; // Materials for our mesh
	LPDIRECT3DTEXTURE9* g_pMeshTextures; // Textures for our mesh
	DWORD               g_dwNumMaterials;   // Number of mesh materials

	ID3DXEffect* g_pEffect, * g_pNVEffect, * currentEffect;
	LPDIRECT3DTEXTURE9   tBump, tEnv, tGradient, envTexture, normalTexture;  // Mesh texture
	int waterType;
	float lastTime = 0.f;
};
