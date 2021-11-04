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
#define objectcnt 20
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Timer(int value);
GLvoid SpecialKey(int key, int x, int y);

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid InitBuffer();

std::random_device rd;
std::default_random_engine dre(rd());
std::uniform_real_distribution <> urd(1, 10);
std::uniform_real_distribution <> randomtrans(-1, 1);

double RED = 1.f, GREEN = 1.f, BLUE = 1.f;
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
	bool clashX, clashZ;
	structure(const char* FileName, float R, float G, float B) {
		VAO = VBO = EBO = vertexNum = faceNum = 0;
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

	structure() {
		VAO = VBO = EBO = vertexNum = faceNum = 0;
		float R = 0.5;
		float G = 1.0;
		float B = 0.8;
		for(int i = 0; i < 1; ++i) {
			scaleX = urd(dre) / 5;		// 0.2 ~ 2 배 까지
			scaleY = urd(dre) / 10;		// 0.1 ~ 1 배 까지
			scaleZ = urd(dre) / 5;		// 0.2 ~ 2 배 까지
			transX = randomtrans(dre);
			transZ = randomtrans(dre);
			if (transX >= -0.15 && transX <= 0.15 && transZ >= -0.15 && transZ <= 0.15) {
				i--;
				continue;
			}
			if (transX + 0.1 * scaleX > -0.15 && transX + 0.1 * scaleX < 0.15) {		// 좌측이 지우개와 겹쳐있을때
				if (transZ >= 0) {
					if (transZ - 0.1 * scaleZ < 0.15) {									// 좌측이 지우개와 겹쳐있을때 하단이 지우개와 겹치면
						i--;
						continue;
					}
				}
				else {
					if (transZ + 0.1 * scaleZ > -0.15) {								// 좌측이 지우개와 겹쳐있을때 상단이 지우개와 겹치면
						i--;
						continue;
					}
				}
			}
			else {																		// 우측이 지우개와 겹쳐있을때
				if (transZ >= 0) {
					if (transZ - 0.1 * scaleZ < 0.15) {									// 우측이 지우개와 겹쳐있을때 하단이 지우개와 겹치면
						i--;
						continue;
					}
				}
				else {
					if (transZ + 0.1 * scaleZ > -0.15) {								// 우측이 지우개와 겹쳐있을때 상단이 지우개와 겹치면
						i--;
						continue;
					}
				}
			}
		}
		//--- 1. 전체 버텍스 개수 및 삼각형 개수 세기
		FILE* objFile = fopen("box.obj", "r");
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
	void setRandom() {
		for (int i = 0; i < 1; ++i) {
			scaleX = urd(dre) / 5;		// 0.2 ~ 2 배 까지
			scaleY = urd(dre) / 10;		// 0.1 ~ 1 배 까지
			scaleZ = urd(dre) / 5;		// 0.2 ~ 2 배 까지
			transX = randomtrans(dre);
			transZ = randomtrans(dre);
			if (transX >= -0.15 && transX <= 0.15 && transZ >= -0.15 && transZ <= 0.15) {
				i--;
				continue;
			}
			if (transX + 0.1 * scaleX > -0.15 && transX + 0.1 * scaleX < 0.15) {		// 좌측이 지우개와 겹쳐있을때
				if (transZ >= 0) {
					if (transZ - 0.1 * scaleZ < 0.15) {									// 좌측이 지우개와 겹쳐있을때 하단이 지우개와 겹치면
						i--;
						continue;
					}
				}
				else {
					if (transZ + 0.1 * scaleZ > -0.15) {								// 좌측이 지우개와 겹쳐있을때 상단이 지우개와 겹치면
						i--;
						continue;
					}
				}
			}
			else {																		// 우측이 지우개와 겹쳐있을때
				if (transZ >= 0) {
					if (transZ - 0.1 * scaleZ < 0.15) {									// 우측이 지우개와 겹쳐있을때 하단이 지우개와 겹치면
						i--;
						continue;
					}
				}
				else {
					if (transZ + 0.1 * scaleZ > -0.15) {								// 우측이 지우개와 겹쳐있을때 상단이 지우개와 겹치면
						i--;
						continue;
					}
				}
			}
		}
	}
};

class Cam {
public:
	float camRot, atRot;
	glm::vec3 camPos;
	glm::vec3 camDir;
	glm::vec3 camUp;
	glm::vec3 camAt;
	glm::vec3 startPos;

	Cam() {
		atRot = 0;
		camPos = glm::vec3(1.0f, 0.8f, 0.0f);							// 카메라 위치
		startPos = camPos;
		camDir = glm::normalize(camPos - camAt);							// at -> cam 방향벡터
		camUp = glm::cross(camDir, glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), camDir)));		// (고정식) 캠의 세로 평면과 수직을 이루는값을 노멀라이즈한 값을 캠의 방향벡터로 up구하기

	}

	Cam(float X, float Y, float Z, glm::vec3 UP) {
		atRot = 0;
		camPos = glm::vec3(X, Y, Z);									// 카메라 위치
		startPos = camPos;
		camDir = glm::normalize(camPos - camAt);							// at -> cam 방향벡터
		camUp = UP;
	}

	void setAT(float X, float Y, float Z) {
		camAt.x = X;
		camAt.y = Y;
		camAt.z = Z;
		camDir = glm::normalize(camPos - camAt);
	}

	void setPos(float X, float Y, float Z) {
		camPos.x = X;
		camPos.y = Y;
		camPos.z = Z;
		camDir = glm::normalize(camPos - camAt);
	}

	void camreset() {
		camPos = startPos;
		camAt = glm::vec3(0, 0, 0);
		atRot = 0;
		camRot = 0;
	}
};

