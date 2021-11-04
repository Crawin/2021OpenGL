#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <gl/glew.h> // 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include "FileToBuf.h"
#include <random>
#include <math.h>

#define WinX 600
#define WinY 600

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Timer(int value);
GLvoid SpecialKey(int key, int x, int y);

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid InitBuffer();

double RED = 0.f, GREEN = 0.f, BLUE = 0.f;
GLuint shaderProgram, WindowID;

class structure {
public:
	GLuint VAO;
	GLuint VBO;
	GLuint Color;
	GLuint EBO;
	int vertexNum;
	int faceNum;
	float* vertexData;
	float* vertexColor;
	unsigned int* vertexFace;
	glm::mat4 trans{ 1.0f };
	float rotX{}, rotY{}, rotZ{}, transX{}, transY{}, transZ{}, scaleX{ 1.0f }, scaleY{ 1.0f }, scaleZ{ 1.0f }, theta{}, r{};

	structure(const char* FileName, float R, float G, float B) {
		VAO = VBO = EBO = vertexNum = faceNum = 0;
		rotZ = 45;
		//--- 1. 전체 버텍스 개수 및 삼각형 개수 세기
		FILE* objFile = fopen(FileName, "r");
		char count[100];
		while (!feof(objFile)) {
			fscanf(objFile, "%s", count);
			if (count[0] == 'v' && count[1] == '\0')
				vertexNum += 1;
			else if (count[0] == 'f' && count[1] == '\0')
				faceNum += 1;
			memset(count, '\0', sizeof(count)); // 배열 초기화
		}
		//--- 2. 메모리 할당
		vertexData = new float[vertexNum * 3];
		vertexFace = new unsigned int[faceNum * 3];
		vertexColor = new float[vertexNum * 3];
		int vertIndex = 0;
		int faceIndex = 0;
		fseek(objFile, 0, SEEK_SET);
		//--- 3. 할당된 메모리에 각 버텍스, 페이스 정보 입력

		while (!feof(objFile)) {
			fscanf(objFile, "%s", count);
			if (count[0] == 'v' && count[1] == '\0') {
				fscanf(objFile, "%f %f %f", &vertexData[vertIndex], &vertexData[vertIndex + 1], &vertexData[vertIndex + 2]);
				vertexColor[vertIndex++] = R;
				vertexColor[vertIndex++] = G;
				vertexColor[vertIndex++] = B;
			}
			else if (count[0] == 'f' && count[1] == '\0') {
				fscanf(objFile, "%d %d %d", &vertexFace[faceIndex], &vertexFace[faceIndex + 1], &vertexFace[faceIndex + 2]);
				--vertexFace[faceIndex++];
				--vertexFace[faceIndex++];
				--vertexFace[faceIndex++];
			}
			memset(count, '\0', sizeof(count)); // 배열 초기화
		}
		fclose(objFile);
	}

	~structure() {
		delete[] vertexData;
		delete[] vertexFace;
		delete[] vertexColor;
	}

	void Translate(float x, float y, float z) {
		unsigned int transformLocation = glGetUniformLocation(shaderProgram, "ModelTransform");
		trans = glm::translate(trans, glm::vec3(x, y, z));
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(trans));
	}
	void Rotate(float degree, float x, float y, float z) {
		unsigned int transformLocation = glGetUniformLocation(shaderProgram, "ModelTransform");
		trans = glm::rotate(trans, glm::radians(degree), glm::vec3(x, y, z));
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(trans));
	}
	void Scale(float x, float y, float z) {
		unsigned int transformLocation = glGetUniformLocation(shaderProgram, "ModelTransform");
		trans = glm::scale(trans, glm::vec3(x, y, z));
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(trans));
	}
	void transReset() {
		trans = glm::mat4(1.0f);
	}
};

