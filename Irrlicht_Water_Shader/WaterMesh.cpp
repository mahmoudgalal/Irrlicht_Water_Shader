
#include "WaterMesh.h"

IDirect3DStateBlock9* pStateBlock = NULL;

WaterMesh::WaterMesh(LPDIRECT3DDEVICE9 device)
{
	g_pd3dDevice = device;
	g_dwNumMaterials = 0;
	mesh = NULL;
	waterType = WATER_TYPE_NV;
}
HRESULT WaterMesh::loadMesh(char* fpath)
{
	if (mesh)
		return S_FALSE;
	LPD3DXBUFFER pD3DXMtrlBuffer;
	ZeroMemory(&pD3DXMtrlBuffer, sizeof(pD3DXMtrlBuffer));
	// Load the mesh from the specified file
	if (FAILED(D3DXLoadMeshFromXA(fpath, D3DXMESH_SYSTEMMEM,
		g_pd3dDevice, NULL,
		&pD3DXMtrlBuffer, NULL, &g_dwNumMaterials,
		&mesh)))
	{
		const CHAR* strPrefix = "..\\";
		CHAR buffer[MAX_PATH];
		strcpy_s(buffer, MAX_PATH, strPrefix);
		strcat_s(buffer, MAX_PATH, fpath);
		// If model is not in current folder, try parent folder
		if (FAILED(D3DXLoadMeshFromXA(buffer, D3DXMESH_SYSTEMMEM,
			g_pd3dDevice, NULL,
			&pD3DXMtrlBuffer, NULL, &g_dwNumMaterials,
			&mesh)))
		{
			MessageBoxA(NULL, "Could not find The file Specified", "ERROR", MB_OK);
			return E_FAIL;
		}
	}


	// We need to extract the material properties and texture names from the 
	// pD3DXMtrlBuffer
	D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
	g_pMeshMaterials = new D3DMATERIAL9[g_dwNumMaterials];
	if (g_pMeshMaterials == NULL)
		return E_OUTOFMEMORY;
	g_pMeshTextures = new LPDIRECT3DTEXTURE9[g_dwNumMaterials];
	if (g_pMeshTextures == NULL)
		return E_OUTOFMEMORY;

	for (DWORD i = 0; i < g_dwNumMaterials; i++)
	{
		// Copy the material
		g_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;

		// Set the ambient color for the material (D3DX does not do this)
		g_pMeshMaterials[i].Ambient = g_pMeshMaterials[i].Diffuse;

		g_pMeshTextures[i] = NULL;
		if (d3dxMaterials[i].pTextureFilename != NULL &&
			lstrlenA(d3dxMaterials[i].pTextureFilename) > 0)
		{
			// Create the texture
			if (FAILED(D3DXCreateTextureFromFileA(g_pd3dDevice,
				d3dxMaterials[i].pTextureFilename,
				&g_pMeshTextures[i])))
			{
				// If texture is not in current folder, try parent folder
				const CHAR* strPrefix = "..\\";
				CHAR strTexture[MAX_PATH];
				strcpy_s(strTexture, MAX_PATH, strPrefix);
				strcat_s(strTexture, MAX_PATH, d3dxMaterials[i].pTextureFilename);
				// If texture is not in current folder, try parent folder
				if (FAILED(D3DXCreateTextureFromFileA(g_pd3dDevice,
					strTexture,
					&g_pMeshTextures[i])))
				{
					CHAR strERROR[MAX_PATH + 30];
					strcpy_s(strERROR, MAX_PATH + 30, "Could not find texture map at\n");
					strcat_s(strERROR, MAX_PATH + 30, d3dxMaterials[i].pTextureFilename);
					MessageBoxA(NULL, strERROR, "ERROR", MB_OK);
				}
			}
		}
	}
	// Done with the material buffer
	pD3DXMtrlBuffer->Release();
	loadEffects();
	currentEffect = (waterType == WATER_TYPE_NV) ? g_pNVEffect : g_pEffect;
	return S_OK;
}

