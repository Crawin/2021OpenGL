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
#define ballNum 5
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Timer(int value);
GLvoid SpecialKey(int key, int x, int y);
GLvoid MouseClick(int button, int state, int x, int y);
GLvoid MouseMove(int x, int y);

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid InitBuffer();

std::random_device rd;
std::default_random_engine dre(rd());
std::uniform_int_distribution<>ballhead(0, 3);
std::uniform_real_distribution<>ballcolor(0, 1);
std::uniform_real_distribution<>ballpos(-0.45, 0.45);
std::uniform_real_distribution<>ballspeed(0.001, 0.01);

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
	float rotX{}, rotY{}, rotZ{}, transX{}, transY{}, transZ{}, scaleX{ 1.0f }, scaleY{ 1.0f }, scaleZ{ 1.0f } , speed;
	int head{};
	structure() {
		VAO = VBO = EBO = vertexNum = faceNum = 0;
		head = ballhead(dre);
		transX = ballpos(dre);
		transY = ballpos(dre);
		transZ = ballpos(dre);
		speed = ballspeed(dre);
		//--- 1. 전체 버텍스 개수 및 삼각형 개수 세기
		FILE* objFile = fopen("Sphere.obj", "r");
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
				vertexColor[vertIndex++] = ballcolor(dre);
				vertexColor[vertIndex++] = ballcolor(dre);
				vertexColor[vertIndex++] = ballcolor(dre);
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
		unsigned int transformLocation = glGetUniformLocation(shaderProgram, "ModelTransform");
		trans = glm::mat4(1.0f);
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(trans));
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

structure back("Plane.obj", 1, 1, 0);
structure top("Plane.obj", 0, 1, 0);
structure bottom("Plane.obj", 0.2, 0.2, 0.2);
structure left("Plane.obj", 0, 0, 1);
structure right("Plane.obj", 1, 0, 1);
Cam c(0, 0, 0.5, glm::vec3(0, 1, 0));
structure balls[ballNum];
structure block1("Box.obj", 0.8, 0.8, 0.8);
structure block2("Box.obj", 0.5, 0.5, 0.5);
structure block3("Box.obj", 0.3, 0.3, 0.3);
bool Lclick, B;
glm::vec2 mousestart, mousepos;