GLuint Xvao, Xvbo, Yvao, Yvbo, Zvao, Zvbo;
structure bottom("box.obj", 0, 1, 1);
structure mid("box.obj", 1, 0, 0);
structure topG("Cylinder.obj", 0, 1, 0);
structure topB("Cylinder.obj", 0, 0, 1);
structure floorbox("box.obj", 0, 0, 0);
bool top, Y, T, M, B, S = true;

GLfloat Xvertex[]{
	1,0,0,		1,0,0,
	-1,0,0,		1,0,0
};

GLfloat Yvertex[]{
	0,1,0,		0,1,0,
	0,-1,0,		0,1,0
};

GLfloat Zvertex[]{
	0,0,1,		0,0,1,
	0,0,-1,		0,0,1
};

class Cam {
public:
	glm::vec3 at;
	float camX, camY, camZ, camRot, atRot;
	glm::vec3 camPos;
	glm::vec3 camDir;
	glm::vec3 camUp;

	Cam() {
		camX = 1.0f, camY = 0.5f, camZ = 0.0f;
		atRot = -1;
		camPos = glm::vec3(camX, camY, camZ);							// 카메라 위치
		camDir = glm::normalize(camPos - at);							// at -> cam 방향벡터
		camUp = glm::cross(camDir, glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), camDir)));		// (고정식) 캠의 세로 평면과 수직을 이루는값을 노멀라이즈한 값을 캠의 방향벡터로 up구하기

	}

	Cam(float X, float Y, float Z, glm::vec3 UP) {
		camX = X, camY =Y, camZ = Z;
		atRot = -1;
		camPos = glm::vec3(camX, camY, camZ);							// 카메라 위치
		camDir = glm::normalize(camPos - at);							// at -> cam 방향벡터
		camUp = UP;

	}

	void setcam() {
		camPos = glm::vec3(camX, camY, camZ);							// 카메라 위치
		at.z = camZ;
		camDir = glm::normalize(camPos - at);		// 카메라가 보는 방향벡터
		camUp = glm::cross(camDir, glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), camDir)));		// (고정식) 캠의 세로 평면과 수직을 이루는값을 노멀라이즈한 값을 캠의 방향벡터로 up구하기
	}
};

Cam c;
Cam t(0, 1, 0,glm::vec3(-1,0,0));
Cam f(1, 0, 0,glm::vec3(0,1,0));
void main(int argc, char** argv)								//--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);										// glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);				// 디스플레이 모드 설정
	glutInitWindowPosition(0, 0);								// 윈도우의 위치 지정
	glutInitWindowSize(WinX, WinY);								// 윈도우의 크기 지정
	WindowID = glutCreateWindow("19");								// 윈도우 생성 (윈도우 이름)

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)									// glew 초기화
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";
	InitBuffer();
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	shaderProgram = make_shaderProgram();
	glutDisplayFunc(drawScene);									// 출력 콜백함수의 지정
	glutReshapeFunc(Reshape);									// 다시 그리기 콜백함수 지정
	glutKeyboardFunc(Keyboard);									// 키보드 입력 콜백함수 지정
	glutSpecialFunc(SpecialKey);
	glutTimerFunc(10, Timer, 0);
	glutMainLoop();												// 이벤트 처리 시작
}