void WaterMesh::drawWater(D3DXVECTOR3& veyePos, unsigned int time)
{

	D3DXVECTOR4 vLightpos(-80, 50, 5, 1);
	D3DXVECTOR4 vEyePos(veyePos, 1);
	D3DXMATRIXA16 mWorld;
	D3DXMATRIXA16 mView;
	D3DXMATRIXA16 mProj;
	D3DXMATRIXA16 mWorldViewProjection;
	UINT iPass, cPasses;
	const float fTime = 2 * time / 1000.0f;
	D3DXMatrixTranslation(&mWorld, 0, 0, 0);
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &mWorld);

	g_pd3dDevice->GetTransform(D3DTS_WORLD, &mWorld);
	g_pd3dDevice->GetTransform(D3DTS_PROJECTION, &mProj);
	g_pd3dDevice->GetTransform(D3DTS_VIEW, &mView);
	mWorldViewProjection = mWorld * mView * mProj;

	if (waterType == WATER_TYPE_NV) {
		currentEffect->SetMatrix("gWorldXf", &mWorld);
		currentEffect->SetMatrix("gWvpXf", &mWorldViewProjection);
		currentEffect->SetMatrix("gViewIXf", D3DXMatrixInverse(NULL, NULL, &mView));
		currentEffect->SetMatrix("gWorldITXf", D3DXMatrixTranspose(NULL, D3DXMatrixInverse(NULL, NULL, &mWorld)));
		currentEffect->SetFloat("gTimer", fTime);
	}
	else {
		currentEffect->SetMatrix("mTot", &mWorldViewProjection);
		currentEffect->SetMatrix("mView", &mView);
		currentEffect->SetFloat("ticks", fTime);
		currentEffect->SetFloat("sinticks", sin(fTime));
		currentEffect->SetValue("lhtPos", vLightpos, sizeof(D3DXVECTOR4));
		currentEffect->SetVector("vEyePos", &vEyePos);
	}
	currentEffect->Begin(&cPasses, 0);

	for (iPass = 0; iPass < cPasses; iPass++)
	{
		currentEffect->BeginPass(iPass);
		if (g_dwNumMaterials != 0)
			for (DWORD i = 0; i < g_dwNumMaterials; i++)
			{
				if (D3DERR_INVALIDCALL == (mesh->DrawSubset(i)))
					OutputDebugStringA("Error in DrawSubset(0)\n");
			}
		currentEffect->EndPass();
	}
	currentEffect->End();
	lastTime = timeGetTime();

}
void WaterMesh::rotateMesh(float x, float y, float z)
{
	D3DXMATRIXA16 matWorld;
	D3DXMatrixRotationYawPitchRoll(&matWorld, x, y, z);
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
}

HRESULT WaterMesh::loadEffects()
{
	if (FAILED(loadNVEffect()))
		return S_FALSE;
	return loadRMEffect();

}

HRESULT WaterMesh::loadRMEffect() {
	LPD3DXBUFFER errorBuffer;
	DWORD dwShaderFlags = 0;

	// dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
	 //dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
	dwShaderFlags |= D3DXSHADER_DEBUG;
	//dwShaderFlags |=D3DXSHADER_SKIPOPTIMIZATION;
	//dwShaderFlags |= D3DXSHADER_NO_PRESHADER;

	if (D3D_OK != (D3DXCreateEffectFromFileA(g_pd3dDevice, "resources/water.fx",
		NULL, // CONST D3DXMACRO* pDefines,
		NULL, // LPD3DXINCLUDE pInclude,
		dwShaderFlags,
		NULL, // LPD3DXEFFECTPOOL pPool,
		&g_pEffect,
		&errorBuffer)))
	{
		MessageBoxA(NULL, "Could not find The effect file Specified", "EEROR", MB_OK);
		MessageBoxA(NULL, (char*)errorBuffer->GetBufferPointer(), "EEROR", MB_OK);
		OutputDebugStringA((char*)errorBuffer->GetBufferPointer());
		goto fail;
	}

	D3DXCreateTextureFromFileExA(g_pd3dDevice, "resources/WaterBump.tga", D3DX_DEFAULT, D3DX_DEFAULT,
		D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
		D3DX_DEFAULT, D3DX_DEFAULT, 0,
		NULL, NULL, &tBump);

	D3DXCreateTextureFromFileExA(g_pd3dDevice, "resources/WaterEnvMap.tga", D3DX_DEFAULT, D3DX_DEFAULT,
		D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
		D3DX_DEFAULT, D3DX_DEFAULT, 0,
		NULL, NULL, &tEnv);
	if (FAILED(D3DXCreateTextureFromFileExA(g_pd3dDevice, "resources/WaterGradient.tga", D3DX_DEFAULT, D3DX_DEFAULT,
		D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
		D3DX_DEFAULT, D3DX_DEFAULT, 0,
		NULL, NULL, &tGradient)))
	{
		MessageBoxA(NULL, "D3DXCreateTextureFromFileExA()", "EEROR", MB_OK);
		goto fail;
	}
	g_pEffect->SetTexture("tBump", tBump);
	g_pEffect->SetTexture("tEnv", tEnv);
	g_pEffect->SetTexture("tGradient", tGradient);
	if (FAILED(g_pEffect->ValidateTechnique("tec0")))
	{
		MessageBoxA(NULL, "ValidateTechnique() Failed", "EEROR", MB_OK);
		goto fail;
	}
	g_pEffect->SetTechnique("tec0");
	if (errorBuffer)
		errorBuffer->Release();

	return S_OK;
fail:
	if (errorBuffer)
		errorBuffer->Release();
	return S_FALSE;

}