void main(int argc, char** argv)								//--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);										// glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);				// 디스플레이 모드 설정
	glutInitWindowPosition(0, 0);								// 윈도우의 위치 지정
	glutInitWindowSize(WinX, WinY);								// 윈도우의 크기 지정
	WindowID = glutCreateWindow("22");								// 윈도우 생성 (윈도우 이름)

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
	glutMouseFunc(MouseClick);
	glutMotionFunc(MouseMove);
	glutSpecialFunc(SpecialKey);
	std::cout << "R : 옆면 투시\nO : 앞면이 열린다\nW : 박스 안쪽으로 로봇 이동\nA : 박스 왼쪽으로 로봇 이동\nS : 박스 바깥쪽으로 로봇 이동\nD : 박스 오른쪽으로 로봇 이동\nJ : 로봇 점프\nY : 카메라 공전\n<- : 카메라 왼쪽으로 회전\n-> : 카메라 오른쪽으로 회전\nz/Z : 카메라 앞/뒤(으)로 이동\nx/X : 카메라 좌/우로 이동" << std::endl;
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
	glLineWidth(1);

	back.Rotate(back.rotZ, 0, 0, 1);
	back.Translate(0, 0, -0.5);
	back.Rotate(90, 1, 0, 0);
	back.Scale(5, 5, 5);
	glBindVertexArray(back.VAO);
	glDrawElements(GL_TRIANGLES, back.faceNum * 3, GL_UNSIGNED_INT, 0);
	back.transReset();

	top.Rotate(back.rotZ, 0, 0, 1);
	top.Translate(0, 0.5, 0);
	top.Rotate(180, 0, 0, 1);
	top.Scale(5, 5, 5);
	glBindVertexArray(top.VAO);
	glDrawElements(GL_TRIANGLES, top.faceNum * 3, GL_UNSIGNED_INT, 0);
	top.transReset();

	bottom.Rotate(back.rotZ, 0, 0, 1);
	bottom.Translate(0, -0.5, 0);
	bottom.Scale(5, 5, 5);
	glBindVertexArray(bottom.VAO);
	glDrawElements(GL_TRIANGLES, bottom.faceNum * 3, GL_UNSIGNED_INT, 0);
	bottom.transReset();

	left.Rotate(back.rotZ, 0, 0, 1);
	left.Translate(-0.5, 0, 0);
	left.Rotate(-90, 0, 0, 1);
	left.Scale(5, 5, 5);
	glBindVertexArray(left.VAO);
	glDrawElements(GL_TRIANGLES, left.faceNum * 3, GL_UNSIGNED_INT, 0);
	left.transReset();

	right.Rotate(back.rotZ, 0, 0, 1);
	right.Translate(0.5, 0, 0);
	right.Rotate(90, 0, 0, 1);
	right.Scale(5, 5, 5);
	glBindVertexArray(right.VAO);
	glDrawElements(GL_TRIANGLES, right.faceNum * 3, GL_UNSIGNED_INT, 0);
	right.transReset();

	if (B) {
		for (int i = 0; i < ballNum; ++i) {
			balls[i].Rotate(back.rotZ, 0, 0, 1);
			balls[i].Translate(balls[i].transX, balls[i].transY, balls[i].transZ);
			balls[i].Scale(0.5, 0.5, 0.5);
			glBindVertexArray(balls[i].VAO);
			glDrawElements(GL_TRIANGLES, balls[i].faceNum * 3, GL_UNSIGNED_INT, 0);
			balls[i].transReset();
		}
	}
	int grav = (int)back.rotZ % 360;
	
	if (grav >= -45 && grav <= 45) {
		block1.Translate(0, -0.5 * glm::sec(glm::radians(back.rotZ)) + 0.0125 * glm::sec(glm::radians(back.rotZ)), 0);
	}
	else if (grav > 45 && grav < 135) {
		block1.Translate(0, -0.5 * glm::csc(glm::radians(back.rotZ)) + 0.0125 * glm::csc(glm::radians(back.rotZ)), 0);
	}
	else if (grav >= 135 && grav <= 225) {
		block1.Translate(0, 0.5 * glm::sec(glm::radians(back.rotZ)) - 0.0125 * glm::sec(glm::radians(back.rotZ)), 0);
	}
	else if (grav > 225 && grav < 315) {
		block1.Translate(0, 0.5 * glm::csc(glm::radians(back.rotZ)) - 0.0125 * glm::csc(glm::radians(back.rotZ)), 0);
	}
	else if (grav >= 315) {
		block1.Translate(0, -0.5 * glm::sec(glm::radians(back.rotZ)) + 0.0125 * glm::sec(glm::radians(back.rotZ)), 0);
	}
	else if (grav <-45 && grav >-135) {
		block1.Translate(0, 0.5 * glm::csc(glm::radians(back.rotZ)) - 0.0125 * glm::csc(glm::radians(back.rotZ)), 0);
	}
	else if (grav <= -135 && grav >= -225) {
		block1.Translate(0, 0.5 * glm::sec(glm::radians(back.rotZ)) - 0.0125 * glm::sec(glm::radians(back.rotZ)), 0);
	}
	else if (grav < -225 && grav > -315) {
		block1.Translate(0, -0.5 * glm::csc(glm::radians(back.rotZ)) + 0.0125 * glm::csc(glm::radians(back.rotZ)), 0);
	}
	else if (grav <= -315) {
		block1.Translate(0, -0.5 * glm::sec(glm::radians(back.rotZ)) + 0.0125 * glm::sec(glm::radians(back.rotZ)), 0);
	}
	block1.Rotate(back.rotZ, 0, 0, 1);
	block1.Scale(0.125, 0.125, 0.125);
	block1.Translate(0, -0.1, 0);
	glBindVertexArray(block1.VAO);
	glDrawElements(GL_TRIANGLES, block1.faceNum * 3, GL_UNSIGNED_INT, 0);
	block1.transReset();

	if (grav >= -45 && grav <= 45) {
		block2.Translate(0, -0.5 * glm::sec(glm::radians(back.rotZ)) + 0.025 * glm::sec(glm::radians(back.rotZ)), -0.05);
	}
	else if (grav > 45 && grav < 135) {
		block2.Translate(0, -0.5 * glm::csc(glm::radians(back.rotZ)) + 0.025 * glm::csc(glm::radians(back.rotZ)), -0.05);
	}
	else if (grav >= 135 && grav <= 225) {
		block2.Translate(0, 0.5 * glm::sec(glm::radians(back.rotZ)) - 0.025 * glm::sec(glm::radians(back.rotZ)), -0.05);
	}
	else if (grav > 225 && grav < 315) {
		block2.Translate(0, 0.5 * glm::csc(glm::radians(back.rotZ)) - 0.025 * glm::csc(glm::radians(back.rotZ)), -0.05);
	}
	else if (grav >= 315) {
		block2.Translate(0, -0.5 * glm::sec(glm::radians(back.rotZ)) + 0.025 * glm::sec(glm::radians(back.rotZ)), -0.05);
	}
	else if (grav <-45 && grav >-135) {
		block2.Translate(0, 0.5 * glm::csc(glm::radians(back.rotZ)) - 0.025 * glm::csc(glm::radians(back.rotZ)), -0.05);
	}
	else if (grav <= -135 && grav >= -225) {
		block2.Translate(0, 0.5 * glm::sec(glm::radians(back.rotZ)) - 0.025 * glm::sec(glm::radians(back.rotZ)), -0.05);
	}
	else if (grav < -225 && grav > -315) {
		block2.Translate(0, -0.5 * glm::csc(glm::radians(back.rotZ)) + 0.025 * glm::csc(glm::radians(back.rotZ)), -0.05);
	}
	else if (grav <= -315) {
		block2.Translate(0, -0.5 * glm::sec(glm::radians(back.rotZ)) + 0.025 * glm::sec(glm::radians(back.rotZ)), -0.05);
	}
	block2.Rotate(back.rotZ, 0, 0, 1);
	block2.Scale(0.25, 0.25, 0.25);
	block2.Translate(0, -0.1, 0);
	glBindVertexArray(block2.VAO);
	glDrawElements(GL_TRIANGLES, block2.faceNum * 3, GL_UNSIGNED_INT, 0);
	block2.transReset();

	if (grav >= -45 && grav <= 45) {
		block3.Translate(0, -0.5 * glm::sec(glm::radians(back.rotZ)) + 0.05 * glm::sec(glm::radians(back.rotZ)), -0.15);
	}
	else if (grav > 45 && grav < 135) {
		block3.Translate(0, -0.5 * glm::csc(glm::radians(back.rotZ)) + 0.05 * glm::csc(glm::radians(back.rotZ)), -0.15);
	}
	else if (grav >= 135 && grav <= 225) {
		block3.Translate(0, 0.5 * glm::sec(glm::radians(back.rotZ)) - 0.05 * glm::sec(glm::radians(back.rotZ)), -0.15);
	}
	else if (grav > 225 && grav < 315) {
		block3.Translate(0, 0.5 * glm::csc(glm::radians(back.rotZ)) - 0.05 * glm::csc(glm::radians(back.rotZ)), -0.15);
	}
	else if (grav >= 315) {
		block3.Translate(0, -0.5 * glm::sec(glm::radians(back.rotZ)) + 0.05 * glm::sec(glm::radians(back.rotZ)), -0.15);
	}
	else if (grav <-45 && grav >-135) {
		block3.Translate(0, 0.5 * glm::csc(glm::radians(back.rotZ)) - 0.05 * glm::csc(glm::radians(back.rotZ)), -0.15);
	}
	else if (grav <= -135 && grav >= -225) {
		block3.Translate(0, 0.5 * glm::sec(glm::radians(back.rotZ)) - 0.05 * glm::sec(glm::radians(back.rotZ)), -0.15);
	}
	else if (grav < -225 && grav > -315) {
		block3.Translate(0, -0.5 * glm::csc(glm::radians(back.rotZ)) + 0.05 * glm::csc(glm::radians(back.rotZ)), -0.15);
	}
	else if (grav <= -315) {
		block3.Translate(0, -0.5 * glm::sec(glm::radians(back.rotZ)) + 0.05 * glm::sec(glm::radians(back.rotZ)), -0.15);
	}
	block3.Rotate(back.rotZ, 0, 0, 1);
	block3.Scale(0.5, 0.5, 0.5);
	block3.Translate(0, -0.1, 0);
	glBindVertexArray(block3.VAO);
	glDrawElements(GL_TRIANGLES, block3.faceNum * 3, GL_UNSIGNED_INT, 0);
	block3.transReset();

	glutSwapBuffers();											// 화면에 출력하기
}
GLvoid Reshape(int w, int h)									//--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'x':
		c.camAt.z -= 0.05;
		c.camPos.z -= 0.05;
		break;
	case 'X':
		c.camAt.z += 0.05;
		c.camPos.z += 0.05;
		break;
	case 'z':
		c.camAt.x -= 0.05;
		c.camPos.x -= 0.05;
		break;
	case 'Z':
		c.camAt.x += 0.05;
		c.camPos.x += 0.05;
		break;
	case 'y':
		glutTimerFunc(10, Timer, 'y');
		break;
	case 'Y':
		glutTimerFunc(10, Timer, 'Y');
		break;
	case 'B':
	case 'b':
		B = true;
		for (int i = 0; i < ballNum; ++i) {
			glutTimerFunc(10, Timer, i);
		}
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
	switch (value) { //0 ~ ballnum 까진 공 타이머
	case 'y':
		c.camRot -= 1;
		glutTimerFunc(10, Timer, value);
		break;
	case 'Y':
		c.camRot += 1;
		glutTimerFunc(10, Timer, value);
		break;
	default:
		glutTimerFunc(10, Timer, value);
		switch (balls[value].head) {
		case 0:
			balls[value].transY += balls[value].speed;
			if (balls[value].transY + 0.05 > 0.5) {
				balls[value].head = 12;
			}
			break;
		case 1:
			balls[value].transY += balls[value].speed;
			balls[value].transZ += balls[value].speed;
			if (balls[value].transY + 0.05 > 0.5) {
				balls[value].head = 13;
			}
			if (balls[value].transZ + 0.05 > 0.5) {
				balls[value].head = 2;
			}
			break;
		case 2:
			balls[value].transY += balls[value].speed;
			balls[value].transZ -= balls[value].speed;
			if (balls[value].transY + 0.05 > 0.5) {
				balls[value].head = 14;
			}
			if (balls[value].transZ - 0.05 < -0.5) {
				balls[value].head = 1;
			}
			break;
		case 3:
			balls[value].transX += balls[value].speed;
			balls[value].transY += balls[value].speed;
			if (balls[value].transX + 0.05 > 0.5) {
				balls[value].head = 21;
			}
			if (balls[value].transY + 0.05 > 0.5) {
				balls[value].head = 9;
			}
		case 4:
			balls[value].transX += balls[value].speed;
			balls[value].transY += balls[value].speed;
			balls[value].transZ += balls[value].speed;
			if (balls[value].transX + 0.05 > 0.5) {
				balls[value].head = 22;
			}
			if (balls[value].transY + 0.05 > 0.5) {
				balls[value].head = 10;
			}
			if (balls[value].transZ + 0.05 > 0.5) {
				balls[value].head = 5;
			}
			break;
		case 5:
			balls[value].transX += balls[value].speed;
			balls[value].transY += balls[value].speed;
			balls[value].transZ -= balls[value].speed;
			if (balls[value].transX + 0.05 > 0.5) {
				balls[value].head = 23;
			}
			if (balls[value].transY + 0.05 > 0.5) {
				balls[value].head = 11;
			}
			if (balls[value].transZ - 0.05 < -0.5) {
				balls[value].head = 4;
			}
			break;
		case 6:
			balls[value].transX += balls[value].speed;
			if (balls[value].transX + 0.05 > 0.5) {
				balls[value].head = 18;
			}
			break;
		case 7:
			balls[value].transX += balls[value].speed;
			balls[value].transZ += balls[value].speed;
			if (balls[value].transX + 0.05 > 0.5) {
				balls[value].head = 19;
			}
			if (balls[value].transZ + 0.05 > 0.5) {
				balls[value].head = 8;
			}
			break;
		case 8:
			balls[value].transX += balls[value].speed;
			balls[value].transZ -= balls[value].speed;
			if (balls[value].transX + 0.05 > 0.5) {
				balls[value].head = 20;
			}
			if (balls[value].transZ - 0.05 < -0.5) {
				balls[value].head = 7;
			}
			break;
		case 9:
			balls[value].transX += balls[value].speed;
			balls[value].transY -= balls[value].speed;
			if (balls[value].transX + 0.05 > 0.5) {
				balls[value].head = 15;
			}
			if (balls[value].transY - 0.05 < -0.5) {
				balls[value].head = 3;
			}
			break;
		case 10:
			balls[value].transX += balls[value].speed;
			balls[value].transY -= balls[value].speed;
			balls[value].transZ += balls[value].speed;
			if (balls[value].transX + 0.05 > 0.5) {
				balls[value].head = 16;
			}
			if (balls[value].transY - 0.05 < -0.5) {
				balls[value].head = 4;
			}
			if (balls[value].transZ + 0.05 > 0.5) {
				balls[value].head = 11;
			}
			break;
		case 11:
			balls[value].transX += balls[value].speed;
			balls[value].transY -= balls[value].speed;
			balls[value].transZ -= balls[value].speed;
			if (balls[value].transX + 0.05 > 0.5) {
				balls[value].head = 18;
			}
			if (balls[value].transY - 0.05 < -0.5) {
				balls[value].head = 5;
			}
			if (balls[value].transZ - 0.05 < -0.5) {
				balls[value].head = 10;
			}
			break;
		case 12:
			balls[value].transY -= balls[value].speed;
			if (balls[value].transY - 0.05 < -0.5) {
				balls[value].head = 0;
			}
			break;
		case 13:
			balls[value].transY -= balls[value].speed;
			balls[value].transZ += balls[value].speed;
			if (balls[value].transY - 0.05 < -0.5) {
				balls[value].head = 1;
			}
			if (balls[value].transZ + 0.05 > 0.5) {
				balls[value].head = 14;
			}
			break;
		case 14:
			balls[value].transY -= balls[value].speed;
			balls[value].transZ -= balls[value].speed;
			if (balls[value].transY - 0.05 < -0.5) {
				balls[value].head = 2;
			}
			if (balls[value].transZ - 0.05 < -0.5) {
				balls[value].head = 13;
			}
			break;
		case 15:
			balls[value].transX -= balls[value].speed;
			balls[value].transY -= balls[value].speed;
			if (balls[value].transX - 0.05 < -0.5) {
				balls[value].head = 9;
			}
			if (balls[value].transY - 0.05 < -0.5) {
				balls[value].head = 21;
			}
			break;
		case 16:
			balls[value].transX -= balls[value].speed;
			balls[value].transY -= balls[value].speed;
			balls[value].transZ += balls[value].speed;
			if (balls[value].transX - 0.05 < -0.5) {
				balls[value].head = 10;
			}
			if (balls[value].transY - 0.05 < -0.5) {
				balls[value].head = 22;
			}
			if (balls[value].transZ + 0.05 > 0.5) {
				balls[value].head = 17;
			}
			break;
		case 17:
			balls[value].transX -= balls[value].speed;
			balls[value].transY -= balls[value].speed;
			balls[value].transZ -= balls[value].speed;
			if (balls[value].transX - 0.05 < -0.5) {
				balls[value].head = 11;
			}
			if (balls[value].transY - 0.05 < -0.5) {
				balls[value].head = 23;
			}
			if (balls[value].transZ - 0.05 < -0.5) {
				balls[value].head = 15;
			}
			break;
		case 18:
			balls[value].transX -= balls[value].speed;
			if (balls[value].transX - 0.05 < -0.5) {
				balls[value].head = 6;
			}
			break;
		case 19:
			balls[value].transX -= balls[value].speed;
			balls[value].transZ += balls[value].speed;
			if (balls[value].transX - 0.05 < -0.5) {
				balls[value].head = 6;
			}
			if (balls[value].transZ + 0.05 > 0.5) {
				balls[value].head = 20;
			}
			break;
		case 20:
			balls[value].transX -= balls[value].speed;
			balls[value].transZ -= balls[value].speed;
			if (balls[value].transX - 0.05 < -0.5) {
				balls[value].head = 6;
			}
			if (balls[value].transZ - 0.05 < -0.5) {
				balls[value].head = 19;
			}
			break;
		case 21:
			balls[value].transX -= balls[value].speed;
			balls[value].transY += balls[value].speed;
			if (balls[value].transX - 0.05 < -0.5) {
				balls[value].head = 3;
			}
			if (balls[value].transY + 0.05 > 0.5) {
				balls[value].head = 15;
			}
			break;
		case 22:
			balls[value].transX -= balls[value].speed;
			balls[value].transY += balls[value].speed;
			balls[value].transZ += balls[value].speed;
			if (balls[value].transX - 0.05 < -0.5) {
				balls[value].head = 4;
			}
			if (balls[value].transY + 0.05 > 0.5) {
				balls[value].head = 16;
			}
			if (balls[value].transZ + 0.05 > 0.5) {
				balls[value].head = 23;
			}
			break;
		case 23:
			balls[value].transX -= balls[value].speed;
			balls[value].transY += balls[value].speed;
			balls[value].transZ -= balls[value].speed;
			if (balls[value].transX - 0.05 < -0.5) {
				balls[value].head = 5;
			}
			if (balls[value].transY + 0.05 > 0.5) {
				balls[value].head = 17;
			}
			if (balls[value].transZ - 0.05 < -0.5) {
				balls[value].head = 22;
			}
			break;
		}
	}
	glutPostRedisplay();
}

