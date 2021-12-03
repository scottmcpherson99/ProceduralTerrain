// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1()
{
	m_Terrain = nullptr;
	shader = nullptr;
	light = nullptr;

	faultMenuOpen = false;
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	textureMgr->loadTexture(L"grass", L"res/grass.png");
	textureMgr->loadTexture(L"white", L"res/DefaultDiffuse.png");

	// Create Mesh object and shader object
	m_Terrain = new TerrainMesh(renderer->getDevice(), renderer->getDeviceContext());
	m_Terrain->Flat();
	m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());

	shader = new LightShader(renderer->getDevice(), hwnd);

	light = new Light;
	light->setAmbientColour( 0.25f, 0.25f, 0.25f, 1.0f );
	light->setDiffuseColour(0.75f, 0.75f, 0.75f, 1.0f);
	light->setDirection(1.0f,-0.0f, 0.0f);

	camera->setPosition(0.0f, 45.0f, -30.0f);
	camera->setRotation( 0.0f, 30.0f,30.0f );

	terrainResolution = m_Terrain->GetResolution();
}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.
	if ( m_Terrain )
	{
		delete m_Terrain;
		m_Terrain = 0;
	}
	if (shader)
	{
		delete shader;
		shader = 0;
	}
}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	// Generate the view matrix based on the camera's position.
	camera->update();

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();

	// Send geometry data, set shader parameters, render object with shader
	m_Terrain->sendData(renderer->getDeviceContext());
	shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"grass"), light);
	shader->render(renderer->getDeviceContext(), m_Terrain->getIndexCount());
	
	// Render GUI
	gui();

	// Swap the buffers
	renderer->endScene();

	return true;
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Text( "Camera Pos: (%.2f, %.2f, %.2f)", camera->getPosition().x, camera->getPosition().y, camera->getPosition().z );
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);

	ImGui::SliderInt( "Terrain Resolution", &terrainResolution, 2, 1024 );

	FaultGUI();
	
	if (ImGui::Button("Smoothen"))
	{
		m_Terrain->Smooth();
		m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
	}

	if( ImGui::Button( "Regenerate Terrain" ) ) {
		if( terrainResolution != m_Terrain->GetResolution() ) {
			m_Terrain->Resize( terrainResolution );
		}
		m_Terrain->Flat();
		m_Terrain->Regenerate( renderer->getDevice(), renderer->getDeviceContext() );
	}

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void App1::FaultGUI()
{
	//modify the variables for the faulting algorithm
	if (faultMenuOpen == false)
	{
		if (ImGui::Button("Open Fault Menu"))
		{
			faultMenuOpen = true;
		}
	}
	if (faultMenuOpen == true)
	{
		ImGui::SliderInt("Number of Faults", &totalFaults, 1, 50);
		ImGui::SliderFloat("Initial Fault Value", &faultValue, 0.1, 10, "Pos : (%.3f)", 1.0f);
		if (ImGui::Button("Apply Fault"))
		{
			m_Terrain->Fault(totalFaults, faultValue);
			m_Terrain->Regenerate(renderer->getDevice(), renderer->getDeviceContext());
		}
		if (ImGui::Button("Close Menu"))
		{
			faultMenuOpen = false;
		}
	}
}