GLvoid drawScene()												//--- 콜백 함수: 그리기 콜백 함수
{
	glUseProgram(shaderProgram);
	//--- 변경된 배경색 설정
	glClearColor(RED, GREEN, BLUE, 1.0f);											// 바탕색을 변경
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// 설정된 색으로 전체를 칠하기

	glViewport(0, WinY / 4, WinX / 2, WinY / 2);
	glEnable(GL_SCISSOR_TEST);
	glScissor(0, WinY / 4, WinX / 2, WinY / 2);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 Model(1.0f);
	int ModelLoc = glGetUniformLocation(shaderProgram, "ModelTransform");
	glUniformMatrix4fv(ModelLoc, 1, GL_FALSE, glm::value_ptr(Model));

	glm::mat4 View(1.0f);
	View = glm::lookAt(c.camPos, c.at, c.camUp);
	View = glm::rotate(View, glm::radians(c.camRot), glm::vec3(0, 1, 0));			// Y 축 기준으로 돌리기 공전
	int ViewLoc = glGetUniformLocation(shaderProgram, "ViewTransform");
	glUniformMatrix4fv(ViewLoc, 1, GL_FALSE, glm::value_ptr(View));

	glm::mat4 Proj(1.0f);
	int ProjLoc = glGetUniformLocation(shaderProgram, "ProjectionTransform");
	Proj = glm::perspective(glm::radians(45.0f), (float)WinX / (float)WinY, 1.5f, 5.0f);
	Proj = glm::translate(Proj, glm::vec3(0.0, 0.0, -2.0));
	glUniformMatrix4fv(ProjLoc, 1, GL_FALSE, glm::value_ptr(Proj));

	glLineWidth(2);
	glBindVertexArray(Xvao);
	glDrawArrays(GL_LINES, 0, 2);

	glBindVertexArray(Yvao);
	glDrawArrays(GL_LINES, 0, 2);

	glBindVertexArray(Zvao);
	glDrawArrays(GL_LINES, 0, 2);

	glLineWidth(1);
	floorbox.Translate(0, -0.2f, 0);
	floorbox.Scale(10.0f, 1.0f, 10.0f);
	glBindVertexArray(floorbox.VAO);
	glDrawElements(GL_TRIANGLES, floorbox.faceNum * 3, GL_UNSIGNED_INT, 0);
	floorbox.transReset();

	bottom.Translate(0, 0, bottom.transZ);
	bottom.Scale(2, 0.5, 2);
	glBindVertexArray(bottom.VAO);
	glDrawElements(GL_TRIANGLES, bottom.faceNum * 3, GL_UNSIGNED_INT, 0);
	bottom.transReset();

	mid.Translate(0, 0, bottom.transZ);
	mid.Translate(0, 0.1, 0);
	mid.Rotate(mid.rotY, 0, 1, 0);
	glBindVertexArray(mid.VAO);
	glDrawElements(GL_TRIANGLES, mid.faceNum * 3, GL_UNSIGNED_INT, 0);
	mid.transReset();

	topG.Translate(0, 0, bottom.transZ);
	topG.Rotate(mid.rotY, 0, 1, 0);
	topG.Translate(0, 0.3, 0.05);
	topG.Rotate(-topB.rotZ, 0, 0, 1);
	topG.Scale(1, 4, 1);
	glBindVertexArray(topG.VAO);
	glDrawElements(GL_TRIANGLES, topG.faceNum * 3, GL_UNSIGNED_INT, 0);
	topG.transReset();

	topB.Translate(0, 0, bottom.transZ);
	topB.Rotate(mid.rotY, 0, 1, 0);
	topB.Translate(0, 0.3, -0.05);
	topB.Rotate(topB.rotZ, 0, 0, 1);
	topB.Scale(1, 4, 1);
	glBindVertexArray(topB.VAO);
	glDrawElements(GL_TRIANGLES, topB.faceNum * 3, GL_UNSIGNED_INT, 0);
	topB.transReset();
	//-------------------------------------------------------------------------------------- default
	glViewport(WinX / 2, WinY / 2, WinX / 2, WinY / 2);
	glScissor(WinX / 2, WinY / 2, WinX / 2, WinY / 2);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	Model = glm::mat4(1.0f);
	glUniformMatrix4fv(ModelLoc, 1, GL_FALSE, glm::value_ptr(Model));

	View = glm::mat4(1.0f);
	View = glm::lookAt(t.camPos, t.at, t.camUp);
	View = glm::rotate(View, glm::radians(c.camRot), glm::vec3(0, 1, 0));			// Y 축 기준으로 돌리기 공전
	glUniformMatrix4fv(ViewLoc, 1, GL_FALSE, glm::value_ptr(View));

	Proj= glm::mat4(1.0f);
	Proj = glm::ortho(-1.5f, 1.5f, -1.5f, 1.5f, 0.f, 10.f);
	Proj = glm::translate(Proj, glm::vec3(0.0, 0.0, -2.0));
	glUniformMatrix4fv(ProjLoc, 1, GL_FALSE, glm::value_ptr(Proj));

	glLineWidth(2);
	glBindVertexArray(Xvao);
	glDrawArrays(GL_LINES, 0, 2);

	glBindVertexArray(Yvao);
	glDrawArrays(GL_LINES, 0, 2);

	glBindVertexArray(Zvao);
	glDrawArrays(GL_LINES, 0, 2);

	glLineWidth(1);
	floorbox.Translate(0, -0.2f, 0);
	floorbox.Scale(10.0f, 1.0f, 10.0f);
	glBindVertexArray(floorbox.VAO);
	glDrawElements(GL_TRIANGLES, floorbox.faceNum * 3, GL_UNSIGNED_INT, 0);
	floorbox.transReset();

	bottom.Translate(0, 0, bottom.transZ);
	bottom.Scale(2, 0.5, 2);
	glBindVertexArray(bottom.VAO);
	glDrawElements(GL_TRIANGLES, bottom.faceNum * 3, GL_UNSIGNED_INT, 0);
	bottom.transReset();

	mid.Translate(0, 0, bottom.transZ);
	mid.Translate(0, 0.1, 0);
	mid.Rotate(mid.rotY, 0, 1, 0);
	glBindVertexArray(mid.VAO);
	glDrawElements(GL_TRIANGLES, mid.faceNum * 3, GL_UNSIGNED_INT, 0);
	mid.transReset();

	topG.Translate(0, 0, bottom.transZ);
	topG.Rotate(mid.rotY, 0, 1, 0);
	topG.Translate(0, 0.3, 0.05);
	topG.Rotate(-topB.rotZ, 0, 0, 1);
	topG.Scale(1, 4, 1);
	glBindVertexArray(topG.VAO);
	glDrawElements(GL_TRIANGLES, topG.faceNum * 3, GL_UNSIGNED_INT, 0);
	topG.transReset();

	topB.Translate(0, 0, bottom.transZ);
	topB.Rotate(mid.rotY, 0, 1, 0);
	topB.Translate(0, 0.3, -0.05);
	topB.Rotate(topB.rotZ, 0, 0, 1);
	topB.Scale(1, 4, 1);
	glBindVertexArray(topB.VAO);
	glDrawElements(GL_TRIANGLES, topB.faceNum * 3, GL_UNSIGNED_INT, 0);
	topB.transReset();
	//-------------------------------------------------------------------------------------- TOP
	glViewport(WinX / 2, 0, WinX / 2, WinY / 2);
	glScissor(WinX / 2, 0, WinX / 2, WinY / 2);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
	Model = glm::mat4(1.0f);
	glUniformMatrix4fv(ModelLoc, 1, GL_FALSE, glm::value_ptr(Model));

	View = glm::mat4(1.0f);
	View = glm::lookAt(f.camPos, f.at, f.camUp);
	View = glm::rotate(View, glm::radians(c.camRot), glm::vec3(0, 1, 0));			// Y 축 기준으로 돌리기 공전
	glUniformMatrix4fv(ViewLoc, 1, GL_FALSE, glm::value_ptr(View));

	Proj = glm::mat4(1.0f);
	Proj = glm::ortho(-1.5f, 1.5f, -1.5f, 1.5f, 0.f, 10.f);
	Proj = glm::translate(Proj, glm::vec3(0.0, 0.0, -2.0));
	glUniformMatrix4fv(ProjLoc, 1, GL_FALSE, glm::value_ptr(Proj));

	glLineWidth(2);
	glBindVertexArray(Xvao);
	glDrawArrays(GL_LINES, 0, 2);

	glBindVertexArray(Yvao);
	glDrawArrays(GL_LINES, 0, 2);

	glBindVertexArray(Zvao);
	glDrawArrays(GL_LINES, 0, 2);

	glLineWidth(1);
	floorbox.Translate(0, -0.2f, 0);
	floorbox.Scale(10.0f, 1.0f, 10.0f);
	glBindVertexArray(floorbox.VAO);
	glDrawElements(GL_TRIANGLES, floorbox.faceNum * 3, GL_UNSIGNED_INT, 0);
	floorbox.transReset();

	bottom.Translate(0, 0, bottom.transZ);
	bottom.Scale(2, 0.5, 2);
	glBindVertexArray(bottom.VAO);
	glDrawElements(GL_TRIANGLES, bottom.faceNum * 3, GL_UNSIGNED_INT, 0);
	bottom.transReset();

	mid.Translate(0, 0, bottom.transZ);
	mid.Translate(0, 0.1, 0);
	mid.Rotate(mid.rotY, 0, 1, 0);
	glBindVertexArray(mid.VAO);
	glDrawElements(GL_TRIANGLES, mid.faceNum * 3, GL_UNSIGNED_INT, 0);
	mid.transReset();

	topG.Translate(0, 0, bottom.transZ);
	topG.Rotate(mid.rotY, 0, 1, 0);
	topG.Translate(0, 0.3, 0.05);
	topG.Rotate(-topB.rotZ, 0, 0, 1);
	topG.Scale(1, 4, 1);
	glBindVertexArray(topG.VAO);
	glDrawElements(GL_TRIANGLES, topG.faceNum * 3, GL_UNSIGNED_INT, 0);
	topG.transReset();

	topB.Translate(0, 0, bottom.transZ);
	topB.Rotate(mid.rotY, 0, 1, 0);
	topB.Translate(0, 0.3, -0.05);
	topB.Rotate(topB.rotZ, 0, 0, 1);
	topB.Scale(1, 4, 1);
	glBindVertexArray(topB.VAO);
	glDrawElements(GL_TRIANGLES, topB.faceNum * 3, GL_UNSIGNED_INT, 0);
	topB.transReset();
	//-------------------------------------------------------------------------------------- Front

	glutSwapBuffers();											// 화면에 출력하기
}
GLvoid Reshape(int w, int h)									//--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}
GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'z':
		c.camZ += 0.05;
		f.camPos.z += 0.05;
		f.at.z += 0.05;
		t.camPos.z += 0.05;
		t.at.z += 0.05;
		c.setcam();
		break;
	case 'Z':
		c.camZ -= 0.05;
		f.camPos.z -= 0.05;
		f.at.z -= 0.05;
		t.camPos.z -= 0.05;
		t.at.z -= 0.05;
		c.setcam();
		break;
	case 'x':
		c.camX += 0.05;
		c.at.x += 0.05;
		f.camPos.x += 0.05;
		f.at.x += 0.05;
		t.camPos.x += 0.05;
		t.at.x += 0.05;
		c.setcam();
		break;
	case 'X':
		c.camX -= 0.05;
		c.at.x -= 0.05;
		f.camPos.x -= 0.05;
		f.at.x -= 0.05;
		t.camPos.x -= 0.05;
		t.at.x -= 0.05;
		c.setcam();
		break;
	case 'r':
		c.camRot -= 1;
		break;
	case 'R':
		c.camRot += 1;
		break;
	case 's':
	case 'S':
		S = false;
		break;
	case 'c':
	case 'C':
		c.camX = 1.0f;
		c.camY = 0.5f;
		c.camZ = 0.0f;
		c.setcam();
		c.at = glm::vec3(0, 0, 0);
		topB.rotZ = 0;
		mid.rotY = 0;
		bottom.transZ = 0;
		c.camRot = 0;
		T = false;
		S = true;
		Y = false;
		top = false;
		M = false;
		B = false;
		break;
	case 'q':
	case 'Q':
		glutDestroyWindow(WindowID);
		break;
	case 'y':
		Y = true;
	case 'a':
	case 'A':
	case 'Y':
	case 'b':
	case 'B':
	case 'm':
	case 'M':
	case 't':
		T = true;
		S = true;
		M = true;
		B = true;
		glutTimerFunc(16, Timer, key);
		break;
	case 'T':
		T = false;
		break;
	}
	glutPostRedisplay();
}

