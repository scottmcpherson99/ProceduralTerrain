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
	void ParticleDepositionGUI();
	void MidPointDisplacementGUI();

private:
	LightShader* shader;
	TerrainMesh* m_Terrain;

	Light* light;

	int terrainResolution = 128;
	
	//faulting parameters
	bool faultMenuOpen;
	int totalFaults = 4;
	float faultValue = 3;

	//particle deposution parameters
	bool particleDepoMenuOpen;
	int numberOfParticlesDropped = 100;
	int XWidth = 10;
	int YWidth = 10;
	bool dropParticles = true;
	int iterations = 3;

	//midpointDisplacement parameters
	bool midPointMenuOpen;
	int maxHeightOfCorners = 5;
	float smoothnessOfTerrain = 2;
};

#endif