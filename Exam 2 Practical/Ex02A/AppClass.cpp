#include "AppClass.h"
using namespace Simplex;
void Application::InitVariables(void)
{
	//Change this to your name and email
	m_sProgrammer = "Sean Foley - stf7169@rit.edu";

	//Set the position and target of the camera
	m_pCameraMngr->SetPositionTargetAndUpward(	vector3(0.0f, 0.0f, 25.0f), //Position
												vector3(0.0f, 0.0f, 24.0f),	//Target
												AXIS_Y );					//Up

	//Set the position of the light
	m_pLightMngr->SetPosition(vector3(10.0f));

	m_pPipe = new Model();
	m_pPipe->Load("Mario\\WarpPipe.obj");

	//Initialize models
	for (uint i = 0; i < 6; i++)
	{
		Model* pModel = new Model();
		m_lModel.push_back(pModel);
	}
	
	m_lModel[0]->Load("Minecraft\\Creeper.obj");
	m_lModel[1]->Load("Minecraft\\Steve.obj");
	m_lModel[2]->Load("Minecraft\\Zombie.obj");
	m_lModel[3]->Load("Minecraft\\Cow.obj");
	m_lModel[4]->Load("Minecraft\\Pig.obj");
	m_lModel[5]->Load("Minecraft\\Cube.obj");

	for (uint i = 0; i < 6; i++)
	{
		MyRigidBody* pRB = new MyRigidBody(m_lModel[i]->GetVertexList());
		m_lRigidBody.push_back(pRB);
	}

	m_v3PipePos = vector3(0.0f, 10.0f, 0.0f);
	for (uint i = 0; i < 6; ++i)
	{
		m_lPosition.push_back( vector3(-1000.0f) );
		m_lOrientations.push_back(glm::angleAxis( glm::radians(glm::linearRand(0.0f,360.0f)), glm::sphericalRand(1.0f)));
	}

	m_uClock = m_pSystem->GenClock();
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();

	m_pPipe->SetModelMatrix(glm::translate(IDENTITY_M4, m_v3PipePos));
	m_pPipe->AddToRenderList();

	for (uint i = 0; i < 6; ++i)
	{
		for (uint j = i + 1; j < 6; ++j)
		{
			m_lRigidBody[i]->IsColliding(m_lRigidBody[j]);
		}
	}

	for (uint i = 0; i < 6; ++i)
	{
		m_lPosition[i] += vector3(0.0f, -0.1f, 0.0f);

		m_lModel[i]->SetModelMatrix(glm::translate(IDENTITY_M4, m_lPosition[i]) * glm::toMat4(m_lOrientations[i]));
		m_lModel[i]->AddToRenderList();
		m_lRigidBody[i]->AddToRenderList();
	}

}
void Application::Display(void)
{
	//Clear the screen
	ClearScreen();

	//Add grid to the scene
	m_pMeshMngr->AddGridToRenderList();

	//Add skybox
	m_pMeshMngr->AddSkyboxToRenderList();
	
	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
		
	//draw gui
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	SafeDelete(m_pPipe);
	for (uint i = 0; i < 6; i++)
	{
		Model* pModel = m_lModel[i];
		SafeDelete(pModel);
		MyRigidBody* pRB = m_lRigidBody[i];
		SafeDelete(pRB);
	}
	
	//release GUI
	ShutdownGUI();
}