GLuint Xvao, Xvbo, Yvao, Yvbo, Zvao, Zvbo;
structure floorbox("box.obj", 0, 0, 0);
structure objects[objectcnt];
structure eraserbottom("box.obj", 0.2, 0.2, 1);
structure erasertop("cylinder.obj", 1, 0, 0);
Cam c;
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


void main(int argc, char** argv)								//--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);										// glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);				// 디스플레이 모드 설정
	glutInitWindowPosition(0, 0);								// 윈도우의 위치 지정
	glutInitWindowSize(WinX, WinY);								// 윈도우의 크기 지정
	WindowID = glutCreateWindow("21");								// 윈도우 생성 (윈도우 이름)

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
	glutTimerFunc(10, Timer, 'c');
	std::cout << "I : 카메라 앞으로 이동\nJ : 카메라 왼쪽으로 이동\nK : 카메라 뒤쪽으로 이동\nL : 카메라 오른쪽으로 이동\n<- : Y축 기준 반시계 회전\n-> : Y축 기준 시계 회전" << std::endl;
	glutMainLoop();												// 이벤트 처리 시작
}

GLvoid drawScene()												//--- 콜백 함수: 그리기 콜백 함수
{
	glUseProgram(shaderProgram);
	//--- 변경된 배경색 설정
	glClearColor(RED, GREEN, BLUE, 1.0f);											// 바탕색을 변경
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// 설정된 색으로 전체를 칠하기

	glm::mat4 Model(1.0f);
	int ModelLoc = glGetUniformLocation(shaderProgram, "ModelTransform");
	glUniformMatrix4fv(ModelLoc, 1, GL_FALSE, glm::value_ptr(Model));

	glm::mat4 View(1.0f);
	View = glm::lookAt(c.camPos, c.camAt, c.camUp);
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
	floorbox.Translate(0, -0.02f, 0);
	floorbox.Scale(10.0f, 0.1f, 10.0f);
	glBindVertexArray(floorbox.VAO);
	glDrawElements(GL_TRIANGLES, floorbox.faceNum * 3, GL_UNSIGNED_INT, 0);
	floorbox.transReset();
	
	eraserbottom.scaleX = 1.5;
	eraserbottom.scaleY = 0.25;
	eraserbottom.scaleZ = 1.5;

	eraserbottom.Translate(eraserbottom.transX, 0, eraserbottom.transZ);
	eraserbottom.Scale(eraserbottom.scaleX, eraserbottom.scaleY, eraserbottom.scaleZ);
	glBindVertexArray(eraserbottom.VAO);
	glDrawElements(GL_TRIANGLES, eraserbottom.faceNum * 3, GL_UNSIGNED_INT, 0);
	eraserbottom.transReset();

	erasertop.Translate(erasertop.transX, 0, erasertop.transZ);
	erasertop.Translate(0, 0.05, 0);
	erasertop.Scale(1, 6, 1);
	glBindVertexArray(erasertop.VAO);
	glDrawElements(GL_TRIANGLES, erasertop.faceNum * 3, GL_UNSIGNED_INT, 0);
	erasertop.transReset();

	for (int i = 0; i < objectcnt; ++i) {
		objects[i].Translate(objects[i].transX, 0, objects[i].transZ);
		objects[i].Scale(objects[i].scaleX, objects[i].scaleY, objects[i].scaleZ);
		glBindVertexArray(objects[i].VAO);
		glDrawElements(GL_TRIANGLES, objects[i].faceNum * 3, GL_UNSIGNED_INT, 0);
		objects[i].transReset();
	}

	glutSwapBuffers();											// 화면에 출력하기
}
GLvoid Reshape(int w, int h)									//--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}
GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'w':
	case 'W':
		if (eraserbottom.transX > -1) {
			eraserbottom.transX -= 0.05;
			erasertop.transX -= 0.05;
		}
		break;
	case 'a':
	case 'A':
		if (eraserbottom.transZ < 1) {
			eraserbottom.transZ += 0.05;
			erasertop.transZ += 0.05;
		}
		break;
	case 's':
	case 'S':
		if (eraserbottom.transX < 1) {
			eraserbottom.transX += 0.05;
			erasertop.transX += 0.05;
		}
		break;
	case 'd':
	case 'D':
		if (eraserbottom.transZ > -1) {
			eraserbottom.transZ -= 0.05;
			erasertop.transZ -= 0.05;
		}
		break;
	case 'f':
	case 'F':
		break;
	case 'i':
	case 'I':
		c.camPos.x -= 0.1;
		c.camAt.x -= 0.1;
		break;
	case 'j':
	case 'J':
		c.camPos.z += 0.1;
		c.camAt.z += 0.1;
		break;
	case 'k':
	case 'K':
		c.camPos.x += 0.1;
		c.camAt.x += 0.1;
		break;
	case 'l':
	case 'L':
		c.camPos.z -= 0.1;
		c.camAt.z -= 0.1;
		break;
	case 'c':
	case 'C':
		for (int i = 0; i < objectcnt; ++i) {
			objects[i].setRandom();
		}
		c.camreset();
		eraserbottom.transX = 0;
		eraserbottom.transY = 0;
		eraserbottom.transZ = 0;
		erasertop.transX = 0;
		erasertop.transY = 0;
		erasertop.transZ = 0;
		break;
	}
	glutPostRedisplay();
}