GLvoid SpecialKey(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		c.camY += 0.05;
		c.setcam();
		break;
	case GLUT_KEY_DOWN:
		c.camY -= 0.05;
		c.setcam();
		break;
	}
	glutPostRedisplay();
}

GLvoid Timer(int value) {
	if (S) {
		switch (value) {
		case 't':
			if (T) {
				if (top) {
					topB.rotZ += 1;
				}
				else {
					topB.rotZ -= 1;
				}

				if (topB.rotZ >= 90 || topB.rotZ <= -90) {
					top = !top;
				}
				glutTimerFunc(16, Timer, 't');
			}
			break;
		case 'm':
			if (M) {
				mid.rotY += 1;
				glutTimerFunc(16, Timer, 'm');
			}
			break;
		case 'M':
			if (M) {
				mid.rotY -= 1;
				glutTimerFunc(16, Timer, 'M');
			}
			break;
		case 'b':
			if (B) {
				if (bottom.transZ <= 1.0f) {
					bottom.transZ += 0.005;
					glutTimerFunc(16, Timer, 'b');
				}
			}
			break;
		case 'B':
			if (B) {
				if (bottom.transZ >= -1.0f) {
					bottom.transZ -= 0.005;
					glutTimerFunc(16, Timer, 'B');
				}
			}
			break;
		case 'a':
			c.camRot += 1;
			glutTimerFunc(16, Timer, 'a');
			break;
		case 'A':
			c.camRot -= 1;
			glutTimerFunc(16, Timer, 'A');
			break;
		case 'y':
			if (Y) {
				glm::mat4 trans{ 1.0f };
				glm::vec4 temp{ c.at,1.0 };
				trans = glm::translate(trans, c.camPos);
				trans = glm::rotate(trans, glm::radians(c.atRot), glm::normalize(c.camUp));
				trans = glm::translate(trans, -c.camPos);
				c.at = trans * temp;
				glutTimerFunc(16, Timer, 'y');
			}
			break;
		case 'Y':
			Y = false;
			//c.at = glm::vec3(0, 0, c.camZ);
			break;
		}
	}
	glutPostRedisplay();
}