HRESULT WaterMesh::loadNVEffect() {
	LPD3DXBUFFER errorBuffer;
	DWORD dwShaderFlags = 0;

	//dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
	//dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
	dwShaderFlags |= D3DXSHADER_DEBUG;
	//dwShaderFlags |=D3DXSHADER_SKIPOPTIMIZATION;
	//dwShaderFlags |= D3DXSHADER_NO_PRESHADER;

	if (D3D_OK != (D3DXCreateEffectFromFile(g_pd3dDevice, L"resources/Ocean.fx",
		NULL, // CONST D3DXMACRO* pDefines,
		NULL, // LPD3DXINCLUDE pInclude,
		dwShaderFlags,
		NULL, // LPD3DXEFFECTPOOL pPool,
		&g_pNVEffect,
		&errorBuffer)))
	{
		MessageBoxA(NULL, "Could not find The effect file Specified", "EEROR", MB_OK);
		MessageBoxA(NULL, (char*)errorBuffer->GetBufferPointer(), "EEROR", MB_OK);
		OutputDebugStringA((char*)errorBuffer->GetBufferPointer());
		goto fail;
	}

	if (FAILED(D3DXCreateTextureFromFileExA(g_pd3dDevice, "resources/waves2.dds", D3DX_DEFAULT, D3DX_DEFAULT,
		D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
		D3DX_DEFAULT, D3DX_DEFAULT, 0,
		NULL, NULL, &normalTexture)))
	{
		MessageBoxA(NULL, "D3DXCreateTextureFromFileExA()", "EEROR", MB_OK);
		goto fail;
	}
	if (FAILED(D3DXCreateTextureFromFileExA(g_pd3dDevice, "resources/CloudyHillsCubemap2.dds", D3DX_DEFAULT, D3DX_DEFAULT,
		D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
		D3DX_DEFAULT, D3DX_DEFAULT, 0,
		NULL, NULL, &envTexture)))
	{
		MessageBoxA(NULL, "D3DXCreateTextureFromFileExA()", "EEROR", MB_OK);
		goto fail;
	}
	//g_pEffect->SetTexture("tBump", tBump);
	g_pNVEffect->SetTexture("NormalTexture", normalTexture);
	g_pNVEffect->SetTexture("gEnvTexture", envTexture);
	if (FAILED(g_pNVEffect->ValidateTechnique("Main")))
	{
		MessageBoxA(NULL, "ValidateTechnique() Failed", "EEROR", MB_OK);
		goto fail;
	}
	if (FAILED(g_pNVEffect->SetTechnique("Main")))
	{
		MessageBoxA(NULL, "SetTechnique() Failed", "EEROR", MB_OK);
		goto fail;
	}
	if (errorBuffer)
		errorBuffer->Release();

	return S_OK;
fail:
	if (errorBuffer)
		errorBuffer->Release();
	return S_FALSE;
}
void WaterMesh::setWaterType(int wType)
{
	waterType = wType;
	currentEffect = (waterType == WATER_TYPE_NV) ? g_pNVEffect : g_pEffect;
}

WaterMesh::~WaterMesh(void)
{
	if (pStateBlock != NULL)
	{
		pStateBlock->Release();
		OutputDebugStringA("pStateBlock->Release() Success\n");
		pStateBlock = NULL;
	}
	if (tBump)
		tBump->Release();
	if (tEnv)
		tEnv->Release();
	if (tGradient)
		tGradient->Release();
	if (normalTexture)
		normalTexture->Release();
	if (envTexture)
		envTexture->Release();

	if (g_pEffect)
		g_pEffect->Release();
	if (g_pNVEffect)
		g_pNVEffect->Release();
	if (mesh != NULL)
		mesh->Release();
}