GLvoid SpecialKey(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_LEFT:
		c.camRot -= 4;
		break;
	case GLUT_KEY_RIGHT:
		c.camRot += 4;
		break;
	}
	glutPostRedisplay();
}

GLvoid Timer(int value) {
	switch (value) {
	case 'c':								// 충돌 체크
		for(int i = 0; i < objectcnt; ++i){
			if (objects[i].transX >= eraserbottom.transX - 0.1 * eraserbottom.scaleX && objects[i].transX <= eraserbottom.transX + 0.1 * eraserbottom.scaleX && objects[i].transZ >= eraserbottom.transZ - 0.1 * eraserbottom.scaleZ && objects[i].transZ <= eraserbottom.transZ + 0.1 * eraserbottom.scaleZ) {
				glutTimerFunc(100, Timer, i);
			}
			if (objects[i].transX + 0.1 * objects[i].scaleX > eraserbottom.transX - 0.1 * eraserbottom.scaleX && objects[i].transX + 0.1 * objects[i].scaleX < eraserbottom.transX + 0.1 * eraserbottom.scaleX) {	// 좌측이 지우개와 겹쳐있을때
				if (objects[i].scaleZ >= eraserbottom.scaleZ) {
					if ((eraserbottom.transZ + 0.1 * eraserbottom.scaleZ >= objects[i].transZ - 0.1 * objects[i].scaleZ && eraserbottom.transZ + 0.1 * eraserbottom.scaleZ <= objects[i].transZ + 0.1 * objects[i].scaleZ) || (eraserbottom.transZ - 0.1 * eraserbottom.scaleZ >= objects[i].transZ - 0.1 * objects[i].scaleZ && eraserbottom.transZ - 0.1 * eraserbottom.scaleZ <= objects[i].transZ + 0.1 * objects[i].scaleZ)) {
						glutTimerFunc(100, Timer, i);
					}
				}
				else {
					if (objects[i].transZ >= eraserbottom.transZ) {
						if (objects[i].transZ - 0.1 * objects[i].scaleZ <= eraserbottom.transZ + 0.1 * eraserbottom.scaleZ && objects[i].transZ - 0.1 * objects[i].scaleZ >= eraserbottom.transZ - 0.1 * eraserbottom.scaleZ) {								// 우측이 지우개와 겹쳐있을때 하단이 지우개와 겹치면
							glutTimerFunc(100, Timer, i);
						}
					}
					else {
						if (objects[i].transZ + 0.1 * objects[i].scaleZ >= eraserbottom.transZ - 0.1 * eraserbottom.scaleZ && objects[i].transZ + 0.1 * objects[i].scaleZ <= eraserbottom.transZ + 0.1 * eraserbottom.scaleZ) {								// 우측이 지우개와 겹쳐있을때 상단이 지우개와 겹치면
							glutTimerFunc(100, Timer, i);
						}
					}
				}
			}

			if(objects[i].transX - 0.1 * objects[i].scaleX > eraserbottom.transX - 0.1 * eraserbottom.scaleX && objects[i].transX - 0.1 * objects[i].scaleX < eraserbottom.transX + 0.1 * eraserbottom.scaleX) {	// 우측이 지우개와 겹쳐있을때
				if (objects[i].scaleZ >= eraserbottom.scaleZ) {
					if ((eraserbottom.transZ + 0.1 * eraserbottom.scaleZ >= objects[i].transZ - 0.1 * objects[i].scaleZ && eraserbottom.transZ + 0.1 * eraserbottom.scaleZ <= objects[i].transZ + 0.1 * objects[i].scaleZ) || (eraserbottom.transZ - 0.1 * eraserbottom.scaleZ >= objects[i].transZ - 0.1 * objects[i].scaleZ && eraserbottom.transZ - 0.1 * eraserbottom.scaleZ <= objects[i].transZ + 0.1 * objects[i].scaleZ)) {
						glutTimerFunc(100, Timer, i);
					}
				}
				else {
					if (objects[i].transZ >= eraserbottom.transZ) {
						if (objects[i].transZ - 0.1 * objects[i].scaleZ <= eraserbottom.transZ + 0.1 * eraserbottom.scaleZ && objects[i].transZ - 0.1 * objects[i].scaleZ >= eraserbottom.transZ - 0.1 * eraserbottom.scaleZ) {								// 우측이 지우개와 겹쳐있을때 하단이 지우개와 겹치면
							glutTimerFunc(100, Timer, i);
						}
					}
					else {
						if (objects[i].transZ + 0.1 * objects[i].scaleZ >= eraserbottom.transZ - 0.1 * eraserbottom.scaleZ && objects[i].transZ + 0.1 * objects[i].scaleZ <= eraserbottom.transZ + 0.1 * eraserbottom.scaleZ) {								// 우측이 지우개와 겹쳐있을때 상단이 지우개와 겹치면
							glutTimerFunc(100, Timer, i);
						}
					}
				}
			}

			if (objects[i].transZ - 0.1 * objects[i].scaleZ <= eraserbottom.transZ + 0.1 * eraserbottom.scaleZ && objects[i].transZ - 0.1 * objects[i].scaleZ >= eraserbottom.transZ - 0.1 * eraserbottom.scaleZ) {								// 우측이 지우개와 겹쳐있을때 하단이 지우개와 겹치면
				if (objects[i].scaleX >= eraserbottom.scaleX) {
					if((eraserbottom.transX + 0.1*eraserbottom.scaleX >= objects[i].transX - 0.1 * objects[i].scaleX && eraserbottom.transX + 0.1 * eraserbottom.scaleX <= objects[i].transX + 0.1 * objects[i].scaleX)|| (eraserbottom.transX - 0.1 * eraserbottom.scaleX >= objects[i].transX - 0.1 * objects[i].scaleX && eraserbottom.transX - 0.1 * eraserbottom.scaleX <= objects[i].transX + 0.1 * objects[i].scaleX))
						glutTimerFunc(100, Timer, i);
				}
			}

			if (objects[i].transZ + 0.1 * objects[i].scaleZ <= eraserbottom.transZ + 0.1 * eraserbottom.scaleZ && objects[i].transZ + 0.1 * objects[i].scaleZ >= eraserbottom.transZ - 0.1 * eraserbottom.scaleZ) {								// 우측이 지우개와 겹쳐있을때 하단이 지우개와 겹치면
				if (objects[i].scaleX >= eraserbottom.scaleX) {
					if ((eraserbottom.transX + 0.1 * eraserbottom.scaleX >= objects[i].transX - 0.1 * objects[i].scaleX && eraserbottom.transX + 0.1 * eraserbottom.scaleX <= objects[i].transX + 0.1 * objects[i].scaleX) || (eraserbottom.transX - 0.1 * eraserbottom.scaleX >= objects[i].transX - 0.1 * objects[i].scaleX && eraserbottom.transX - 0.1 * eraserbottom.scaleX <= objects[i].transX + 0.1 * objects[i].scaleX))
						glutTimerFunc(100, Timer, i);
				}
			}
		}
		glutTimerFunc(10, Timer, 'c');
		break;
	default:
		if (objects[value].scaleX > 0)
			objects[value].scaleX -= 0.1;
		if (objects[value].scaleY > 0)
			objects[value].scaleY -= 0.1;
		if (objects[value].scaleZ > 0)
			objects[value].scaleZ -= 0.1;
		if (objects[value].scaleX > 0 || objects[value].scaleY > 0 || objects[value].scaleZ > 0)
			glutTimerFunc(100, Timer, value);
		break;
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
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &eraserbottom.VAO);
	glBindVertexArray(eraserbottom.VAO);

	glGenBuffers(1, &eraserbottom.Color);
	glBindBuffer(GL_ARRAY_BUFFER, eraserbottom.Color);
	glBufferData(GL_ARRAY_BUFFER, eraserbottom.vertexNum * 3 * sizeof(float), eraserbottom.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &eraserbottom.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, eraserbottom.VBO);
	glBufferData(GL_ARRAY_BUFFER, eraserbottom.vertexNum * 3 * sizeof(float), eraserbottom.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &eraserbottom.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eraserbottom.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, eraserbottom.faceNum * 3 * sizeof(unsigned int), eraserbottom.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &erasertop.VAO);
	glBindVertexArray(erasertop.VAO);

	glGenBuffers(1, &erasertop.Color);
	glBindBuffer(GL_ARRAY_BUFFER, erasertop.Color);
	glBufferData(GL_ARRAY_BUFFER, erasertop.vertexNum * 3 * sizeof(float), erasertop.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &erasertop.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, erasertop.VBO);
	glBufferData(GL_ARRAY_BUFFER, erasertop.vertexNum * 3 * sizeof(float), erasertop.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &erasertop.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, erasertop.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, erasertop.faceNum * 3 * sizeof(unsigned int), erasertop.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	for (int i = 0; i < objectcnt; ++i) {
		glGenVertexArrays(1, &objects[i].VAO);
		glBindVertexArray(objects[i].VAO);

		glGenBuffers(1, &objects[i].Color);
		glBindBuffer(GL_ARRAY_BUFFER, objects[i].Color);
		glBufferData(GL_ARRAY_BUFFER, objects[i].vertexNum * 3 * sizeof(float), objects[i].vertexColor, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);

		glGenBuffers(1, &objects[i].VBO);
		glBindBuffer(GL_ARRAY_BUFFER, objects[i].VBO);
		glBufferData(GL_ARRAY_BUFFER, objects[i].vertexNum * 3 * sizeof(float), objects[i].vertexData, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &objects[i].EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objects[i].EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, objects[i].faceNum * 3 * sizeof(unsigned int), objects[i].vertexFace, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	}
}