// 버텍스 셰이더 코드 - 위치
GLuint vertexShader;
void make_vertexShaders() {
	std::string buf = filetobuf("vertex.glsl");
	const GLchar* vertexSource;
	vertexSource = buf.c_str();
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	GLint result;
	GLchar errorlog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderInfoLog(vertexShader, 512, NULL, errorlog);
		std::cerr << "EROOR: vertex shader error\n" << errorlog << std::endl;
		return;
	}
}

//프래그먼트 셰이더 - 색상
GLuint fragmentShader;
void make_fragmentShaders() {
	std::string buf = filetobuf("fragment.glsl");
	const GLchar* fragmentSource;
	fragmentSource = buf.c_str();
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	GLint result;
	GLchar errorlog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorlog);
		std::cerr << "ERROR: fragment shader error\n" << errorlog << std::endl;
		return;
	}
}
GLuint make_shaderProgram() {

	make_vertexShaders();
	make_fragmentShaders();

	GLuint ShaderProgramID = glCreateProgram();

	glAttachShader(ShaderProgramID, vertexShader);
	glAttachShader(ShaderProgramID, fragmentShader);

	glLinkProgram(ShaderProgramID);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	GLint result;
	GLchar errorlog[512];
	glGetProgramiv(ShaderProgramID, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(ShaderProgramID, 512, NULL, errorlog);
		std::cerr << "ERROR: shader program 연결 실패\n" << errorlog << std::endl;
		return false;
	}

	glUseProgram(ShaderProgramID);

	return ShaderProgramID;
}