GLvoid MouseClick(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			std::cout << "( " << x << ", " << y << " )" << std::endl;
			mousestart = glm::vec2(x, y);
			Lclick = true;
		}
		else if (state == GLUT_UP) {
			Lclick = false;
			top.rotZ = back.rotZ;
		}
	}
	glutPostRedisplay();
}
GLvoid MouseMove(int x, int y) {
	if (Lclick) {
		mousepos = glm::vec2(x, y);
		back.rotZ = (mousepos.x - mousestart.x) * 90 / 361 + top.rotZ;
		//std::cout << "( " << x << ", " << y << " ), degree - "<< back.rotZ << std::endl;
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
	glGenVertexArrays(1, &back.VAO);
	glBindVertexArray(back.VAO);

	glGenBuffers(1, &back.Color);
	glBindBuffer(GL_ARRAY_BUFFER, back.Color);
	glBufferData(GL_ARRAY_BUFFER, back.vertexNum * 3 * sizeof(float), back.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &back.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, back.VBO);
	glBufferData(GL_ARRAY_BUFFER, back.vertexNum * 3 * sizeof(float), back.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &back.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, back.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, back.faceNum * 3 * sizeof(unsigned int), back.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &top.VAO);
	glBindVertexArray(top.VAO);

	glGenBuffers(1, &top.Color);
	glBindBuffer(GL_ARRAY_BUFFER, top.Color);
	glBufferData(GL_ARRAY_BUFFER, top.vertexNum * 3 * sizeof(float), top.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &top.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, top.VBO);
	glBufferData(GL_ARRAY_BUFFER, top.vertexNum * 3 * sizeof(float), top.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &top.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, top.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, top.faceNum * 3 * sizeof(unsigned int), top.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
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
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &left.VAO);
	glBindVertexArray(left.VAO);

	glGenBuffers(1, &left.Color);
	glBindBuffer(GL_ARRAY_BUFFER, left.Color);
	glBufferData(GL_ARRAY_BUFFER, left.vertexNum * 3 * sizeof(float), left.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &left.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, left.VBO);
	glBufferData(GL_ARRAY_BUFFER, left.vertexNum * 3 * sizeof(float), left.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &left.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, left.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, left.faceNum * 3 * sizeof(unsigned int), left.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &right.VAO);
	glBindVertexArray(right.VAO);

	glGenBuffers(1, &right.Color);
	glBindBuffer(GL_ARRAY_BUFFER, right.Color);
	glBufferData(GL_ARRAY_BUFFER, right.vertexNum * 3 * sizeof(float), right.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &right.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, right.VBO);
	glBufferData(GL_ARRAY_BUFFER, right.vertexNum * 3 * sizeof(float), right.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &right.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, right.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, right.faceNum * 3 * sizeof(unsigned int), right.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	for (int i = 0; i < ballNum; ++i) {
		glGenVertexArrays(1, &balls[i].VAO);
		glBindVertexArray(balls[i].VAO);

		glGenBuffers(1, &balls[i].Color);
		glBindBuffer(GL_ARRAY_BUFFER, balls[i].Color);
		glBufferData(GL_ARRAY_BUFFER, balls[i].vertexNum * 3 * sizeof(float), balls[i].vertexColor, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);

		glGenBuffers(1, &balls[i].VBO);
		glBindBuffer(GL_ARRAY_BUFFER, balls[i].VBO);
		glBufferData(GL_ARRAY_BUFFER, balls[i].vertexNum * 3 * sizeof(float), balls[i].vertexData, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &balls[i].EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, balls[i].EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, balls[i].faceNum * 3 * sizeof(unsigned int), balls[i].vertexFace, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	}
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &block1.VAO);
	glBindVertexArray(block1.VAO);

	glGenBuffers(1, &block1.Color);
	glBindBuffer(GL_ARRAY_BUFFER, block1.Color);
	glBufferData(GL_ARRAY_BUFFER, block1.vertexNum * 3 * sizeof(float), block1.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &block1.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, block1.VBO);
	glBufferData(GL_ARRAY_BUFFER, block1.vertexNum * 3 * sizeof(float), block1.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &block1.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, block1.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, block1.faceNum * 3 * sizeof(unsigned int), block1.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &block2.VAO);
	glBindVertexArray(block2.VAO);

	glGenBuffers(1, &block2.Color);
	glBindBuffer(GL_ARRAY_BUFFER, block2.Color);
	glBufferData(GL_ARRAY_BUFFER, block2.vertexNum * 3 * sizeof(float), block2.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &block2.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, block2.VBO);
	glBufferData(GL_ARRAY_BUFFER, block2.vertexNum * 3 * sizeof(float), block2.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &block2.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, block2.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, block2.faceNum * 3 * sizeof(unsigned int), block2.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &block3.VAO);
	glBindVertexArray(block3.VAO);

	glGenBuffers(1, &block3.Color);
	glBindBuffer(GL_ARRAY_BUFFER, block3.Color);
	glBufferData(GL_ARRAY_BUFFER, block3.vertexNum * 3 * sizeof(float), block3.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &block3.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, block3.VBO);
	glBufferData(GL_ARRAY_BUFFER, block3.vertexNum * 3 * sizeof(float), block3.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &block3.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, block3.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, block3.faceNum * 3 * sizeof(unsigned int), block3.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
}