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

std::random_device rd;
std::default_random_engine dre(rd());
std::uniform_real_distribution<> field(-0.5, 0.5);
std::uniform_real_distribution<> color(0, 1.0);
std::uniform_real_distribution<> scale(1.0, 2.0);

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
	float rotX{}, rotY{}, rotZ{}, transX{}, transY{}, transZ{}, scaleX{ 1.0f }, scaleY{ 1.0f }, scaleZ{ 1.0f }, theta{}, r{}, jumpspeed{};
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

structure front("Plane.obj", 1, 0, 0);
structure back("Plane.obj", 1, 1, 0);
structure top("Plane.obj", 0, 1, 0);
structure bottom("Plane.obj", 0.2, 0.2, 0.2);
structure left("Plane.obj", 0, 0, 1);
structure right("Plane.obj", 1, 0, 1);
structure frontcw("Plane.obj", 1, 0, 0);
structure backcw("Plane.obj", 1, 1, 0);
structure topcw("Plane.obj", 0, 1, 0);
structure bottomcw("Plane.obj", 0.2, 0.2, 0.2);
structure leftcw("Plane.obj", 0, 0, 1);
structure rightcw("Plane.obj", 1, 0, 1);
structure robotHead("Box.obj", 0, 0.5, 1);
structure robotNose("Cylinder.obj", 0.55, 0.33, 0.1);
structure robotBody("Box.obj", 0, 0, 1);
structure robotLArm("Cylinder.obj", 0.7, 0.7, 0);
structure robotRArm("Cylinder.obj", 0.3, 0.3, 0);
structure robotLLeg("Cylinder.obj", 0.1, 0.3, 0.8);
structure robotRLeg("Cylinder.obj", 0, 0.7, 0.2);
structure box1("Box.obj", color(dre), color(dre), color(dre));
structure box2("Box.obj", color(dre), color(dre), color(dre));
Cam c(0, 0, 0.5, glm::vec3(0, 1, 0));
bool frontup, J, W, A, S, D, R, I = true, ON[2];
char head;
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
	for (int i = 0; i < 1; ++i) {
		box1.transX = field(dre);
		box1.transZ = field(dre);
		box1.scaleX = scale(dre);
		box1.scaleZ = scale(dre);
		if (box1.transX + 0.1 * box1.scaleX >= 0.5) {
			--i;
			continue;
		}
		if (box1.transX - 0.1 * box1.scaleX <= -0.5) {
			--i;
			continue;
		}
		if (box1.transZ + 0.1 * box1.scaleZ >= 0.5) {
			--i;
			continue;
		}
		if (box1.transZ - 0.1 * box1.scaleZ <= -0.5) {
			--i;
			continue;
		}
	}
	for (int i = 0; i < 1; ++i) {
		box2.transX = field(dre);
		box2.transZ = field(dre);
		box2.scaleX = scale(dre);
		box2.scaleZ = scale(dre);
		if (box2.transX + 0.1 * box2.scaleX >= 0.5) {
			--i;
			continue;
		}
		if (box2.transX - 0.1 * box2.scaleX <= -0.5) {
			--i;
			continue;
		}
		if (box2.transZ + 0.1 * box2.scaleZ >= 0.5) {
			--i;
			continue;
		}
		if (box2.transZ - 0.1 * box2.scaleZ <= -0.5) {
			--i;
			continue;
		}

		if (box2.transX + 0.1 * box2.scaleX >= box1.transX - 0.1 * box1.scaleX && box2.transX + 0.1 * box2.scaleX <= box1.transX + 0.1 * box1.scaleX) {
			if ((box2.transZ + 0.1 * box2.scaleZ >= box1.transZ - 0.1 * box1.scaleZ && box2.transZ + 0.1 * box2.scaleZ <= box1.transZ + 0.1 * box1.scaleZ) || (box2.transZ - 0.1 * box2.scaleZ <= box1.transZ + 0.1 * box1.scaleZ && box2.transZ - 0.1 * box2.scaleZ >= box1.transZ - 0.1 * box1.scaleZ)) {
				--i;
				continue;
			}
		}
		if (box2.transX - 0.1 * box2.scaleX <= box1.transX + 0.1 * box1.scaleX && box2.transX - 0.1 * box2.scaleX >= box1.transX - 0.1 * box1.scaleX) {
			if ((box2.transZ + 0.1 * box2.scaleZ <= box1.transZ + 0.1 * box1.scaleZ && box2.transZ + 0.1 * box2.scaleZ >= box1.transZ - 0.1 * box1.scaleZ) || (box2.transZ - 0.1 * box2.scaleZ <= box1.transZ + 0.1 * box1.scaleZ && box2.transZ - 0.1 * box2.scaleZ >= box1.transZ - 0.1 * box1.scaleZ)) {
				--i;
				continue;
			}
		}
		if (box2.transZ + 0.1 * box2.scaleZ <= box1.transZ + 0.1 * box1.scaleZ && box2.transZ + 0.1 * box2.scaleZ >= box1.transZ - 0.1 * box1.scaleZ) {
			if ((box2.transX + 0.1 * box2.scaleX <= box1.transX + 0.1 * box1.scaleX && box2.transX + 0.1 * box2.scaleX >= box1.transX - 0.1 * box1.scaleX) || (box2.transX - 0.1 * box2.scaleX <= box1.transX + 0.1 * box1.scaleX && box2.transX - 0.1 * box2.scaleX >= box1.transX - 0.1 * box1.scaleX)) {
				--i;
				continue;
			}
		}
		if (box2.transZ - 0.1 * box2.scaleZ <= box1.transZ + 0.1 * box1.scaleZ && box2.transZ - 0.1 * box2.scaleZ >= box1.transZ - 0.1 * box1.scaleZ) {
			if ((box2.transX + 0.1 * box2.scaleX <= box1.transX + 0.1 * box1.scaleX && box2.transX + 0.1 * box2.scaleX >= box1.transX - 0.1 * box1.scaleX) || (box2.transX - 0.1 * box2.scaleX <= box1.transX + 0.1 * box1.scaleX && box2.transX - 0.1 * box2.scaleX >= box1.transX - 0.1 * box1.scaleX)) {
				--i;
				continue;
			}
		}
		if ((box2.transX >= box1.transX - 0.1 * box1.scaleX && box2.transX <= box1.transX + 0.1 * box1.scaleX) && (box2.transZ >= box1.transZ - 0.1 * box1.scaleZ && box2.transZ <= box1.transZ + 0.1 * box1.scaleZ)) {
			--i;
			continue;
		}
	}
	shaderProgram = make_shaderProgram();
	glutDisplayFunc(drawScene);									// 출력 콜백함수의 지정
	glutReshapeFunc(Reshape);									// 다시 그리기 콜백함수 지정
	glutKeyboardFunc(Keyboard);									// 키보드 입력 콜백함수 지정
	glutSpecialFunc(SpecialKey);
	glutTimerFunc(100, Timer, 0);
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
	
	glFrontFace(GL_CW);

	frontcw.Translate(0, 0, 0.5);
	frontcw.Translate(0, -0.5, 0);
	frontcw.Rotate(front.rotX, 1, 0, 0);
	frontcw.Translate(0, 0.5, 0);
	frontcw.Rotate(90, 1, 0, 0);
	frontcw.Scale(5, 5, 5);
	glBindVertexArray(frontcw.VAO);
	glDrawElements(GL_TRIANGLES, frontcw.faceNum * 3, GL_UNSIGNED_INT, 0);
	frontcw.transReset();

	glFrontFace(GL_CCW);

	front.Translate(0, 0, 0.5);
	front.Translate(0, -0.5, 0);
	front.Rotate(front.rotX, 1, 0, 0);
	front.Translate(0, 0.5, 0);
	front.Rotate(90, 1, 0, 0);
	front.Scale(5, 5, 5);
	glBindVertexArray(front.VAO);
	glDrawElements(GL_TRIANGLES, front.faceNum * 3, GL_UNSIGNED_INT, 0);
	front.transReset();

	back.Translate(0, 0, -0.5);
	back.Rotate(90, 1, 0, 0);
	back.Scale(5, 5, 5);
	glBindVertexArray(back.VAO);
	glDrawElements(GL_TRIANGLES, back.faceNum * 3, GL_UNSIGNED_INT, 0);
	back.transReset();

	top.Translate(0, 0.5, 0);
	top.Rotate(180, 0, 0, 1);
	top.Scale(5, 5, 5);
	glBindVertexArray(top.VAO);
	glDrawElements(GL_TRIANGLES, top.faceNum * 3, GL_UNSIGNED_INT, 0);
	top.transReset();

	bottom.Translate(0, -0.5, 0);
	bottom.Scale(5, 5, 5);
	glBindVertexArray(bottom.VAO);
	glDrawElements(GL_TRIANGLES, bottom.faceNum * 3, GL_UNSIGNED_INT, 0);
	bottom.transReset();

	left.Translate(-0.5, 0, 0);
	left.Rotate(-90, 0, 0, 1);
	left.Scale(5, 5, 5);
	glBindVertexArray(left.VAO);
	glDrawElements(GL_TRIANGLES, left.faceNum * 3, GL_UNSIGNED_INT, 0);
	left.transReset();

	right.Translate(0.5, 0, 0);
	right.Rotate(90, 0, 0, 1);
	right.Scale(5, 5, 5);
	glBindVertexArray(right.VAO);
	glDrawElements(GL_TRIANGLES, right.faceNum * 3, GL_UNSIGNED_INT, 0);
	right.transReset();

	robotHead.Translate(robotHead.transX, robotHead.transY, robotHead.transZ);
	robotHead.Rotate(robotHead.rotY, 0, 1, 0);
	robotHead.Translate(0, -0.3, 0);
	robotHead.Scale(0.25, 0.25, 0.25);
	glBindVertexArray(robotHead.VAO);
	glDrawElements(GL_TRIANGLES, robotHead.faceNum * 3, GL_UNSIGNED_INT, 0);
	robotHead.transReset();

	robotNose.Translate(robotHead.transX, robotHead.transY, robotHead.transZ);
	robotNose.Rotate(robotHead.rotY, 0, 1, 0);
	robotNose.Translate(0, -0.28, 0.05);
	robotNose.Rotate(90, 1, 0, 0);
	robotNose.Scale(0.25, 1, 0.25);
	robotNose.Translate(0, -0.05, 0);
	glBindVertexArray(robotNose.VAO);
	glDrawElements(GL_TRIANGLES, robotNose.faceNum * 3, GL_UNSIGNED_INT, 0);
	robotNose.transReset();

	robotBody.Translate(robotHead.transX, robotHead.transY, robotHead.transZ);
	robotBody.Rotate(robotHead.rotY, 0, 1, 0);
	robotBody.Translate(0, -0.4, 0);
	robotBody.Scale(0.5, 0.5, 0.5);
	glBindVertexArray(robotBody.VAO);
	glDrawElements(GL_TRIANGLES, robotBody.faceNum * 3, GL_UNSIGNED_INT, 0);
	robotBody.transReset();

	robotLArm.Translate(robotHead.transX, robotHead.transY, robotHead.transZ);
	robotLArm.Rotate(robotHead.rotY, 0, 1, 0);
	robotLArm.Translate(-0.03, -0.32, 0);
	robotLArm.Rotate(-30, 0, 0, 1);
	robotLArm.Rotate(robotLArm.rotX, -1, 0, 0);
	robotLArm.Translate(0, -0.1, 0);
	robotLArm.Scale(0.25, 1, 0.25);
	glBindVertexArray(robotLArm.VAO);
	glDrawElements(GL_TRIANGLES, robotLArm.faceNum * 3, GL_UNSIGNED_INT, 0);
	robotLArm.transReset();

	robotRArm.Translate(robotHead.transX, robotHead.transY, robotHead.transZ);
	robotRArm.Rotate(robotHead.rotY, 0, 1, 0);
	robotRArm.Translate(0.03, -0.32, 0);
	robotRArm.Rotate(30, 0, 0, 1);
	robotRArm.Rotate(robotRArm.rotX, 1, 0, 0);
	robotRArm.Translate(0, -0.1, 0);
	robotRArm.Scale(0.25, 1, 0.25);
	glBindVertexArray(robotRArm.VAO);
	glDrawElements(GL_TRIANGLES, robotRArm.faceNum * 3, GL_UNSIGNED_INT, 0);
	robotRArm.transReset();

	robotLLeg.Translate(robotHead.transX, robotHead.transY, robotHead.transZ);
	robotLLeg.Rotate(robotHead.rotY, 0, 1, 0);
	robotLLeg.Translate(-0.02, -0.5, 0);
	robotLLeg.Translate(0, 0.1, 0);
	robotLLeg.Rotate(robotRArm.rotX, 1, 0, 0);
	robotLLeg.Translate(0, -0.1, 0);
	robotLLeg.Scale(0.25, 1, 0.25);
	glBindVertexArray(robotLLeg.VAO);
	glDrawElements(GL_TRIANGLES, robotLLeg.faceNum * 3, GL_UNSIGNED_INT, 0);
	robotLLeg.transReset();

	robotRLeg.Translate(robotHead.transX, robotHead.transY, robotHead.transZ);
	robotRLeg.Rotate(robotHead.rotY, 0, 1, 0);
	robotRLeg.Translate(0.02, -0.5, 0);
	robotRLeg.Translate(0, 0.1, 0);
	robotRLeg.Rotate(robotRArm.rotX, -1, 0, 0);
	robotRLeg.Translate(0, -0.1, 0);
	robotRLeg.Scale(0.25, 1, 0.25);
	glBindVertexArray(robotRLeg.VAO);
	glDrawElements(GL_TRIANGLES, robotRLeg.faceNum * 3, GL_UNSIGNED_INT, 0);
	robotRLeg.transReset();

	box1.Translate(box1.transX, -0.5, box1.transZ);
	box1.Scale(box1.scaleX, 0.5, box1.scaleZ);
	glBindVertexArray(box1.VAO);
	glDrawElements(GL_TRIANGLES, box1.faceNum * 3, GL_UNSIGNED_INT, 0);
	box1.transReset();

	box2.Translate(box2.transX, -0.5, box2.transZ);
	box2.Scale(box2.scaleX, 0.5, box2.scaleZ);
	glBindVertexArray(box2.VAO);
	glDrawElements(GL_TRIANGLES, box2.faceNum * 3, GL_UNSIGNED_INT, 0);
	box2.transReset();

	if (!R) {
		glFrontFace(GL_CW);
		backcw.Translate(0, 0, -0.5);
		backcw.Rotate(90, 1, 0, 0);
		backcw.Scale(5, 5, 5);
		glBindVertexArray(backcw.VAO);
		glDrawElements(GL_TRIANGLES, backcw.faceNum * 3, GL_UNSIGNED_INT, 0);
		backcw.transReset();

		topcw.Translate(0, 0.5, 0);
		topcw.Rotate(180, 0, 0, 1);
		topcw.Scale(5, 5, 5);
		glBindVertexArray(topcw.VAO);
		glDrawElements(GL_TRIANGLES, topcw.faceNum * 3, GL_UNSIGNED_INT, 0);
		topcw.transReset();

		bottomcw.Translate(0, -0.5, 0);
		bottomcw.Scale(5, 5, 5);
		glBindVertexArray(bottomcw.VAO);
		glDrawElements(GL_TRIANGLES, bottomcw.faceNum * 3, GL_UNSIGNED_INT, 0);
		bottomcw.transReset();

		leftcw.Translate(-0.5, 0, 0);
		leftcw.Rotate(-90, 0, 0, 1);
		leftcw.Scale(5, 5, 5);
		glBindVertexArray(leftcw.VAO);
		glDrawElements(GL_TRIANGLES, leftcw.faceNum * 3, GL_UNSIGNED_INT, 0);
		leftcw.transReset();

		rightcw.Translate(0.5, 0, 0);
		rightcw.Rotate(90, 0, 0, 1);
		rightcw.Scale(5, 5, 5);
		glBindVertexArray(rightcw.VAO);
		glDrawElements(GL_TRIANGLES, rightcw.faceNum * 3, GL_UNSIGNED_INT, 0);
		rightcw.transReset();
	}
	glFrontFace(GL_CCW);

	glutSwapBuffers();											// 화면에 출력하기
}
GLvoid Reshape(int w, int h)									//--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}
GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'o':
	case 'O':
		glutTimerFunc(10, Timer, key);
		break;
	case 'w':
	case 'W':
		if (!W) {
			I = false;
			W = true;
			A = false;
			S = false;
			D = false;
			head = 'w';
			glutTimerFunc(10, Timer, key);
		}
		break;
	case 'a':
	case 'A':
		if (!A) {
			I = false;
			W = false;
			A = true;
			S = false;
			D = false;
			head = 'a';
			glutTimerFunc(10, Timer, key);
		}
		break;
	case 's':
	case 'S':
		if (!S) {
			I = false;
			W = false;
			A = false;
			S = true;
			D = false;
			head = 's';
			glutTimerFunc(10, Timer, key);
		}
		break;
	case 'd':
	case 'D':
		if (!D) {
			I = false;
			W = false;
			A = false;
			S = false;
			D = true;
			head = 'd';
			glutTimerFunc(10, Timer, key);
		}
		break;
	case 'j':
	case 'J':
		if (!J) {
			J = true;
			robotHead.jumpspeed = 0.05;
			glutTimerFunc(10, Timer, key);
		}
		break;
	case 'y':
		glutTimerFunc(10, Timer, key);
		break;
	case 'Y':
		glutTimerFunc(10, Timer, key);
		break;
	case 'r':
	case 'R':
		R = !R;
		break;
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
	case 'i':
	case 'I':
		c.camreset();
		I = true;
		frontup = J = W = A = S = D = R = false;
		head = 0;
		robotHead.transX = robotHead.transY = robotHead.transZ = 0;
		front.rotX = 0;
		robotHead.rotY = 0;
		for (int i = 0; i < 1; ++i) {
			box1.transX = field(dre);
			box1.transZ = field(dre);
			box1.scaleX = scale(dre);
			box1.scaleZ = scale(dre);
			if (box1.transX + 0.1 * box1.scaleX >= 0.5) {
				--i;
				continue;
			}
			if (box1.transX - 0.1 * box1.scaleX <= -0.5) {
				--i;
				continue;
			}
			if (box1.transZ + 0.1 * box1.scaleZ >= 0.5) {
				--i;
				continue;
			}
			if (box1.transZ - 0.1 * box1.scaleZ <= -0.5) {
				--i;
				continue;
			}
		}
		for (int i = 0; i < 1; ++i) {
			box2.transX = field(dre);
			box2.transZ = field(dre);
			box2.scaleX = scale(dre);
			box2.scaleZ = scale(dre);
			if (box2.transX + 0.1 * box2.scaleX >= 0.5) {
				--i;
				continue;
			}
			if (box2.transX - 0.1 * box2.scaleX <= -0.5) {
				--i;
				continue;
			}
			if (box2.transZ + 0.1 * box2.scaleZ >= 0.5) {
				--i;
				continue;
			}
			if (box2.transZ - 0.1 * box2.scaleZ <= -0.5) {
				--i;
				continue;
			}

			if (box2.transX + 0.1 * box2.scaleX >= box1.transX - 0.1 * box1.scaleX && box2.transX + 0.1 * box2.scaleX <= box1.transX + 0.1 * box1.scaleX) {
				if ((box2.transZ + 0.1 * box2.scaleZ >= box1.transZ - 0.1 * box1.scaleZ && box2.transZ + 0.1 * box2.scaleZ <= box1.transZ + 0.1 * box1.scaleZ) || (box2.transZ - 0.1 * box2.scaleZ <= box1.transZ + 0.1 * box1.scaleZ && box2.transZ - 0.1 * box2.scaleZ >= box1.transZ - 0.1 * box1.scaleZ)) {
					--i;
					continue;
				}
			}
			if (box2.transX - 0.1 * box2.scaleX <= box1.transX + 0.1 * box1.scaleX && box2.transX - 0.1 * box2.scaleX >= box1.transX - 0.1 * box1.scaleX) {
				if ((box2.transZ + 0.1 * box2.scaleZ <= box1.transZ + 0.1 * box1.scaleZ && box2.transZ + 0.1 * box2.scaleZ >= box1.transZ - 0.1 * box1.scaleZ) || (box2.transZ - 0.1 * box2.scaleZ <= box1.transZ + 0.1 * box1.scaleZ && box2.transZ - 0.1 * box2.scaleZ >= box1.transZ - 0.1 * box1.scaleZ)) {
					--i;
					continue;
				}
			}
			if (box2.transZ + 0.1 * box2.scaleZ <= box1.transZ + 0.1 * box1.scaleZ && box2.transZ + 0.1 * box2.scaleZ >= box1.transZ - 0.1 * box1.scaleZ) {
				if ((box2.transX + 0.1 * box2.scaleX <= box1.transX + 0.1 * box1.scaleX && box2.transX + 0.1 * box2.scaleX >= box1.transX - 0.1 * box1.scaleX) || (box2.transX - 0.1 * box2.scaleX <= box1.transX + 0.1 * box1.scaleX && box2.transX - 0.1 * box2.scaleX >= box1.transX - 0.1 * box1.scaleX)) {
					--i;
					continue;
				}
			}
			if (box2.transZ - 0.1 * box2.scaleZ <= box1.transZ + 0.1 * box1.scaleZ && box2.transZ - 0.1 * box2.scaleZ >= box1.transZ - 0.1 * box1.scaleZ) {
				if ((box2.transX + 0.1 * box2.scaleX <= box1.transX + 0.1 * box1.scaleX && box2.transX + 0.1 * box2.scaleX >= box1.transX - 0.1 * box1.scaleX) || (box2.transX - 0.1 * box2.scaleX <= box1.transX + 0.1 * box1.scaleX && box2.transX - 0.1 * box2.scaleX >= box1.transX - 0.1 * box1.scaleX)) {
					--i;
					continue;
				}
			}
			if ((box2.transX >= box1.transX - 0.1 * box1.scaleX && box2.transX <= box1.transX + 0.1 * box1.scaleX) && (box2.transZ >= box1.transZ - 0.1 * box1.scaleZ && box2.transZ <= box1.transZ + 0.1 * box1.scaleZ)) {
				--i;
				continue;
			}
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
	switch (value) {
	case 0:
		robotLArm.rotX++;
		robotRArm.rotX++;
		if (robotLArm.rotX > 30) {
			glutTimerFunc(10, Timer, 1);
		}
		else {
			glutTimerFunc(10, Timer, 0);
		}
		break;
	case 1:
		robotLArm.rotX--;
		robotRArm.rotX--;
		if (robotLArm.rotX < -30) {
			glutTimerFunc(10, Timer, 0);
		}
		else {
			glutTimerFunc(10, Timer, 1);
		}
		break;
	case 'o':
	case 'O':
		if (front.rotX < 90) {
			front.rotX += 0.4;
			if (front.rotX > 77) {
				frontup = true;
			}
			glutTimerFunc(10, Timer, 'o');
		}
		break;
	case 'w':
	case 'W':
		if (!I) {
			if (robotHead.transZ < -0.55) {
				robotHead.transZ = 0.55;
			}
			robotHead.transZ -= 0.003;
			robotHead.rotY = 180;
			if (robotHead.transZ - 0.05 <= box1.transZ + 0.1 * box1.scaleZ && robotHead.transZ - 0.05 >= box1.transZ - 0.1 * box1.scaleZ) {
				if ((robotHead.transX - 0.05 >= box1.transX - 0.1 * box1.scaleX && robotHead.transX - 0.05 <= box1.transX + 0.1 * box1.scaleX)||(robotHead.transX + 0.05 >= box1.transX - 0.1 * box1.scaleX && robotHead.transX + 0.05 <= box1.transX + 0.1 * box1.scaleX)) {
					if (robotHead.transY < 0.1) {
						robotHead.transZ += 0.003;
					}
					else {						// 블럭 위에 있으면
						ON[0] = true;
					}
				}
			}
			if (ON[0]) {
				if (robotHead.transZ + 0.05 < box1.transZ - 0.1 * box1.scaleZ) {
					ON[0] = false;
				}
			}
			if (robotHead.transZ - 0.05 <= box2.transZ + 0.1 * box2.scaleZ && robotHead.transZ - 0.05 >= box2.transZ - 0.1 * box2.scaleZ) {
				if ((robotHead.transX - 0.05 >= box2.transX - 0.1 * box2.scaleX && robotHead.transX - 0.05 <= box2.transX + 0.1 * box2.scaleX) || (robotHead.transX + 0.05 >= box2.transX - 0.1 * box2.scaleX && robotHead.transX + 0.05 <= box2.transX + 0.1 * box2.scaleX)) {
					if (robotHead.transY < 0.1) {
						robotHead.transZ += 0.003;
					}
					else {						// 블럭 위에 있으면
						ON[1] = true;
					}
				}
			}
			if (ON[1]) {
				if (robotHead.transZ + 0.05 < box2.transZ - 0.1 * box2.scaleZ) {
					ON[1] = false;
				}
			}
			if (head == 'w') {
				glutTimerFunc(10, Timer, 'w');
			}
		}
		break;
	case 'a':
	case 'A':
		if (!I) {
			if (robotHead.transX < -0.55) {
				robotHead.transX = 0.55;
			}
			robotHead.transX -= 0.003;
			robotHead.rotY = -90;
			if (robotHead.transX - 0.05 <= box1.transX + 0.1 * box1.scaleX && robotHead.transX - 0.05 >= box1.transX - 0.1 * box1.scaleX) {
				if ((robotHead.transZ - 0.05 >= box1.transZ - 0.1 * box1.scaleZ && robotHead.transZ - 0.05 <= box1.transZ + 0.1 * box1.scaleZ)||(robotHead.transZ + 0.05 >= box1.transZ - 0.1 * box1.scaleZ && robotHead.transZ + 0.05 <= box1.transZ + 0.1 * box1.scaleZ)) {
					if (robotHead.transY < 0.1) {
						robotHead.transX += 0.003;
					}
					else {						// 블럭 위에 있으면
						ON[0] = true;
					}
				}
			}
			if (ON[0]) {
				if (robotHead.transX + 0.05 < box1.transX - 0.1 * box1.scaleX) {
					ON[0] = false;
				}
			}
			if (robotHead.transX - 0.05 <= box2.transX + 0.1 * box2.scaleX && robotHead.transX - 0.05 >= box2.transX - 0.1 * box2.scaleX) {
				if ((robotHead.transZ - 0.05 >= box2.transZ - 0.1 * box2.scaleZ && robotHead.transZ - 0.05 <= box2.transZ + 0.1 * box2.scaleZ) || (robotHead.transZ + 0.05 >= box2.transZ - 0.1 * box2.scaleZ && robotHead.transZ + 0.05 <= box2.transZ + 0.1 * box2.scaleZ)) {
					if (robotHead.transY < 0.1) {
						robotHead.transX += 0.003;
					}
					else {						// 블럭 위에 있으면
						ON[1] = true;
					}
				}
			}
			if (ON[1]) {
				if (robotHead.transX + 0.05 < box2.transX - 0.1 * box2.scaleX) {
					ON[1] = false;
				}
			}
			if (head == 'a') {
				glutTimerFunc(10, Timer, 'a');
			}
		}
		break;
	case 's':
	case 'S':
		if (!I) {
			if (robotHead.transZ > 0.55) {
				robotHead.transZ = -0.55;
			}
			robotHead.transZ += 0.003;
			robotHead.rotY = 0;
			if (robotHead.transZ + 0.05 <= box1.transZ + 0.1 * box1.scaleZ && robotHead.transZ + 0.05 >= box1.transZ - 0.1 * box1.scaleZ) {
				if ((robotHead.transX - 0.05 >= box1.transX - 0.1 * box1.scaleX && robotHead.transX - 0.05 <= box1.transX + 0.1 * box1.scaleX) || (robotHead.transX + 0.05 >= box1.transX - 0.1 * box1.scaleX && robotHead.transX + 0.05 <= box1.transX + 0.1 * box1.scaleX)) {
					if (robotHead.transY < 0.1) {
						robotHead.transZ -= 0.003;
					}
					else {						// 블럭 위에 있으면
						ON[0] = true;
					}
				}
			}
			if (ON[0]) {
				if (robotHead.transZ - 0.05 >= box1.transZ + 0.1 * box1.scaleZ) {
					ON[0] = false;
				}
			}
			if (robotHead.transZ + 0.05 <= box2.transZ + 0.1 * box2.scaleZ && robotHead.transZ + 0.05 >= box2.transZ - 0.1 * box2.scaleZ) {
				if ((robotHead.transX - 0.05 >= box2.transX - 0.1 * box2.scaleX && robotHead.transX - 0.05 <= box2.transX + 0.1 * box2.scaleX) || (robotHead.transX + 0.05 >= box2.transX - 0.1 * box2.scaleX && robotHead.transX + 0.05 <= box2.transX + 0.1 * box2.scaleX)) {
					if (robotHead.transY < 0.1) {
						robotHead.transZ -= 0.003;
					}
					else {						// 블럭 위에 있으면
						ON[1] = true;
					}
				}
			}
			if (ON[1]) {
				if (robotHead.transZ - 0.05 > box2.transZ + 0.1 * box2.scaleZ) {
					ON[1] = false;
				}
			}
			if (head == 's') {
				glutTimerFunc(10, Timer, 's');
			}
		}
		break;
	case 'd':
	case 'D':
		if (!I) {
			if (robotHead.transX > 0.55) {
				robotHead.transX = -0.55;
			}
			robotHead.transX += 0.003;
			robotHead.rotY = 90;
			if (robotHead.transX + 0.05 <= box1.transX + 0.1 * box1.scaleX && robotHead.transX + 0.05 >= box1.transX - 0.1 * box1.scaleX) {
				if ((robotHead.transZ - 0.05>= box1.transZ - 0.1 * box1.scaleZ && robotHead.transZ -0.05<= box1.transZ + 0.1 * box1.scaleZ)||(robotHead.transZ + 0.05 >= box1.transZ - 0.1 * box1.scaleZ && robotHead.transZ + 0.05 <= box1.transZ + 0.1 * box1.scaleZ)) {
					if (robotHead.transY < 0.1) {
						robotHead.transX -= 0.003;
					}
					else {						// 블럭 위에 있으면
						ON[0] = true;
					}
				}
			}
			if (ON[0]) {
				if (robotHead.transX - 0.05 > box1.transX + 0.1 * box1.scaleX) {
					ON[0] = false;
				}
			}
			if (robotHead.transX + 0.05 <= box2.transX + 0.1 * box2.scaleX && robotHead.transX + 0.05 >= box2.transX - 0.1 * box2.scaleX) {
				if ((robotHead.transZ - 0.05 >= box2.transZ - 0.1 * box2.scaleZ && robotHead.transZ - 0.05 <= box2.transZ + 0.1 * box2.scaleZ) || (robotHead.transZ + 0.05 >= box2.transZ - 0.1 * box2.scaleZ && robotHead.transZ + 0.05 <= box2.transZ + 0.1 * box2.scaleZ)) {
					if (robotHead.transY < 0.1) {
						robotHead.transX -= 0.003;
					}
					else {						// 블럭 위에 있으면
						ON[1] = true;
					}
				}
			}
			if (ON[1]) {
				if (robotHead.transX - 0.05 > box2.transX + 0.1 * box2.scaleX) {
					ON[1] = false;
				}
			}
			if (head == 'd') {
				glutTimerFunc(10, Timer, 'd');
			}
		}
		break;
	case 'j':
	case 'J':
		if (ON[0] || ON[1]) {
			if (robotHead.transY + robotHead.jumpspeed > 0.1) {
				robotHead.transY += robotHead.jumpspeed;
				robotHead.jumpspeed -= 0.005;
			}
		}
		else {
			robotHead.transY += robotHead.jumpspeed;
			robotHead.jumpspeed -= 0.005;
		}
		if (robotHead.transY > -0.05) {
			glutTimerFunc(25, Timer, 'j');
		}
		else {
			robotHead.transY = 0;
			J = false;
		}
		break;
	case 'y':
		c.camRot -= 0.2;
		glutTimerFunc(10, Timer, 'y');
		break;
	case 'Y':
		c.camRot += 0.2;
		glutTimerFunc(10, Timer, 'Y');
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
	glGenVertexArrays(1, &front.VAO);
	glBindVertexArray(front.VAO);

	glGenBuffers(1, &front.Color);
	glBindBuffer(GL_ARRAY_BUFFER, front.Color);
	glBufferData(GL_ARRAY_BUFFER, front.vertexNum * 3 * sizeof(float), front.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &front.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, front.VBO);
	glBufferData(GL_ARRAY_BUFFER, front.vertexNum * 3 * sizeof(float), front.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &front.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, front.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, front.faceNum * 3 * sizeof(unsigned int), front.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
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
	glGenVertexArrays(1, &frontcw.VAO);
	glBindVertexArray(frontcw.VAO);

	glGenBuffers(1, &frontcw.Color);
	glBindBuffer(GL_ARRAY_BUFFER, frontcw.Color);
	glBufferData(GL_ARRAY_BUFFER, frontcw.vertexNum * 3 * sizeof(float), frontcw.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &frontcw.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, frontcw.VBO);
	glBufferData(GL_ARRAY_BUFFER, frontcw.vertexNum * 3 * sizeof(float), frontcw.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &frontcw.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, frontcw.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, frontcw.faceNum * 3 * sizeof(unsigned int), frontcw.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &backcw.VAO);
	glBindVertexArray(backcw.VAO);

	glGenBuffers(1, &backcw.Color);
	glBindBuffer(GL_ARRAY_BUFFER, backcw.Color);
	glBufferData(GL_ARRAY_BUFFER, backcw.vertexNum * 3 * sizeof(float), backcw.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &backcw.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, backcw.VBO);
	glBufferData(GL_ARRAY_BUFFER, backcw.vertexNum * 3 * sizeof(float), backcw.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &backcw.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backcw.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, backcw.faceNum * 3 * sizeof(unsigned int), backcw.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &topcw.VAO);
	glBindVertexArray(topcw.VAO);

	glGenBuffers(1, &topcw.Color);
	glBindBuffer(GL_ARRAY_BUFFER, topcw.Color);
	glBufferData(GL_ARRAY_BUFFER, topcw.vertexNum * 3 * sizeof(float), topcw.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &topcw.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, topcw.VBO);
	glBufferData(GL_ARRAY_BUFFER, topcw.vertexNum * 3 * sizeof(float), topcw.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &topcw.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, topcw.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, topcw.faceNum * 3 * sizeof(unsigned int), topcw.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &bottomcw.VAO);
	glBindVertexArray(bottomcw.VAO);

	glGenBuffers(1, &bottomcw.Color);
	glBindBuffer(GL_ARRAY_BUFFER, bottomcw.Color);
	glBufferData(GL_ARRAY_BUFFER, bottomcw.vertexNum * 3 * sizeof(float), bottomcw.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &bottomcw.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, bottomcw.VBO);
	glBufferData(GL_ARRAY_BUFFER, bottomcw.vertexNum * 3 * sizeof(float), bottomcw.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &bottomcw.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bottomcw.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, bottomcw.faceNum * 3 * sizeof(unsigned int), bottomcw.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &leftcw.VAO);
	glBindVertexArray(leftcw.VAO);

	glGenBuffers(1, &leftcw.Color);
	glBindBuffer(GL_ARRAY_BUFFER, leftcw.Color);
	glBufferData(GL_ARRAY_BUFFER, leftcw.vertexNum * 3 * sizeof(float), leftcw.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &leftcw.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, leftcw.VBO);
	glBufferData(GL_ARRAY_BUFFER, leftcw.vertexNum * 3 * sizeof(float), leftcw.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &leftcw.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, leftcw.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, leftcw.faceNum * 3 * sizeof(unsigned int), leftcw.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &rightcw.VAO);
	glBindVertexArray(rightcw.VAO);

	glGenBuffers(1, &rightcw.Color);
	glBindBuffer(GL_ARRAY_BUFFER, rightcw.Color);
	glBufferData(GL_ARRAY_BUFFER, rightcw.vertexNum * 3 * sizeof(float), rightcw.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &rightcw.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, rightcw.VBO);
	glBufferData(GL_ARRAY_BUFFER, rightcw.vertexNum * 3 * sizeof(float), rightcw.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &rightcw.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rightcw.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, rightcw.faceNum * 3 * sizeof(unsigned int), rightcw.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &robotHead.VAO);
	glBindVertexArray(robotHead.VAO);

	glGenBuffers(1, &robotHead.Color);
	glBindBuffer(GL_ARRAY_BUFFER, robotHead.Color);
	glBufferData(GL_ARRAY_BUFFER, robotHead.vertexNum * 3 * sizeof(float), robotHead.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &robotHead.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, robotHead.VBO);
	glBufferData(GL_ARRAY_BUFFER, robotHead.vertexNum * 3 * sizeof(float), robotHead.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &robotHead.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, robotHead.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, robotHead.faceNum * 3 * sizeof(unsigned int), robotHead.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &robotNose.VAO);
	glBindVertexArray(robotNose.VAO);

	glGenBuffers(1, &robotNose.Color);
	glBindBuffer(GL_ARRAY_BUFFER, robotNose.Color);
	glBufferData(GL_ARRAY_BUFFER, robotNose.vertexNum * 3 * sizeof(float), robotNose.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &robotNose.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, robotNose.VBO);
	glBufferData(GL_ARRAY_BUFFER, robotNose.vertexNum * 3 * sizeof(float), robotNose.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &robotNose.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, robotNose.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, robotNose.faceNum * 3 * sizeof(unsigned int), robotNose.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &robotBody.VAO);
	glBindVertexArray(robotBody.VAO);

	glGenBuffers(1, &robotBody.Color);
	glBindBuffer(GL_ARRAY_BUFFER, robotBody.Color);
	glBufferData(GL_ARRAY_BUFFER, robotBody.vertexNum * 3 * sizeof(float), robotBody.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &robotBody.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, robotBody.VBO);
	glBufferData(GL_ARRAY_BUFFER, robotBody.vertexNum * 3 * sizeof(float), robotBody.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &robotBody.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, robotBody.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, robotBody.faceNum * 3 * sizeof(unsigned int), robotBody.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &robotLArm.VAO);
	glBindVertexArray(robotLArm.VAO);

	glGenBuffers(1, &robotLArm.Color);
	glBindBuffer(GL_ARRAY_BUFFER, robotLArm.Color);
	glBufferData(GL_ARRAY_BUFFER, robotLArm.vertexNum * 3 * sizeof(float), robotLArm.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &robotLArm.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, robotLArm.VBO);
	glBufferData(GL_ARRAY_BUFFER, robotLArm.vertexNum * 3 * sizeof(float), robotLArm.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &robotLArm.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, robotLArm.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, robotLArm.faceNum * 3 * sizeof(unsigned int), robotLArm.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &robotRArm.VAO);
	glBindVertexArray(robotRArm.VAO);

	glGenBuffers(1, &robotRArm.Color);
	glBindBuffer(GL_ARRAY_BUFFER, robotRArm.Color);
	glBufferData(GL_ARRAY_BUFFER, robotRArm.vertexNum * 3 * sizeof(float), robotRArm.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &robotRArm.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, robotRArm.VBO);
	glBufferData(GL_ARRAY_BUFFER, robotRArm.vertexNum * 3 * sizeof(float), robotRArm.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &robotRArm.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, robotRArm.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, robotRArm.faceNum * 3 * sizeof(unsigned int), robotRArm.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &robotLLeg.VAO);
	glBindVertexArray(robotLLeg.VAO);

	glGenBuffers(1, &robotLLeg.Color);
	glBindBuffer(GL_ARRAY_BUFFER, robotLLeg.Color);
	glBufferData(GL_ARRAY_BUFFER, robotLLeg.vertexNum * 3 * sizeof(float), robotLLeg.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &robotLLeg.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, robotLLeg.VBO);
	glBufferData(GL_ARRAY_BUFFER, robotLLeg.vertexNum * 3 * sizeof(float), robotLLeg.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &robotLLeg.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, robotLLeg.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, robotLLeg.faceNum * 3 * sizeof(unsigned int), robotLLeg.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &robotRLeg.VAO);
	glBindVertexArray(robotRLeg.VAO);

	glGenBuffers(1, &robotRLeg.Color);
	glBindBuffer(GL_ARRAY_BUFFER, robotRLeg.Color);
	glBufferData(GL_ARRAY_BUFFER, robotRLeg.vertexNum * 3 * sizeof(float), robotRLeg.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &robotRLeg.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, robotRLeg.VBO);
	glBufferData(GL_ARRAY_BUFFER, robotRLeg.vertexNum * 3 * sizeof(float), robotRLeg.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &robotRLeg.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, robotRLeg.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, robotRLeg.faceNum * 3 * sizeof(unsigned int), robotRLeg.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &box1.VAO);
	glBindVertexArray(box1.VAO);

	glGenBuffers(1, &box1.Color);
	glBindBuffer(GL_ARRAY_BUFFER, box1.Color);
	glBufferData(GL_ARRAY_BUFFER, box1.vertexNum * 3 * sizeof(float), box1.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &box1.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, box1.VBO);
	glBufferData(GL_ARRAY_BUFFER, box1.vertexNum * 3 * sizeof(float), box1.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &box1.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, box1.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, box1.faceNum * 3 * sizeof(unsigned int), box1.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &box2.VAO);
	glBindVertexArray(box2.VAO);

	glGenBuffers(1, &box2.Color);
	glBindBuffer(GL_ARRAY_BUFFER, box2.Color);
	glBufferData(GL_ARRAY_BUFFER, box2.vertexNum * 3 * sizeof(float), box2.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &box2.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, box2.VBO);
	glBufferData(GL_ARRAY_BUFFER, box2.vertexNum * 3 * sizeof(float), box2.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &box2.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, box2.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, box2.faceNum * 3 * sizeof(unsigned int), box2.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
}