#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader);

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));

	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue, -fValue, fValue); //0
	vector3 point1(fValue, -fValue, fValue); //1
	vector3 point2(fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue, -fValue, -fValue); //4
	vector3 point5(fValue, -fValue, -fValue); //5
	vector3 point6(fValue, fValue, -fValue); //6
	vector3 point7(-fValue, fValue, -fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	float angleDelta = (2 * PI) / a_nSubdivisions;

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		AddTri(vector3(0, 0, 0), vector3(cos(angleDelta * (i + 1)) * a_fRadius, 0, sin(angleDelta * (i + 1)) * a_fRadius), vector3(cos(angleDelta * i) * a_fRadius, 0, sin(angleDelta * i) * a_fRadius));
		AddTri(vector3(cos(angleDelta * i) * a_fRadius, 0, sin(angleDelta * i) * a_fRadius), vector3(cos(angleDelta * (i + 1)) * a_fRadius, 0, sin(angleDelta * (i + 1)) * a_fRadius), vector3(0, a_fHeight, 0));
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	float angleDelta = (2 * PI) / a_nSubdivisions;

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		AddTri(vector3(0, 0, 0), vector3(cos(angleDelta * i) * a_fRadius, 0, sin(angleDelta * i) * a_fRadius), vector3(cos(angleDelta * (i + 1)) * a_fRadius, 0, sin(angleDelta * (i + 1)) * a_fRadius));
		AddTri(vector3(0, a_fHeight, 0), vector3(cos(angleDelta * (i + 1)) * a_fRadius, a_fHeight, sin(angleDelta * (i + 1)) * a_fRadius), vector3(cos(angleDelta * i) * a_fRadius, a_fHeight, sin(angleDelta * i) * a_fRadius));
		AddQuad(vector3(cos(angleDelta * i) * a_fRadius, a_fHeight, sin(angleDelta * i) * a_fRadius), vector3(cos(angleDelta * (i + 1)) * a_fRadius, a_fHeight, sin(angleDelta * (i + 1)) * a_fRadius),
			vector3(cos(angleDelta * i) * a_fRadius, 0, sin(angleDelta * i) * a_fRadius), vector3(cos(angleDelta * (i + 1)) * a_fRadius, 0, sin(angleDelta * (i + 1)) * a_fRadius));
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	float angleDelta = (2 * PI) / a_nSubdivisions;

	for (int i = 0; i < a_nSubdivisions; i++)
	{
		AddQuad(vector3(cos(angleDelta * i) * a_fOuterRadius, 0, sin(angleDelta * i) * a_fOuterRadius), vector3(cos(angleDelta * (i + 1)) * a_fOuterRadius, 0, sin(angleDelta * (i + 1)) * a_fOuterRadius),
			vector3(cos(angleDelta * i) * a_fInnerRadius, 0, sin(angleDelta * i) * a_fInnerRadius), vector3(cos(angleDelta * (i + 1)) * a_fInnerRadius, 0, sin(angleDelta * (i + 1)) * a_fInnerRadius));

		AddQuad(vector3(cos(angleDelta * (i + 1)) * a_fOuterRadius, a_fHeight, sin(angleDelta * (i + 1)) * a_fOuterRadius), vector3(cos(angleDelta * i) * a_fOuterRadius, a_fHeight, sin(angleDelta * i) * a_fOuterRadius),
			vector3(cos(angleDelta * (i + 1)) * a_fInnerRadius, a_fHeight, sin(angleDelta * (i + 1)) * a_fInnerRadius), vector3(cos(angleDelta * i) * a_fInnerRadius, a_fHeight, sin(angleDelta * i) * a_fInnerRadius));

		AddQuad(vector3(cos(angleDelta * i) * a_fOuterRadius, a_fHeight, sin(angleDelta * i) * a_fOuterRadius), vector3(cos(angleDelta * (i + 1)) * a_fOuterRadius, a_fHeight, sin(angleDelta * (i + 1)) * a_fOuterRadius),
			vector3(cos(angleDelta * i) * a_fOuterRadius, 0, sin(angleDelta * i) * a_fOuterRadius), vector3(cos(angleDelta * (i + 1)) * a_fOuterRadius, 0, sin(angleDelta * (i + 1)) * a_fOuterRadius));

		AddQuad(vector3(cos(angleDelta * (i + 1)) * a_fInnerRadius, a_fHeight, sin(angleDelta * (i + 1)) * a_fInnerRadius), vector3(cos(angleDelta * i) * a_fInnerRadius, a_fHeight, sin(angleDelta * i) * a_fInnerRadius),
			vector3(cos(angleDelta * (i + 1)) * a_fInnerRadius, 0, sin(angleDelta * (i + 1)) * a_fInnerRadius), vector3(cos(angleDelta * i) * a_fInnerRadius, 0, sin(angleDelta * i) * a_fInnerRadius));
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	float angleDeltaA = (2 * PI) / a_nSubdivisionsA;
	float angleDeltaB = (2 * PI) / a_nSubdivisionsB;
	vector3 relativeCenter;
	vector3 nextRelativeCenter;

	for (int n = 0; n < a_nSubdivisionsB; n++)
	{
		relativeCenter = vector3(cos(angleDeltaB * n) * a_fInnerRadius, 0, sin(angleDeltaB * n) * a_fInnerRadius);
		nextRelativeCenter = vector3(cos(angleDeltaB * (n + 1)) * a_fInnerRadius, 0, sin(angleDeltaB * (n + 1)) * a_fInnerRadius);
		//std::cout << "Relative Center: " << relativeCenter.x << " " << relativeCenter.y << " " << relativeCenter.z << std::endl;
		//std::cout << "Next Center: " << nextRelativeCenter.x << " " << nextRelativeCenter.y << " " << nextRelativeCenter.z << std::endl;
		for (int i = 0; i < a_nSubdivisionsA; i++)
		{
			AddQuad(vector3(cos(angleDeltaA * i) * a_fOuterRadius + relativeCenter.x, sin(angleDeltaA * i) * a_fOuterRadius, relativeCenter.z), vector3(cos(angleDeltaA * (i + 1)) * a_fOuterRadius + relativeCenter.x, sin(angleDeltaA * (i + 1)) * a_fOuterRadius, relativeCenter.z),
				vector3(cos(angleDeltaA * i) * a_fOuterRadius + nextRelativeCenter.x, sin(angleDeltaA * i) * a_fOuterRadius, nextRelativeCenter.z), vector3(cos(angleDeltaA * (i + 1)) * a_fOuterRadius + nextRelativeCenter.x, sin(angleDeltaA * (i + 1)) * a_fOuterRadius, nextRelativeCenter.z));
		}

		for (int i = 0; i < a_nSubdivisionsA; i++)
		{
			AddQuad(vector3(cos(angleDeltaA * (i + 1)) * a_fOuterRadius + relativeCenter.x, sin(angleDeltaA * (i + 1)) * a_fOuterRadius, relativeCenter.z), vector3(cos(angleDeltaA * i) * a_fOuterRadius + relativeCenter.x, sin(angleDeltaA * i) * a_fOuterRadius, relativeCenter.z),
				vector3(cos(angleDeltaA * (i + 1)) * a_fOuterRadius + nextRelativeCenter.x, sin(angleDeltaA * (i + 1)) * a_fOuterRadius, nextRelativeCenter.z), vector3(cos(angleDeltaA * i) * a_fOuterRadius + nextRelativeCenter.x, sin(angleDeltaA * i) * a_fOuterRadius, nextRelativeCenter.z));
		}
	}



	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 6)
		a_nSubdivisions = 6;

	Release();
	Init();

	float angleDelta = PI / a_nSubdivisions;

	for (int i = 0; i <= a_nSubdivisions; i++)
	{
		for (int n = 0; n <= a_nSubdivisions; n++)
		{
			AddQuad(
				vector3(cos(angleDelta * n) * a_fRadius, sin(angleDelta * n) * a_fRadius, sin(angleDelta * (i + 1)) * a_fRadius),
				vector3(cos(angleDelta * n) * a_fRadius, sin(angleDelta * n) * a_fRadius, sin(angleDelta * i) * a_fRadius),
				vector3(cos(angleDelta * (n + 1)) * a_fRadius, sin(angleDelta * (n + 1)) * a_fRadius, sin(angleDelta * (i + 1)) * a_fRadius),
				vector3(cos(angleDelta * (n + 1)) * a_fRadius, sin(angleDelta * (n + 1)) * a_fRadius, sin(angleDelta * i) * a_fRadius)
			);
	
			std::cout << "I: " << i << " N: " << n << std::endl;
			std::cout << cos(angleDelta * n) * a_fRadius << " " << sin(angleDelta * n) * a_fRadius << " " << sin(angleDelta * (i + 1)) * a_fRadius << std::endl;
			std::cout << cos(angleDelta * n) * a_fRadius << " " << sin(angleDelta * n) * a_fRadius << " " << sin(angleDelta * i) * a_fRadius << std::endl;
			std::cout << cos(angleDelta * (n + 1)) * a_fRadius << " " << sin(angleDelta * (n + 1)) * a_fRadius << " " << sin(angleDelta * (i + 1)) * a_fRadius << std::endl;
			std::cout << cos(angleDelta * (n + 1)) * a_fRadius << " " << sin(angleDelta * (n + 1)) * a_fRadius << " " << sin(angleDelta * i) * a_fRadius << std::endl;
			std::cout << std::endl;
		}

		for (int n = 0; n <= a_nSubdivisions; n++)
		{
			AddQuad(
				vector3(cos(angleDelta * n) * a_fRadius, -sin(angleDelta * n) * a_fRadius, sin(angleDelta * (i + 1)) * a_fRadius),
				vector3(cos(angleDelta * n) * a_fRadius, -sin(angleDelta * n) * a_fRadius, sin(angleDelta * i) * a_fRadius),
				vector3(cos(angleDelta * (n + 1)) * a_fRadius, -sin(angleDelta * (n + 1)) * a_fRadius, sin(angleDelta * (i + 1)) * a_fRadius),
				vector3(cos(angleDelta * (n + 1)) * a_fRadius, -sin(angleDelta * (n + 1)) * a_fRadius, sin(angleDelta * i) * a_fRadius)
			);

			std::cout << "I: " << i << " N: " << n << std::endl;
			std::cout << cos(angleDelta * n) * a_fRadius << " " << -sin(angleDelta * n) * a_fRadius << " " << sin(angleDelta * (i + 1)) * a_fRadius << std::endl;
			std::cout << cos(angleDelta * n) * a_fRadius << " " << -sin(angleDelta * n) * a_fRadius << " " << sin(angleDelta * i) * a_fRadius << std::endl;
			std::cout << cos(angleDelta * (n + 1)) * a_fRadius << " " << -sin(angleDelta * (n + 1)) * a_fRadius << " " << sin(angleDelta * (i + 1)) * a_fRadius << std::endl;
			std::cout << cos(angleDelta * (n + 1)) * a_fRadius << " " << -sin(angleDelta * (n + 1)) * a_fRadius << " " << sin(angleDelta * i) * a_fRadius << std::endl;
			std::cout << std::endl;
		}
	
		
	}

	//for (int i = 0; i < a_nSubdivisions; i++)
	//{
	//	for (int n = -1 * (a_nSubdivisions / 2); n < a_nSubdivisions / 2; n++)
	//	{
	//		AddQuad(
	//			vector3(cos(angleDelta * n) * a_fRadius, sin(angleDelta * n) * a_fRadius, cos(angleDelta * (n + 1)) * sin(angleDelta * (i + 1)) * a_fRadius),
	//			vector3(cos(angleDelta * n) * a_fRadius, sin(angleDelta * n) * a_fRadius, sin(angleDelta * i) * a_fRadius),
	//			vector3(cos(angleDelta * (n + 1)) * a_fRadius, sin(angleDelta * (n + 1)) * a_fRadius, cos(angleDelta * (n + 1)) * sin(angleDelta * (i + 1)) * a_fRadius),
	//			vector3(cos(angleDelta * (n + 1)) * a_fRadius, sin(angleDelta * (n + 1)) * a_fRadius, sin(angleDelta * i) * a_fRadius)
	//		);
	//
	//		std::cout << "I: " << i << " N: " << n << std::endl;
	//		std::cout << cos(angleDelta * n) * a_fRadius << " " << sin(angleDelta * n) * a_fRadius << " " << sin(angleDelta * (i + 1)) * a_fRadius << std::endl;
	//		std::cout << cos(angleDelta * n) * a_fRadius << " " << sin(angleDelta * n) * a_fRadius << " " << sin(angleDelta * i) * a_fRadius << std::endl;
	//		std::cout << cos(angleDelta * (n + 1)) * a_fRadius << " " << sin(angleDelta * (n + 1)) * a_fRadius << " " << sin(angleDelta * (i + 1)) * a_fRadius << std::endl;
	//		std::cout << cos(angleDelta * (n + 1)) * a_fRadius << " " << sin(angleDelta * (n + 1)) * a_fRadius << " " << sin(angleDelta * i) * a_fRadius << std::endl;
	//		std::cout << std::endl;
	//	}
	//
	//	//for (int n = 0; n < a_nSubdivisions; n++)
	//	//{
	//	//	AddQuad(
	//	//		vector3(cos(angleDelta * n) * a_fRadius, -sin(angleDelta * n) * a_fRadius, sin(angleDelta * (n + 1)) * cos(angleDelta * (i + 1)) * a_fRadius),
	//	//		vector3(cos(angleDelta * n) * a_fRadius, -sin(angleDelta * n) * a_fRadius, sin(angleDelta * n) * cos(angleDelta * i) * a_fRadius),
	//	//		vector3(cos(angleDelta * (n + 1)) * a_fRadius, -sin(angleDelta * (n + 1)) * a_fRadius, sin(angleDelta * (n + 1)) * cos(angleDelta * (i + 1)) * a_fRadius),
	//	//		vector3(cos(angleDelta * (n + 1)) * a_fRadius, -sin(angleDelta * (n + 1)) * a_fRadius, sin(angleDelta * n) * cos(angleDelta * i) * a_fRadius)
	//	//	);
	//	//
	//	//	std::cout << "I: " << i << " N: " << n << std::endl;
	//	//	std::cout << cos(angleDelta * n) * a_fRadius << " " << sin(angleDelta * n) * a_fRadius << " " << sin(angleDelta * (i + 1)) * a_fRadius << std::endl;
	//	//	std::cout << cos(angleDelta * n) * a_fRadius << " " << sin(angleDelta * n) * a_fRadius << " " << sin(angleDelta * i) * a_fRadius << std::endl;
	//	//	std::cout << cos(angleDelta * (n + 1)) * a_fRadius << " " << sin(angleDelta * (n + 1)) * a_fRadius << " " << sin(angleDelta * (i + 1)) * a_fRadius << std::endl;
	//	//	std::cout << cos(angleDelta * (n + 1)) * a_fRadius << " " << sin(angleDelta * (n + 1)) * a_fRadius << " " << sin(angleDelta * i) * a_fRadius << std::endl;
	//	//	std::cout << std::endl;
	//	//}
	//}

	//float zDirection = 2 * PI / a_nSubdivisions;
	//float yDirection = PI / a_nSubdivisions;
	//float angleDeltaZ;
	//float angleDeltaZ2;
	//float angleDeltaY;
	//float angleDeltaY2;
	//float vertical;
	//float vertical2;
	//float horizontal;
	//float horizontal2;
	//
	//for (int i = 0; i < a_nSubdivisions; i++)
	//{
	//	angleDeltaY = PI / 2 - i * yDirection;
	//	angleDeltaY2 = PI / 2 - (i + 1) * yDirection;
	//	vertical = a_fRadius * cos(angleDeltaY);
	//	vertical2= a_fRadius * cos(angleDeltaY2);
	//	horizontal = a_fRadius * sin(angleDeltaY);
	//	horizontal2 = a_fRadius * sin(angleDeltaY2);
	//
	//	for (int n = 0; n < a_nSubdivisions; n++)
	//	{
	//		angleDeltaZ = n * zDirection;
	//		angleDeltaZ2 = (n + 1) * zDirection;
	//
	//		AddQuad(
	//			vector3(vertical * cos(angleDeltaZ2), vertical * sin(angleDeltaZ2), horizontal2),
	//			vector3(vertical * cos(angleDeltaZ), vertical * sin(angleDeltaZ), horizontal),
	//			vector3(vertical2 * cos(angleDeltaZ2), vertical2 * sin(angleDeltaZ2), horizontal2),
	//			vector3(vertical2 * cos(angleDeltaZ), vertical2 * sin(angleDeltaZ), horizontal)
	//		);
	//
	//		//AddQuad(
	//		//	vector3(vertical * cos(angleDeltaZ), vertical * sin(angleDeltaZ), horizontal),
	//		//	vector3(vertical * cos(angleDeltaZ2), vertical * sin(angleDeltaZ2), horizontal2),
	//		//	vector3(vertical2 * cos(angleDeltaZ), vertical2 * sin(angleDeltaZ), horizontal),
	//		//	vector3(vertical2 * cos(angleDeltaZ2), vertical2 * sin(angleDeltaZ2), horizontal2)
	//		//);
	//	}
	//}

	//float verticalAngleDelta = PI / a_nSubdivisions;
	//float horizontalAngleDelta = (2 * PI) / a_nSubdivisions;
	//
	//for (int i = 0; i < a_nSubdivisions; i++)
	//{
	//	for (int n = 0; n < a_nSubdivisions; n++)
	//	{
	//		AddQuad(
	//			vector3(a_fRadius * cos(verticalAngleDelta * (PI / 2) * (i + 1)), a_fRadius * sin(verticalAngleDelta * (PI / 2) * (i)), a_fRadius * sin(horizontalAngleDelta * PI * (n + 1))),
	//			vector3(a_fRadius * cos(verticalAngleDelta * (PI / 2) * (i)), a_fRadius * sin(verticalAngleDelta * (PI / 2) * (i)), a_fRadius * sin(horizontalAngleDelta * PI * (n))),
	//			vector3(a_fRadius * cos(verticalAngleDelta * (PI / 2) * (i + 1)), a_fRadius * sin(verticalAngleDelta * (PI / 2) * (i + 1)), a_fRadius * sin(horizontalAngleDelta * PI * (n + 1))),
	//			vector3(a_fRadius * cos(verticalAngleDelta * (PI / 2) * (i)), a_fRadius * sin(verticalAngleDelta * (PI / 2) * (i + 1)), a_fRadius * sin(horizontalAngleDelta * PI * (n)))
	//		);
	//	}
	//}

	//for (int i = 0; i < a_nSubdivisions; i++)
	//{
	//	for (int n = 0; n < a_nSubdivisions; n++)
	//	{
	//		AddQuad(
	//			vector3(sqrt(pow(a_fRadius, 2) - pow((a_fRadius - abs(angleDelta * (n + 1))), 2)) * cos(angleDelta * (i + 1)), a_fRadius - abs(angleDelta * i), sqrt(pow(a_fRadius, 2) - pow((a_fRadius - abs(angleDelta * (n + 1))), 2)) * sin(angleDelta * (i + 1))),
	//			vector3(sqrt(pow(a_fRadius, 2) - pow((a_fRadius - abs(angleDelta * n)), 2)) * cos(angleDelta * i), a_fRadius - abs(angleDelta * i), sqrt(pow(a_fRadius, 2) - pow((a_fRadius - abs(angleDelta * n)), 2)) * sin(angleDelta * i)),
	//			vector3(sqrt(pow(a_fRadius, 2) - pow((a_fRadius - abs(angleDelta * (n + 1))), 2)) * cos(angleDelta * (i + 1)), a_fRadius - abs(angleDelta * (i + 1)), sqrt(pow(a_fRadius, 2) - pow((a_fRadius - abs(angleDelta * (n + 1))), 2)) * sin(angleDelta * (i + 1))),
	//			vector3(sqrt(pow(a_fRadius, 2) - pow((a_fRadius - abs(angleDelta * n)), 2)) * cos(angleDelta * i), a_fRadius - abs(angleDelta * (i + 1)), sqrt(pow(a_fRadius, 2) - pow((a_fRadius - abs(angleDelta * n)), 2)) * sin(angleDelta * i))
	//		);
	//	}
	//}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}