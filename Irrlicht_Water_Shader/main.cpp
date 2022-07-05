#define NOMINMAX
#include <windows.h>
#include <irrlicht.h>
#include "WaterMesh.h"
#include <memory>

#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


IDirect3DStateBlock9* pStateBlock11 = NULL;

bool appExit = false;
std::unique_ptr<WaterMesh> wm = NULL;


ISceneManager* smgr = NULL;
ICameraSceneNode* fpsCam = NULL;
IVideoDriver* driver = NULL;
IrrlichtDevice* device = NULL;



class MyEventReceiver : public IEventReceiver
{
public:
	// This is the one method that we have to implement
	virtual bool OnEvent(const SEvent& event)
	{
		// Remember whether each key is down or up
		if (event.EventType == irr::EET_KEY_INPUT_EVENT)
		{
			KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
			if (irr::KEY_KEY_W == event.KeyInput.Key && event.KeyInput.PressedDown)
			{
				//createCubeFromEye(3,fpsCam->getPosition(),fpsCam->getTarget()- fpsCam->getPosition());

			}
			if (irr::KEY_KEY_S == event.KeyInput.Key && event.KeyInput.PressedDown)
			{
				//createSphereFromEye(3,fpsCam->getPosition(),fpsCam->getTarget()-fpsCam->getPosition());
				if (wm)
					wm->switchWaterType();
			}
			if (irr::KEY_KEY_D == event.KeyInput.Key && event.KeyInput.PressedDown)
			{
				//CreateStack(10);
			}
			if (irr::KEY_ESCAPE == event.KeyInput.Key && event.KeyInput.PressedDown)
			{
				appExit = true;
			}

		}
		// event.KeyInput.
		return false;
	}

	// This is used to check whether a key is being held down
	virtual bool IsKeyDown(EKEY_CODE keyCode) const
	{
		return KeyIsDown[keyCode];
	}

	MyEventReceiver()
	{
		for (u32 i = 0; i < KEY_KEY_CODES_COUNT; ++i)
			KeyIsDown[i] = false;
	}

private:
	// We use this array to store the current state of each key
	bool KeyIsDown[KEY_KEY_CODES_COUNT];
};

int main()
{
	MyEventReceiver receiver;
	// create device
	device = createDevice(video::EDT_DIRECT3D9, dimension2d<u32>(1600, 900), 32,
			true, false, false, &receiver);

	if (!device)
		return 1;
	driver = device->getVideoDriver();
	smgr = device->getSceneManager();
	string<wchar_t> title = L"Irrlicht-WaterShader Demo";
	device->setWindowCaption(title.c_str());
	device->getCursorControl()->setVisible(false);

	IGUIEnvironment* guienv = device->getGUIEnvironment();

	IGUIStaticText* txt = guienv->addStaticText(title.c_str(),
		rect<s32>(10, 10, 260, 42), true);
	txt->setBackgroundColor(SColor(155, 255, 255, 255));
	txt->setDrawBorder(true);

	fpsCam = smgr->addCameraSceneNodeFPS(NULL, 70, 0.08f);
	fpsCam->setPosition(vector3df(0, 20, 40));
	fpsCam->setTarget(core::vector3df(0.0f, 60, 10));

	fpsCam->setFarValue(42000.0f);

	// create skybox and skydome
	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
	driver->setTextureCreationFlag(video::ETCF_ALLOW_NON_POWER_2, true);
	driver->setTextureCreationFlag(video::ETCF_ALWAYS_32_BIT, true);

	scene::ISceneNode* skybox = smgr->addSkyBoxSceneNode(
		driver->getTexture("resources/irrlicht2_up.jpg"),
		driver->getTexture("resources/irrlicht2_dn.jpg"),
		driver->getTexture("resources/irrlicht2_lf.jpg"),
		driver->getTexture("resources/irrlicht2_rt.jpg"),
		driver->getTexture("resources/irrlicht2_ft.jpg"),
		driver->getTexture("resources/irrlicht2_bk.jpg"));

	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);

	wm = std::make_unique<WaterMesh>(driver->getExposedVideoData().D3D9.D3DDev9);
	if (wm)
		if (FAILED(wm->loadMesh("resources/WaterSurface.x")))
		{
			OutputDebugStringA("error loading mesh\n");
		}

	while (device->run())
	{
		if (appExit)
			break;
		driver->beginScene(true, true, SColor(255, 100, 101, 140));
		title = L"Irrlicht-WaterShader Demo , FPS:";
		title += string<wchar_t>(driver->getFPS());
		title += L"\nPress (S) to Switch Water Type";
		txt->setText(title.c_str());

		if (!pStateBlock11)
			driver->getExposedVideoData().D3D9.D3DDev9->CreateStateBlock
			(D3DSBT_VERTEXSTATE, &pStateBlock11);
		else
			pStateBlock11->Apply();
		smgr->drawAll();
		if (wm)
			wm->drawWater(D3DXVECTOR3(fpsCam->getPosition().X,
				fpsCam->getPosition().Y,
				fpsCam->getPosition().Z), device->getTimer()->getRealTime());

		pStateBlock11->Apply();
		guienv->drawAll();

		driver->endScene();
	}

	if (pStateBlock11)
		pStateBlock11->Release();
	device->drop();

	return 0;
}