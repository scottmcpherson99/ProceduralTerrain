// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "LightShader.h"
#include "TerrainMesh.h"

class App1 : public BaseApplication
{
public:
	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void gui();
	void FaultGUI();

private:
	LightShader* shader;
	TerrainMesh* m_Terrain;

	Light* light;

	int terrainResolution = 128;

	bool faultMenuOpen;
	int totalFaults = 4;
	float faultValue = 3;
};

#endif