GLvoid InitBuffer() {
	glGenVertexArrays(1, &Xvao);
	glBindVertexArray(Xvao);

	glGenBuffers(1, &Xvbo);
	glBindBuffer(GL_ARRAY_BUFFER, Xvbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Xvertex), Xvertex, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &Yvao);
	glBindVertexArray(Yvao);

	glGenBuffers(1, &Yvbo);
	glBindBuffer(GL_ARRAY_BUFFER, Yvbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Yvertex), Yvertex, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &Zvao);
	glBindVertexArray(Zvao);

	glGenBuffers(1, &Zvbo);
	glBindBuffer(GL_ARRAY_BUFFER, Zvbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Zvertex), Zvertex, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &bottom.VAO);
	glBindVertexArray(bottom.VAO);

	glGenBuffers(1, &bottom.Color);
	glBindBuffer(GL_ARRAY_BUFFER, bottom.Color);
	glBufferData(GL_ARRAY_BUFFER, bottom.vertexNum * 3 * sizeof(float), bottom.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &bottom.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, bottom.VBO);
	glBufferData(GL_ARRAY_BUFFER, bottom.vertexNum * 3 * sizeof(float), bottom.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &bottom.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bottom.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, bottom.faceNum * 3 * sizeof(unsigned int), bottom.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//------------------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &mid.VAO);
	glBindVertexArray(mid.VAO);

	glGenBuffers(1, &mid.Color);
	glBindBuffer(GL_ARRAY_BUFFER, mid.Color);
	glBufferData(GL_ARRAY_BUFFER, mid.vertexNum * 3 * sizeof(float), mid.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &mid.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, mid.VBO);
	glBufferData(GL_ARRAY_BUFFER, mid.vertexNum * 3 * sizeof(float), mid.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &mid.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mid.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mid.faceNum * 3 * sizeof(unsigned int), mid.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//----------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &topG.VAO);
	glBindVertexArray(topG.VAO);

	glGenBuffers(1, &topG.Color);
	glBindBuffer(GL_ARRAY_BUFFER, topG.Color);
	glBufferData(GL_ARRAY_BUFFER, topG.vertexNum * 3 * sizeof(float), topG.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &topG.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, topG.VBO);
	glBufferData(GL_ARRAY_BUFFER, topG.vertexNum * 3 * sizeof(float), topG.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &topG.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, topG.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, topG.faceNum * 3 * sizeof(unsigned int), topG.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//------------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &topB.VAO);
	glBindVertexArray(topB.VAO);

	glGenBuffers(1, &topB.Color);
	glBindBuffer(GL_ARRAY_BUFFER, topB.Color);
	glBufferData(GL_ARRAY_BUFFER, topB.vertexNum * 3 * sizeof(float), topB.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &topB.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, topB.VBO);
	glBufferData(GL_ARRAY_BUFFER, topB.vertexNum * 3 * sizeof(float), topB.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &topB.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, topB.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, topB.faceNum * 3 * sizeof(unsigned int), topB.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//------------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &floorbox.VAO);
	glBindVertexArray(floorbox.VAO);

	glGenBuffers(1, &floorbox.Color);
	glBindBuffer(GL_ARRAY_BUFFER, floorbox.Color);
	glBufferData(GL_ARRAY_BUFFER, floorbox.vertexNum * 3 * sizeof(float), floorbox.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &floorbox.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, floorbox.VBO);
	glBufferData(GL_ARRAY_BUFFER, floorbox.vertexNum * 3 * sizeof(float), floorbox.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &floorbox.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorbox.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, floorbox.faceNum * 3 * sizeof(unsigned int), floorbox.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
}