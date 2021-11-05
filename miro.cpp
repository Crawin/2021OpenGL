// 2021-11-03 미로 길찾기 완료.
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
#define FieldScale 1000
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Timer(int value);
GLvoid MouseMove(int x, int y);
GLvoid setroute();
GLvoid choongdol(char c);

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid InitBuffer();
GLvoid SpecialKey(int key, int x, int y);
GLvoid hwakin(char key, int side);
std::random_device rd;
std::default_random_engine dre(rd());
std::uniform_int_distribution<>uid(0, 3);
std::uniform_real_distribution<>urd(50, 500);

double RED = 0.3f, GREEN = 0.3f, BLUE = 0.5f;
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
	float rotX{}, rotY{}, rotZ{}, transX{}, transY{}, transZ{}, scaleX{ 1.0f }, scaleY{ 1.0f }, scaleZ{ 1.0f }, speed, x, z, maxY{}, r;
	bool up;
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
		up = true;
		maxY = urd(dre);
		std::uniform_real_distribution<>now(25, maxY);
		scaleY = now(dre);
		speed = uid(dre) + 1;
		//--- 1. 전체 버텍스 개수 및 삼각형 개수 세기
		FILE* objFile = fopen("Box.obj", "r");
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
				vertexColor[vertIndex++] = 0.8;
				vertexColor[vertIndex++] = 0.8;
				vertexColor[vertIndex++] = 0.8;
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
		camPos = glm::vec3(0, 0, 0);
		camAt = glm::vec3(0, 0, 0);
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
		atRot = 0;
		camRot = 0;
	}
};

int** field;
int** route;
int** mountain;
int hor, ver;
int blockNum;
bool TAB, R, O, V;
int perspective = 2;
glm::vec2 startmouse(WinX/2, WinY/2);
structure bottom("Plane.obj", 0, 0, 0);
structure start("Plane.obj", 1, 0.3, 1);
structure finish("Plane.obj", 0, 1, 0);
structure player("sphere.obj", 1, 0, 0);
structure* blocks;
structure* mountains;
Cam top(0, 300, 0, glm::vec3(0, 0, -1));
Cam smalltop(0, 100, 0, glm::vec3(0, 0, -1));
Cam FPP(0, 0, 0, glm::vec3(0, 1, 0));
Cam TPP(-20, 0, -50, glm::vec3(0, 1, 0));
Cam Map(0, 100, -300, glm::vec3(0, 1, 0));
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
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	shaderProgram = make_shaderProgram();
	glutDisplayFunc(drawScene);									// 출력 콜백함수의 지정
	glutReshapeFunc(Reshape);									// 다시 그리기 콜백함수 지정
	glutKeyboardFunc(Keyboard);									// 키보드 입력 콜백함수 지정
	glutPassiveMotionFunc(MouseMove);
	glutSpecialFunc(SpecialKey);
	for (int i = 0; i < 1; ++i) {
		std::cout << "가로, 세로 : ";
		std::cin >> hor >> ver;
		if (hor < 5 || ver < 5) {
			std::cout << "최소 6 이상을 입력하셔야 합니다." << std::endl;
			i--;
		}
	}
	field = new int* [ver];
	route = new int* [ver];
	mountain = new int* [ver];
	for (int i = 0; i < ver; ++i) {
		field[i] = new int[hor];
		route[i] = new int[hor];
		mountain[i] = new int[hor];
	}
	std::cout << "1: 1인칭 시점\n2: 맵을 전체를 보는 시점\n3: 3인칭 시점\nTab: 미니맵 전환\nQ: 프로그램 종료\nW: 객체 앞으로 이동\nA: 객체 왼쪽으로 이동\nS: 객체 뒤로 이동\nD: 객체 오른쪽으로 이동\nR: 미로 제작\n"<<
		"O: 직각 투영\nP: 원근 투영\nY: 맵 전체를 보는 시점에서 카메라가 Y축 기준으로 음 방향 회전\ny: 맵 전체를 보는 시점에서 카메라가 Y축 기준으로 양 방향 회전\nX: 맵 전체를 보는 시점에서 카메라가 X축 기준으로 음 방향 회전\nx: 맵 전체를 보는 시점에서 카메라가 X축 기준으로 양 방향 회전\n"<<
		"+: 육면체 이동하는 속도 증가\n-: 육면체 이동하는 속도 감소(최소 한계치 있음)\n<<C: 모든 값 초기화\nV:육면체들 움직임이 멈추고 낮은 높이로 변함\nM: 육면체들이 움직인다\n<-: 맵 전체를 보는 시점에서 카메라가 Y축 기준으로 음 방향 회전(애니메이션 X)\n->: 맵 전체를 보는 시점에서 카메라가 Y축 기준으로 양 방향 회전(애니메이션 X)" << std::endl;
	InitBuffer();
	glutMainLoop();												// 이벤트 처리 시작
}
typedef struct way {
	way* next;
	way* previous;
	int xPos;
	int zPos;
	int cnt;
};

GLvoid setroute() {
	for (int z = 0; z < ver; ++z) {
		for (int x = 0; x < hor; ++x) {
			field[z][x] = 0;
			route[z][x] = 0;
		}
	}	// 0으로 초기화
	way* first = new way;
	first->xPos = 0;
	first->zPos = 0;
	first->previous = NULL;
	first->cnt = 1;
	way* now = first;
	int head = 0;
	int maxcnt = 1;
	route[first->zPos][first->xPos] = 1;
	field[first->zPos][first->xPos] = 1;

	while (!(now->zPos == ver - 1 && now->xPos == hor - 1)) {
		if (now->zPos == 0) {
			if (now->xPos == 0) {		// 볼 곳이 두군데
				if (route[now->zPos][now->xPos + 1] == 1 && route[now->zPos + 1][now->xPos] == 1) { // 사방이 다 막혀있으면
					field[now->zPos][now->xPos] = 0;
					now = now->previous;
					maxcnt--;
					delete now->next;
				}
				else {
					int cango[4] = {};
					if (route[now->zPos][now->xPos + 1] == 0) {
						cango[1] = 1;
					}
					if (route[now->zPos + 1][now->xPos] == 0) {
						cango[2] = 1;
					}
					for (int i = 0; i < 1; ++i) {			// 갈 수 있는 곳중에 한 곳 선택하고
						head = uid(dre);						//	1,2
						if (cango[head] != 1)
							--i;
					}
					switch (head) {
					case 1:
						if (route[now->zPos][now->xPos + 1] != 1) {
							way* newly = new way;
							newly->xPos = now->xPos + 1;
							newly->zPos = now->zPos;
							newly->cnt = ++maxcnt;
							newly->previous = now;
							now->next = newly;
							now = newly;
							route[newly->zPos][newly->xPos] = 1;
							field[newly->zPos][newly->xPos] = newly->cnt;
						}
						break;
					case 2:
						if (route[now->zPos + 1][now->xPos] != 1) {
							way* newly = new way;
							newly->xPos = now->xPos;
							newly->zPos = now->zPos + 1;
							newly->cnt = ++maxcnt;
							newly->previous = now;
							now->next = newly;
							now = newly;
							route[newly->zPos][newly->xPos] = 1;
							field[newly->zPos][newly->xPos] = newly->cnt;
						}
						break;
					}
				}
			}
			else if (now->xPos == hor - 1) { // 볼 곳이 두군데
				if (route[now->zPos + 1][now->xPos] == 1 && route[now->zPos][now->xPos - 1] == 1) { // 사방이 다 막혀있으면
					field[now->zPos][now->xPos] = 0;
					now = now->previous;
					maxcnt--;
					delete now->next;
				}
				else {
					int cango[4] = {};
					if (route[now->zPos + 1][now->xPos] == 0) {
						cango[2] = 1;
					}
					if (route[now->zPos][now->xPos - 1] == 0) {
						cango[3] = 1;
					}
					for (int i = 0; i < 1; ++i) {			// 갈 수 있는 곳중에 한 곳 선택하고
						head = uid(dre);						//	2,3
						if (cango[head] != 1)
							--i;
					}
					switch (head) {
					case 2:
						if (route[now->zPos + 1][now->xPos] != 1) {
							way* newly = new way;
							newly->xPos = now->xPos;
							newly->zPos = now->zPos + 1;
							newly->cnt = ++maxcnt;
							newly->previous = now;
							now->next = newly;
							now = newly;
							route[newly->zPos][newly->xPos] = 1;
							field[newly->zPos][newly->xPos] = newly->cnt;
						}
						break;
					case 3:
						if (route[now->zPos][now->xPos - 1] != 1) {
							way* newly = new way;
							newly->xPos = now->xPos - 1;
							newly->zPos = now->zPos;
							newly->cnt = ++maxcnt;
							newly->previous = now;
							now->next = newly;
							now = newly;
							route[newly->zPos][newly->xPos] = 1;
							field[newly->zPos][newly->xPos] = newly->cnt;
						}
						break;
					}
				}
			}
			else {								// 볼 곳이 세군데
				if (route[now->zPos][now->xPos + 1] == 1 && route[now->zPos + 1][now->xPos] == 1 && route[now->zPos][now->xPos - 1] == 1) { // 사방이 다 막혀있으면
					field[now->zPos][now->xPos] = 0;
					now = now->previous;
					maxcnt--;
					delete now->next;
				}
				else {
					int cango[4] = {};
					if (route[now->zPos][now->xPos + 1] == 0) {
						cango[1] = 1;
					}
					if (route[now->zPos + 1][now->xPos] == 0) {
						cango[2] = 1;
					}
					if (route[now->zPos][now->xPos - 1] == 0) {
						cango[3] = 1;
					}
					for (int i = 0; i < 1; ++i) {			// 갈 수 있는 곳중에 한 곳 선택하고
						head = uid(dre);						// 1,2,3
						if (cango[head] != 1)
							--i;
					}
					switch (head) {
					case 1:
						if (route[now->zPos][now->xPos + 1] != 1) {
							way* newly = new way;
							newly->xPos = now->xPos + 1;
							newly->zPos = now->zPos;
							newly->cnt = ++maxcnt;
							newly->previous = now;
							now->next = newly;
							now = newly;
							route[newly->zPos][newly->xPos] = 1;
							field[newly->zPos][newly->xPos] = newly->cnt;
						}
						break;
					case 2:
						if (route[now->zPos + 1][now->xPos] != 1) {
							way* newly = new way;
							newly->xPos = now->xPos;
							newly->zPos = now->zPos + 1;
							newly->cnt = ++maxcnt;
							newly->previous = now;
							now->next = newly;
							now = newly;
							route[newly->zPos][newly->xPos] = 1;
							field[newly->zPos][newly->xPos] = newly->cnt;
						}
						break;
					case 3:
						if (route[now->zPos][now->xPos - 1] != 1) {
							way* newly = new way;
							newly->xPos = now->xPos - 1;
							newly->zPos = now->zPos;
							newly->cnt = ++maxcnt;
							newly->previous = now;
							now->next = newly;
							now = newly;
							route[newly->zPos][newly->xPos] = 1;
							field[newly->zPos][newly->xPos] = newly->cnt;
						}
						break;
					}
				}
			}
		}
		else if (now->zPos == ver - 1) {
			if (now->xPos == 0) {		// 볼 곳이 두군데
				if (route[now->zPos - 1][now->xPos] == 1 && route[now->zPos][now->xPos + 1] == 1) { // 사방이 다 막혀있으면
					field[now->zPos][now->xPos] = 0;
					now = now->previous;
					maxcnt--;
					delete now->next;
				}
				else {
					int cango[4] = {};
					if (route[now->zPos - 1][now->xPos] == 0) {
						cango[0] = 1;
					}
					if (route[now->zPos][now->xPos + 1] == 0) {
						cango[1] = 1;
					}
					for (int i = 0; i < 1; ++i) {			// 갈 수 있는 곳중에 한 곳 선택하고
						head = uid(dre);		// 0 , 1
						if (cango[head] != 1)
							--i;
					}
					switch (head) {
					case 0:
						if (route[now->zPos - 1][now->xPos] != 1) {
							way* newly = new way;
							newly->xPos = now->xPos;
							newly->zPos = now->zPos - 1;
							newly->cnt = ++maxcnt;
							newly->previous = now;
							now->next = newly;
							now = newly;
							route[newly->zPos][newly->xPos] = 1;
							field[newly->zPos][newly->xPos] = newly->cnt;
						}
						break;
					case 1:
						if (route[now->zPos][now->xPos + 1] != 1) {
							way* newly = new way;
							newly->xPos = now->xPos + 1;
							newly->zPos = now->zPos;
							newly->cnt = ++maxcnt;
							newly->previous = now;
							now->next = newly;
							now = newly;
							route[newly->zPos][newly->xPos] = 1;
							field[newly->zPos][newly->xPos] = newly->cnt;
						}
						break;
					}
				}
			}
			else {// 볼 곳이 세군데
				if (route[now->zPos - 1][now->xPos] == 1 && route[now->zPos][now->xPos + 1] == 1 && route[now->zPos][now->xPos - 1] == 1) { // 사방이 다 막혀있으면
					field[now->zPos][now->xPos] = 0;
					now = now->previous;
					maxcnt--;
					delete now->next;
				}
				else {
					int cango[4] = {};
					if (route[now->zPos - 1][now->xPos] == 0) {
						cango[0] = 1;
					}
					if (route[now->zPos][now->xPos + 1] == 0) {
						cango[1] = 1;
					}
					if (route[now->zPos][now->xPos - 1] == 0) {
						cango[3] = 1;
					}
					for (int i = 0; i < 1; ++i) {			// 갈 수 있는 곳중에 한 곳 선택하고
						head = uid(dre);			// 0, 1, 3
						if (cango[head] != 1)
							--i;
					}
					switch (head) {
					case 0:
						if (route[now->zPos - 1][now->xPos] != 1) {
							way* newly = new way;
							newly->xPos = now->xPos;
							newly->zPos = now->zPos - 1;
							newly->cnt = ++maxcnt;
							newly->previous = now;
							now->next = newly;
							now = newly;
							route[newly->zPos][newly->xPos] = 1;
							field[newly->zPos][newly->xPos] = newly->cnt;
						}
						break;
					case 1:
						if (route[now->zPos][now->xPos + 1] != 1) {
							way* newly = new way;
							newly->xPos = now->xPos + 1;
							newly->zPos = now->zPos;
							newly->cnt = ++maxcnt;
							newly->previous = now;
							now->next = newly;
							now = newly;
							route[newly->zPos][newly->xPos] = 1;
							field[newly->zPos][newly->xPos] = newly->cnt;
						}
						break;
					case 3:
						if (route[now->zPos][now->xPos - 1] != 1) {
							way* newly = new way;
							newly->xPos = now->xPos - 1;
							newly->zPos = now->zPos;
							newly->cnt = ++maxcnt;
							newly->previous = now;
							now->next = newly;
							now = newly;
							route[newly->zPos][newly->xPos] = 1;
							field[newly->zPos][newly->xPos] = newly->cnt;
						}
						break;
					}
				}
			}
		}
		else {
			if (now->xPos == 0) {		// 볼 곳이 세군데
				if (route[now->zPos - 1][now->xPos] == 1 && route[now->zPos][now->xPos + 1] == 1 && route[now->zPos + 1][now->xPos] == 1) { // 사방이 다 막혀있으면
					field[now->zPos][now->xPos] = 0;
					now = now->previous;
					maxcnt--;
					delete now->next;
				}
				else {
					int cango[4] = {};
					if (route[now->zPos - 1][now->xPos] == 0) {
						cango[0] = 1;
					}
					if (route[now->zPos][now->xPos + 1] == 0) {
						cango[1] = 1;
					}
					if (route[now->zPos + 1][now->xPos] == 0) {
						cango[2] = 1;
					}
					for (int i = 0; i < 1; ++i) {			// 갈 수 있는 곳중에 한 곳 선택하고
						head = uid(dre);
						if (cango[head] != 1)
							--i;
					}
					switch (head) {
					case 0:
						if (route[now->zPos - 1][now->xPos] != 1) {
							way* newly = new way;
							newly->xPos = now->xPos;
							newly->zPos = now->zPos - 1;
							newly->cnt = ++maxcnt;
							newly->previous = now;
							now->next = newly;
							now = newly;
							route[newly->zPos][newly->xPos] = 1;
							field[newly->zPos][newly->xPos] = newly->cnt;
						}
						break;
					case 1:
						if (route[now->zPos][now->xPos + 1] != 1) {
							way* newly = new way;
							newly->xPos = now->xPos + 1;
							newly->zPos = now->zPos;
							newly->cnt = ++maxcnt;
							newly->previous = now;
							now->next = newly;
							now = newly;
							route[newly->zPos][newly->xPos] = 1;
							field[newly->zPos][newly->xPos] = newly->cnt;
						}
						break;
					case 2:
						if (route[now->zPos + 1][now->xPos] != 1) {
							way* newly = new way;
							newly->xPos = now->xPos;
							newly->zPos = now->zPos + 1;
							newly->cnt = ++maxcnt;
							newly->previous = now;
							now->next = newly;
							now = newly;
							route[newly->zPos][newly->xPos] = 1;
							field[newly->zPos][newly->xPos] = newly->cnt;
						}
						break;
					}
				}
			}
			else if (now->xPos == hor - 1) {
				if (route[now->zPos - 1][now->xPos] == 1 && route[now->zPos + 1][now->xPos] == 1 && route[now->zPos][now->xPos - 1] == 1) { // 사방이 다 막혀있으면
					field[now->zPos][now->xPos] = 0;
					now = now->previous;
					maxcnt--;
					delete now->next;
				}
				else {
					int cango[4] = {};
					if (route[now->zPos - 1][now->xPos] == 0) {
						cango[0] = 1;
					}
					if (route[now->zPos + 1][now->xPos] == 0) {
						cango[2] = 1;
					}
					if (route[now->zPos][now->xPos - 1] == 0) {
						cango[3] = 1;
					}
					for (int i = 0; i < 1; ++i) {			// 갈 수 있는 곳중에 한 곳 선택하고
						head = uid(dre);
						if (cango[head] != 1)
							--i;
					}
					switch (head) {
					case 0:
						if (route[now->zPos - 1][now->xPos] != 1) {
							way* newly = new way;
							newly->xPos = now->xPos;
							newly->zPos = now->zPos - 1;
							newly->cnt = ++maxcnt;
							newly->previous = now;
							now->next = newly;
							now = newly;
							route[newly->zPos][newly->xPos] = 1;
							field[newly->zPos][newly->xPos] = newly->cnt;
						}
						break;
					case 2:
						if (route[now->zPos + 1][now->xPos] != 1) {
							way* newly = new way;
							newly->xPos = now->xPos;
							newly->zPos = now->zPos + 1;
							newly->cnt = ++maxcnt;
							newly->previous = now;
							now->next = newly;
							now = newly;
							route[newly->zPos][newly->xPos] = 1;
							field[newly->zPos][newly->xPos] = newly->cnt;
						}
						break;
					case 3:
						if (route[now->zPos][now->xPos - 1] != 1) {
							way* newly = new way;
							newly->xPos = now->xPos - 1;
							newly->zPos = now->zPos;
							newly->cnt = ++maxcnt;
							newly->previous = now;
							now->next = newly;
							now = newly;
							route[newly->zPos][newly->xPos] = 1;
							field[newly->zPos][newly->xPos] = newly->cnt;
						}
						break;
					}
				}
			}
			else {				// 가운데
				if (route[now->zPos - 1][now->xPos] == 1 && route[now->zPos][now->xPos + 1] == 1 && route[now->zPos + 1][now->xPos] == 1 && route[now->zPos][now->xPos - 1] == 1) { // 사방이 다 막혀있으면
					field[now->zPos][now->xPos] = 0;
					now = now->previous;
					maxcnt--;
					delete now->next;
				}
				else {
					int cango[4] = {};
					if (route[now->zPos - 1][now->xPos] == 0) {
						cango[0] = 1;
					}
					if (route[now->zPos][now->xPos + 1] == 0) {
						cango[1] = 1;
					}
					if (route[now->zPos + 1][now->xPos] == 0) {
						cango[2] = 1;
					}
					if (route[now->zPos][now->xPos - 1] == 0) {
						cango[3] = 1;
					}
					for (int i = 0; i < 1; ++i) {			// 갈 수 있는 곳중에 한 곳 선택하고
						head = uid(dre);
						if (cango[head] != 1)
							--i;
					}
					head = uid(dre);
					switch (head) {
					case 0:
						if (route[now->zPos - 1][now->xPos] != 1) {
							way* newly = new way;
							newly->xPos = now->xPos;
							newly->zPos = now->zPos - 1;
							newly->cnt = ++maxcnt;
							newly->previous = now;
							now->next = newly;
							now = newly;
							route[newly->zPos][newly->xPos] = 1;
							field[newly->zPos][newly->xPos] = newly->cnt;
						}
						break;
					case 1:
						if (route[now->zPos][now->xPos + 1] != 1) {
							way* newly = new way;
							newly->xPos = now->xPos + 1;
							newly->zPos = now->zPos;
							newly->cnt = ++maxcnt;
							newly->previous = now;
							now->next = newly;
							now = newly;
							route[newly->zPos][newly->xPos] = 1;
							field[newly->zPos][newly->xPos] = newly->cnt;
						}
						break;
					case 2:
						if (route[now->zPos + 1][now->xPos] != 1) {
							way* newly = new way;
							newly->xPos = now->xPos;
							newly->zPos = now->zPos + 1;
							newly->cnt = ++maxcnt;
							newly->previous = now;
							now->next = newly;
							now = newly;
							route[newly->zPos][newly->xPos] = 1;
							field[newly->zPos][newly->xPos] = newly->cnt;
						}
						break;
					case 3:
						if (route[now->zPos][now->xPos - 1] != 1) {
							way* newly = new way;
							newly->xPos = now->xPos - 1;
							newly->zPos = now->zPos;
							newly->cnt = ++maxcnt;
							newly->previous = now;
							now->next = newly;
							now = newly;
							route[newly->zPos][newly->xPos] = 1;
							field[newly->zPos][newly->xPos] = newly->cnt;
						}
						break;
					}
				}
			}
		}
	}
	now->next = NULL;

	//std::cout << "Field" << std::endl;
	for (int z = 0; z < ver; ++z) {
		for (int x = 0; x < hor; ++x) {
			//std::cout << field[z][x] << "\t";
			if (field[z][x] == 0) {
				blockNum++;
			}
		}
		std::cout << std::endl;
	}
	blocks = new structure[blockNum];
	int i = 0;
	for (int z = 0; z < ver; ++z) {
		for (int x = 0; x < hor; ++x) {
			if (field[z][x] == 0) {
				blocks[i].z = z;
				blocks[i].x = x;
				i++;
			}
		}
	}
	//std::cout << "--------------------------------------------------------------------------------------------" << std::endl;
	//std::cout << "route" << std::endl;
	for (int z = 0; z < ver; ++z) {
		for (int x = 0; x < hor; ++x) {
			//std::cout << route[z][x] << "\t";
		}
		//std::cout << std::endl;
	}
	if (hor < ver) {
		player.r = 0.1*FieldScale / hor / 4;
	}
	else {
		player.r = 0.1*FieldScale / ver / 4;
	}
	player.transY = FieldScale * 0.1 / hor * 0.25;
	player.transX = -(FieldScale * 0.1 - FieldScale * 0.1 / hor);
	player.transZ = -(FieldScale * 0.1 - FieldScale * 0.1 / ver);
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
	int ViewLoc = glGetUniformLocation(shaderProgram, "ViewTransform");
	glm::mat4 Proj(1.0f);
	int ProjLoc = glGetUniformLocation(shaderProgram, "ProjectionTransform");

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, WinX, WinY);
	Model = glm::mat4(1.0f);
	glUniformMatrix4fv(ModelLoc, 1, GL_FALSE, glm::value_ptr(Model));

	View = glm::mat4(1.0f);
	switch (perspective) {
	case 1:									// 1인칭
		FPP.setAT(0, 0, 1);
		View = glm::lookAt(FPP.camPos, FPP.camAt, FPP.camUp);
		View = glm::rotate(View, glm::radians(player.rotX), glm::vec3(1, 0, 0));
		View = glm::rotate(View, glm::radians(player.rotY), glm::vec3(0, 1, 0));
		View = glm::translate(View, glm::vec3(-player.transX, -player.transY - 5, -player.transZ));
		break;
	case 2:									// 맵보기
		View = glm::lookAt(Map.camPos, Map.camAt, Map.camUp);
		View = glm::rotate(View, glm::radians(Map.camRot), glm::vec3(0, 1, 0));			// Y 축 기준으로 돌리기 공전
		View = glm::rotate(View, glm::radians(Map.atRot), glm::vec3(1, 0, 0));			// Y 축 기준으로 돌리기 공전
		break;
	case 3:									// 3인칭
		View = glm::lookAt(TPP.camPos, TPP.camAt, TPP.camUp);
		View = glm::rotate(View, glm::radians(player.rotY+25), glm::vec3(0, 1, 0));
		View = glm::translate(View, glm::vec3(-player.transX, -player.transY - 2, -player.transZ));
		break;
	}
	glUniformMatrix4fv(ViewLoc, 1, GL_FALSE, glm::value_ptr(View));
	switch (perspective) {
	case 2:
		Proj = glm::mat4(1.0f);
		if (O) {
			Proj = glm::ortho(-100.f, 100.f, -100.f, 100.f, 0.f, 1000.f);
		}
		else {
			Proj = glm::perspective(glm::radians(45.0f), (float)WinX / (float)WinY, 1.5f, 1000.f);
		}
		break;
	default:
		Proj = glm::mat4(1.0f);
		if (O) {
			Proj = glm::ortho(-30.f, 30.f, -30.f, 30.f, 1.5f, 200.f);
		}
		else {
			Proj = glm::perspective(glm::radians(45.0f), (float)WinX / (float)WinY, 1.5f, 200.f);
		}
		break;
	}
	glUniformMatrix4fv(ProjLoc, 1, GL_FALSE, glm::value_ptr(Proj));
	bottom.Scale(FieldScale, 0, FieldScale);
	glBindVertexArray(bottom.VAO);
	glDrawElements(GL_TRIANGLES, bottom.faceNum * 3, GL_UNSIGNED_INT, 0);
	bottom.transReset();

	start.Translate(-(FieldScale / 10 - FieldScale / 10 / hor), 0.01, -(FieldScale / 10 - FieldScale / 10 / ver));
	start.Scale(FieldScale / hor, 0, FieldScale / ver);
	glBindVertexArray(start.VAO);
	glDrawElements(GL_TRIANGLES, start.faceNum * 3, GL_UNSIGNED_INT, 0);
	start.transReset();

	finish.Translate(FieldScale / 10 - FieldScale / 10 / hor, 0.01, FieldScale / 10 - FieldScale / 10 / ver);
	finish.Scale(FieldScale / hor, 0, FieldScale / ver);
	glBindVertexArray(finish.VAO);
	glDrawElements(GL_TRIANGLES, finish.faceNum * 3, GL_UNSIGNED_INT, 0);
	finish.transReset();

	if (R) {
		for (int i = 0; i < blockNum; ++i) {
			blocks[i].Translate(-FieldScale / 10, 0, -FieldScale / 10);
			blocks[i].Translate(FieldScale * 2 / 10 * blocks[i].x / hor, 0, FieldScale * 2 / 10 * blocks[i].z / ver);
			blocks[i].Translate(FieldScale / 10 / hor, 0.1, FieldScale / 10 / ver);
			blocks[i].Scale(FieldScale / hor, blocks[i].scaleY, FieldScale / ver);
			glBindVertexArray(blocks[i].VAO);
			glDrawElements(GL_TRIANGLES, blocks[i].faceNum * 3, GL_UNSIGNED_INT, 0);
			blocks[i].transReset();
			if (hor < ver) {
				player.Translate(player.transX, player.transY, player.transZ);
				player.Scale(FieldScale / hor / 4, FieldScale / hor / 4, FieldScale / hor / 4);

			}
			else {
				player.Translate(player.transX, player.transY, player.transZ);
				player.Scale(FieldScale / ver / 4, FieldScale / ver / 4, FieldScale / ver / 4);
			}
			glBindVertexArray(player.VAO);
			glDrawElements(GL_TRIANGLES, player.faceNum * 3, GL_UNSIGNED_INT, 0);
			player.transReset();
		}
	}
	else {
		for (int i = 0; i < hor*ver; ++i) {
			mountains[i].Translate(-FieldScale / 10, 0, -FieldScale / 10);
			mountains[i].Translate(FieldScale * 2 / 10 * mountains[i].x / hor, 0, FieldScale * 2 / 10 * mountains[i].z / ver);
			mountains[i].Translate(FieldScale / 10 / hor, 0.1, FieldScale / 10 / ver);
			mountains[i].Scale(FieldScale / hor, mountains[i].scaleY, FieldScale / ver);
			glBindVertexArray(mountains[i].VAO);
			glDrawElements(GL_TRIANGLES, mountains[i].faceNum * 3, GL_UNSIGNED_INT, 0);
			mountains[i].transReset();
		}
	}
	//-------------------------------------------------------------------------------------------------------------------------
	glViewport(WinX * 4 / 5, WinY * 4 / 5, WinX / 5, WinY / 5);
	glDisable(GL_DEPTH_TEST);
	if (TAB) {
		View = glm::lookAt(top.camPos, top.camAt, top.camUp);
		View = glm::rotate(View, glm::radians(180.f), glm::vec3(0, 1, 0));
		glUniformMatrix4fv(ViewLoc, 1, GL_FALSE, glm::value_ptr(View));

		Proj = glm::ortho(-100.f, 100.f, -100.f, 100.f, 0.f, 301.f);
		glUniformMatrix4fv(ProjLoc, 1, GL_FALSE, glm::value_ptr(Proj));
	}
	else {
		View = glm::lookAt(smalltop.camPos, smalltop.camAt, smalltop.camUp);
		View = glm::translate(View, glm::vec3(player.transX, 0, player.transZ));
		View = glm::rotate(View, glm::radians(180.f), glm::vec3(0, 1, 0));
		glUniformMatrix4fv(ViewLoc, 1, GL_FALSE, glm::value_ptr(View));

		Proj = glm::ortho(-30.f, 30.f, -30.f, 30.f, 0.f, 301.f);
		glUniformMatrix4fv(ProjLoc, 1, GL_FALSE, glm::value_ptr(Proj));
	}

	bottom.Scale(FieldScale, 0, FieldScale);
	glBindVertexArray(bottom.VAO);
	glDrawElements(GL_TRIANGLES, bottom.faceNum * 3, GL_UNSIGNED_INT, 0);
	bottom.transReset();

	start.Translate(-(FieldScale / 10 - FieldScale / 10 / hor), 0.01, -(FieldScale / 10 - FieldScale / 10 / ver));
	start.Scale(FieldScale / hor, 0, FieldScale / ver);
	glBindVertexArray(start.VAO);
	glDrawElements(GL_TRIANGLES, start.faceNum * 3, GL_UNSIGNED_INT, 0);
	start.transReset();

	finish.Translate(FieldScale / 10 - FieldScale / 10 / hor, 0.01, FieldScale / 10 - FieldScale / 10 / ver);
	finish.Scale(FieldScale / hor, 0, FieldScale / ver);
	glBindVertexArray(finish.VAO);
	glDrawElements(GL_TRIANGLES, finish.faceNum * 3, GL_UNSIGNED_INT, 0);
	finish.transReset();
	if (R) {
		for (int i = 0; i < blockNum; ++i) {
			blocks[i].Translate(-FieldScale / 10, 0, -FieldScale / 10);
			blocks[i].Translate(FieldScale * 2 / 10 * blocks[i].x / hor, 0, FieldScale * 2 / 10 * blocks[i].z / ver);
			blocks[i].Translate(FieldScale / 10 / hor, 0.1, FieldScale / 10 / ver);
			blocks[i].Scale(FieldScale / hor, 0, FieldScale / ver);
			glBindVertexArray(blocks[i].VAO);
			glDrawElements(GL_TRIANGLES, blocks[i].faceNum * 3, GL_UNSIGNED_INT, 0);
			blocks[i].transReset();
			if (hor < ver) {
				player.Translate(player.transX, player.transY, player.transZ);
				player.Scale(FieldScale / hor / 4, FieldScale / hor / 4, FieldScale / hor / 4);

			}
			else {
				player.Translate(player.transX, player.transY, player.transZ);
				player.Scale(FieldScale / ver / 4, FieldScale / ver / 4, FieldScale / ver / 4);
			}
			glBindVertexArray(player.VAO);
			glDrawElements(GL_TRIANGLES, player.faceNum * 3, GL_UNSIGNED_INT, 0);
			player.transReset();
		}
	}
	else {
		for (int i = 0; i < hor*ver; ++i) {
			mountains[i].Translate(-FieldScale / 10, 0, -FieldScale / 10);
			mountains[i].Translate(FieldScale * 2 / 10 * mountains[i].x / hor, 0, FieldScale * 2 / 10 * mountains[i].z / ver);
			mountains[i].Translate(FieldScale / 10 / hor, 0.1, FieldScale / 10 / ver);
			mountains[i].Scale(FieldScale / hor, 0, FieldScale / ver);
			glBindVertexArray(mountains[i].VAO);
			glDrawElements(GL_TRIANGLES, mountains[i].faceNum * 3, GL_UNSIGNED_INT, 0);
			mountains[i].transReset();

		}
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
	case '1':
		perspective = 1;
		break;
	case '2':
		perspective = 2;
		Map.camDir = glm::normalize(Map.camPos - Map.camAt);							// at -> cam 방향벡터
		Map.camUp = glm::cross(Map.camDir, glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), Map.camDir)));
		break;
	case '3':
		perspective = 3;
		break;
	case '\t':
		TAB = !TAB;
		break;
	case 'q':
	case 'Q':
		for (int i = 0; i < ver; ++i) {
			delete[] field[i];
			delete[] route[i];
			delete[] mountain[i];
		}
		delete[] mountains;
		delete[] field;
		delete[] route;
		glutDestroyWindow(WindowID);
		break;
	case 'w':
	case 'W':
		if (R) {
			player.transZ += glm::cos(glm::radians(player.rotY));
			player.transX += glm::cos(glm::radians(90 + player.rotY));
			std::cout << "(" << player.transX << ", " << player.transZ << ")" << std::endl;
			choongdol('w');
		}
		break;
	case 'a':
	case 'A':
		if (R) {
			player.transZ -= glm::cos(glm::radians(90 + player.rotY));
			player.transX += glm::cos(glm::radians(player.rotY));
			choongdol('a');
		}
		break;
	case 's':
	case 'S':
		if (R) {
			player.transZ -= glm::cos(glm::radians(player.rotY));
			player.transX -= glm::cos(glm::radians(90 + player.rotY));
			choongdol('s');
		}
		break;
	case 'd':
	case 'D':
		if (R) {
			player.transZ += glm::cos(glm::radians(90 + player.rotY));
			player.transX -= glm::cos(glm::radians(player.rotY));
			choongdol('d');
		}
		break;
	case 'R':
	case 'r':
		if (!R) {
			R = true;
			setroute();
			for (int i = 0; i < blockNum; ++i) {
				glGenVertexArrays(1, &blocks[i].VAO);
				glBindVertexArray(blocks[i].VAO);

				glGenBuffers(1, &blocks[i].Color);
				glBindBuffer(GL_ARRAY_BUFFER, blocks[i].Color);
				glBufferData(GL_ARRAY_BUFFER, blocks[i].vertexNum * 3 * sizeof(float), blocks[i].vertexColor, GL_STATIC_DRAW);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(1);

				glGenBuffers(1, &blocks[i].VBO);
				glBindBuffer(GL_ARRAY_BUFFER, blocks[i].VBO);
				glBufferData(GL_ARRAY_BUFFER, blocks[i].vertexNum * 3 * sizeof(float), blocks[i].vertexData, GL_STATIC_DRAW);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);

				glGenBuffers(1, &blocks[i].EBO);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, blocks[i].EBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, blocks[i].faceNum * 3 * sizeof(unsigned int), blocks[i].vertexFace, GL_STATIC_DRAW);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
			}
		}
		break;
	case 'O':
	case 'o':
		O = true;
		break;
	case 'P':
	case 'p':
		O = false;
		break;
	case 'y':
		glutTimerFunc(10, Timer, 'y');
		break;
	case 'Y':
		glutTimerFunc(10, Timer, 'Y');
		break;
	case 'x':
		glutTimerFunc(10, Timer, 'x');
		break;
	case 'X':
		glutTimerFunc(10, Timer, 'X');
		break;
	case '=':
		if (R) {
			for (int i = 0; i < blockNum; ++i) {
				blocks[i].speed++;
			}
		}
		else {
			for (int i = 0; i < hor * ver; ++i) {
				mountains[i].speed++;
				if (mountains[i].speed <= 0) {
					mountains[i].speed--;
				}
			}
		}
		break;
	case '-':
		if (R) {
			for (int i = 0; i < blockNum; ++i) {
				blocks[i].speed--;
				if (blocks[i].speed <= 0) {
					blocks[i].speed++;
				}
			}
		}
		else {
			for (int i = 0; i < hor * ver; ++i) {
				mountains[i].speed--;
				if (mountains[i].speed <= 0) {
					mountains[i].speed++;
				}
			}
		}
		break;
	case 'c':
	case 'C':
		TAB = R = O = V = false;
		perspective = 2;
		Map.camreset();
		if (hor < ver) {
			player.r = 0.1 * FieldScale / hor / 4;
		}
		else {
			player.r = 0.1 * FieldScale / ver / 4;
		}
		player.transY = FieldScale * 0.1 / hor * 0.25;
		player.transX = -(FieldScale * 0.1 - FieldScale * 0.1 / hor);
		player.transZ = -(FieldScale * 0.1 - FieldScale * 0.1 / ver);
		player.x = 0;
		player.z = 0;
		break;
	case 'V':
	case 'v':
		if (!V) {
			glutTimerFunc(10, Timer, 0);
		}
		else {
			if (R) {
				if (hor <= ver) {
					for (int i = 0; i < blockNum; ++i) {
						blocks[i].scaleY = FieldScale / 10 / hor / 2;
					}
				}
				else {
					for (int i = 0; i < blockNum; ++i) {
						blocks[i].scaleY = FieldScale / 10 / ver / 2;
					}
				}
			}
			else {
				if (hor <= ver) {
					for (int i = 0; i < hor * ver; ++i) {
						mountains[i].scaleY = FieldScale / 10 / hor / 2;
					}
				}
				else {
					for (int i = 0; i < hor * ver; ++i) {
						mountains[i].scaleY = FieldScale / 10 / ver / 2;
					}
				}
				
			}
		}
		V = !V;
		break;
	case 'm':
	case 'M':
		glutTimerFunc(10, Timer, 0);
		V = !V;
		break;
	}
	glutPostRedisplay();
}

GLvoid choongdol(char c) {
	switch (c) {
	case 'w':
		if (player.z == 0) {
			if (player.x == 0) {
				if (player.transX - player.r+ FieldScale *0.1 <= 0 || player.transZ - player.r + FieldScale * 0.1 <= 0) {		// 맵 좌상단 밖
					player.transZ -= glm::cos(glm::radians(player.rotY));
					player.transX -= glm::cos(glm::radians(90 + player.rotY));
				}
				hwakin('w', 3);
				hwakin('w', 5);
				hwakin('w', 6);
			}
			else if (player.x == hor - 1) {
				if (player.transX + player.r + FieldScale * 0.1 >=200 || player.transZ - player.r + FieldScale * 0.1 <=0) {		// 맵 우상단 밖
					player.transZ -= glm::cos(glm::radians(player.rotY));
					player.transX -= glm::cos(glm::radians(90 + player.rotY));
				}
				hwakin('w', 9);
				hwakin('w', 7);
				hwakin('w', 6);
			}
			else {
				if (player.transZ - player.r + FieldScale * 0.1 <= 0) {		// 맵 상단 밖
					player.transZ -= glm::cos(glm::radians(player.rotY));
					player.transX -= glm::cos(glm::radians(90 + player.rotY));
				}
				hwakin('w', 3);
				hwakin('w', 5);
				hwakin('w', 6);
				hwakin('w', 7);
				hwakin('w', 9);
			}
		}
		else if (player.z == ver - 1) {
			if (player.x == 0) {
				if (player.transX - player.r + FieldScale * 0.1 <= 0 || player.transZ + player.r + FieldScale * 0.1 >= 200) {
					player.transZ -= glm::cos(glm::radians(player.rotY));
					player.transX -= glm::cos(glm::radians(90 + player.rotY));
				}
				hwakin('w', 12);
				hwakin('w', 1);
				hwakin('w', 3);
			}
			else if (player.x == hor - 1) {
				if (player.transX + player.r + FieldScale * 0.1 >= 200 || player.transZ + player.r + FieldScale * 0.1 >= 200) {
					player.transZ -= glm::cos(glm::radians(player.rotY));
					player.transX -= glm::cos(glm::radians(90 + player.rotY));
				}
				hwakin('w', 12);
				hwakin('w', 11);
				hwakin('w', 9);
			}
			else {
				if (player.transZ + player.r + FieldScale * 0.1 >= 200) {
					player.transZ -= glm::cos(glm::radians(player.rotY));
					player.transX -= glm::cos(glm::radians(90 + player.rotY));
				}
				hwakin('w', 12);
				hwakin('w', 11);
				hwakin('w', 9);
				hwakin('w', 3);
				hwakin('w', 1);
				
			}
		}
		else {
			if (player.x == 0) {
				if (player.transX - player.r + FieldScale * 0.1 <= 0) {
					player.transZ -= glm::cos(glm::radians(player.rotY));
					player.transX -= glm::cos(glm::radians(90 + player.rotY));
				}
				hwakin('w', 12);
				hwakin('w', 1);
				hwakin('w', 3);
				hwakin('w', 5);
				hwakin('w', 6);
			}
			else if (player.x == hor - 1) {
				if (player.transX + player.r + FieldScale * 0.1 >= 200) {
					player.transZ -= glm::cos(glm::radians(player.rotY));
					player.transX -= glm::cos(glm::radians(90 + player.rotY));
				}
				hwakin('w', 12);
				hwakin('w', 11);
				hwakin('w', 9);
				hwakin('w', 7);
				hwakin('w', 6);
			}
			else {
				hwakin('w', 12);
				hwakin('w', 1);
				hwakin('w', 3);
				hwakin('w', 5);
				hwakin('w', 6);
				hwakin('w', 7);
				hwakin('w', 9);
				hwakin('w', 11);
			}
		}
		break;
	case 'a':
		if (player.z == 0) {
			if (player.x == 0) {
				if (player.transX - player.r + FieldScale * 0.1 <= 0 || player.transZ - player.r + FieldScale * 0.1 <= 0) {		// 맵 좌상단 밖
					player.transZ += glm::cos(glm::radians(90 + player.rotY));
					player.transX -= glm::cos(glm::radians(player.rotY));
				}
				hwakin('a', 3);
				hwakin('a', 5);
				hwakin('a', 6);
			}
			else if (player.x == hor - 1) {
				if (player.transX + player.r + FieldScale * 0.1 >= 200 || player.transZ - player.r + FieldScale * 0.1 <= 0) {		// 맵 우상단 밖
					player.transZ += glm::cos(glm::radians(90 + player.rotY));
					player.transX -= glm::cos(glm::radians(player.rotY));
				}
				hwakin('a', 9);
				hwakin('a', 7);
				hwakin('a', 6);
			}
			else {
				if (player.transZ - player.r + FieldScale * 0.1 <= 0) {		// 맵 상단 밖
					player.transZ += glm::cos(glm::radians(90 + player.rotY));
					player.transX -= glm::cos(glm::radians(player.rotY));
				}
				hwakin('a', 3);
				hwakin('a', 5);
				hwakin('a', 6);
				hwakin('a', 7);
				hwakin('a', 9);
			}
		}
		else if (player.z == ver - 1) {
			if (player.x == 0) {
				if (player.transX - player.r + FieldScale * 0.1 <= 0 || player.transZ + player.r + FieldScale * 0.1 >= 200) {
					player.transZ += glm::cos(glm::radians(90 + player.rotY));
					player.transX -= glm::cos(glm::radians(player.rotY));
				}
				hwakin('a', 12);
				hwakin('a', 1);
				hwakin('a', 3);
			}
			else if (player.x == hor - 1) {
				if (player.transX + player.r + FieldScale * 0.1 >= 200 || player.transZ + player.r + FieldScale * 0.1 >= 200) {
					player.transZ += glm::cos(glm::radians(90 + player.rotY));
					player.transX -= glm::cos(glm::radians(player.rotY));
				}
				hwakin('a', 12);
				hwakin('a', 11);
				hwakin('a', 9);
			}
			else {
				if (player.transZ + player.r + FieldScale * 0.1 >= 200) {
					player.transZ += glm::cos(glm::radians(90 + player.rotY));
					player.transX -= glm::cos(glm::radians(player.rotY));
				}
				hwakin('a', 12);
				hwakin('a', 11);
				hwakin('a', 9);
				hwakin('a', 3);
				hwakin('a', 1);

			}
		}
		else {
			if (player.x == 0) {
				if (player.transX - player.r + FieldScale * 0.1 <= 0) {
					player.transZ += glm::cos(glm::radians(90 + player.rotY));
					player.transX -= glm::cos(glm::radians(player.rotY));
				}
				hwakin('a', 12);
				hwakin('a', 1);
				hwakin('a', 3);
				hwakin('a', 5);
				hwakin('a', 6);
			}
			else if (player.x == hor - 1) {
				if (player.transX + player.r + FieldScale * 0.1 >= 200) {
					player.transZ += glm::cos(glm::radians(90 + player.rotY));
					player.transX -= glm::cos(glm::radians(player.rotY));
				}
				hwakin('a', 12);
				hwakin('a', 11);
				hwakin('a', 9);
				hwakin('a', 7);
				hwakin('a', 6);
			}
			else {
				hwakin('a', 12);
				hwakin('a', 1);
				hwakin('a', 3);
				hwakin('a', 5);
				hwakin('a', 6);
				hwakin('a', 7);
				hwakin('a', 9);
				hwakin('a', 11);
			}
		}
		break;
	case 's':
		if (player.z == 0) {
			if (player.x == 0) {
				if (player.transX - player.r + FieldScale * 0.1 <= 0 || player.transZ - player.r + FieldScale * 0.1 <= 0) {		// 맵 좌상단 밖
					player.transZ += glm::cos(glm::radians(player.rotY));
					player.transX += glm::cos(glm::radians(90 + player.rotY));
				}
				hwakin('s', 3);
				hwakin('s', 5);
				hwakin('s', 6);
			}
			else if (player.x == hor - 1) {
				if (player.transX + player.r + FieldScale * 0.1 >= 200 || player.transZ - player.r + FieldScale * 0.1 <= 0) {		// 맵 우상단 밖
					player.transZ += glm::cos(glm::radians(player.rotY));
					player.transX += glm::cos(glm::radians(90 + player.rotY));
				}
				hwakin('s', 9);
				hwakin('s', 7);
				hwakin('s', 6);
			}
			else {
				if (player.transZ - player.r + FieldScale * 0.1 <= 0) {		// 맵 상단 밖
					player.transZ += glm::cos(glm::radians(player.rotY));
					player.transX += glm::cos(glm::radians(90 + player.rotY));
				}
				hwakin('s', 3);
				hwakin('s', 5);
				hwakin('s', 6);
				hwakin('s', 7);
				hwakin('s', 9);
			}
		}
		else if (player.z == ver - 1) {
			if (player.x == 0) {
				if (player.transX - player.r + FieldScale * 0.1 <= 0 || player.transZ + player.r + FieldScale * 0.1 >= 200) {
					player.transZ += glm::cos(glm::radians(player.rotY));
					player.transX += glm::cos(glm::radians(90 + player.rotY));
				}
				hwakin('s', 12);
				hwakin('s', 1);
				hwakin('s', 3);
			}
			else if (player.x == hor - 1) {
				if (player.transX + player.r + FieldScale * 0.1 >= 200 || player.transZ + player.r + FieldScale * 0.1 >= 200) {
					player.transZ += glm::cos(glm::radians(player.rotY));
					player.transX += glm::cos(glm::radians(90 + player.rotY));
				}
				hwakin('s', 12);
				hwakin('s', 11);
				hwakin('s', 9);
			}
			else {
				if (player.transZ + player.r + FieldScale * 0.1 >= 200) {
					player.transZ += glm::cos(glm::radians(player.rotY));
					player.transX += glm::cos(glm::radians(90 + player.rotY));
				}
				hwakin('s', 12);
				hwakin('s', 11);
				hwakin('s', 9);
				hwakin('s', 3);
				hwakin('s', 1);

			}
		}
		else {
			if (player.x == 0) {
				if (player.transX - player.r + FieldScale * 0.1 <= 0) {
					player.transZ += glm::cos(glm::radians(player.rotY));
					player.transX += glm::cos(glm::radians(90 + player.rotY));
				}
				hwakin('s', 12);
				hwakin('s', 1);
				hwakin('s', 3);
				hwakin('s', 5);
				hwakin('s', 6);
			}
			else if (player.x == hor - 1) {
				if (player.transX + player.r + FieldScale * 0.1 >= 200) {
					player.transZ += glm::cos(glm::radians(player.rotY));
					player.transX += glm::cos(glm::radians(90 + player.rotY));
				}
				hwakin('s', 12);
				hwakin('s', 11);
				hwakin('s', 9);
				hwakin('s', 7);
				hwakin('s', 6);
			}
			else {
				hwakin('s', 12);
				hwakin('s', 1);
				hwakin('s', 3);
				hwakin('s', 5);
				hwakin('s', 6);
				hwakin('s', 7);
				hwakin('s', 9);
				hwakin('s', 11);
			}
		}
		break;
	case 'd':
		if (player.z == 0) {
			if (player.x == 0) {
				if (player.transX - player.r + FieldScale * 0.1 <= 0 || player.transZ - player.r + FieldScale * 0.1 <= 0) {		// 맵 좌상단 밖
					player.transZ -= glm::cos(glm::radians(90 + player.rotY));
					player.transX += glm::cos(glm::radians(player.rotY));
				}
				hwakin('d', 3);
				hwakin('d', 5);
				hwakin('d', 6);
			}
			else if (player.x == hor - 1) {
				if (player.transX + player.r + FieldScale * 0.1 >= 200 || player.transZ - player.r + FieldScale * 0.1 <= 0) {		// 맵 우상단 밖
					player.transZ -= glm::cos(glm::radians(90 + player.rotY));
					player.transX += glm::cos(glm::radians(player.rotY));
				}
				hwakin('d', 9);
				hwakin('d', 7);
				hwakin('d', 6);
			}
			else {
				if (player.transZ - player.r + FieldScale * 0.1 <= 0) {		// 맵 상단 밖
					player.transZ -= glm::cos(glm::radians(90 + player.rotY));
					player.transX += glm::cos(glm::radians(player.rotY));
				}
				hwakin('d', 3);
				hwakin('d', 5);
				hwakin('d', 6);
				hwakin('d', 7);
				hwakin('d', 9);
			}
		}
		else if (player.z == ver - 1) {
			if (player.x == 0) {
				if (player.transX - player.r + FieldScale * 0.1 <= 0 || player.transZ + player.r + FieldScale * 0.1 >= 200) {
					player.transZ -= glm::cos(glm::radians(90 + player.rotY));
					player.transX += glm::cos(glm::radians(player.rotY));
				}
				hwakin('d', 12);
				hwakin('d', 1);
				hwakin('d', 3);
			}
			else if (player.x == hor - 1) {
				if (player.transX + player.r + FieldScale * 0.1 >= 200 || player.transZ + player.r + FieldScale * 0.1 >= 200) {
					player.transZ -= glm::cos(glm::radians(90 + player.rotY));
					player.transX += glm::cos(glm::radians(player.rotY));
				}
				hwakin('d', 12);
				hwakin('d', 11);
				hwakin('d', 9);
			}
			else {
				if (player.transZ + player.r + FieldScale * 0.1 >= 200) {
					player.transZ -= glm::cos(glm::radians(90 + player.rotY));
					player.transX += glm::cos(glm::radians(player.rotY));
				}
				hwakin('d', 12);
				hwakin('d', 11);
				hwakin('d', 9);
				hwakin('d', 3);
				hwakin('d', 1);

			}
		}
		else {
			if (player.x == 0) {
				if (player.transX - player.r + FieldScale * 0.1 <= 0) {
					player.transZ -= glm::cos(glm::radians(90 + player.rotY));
					player.transX += glm::cos(glm::radians(player.rotY));
				}
				hwakin('d', 12);
				hwakin('d', 1);
				hwakin('d', 3);
				hwakin('d', 5);
				hwakin('d', 6);
			}
			else if (player.x == hor - 1) {
				if (player.transX + player.r + FieldScale * 0.1 >= 200) {
					player.transZ -= glm::cos(glm::radians(90 + player.rotY));
					player.transX += glm::cos(glm::radians(player.rotY));
				}
				hwakin('d', 12);
				hwakin('d', 11);
				hwakin('d', 9);
				hwakin('d', 7);
				hwakin('d', 6);
			}
			else {
				hwakin('d', 12);
				hwakin('d', 1);
				hwakin('d', 3);
				hwakin('d', 5);
				hwakin('d', 6);
				hwakin('d', 7);
				hwakin('d', 9);
				hwakin('d', 11);
			}
		}
		break;
	}
	
}

GLvoid hwakin(char key, int side) {
	switch (side) {
	case 12:
		if (player.transX + player.r + FieldScale * 0.1 < FieldScale * 0.2 * (player.x + 1) / hor&& player.transX - player.r + FieldScale * 0.1 >= FieldScale * 0.2 * player.x / hor && player.transZ - player.r + FieldScale * 0.1 <= FieldScale * 0.2 * (player.z / ver)) {		// [0][0] -> [0][1]
			if (field[(int)player.z-1][(int)player.x] != 0) {
				--player.z;
			}
			else {
				switch (key) {
				case 'w':
					player.transZ -= glm::cos(glm::radians(player.rotY));
					player.transX -= glm::cos(glm::radians(90 + player.rotY));
					break;
				case 'a':
					player.transZ += glm::cos(glm::radians(90 + player.rotY));
					player.transX -= glm::cos(glm::radians(player.rotY));
					break;
				case 's':
					player.transZ += glm::cos(glm::radians(player.rotY));
					player.transX += glm::cos(glm::radians(90 + player.rotY));
					break;
				case 'd':
					player.transZ -= glm::cos(glm::radians(90 + player.rotY));
					player.transX += glm::cos(glm::radians(player.rotY));
					break;
				}
			}
		}
		break;
	case 3:
		if (player.transX + player.r + FieldScale * 0.1 >= FieldScale * 0.2 * (player.x + 1) / hor && player.transZ + player.r + FieldScale * 0.1 < FieldScale * 0.2 * ((player.z) + 1) / ver&& player.transZ - player.r + FieldScale * 0.1 > FieldScale * 0.2 * (player.z / ver)) {		// [0][0] -> [0][1]
			if (field[(int)player.z][(int)player.x + 1] != 0) {
				++player.x;
			}
			else {
				switch (key) {
				case 'w':
					player.transZ -= glm::cos(glm::radians(player.rotY));
					player.transX -= glm::cos(glm::radians(90 + player.rotY));
					break;
				case 'a':
					player.transZ += glm::cos(glm::radians(90 + player.rotY));
					player.transX -= glm::cos(glm::radians(player.rotY));
					break;
				case 's':
					player.transZ += glm::cos(glm::radians(player.rotY));
					player.transX += glm::cos(glm::radians(90 + player.rotY));
					break;
				case 'd':
					player.transZ -= glm::cos(glm::radians(90 + player.rotY));
					player.transX += glm::cos(glm::radians(player.rotY));
					break;
				}
			}
		}
		break;
	case 6:
		if (player.transX + player.r + FieldScale * 0.1 < FieldScale * 0.2 * (player.x + 1) / hor && player.transX - player.r + FieldScale * 0.1 > FieldScale * 0.2 * player.x / hor && player.transZ + player.r + FieldScale * 0.1 >= FieldScale * 0.2 * ((player.z) + 1) / ver) {		// [0][0] -> [0][1]
			if (field[(int)player.z + 1][(int)player.x] != 0) {
				++player.z;
			}
			else {
				switch (key) {
				case 'w':
					player.transZ -= glm::cos(glm::radians(player.rotY));
					player.transX -= glm::cos(glm::radians(90 + player.rotY));
					break;
				case 'a':
					player.transZ += glm::cos(glm::radians(90 + player.rotY));
					player.transX -= glm::cos(glm::radians(player.rotY));
					break;
				case 's':
					player.transZ += glm::cos(glm::radians(player.rotY));
					player.transX += glm::cos(glm::radians(90 + player.rotY));
					break;
				case 'd':
					player.transZ -= glm::cos(glm::radians(90 + player.rotY));
					player.transX += glm::cos(glm::radians(player.rotY));
					break;
				}
			}
		}
		break;
	case 9:
		if (player.transX - player.r + FieldScale * 0.1 <= FieldScale * 0.2 * player.x / hor && player.transZ + player.r + FieldScale * 0.1 < FieldScale * 0.2 * ((player.z) + 1) / ver&& player.transZ - player.r + FieldScale * 0.1 > FieldScale * 0.2 * (player.z / ver)) {		// [0][0] -> [0][1]
			if (field[(int)player.z][(int)player.x - 1] != 0) {
				--player.x;
			}
			else {
				switch (key) {
				case 'w':
					player.transZ -= glm::cos(glm::radians(player.rotY));
					player.transX -= glm::cos(glm::radians(90 + player.rotY));
					break;
				case 'a':
					player.transZ += glm::cos(glm::radians(90 + player.rotY));
					player.transX -= glm::cos(glm::radians(player.rotY));
					break;
				case 's':
					player.transZ += glm::cos(glm::radians(player.rotY));
					player.transX += glm::cos(glm::radians(90 + player.rotY));
					break;
				case 'd':
					player.transZ -= glm::cos(glm::radians(90 + player.rotY));
					player.transX += glm::cos(glm::radians(player.rotY));
					break;
				}
			}
		}
		break;
	case 1:
		if (player.transX + player.r + FieldScale * 0.1 >= FieldScale * 0.2 * (player.x + 1) / hor && player.transZ - player.r + FieldScale * 0.1 <= FieldScale * 0.2 * (player.z / ver)) {		// [0][0] -> [0][1]
			if (field[(int)player.z - 1][(int)player.x+1] != 0) {
				--player.z;
				++player.x;
			}
			else {
				switch (key) {
				case 'w':
					player.transZ -= glm::cos(glm::radians(player.rotY));
					player.transX -= glm::cos(glm::radians(90 + player.rotY));
					break;
				case 'a':
					player.transZ += glm::cos(glm::radians(90 + player.rotY));
					player.transX -= glm::cos(glm::radians(player.rotY));
					break;
				case 's':
					player.transZ += glm::cos(glm::radians(player.rotY));
					player.transX += glm::cos(glm::radians(90 + player.rotY));
					break;
				case 'd':
					player.transZ -= glm::cos(glm::radians(90 + player.rotY));
					player.transX += glm::cos(glm::radians(player.rotY));
					break;
				}
			}
		}
		break;
	case 5:
		if (player.transX + player.r + FieldScale * 0.1 >= FieldScale * 0.2 * (player.x + 1) / hor && player.transZ + player.r + FieldScale * 0.1 >= FieldScale * 0.2 * ((player.z+1) / ver)) {		// [0][0] -> [0][1]
			if (field[(int)player.z + 1][(int)player.x + 1] != 0) {
				++player.z;
				++player.x;
			}
			else {
				switch (key) {
				case 'w':
					player.transZ -= glm::cos(glm::radians(player.rotY));
					player.transX -= glm::cos(glm::radians(90 + player.rotY));
					break;
				case 'a':
					player.transZ += glm::cos(glm::radians(90 + player.rotY));
					player.transX -= glm::cos(glm::radians(player.rotY));
					break;
				case 's':
					player.transZ += glm::cos(glm::radians(player.rotY));
					player.transX += glm::cos(glm::radians(90 + player.rotY));
					break;
				case 'd':
					player.transZ -= glm::cos(glm::radians(90 + player.rotY));
					player.transX += glm::cos(glm::radians(player.rotY));
					break;
				}
			}
		}
		break;
	case 7:
		if (player.transX - player.r + FieldScale * 0.1 <= FieldScale * 0.2 * player.x / hor && player.transZ + player.r + FieldScale * 0.1 >= FieldScale * 0.2 * ((player.z+1) / ver)) {		// [0][0] -> [0][1]
			if (field[(int)player.z + 1][(int)player.x - 1] != 0) {
				++player.z;
				--player.x;
			}
			else {
				switch (key) {
				case 'w':
					player.transZ -= glm::cos(glm::radians(player.rotY));
					player.transX -= glm::cos(glm::radians(90 + player.rotY));
					break;
				case 'a':
					player.transZ += glm::cos(glm::radians(90 + player.rotY));
					player.transX -= glm::cos(glm::radians(player.rotY));
					break;
				case 's':
					player.transZ += glm::cos(glm::radians(player.rotY));
					player.transX += glm::cos(glm::radians(90 + player.rotY));
					break;
				case 'd':
					player.transZ -= glm::cos(glm::radians(90 + player.rotY));
					player.transX += glm::cos(glm::radians(player.rotY));
					break;
				}
			}
		}
		break;
	case 11:
		if (player.transX - player.r + FieldScale * 0.1 <= FieldScale * 0.2 * player.x / hor && player.transZ - player.r + FieldScale * 0.1 <= FieldScale * 0.2 * (player.z / ver)) {		// [0][0] -> [0][1]
			if (field[(int)player.z - 1][(int)player.x - 1] != 0) {
				--player.z;
				--player.x;
			}
			else {
				switch (key) {
				case 'w':
					player.transZ -= glm::cos(glm::radians(player.rotY));
					player.transX -= glm::cos(glm::radians(90 + player.rotY));
					break;
				case 'a':
					player.transZ += glm::cos(glm::radians(90 + player.rotY));
					player.transX -= glm::cos(glm::radians(player.rotY));
					break;
				case 's':
					player.transZ += glm::cos(glm::radians(player.rotY));
					player.transX += glm::cos(glm::radians(90 + player.rotY));
					break;
				case 'd':
					player.transZ -= glm::cos(glm::radians(90 + player.rotY));
					player.transX += glm::cos(glm::radians(player.rotY));
					break;
				}
			}
		}
		break;
	}
}

GLvoid SpecialKey(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_LEFT:
		Map.camRot -= 4;
		break;
	case GLUT_KEY_RIGHT:
		Map.camRot += 4;
		break;
	}
	glutPostRedisplay();
}

GLvoid Timer(int value) {
	switch (value) {
	case 0:												// 장애물 높이 타이머
		if (R) {
			for (int i = 0; i < blockNum; ++i) {
				if (blocks[i].up) {
					blocks[i].scaleY += blocks[i].speed;
				}
				else {
					blocks[i].scaleY -= blocks[i].speed;
				}
				if (hor <= ver) {
					if (blocks[i].scaleY <= FieldScale / 10 / hor / 2) {
						blocks[i].up = true;
					}
					else if (blocks[i].scaleY >= blocks[i].maxY) {
						blocks[i].up = false;
					}
				}
				else {
					if (blocks[i].scaleY <= FieldScale / 10 / ver / 2) {
						blocks[i].up = true;
					}
					else if (blocks[i].scaleY >= blocks[i].maxY) {
						blocks[i].up = false;
					}
				}
			}
		}
		else {
			for (int i = 0; i < hor * ver; ++i) {
				if (mountains[i].up) {
					mountains[i].scaleY += mountains[i].speed;
				}
				else {
					mountains[i].scaleY -= mountains[i].speed;
				}

				if (hor <= ver) {
					if (mountains[i].scaleY <= FieldScale / 10 / hor / 2) {
						mountains[i].up = true;
					}
					else if (mountains[i].scaleY >= mountains[i].maxY) {
						mountains[i].up = false;
					}
				}
				else {
					if (mountains[i].scaleY <= FieldScale / 10 / ver / 2) {
						mountains[i].up = true;
					}
					else if (mountains[i].scaleY >= mountains[i].maxY) {
						mountains[i].up = false;
					}
				}
			}
		}
		if (V) {
			glutTimerFunc(10, Timer, 0);
		}
		break;
	case 'y':
		Map.camRot += 1;
		glutTimerFunc(10, Timer, 'y');
		break;
	case 'Y':
		Map.camRot -= 1;
		glutTimerFunc(10, Timer, 'Y');
		break;
	case 'x':
		Map.atRot += 1;
		glutTimerFunc(10, Timer, 'x');
		break;
	case 'X':
		Map.atRot -= 1;
		glutTimerFunc(10, Timer, 'X');
	}
	glutPostRedisplay();
}

GLvoid MouseMove(int x, int y) {
	player.rotY = (x - WinX / 2) * 3 / 5;
	player.rotX = -(y - WinY / 2) * 3 / 10;
	//std::cout << "(" << player.rotY << ", " << player.rotX << ")" << std::endl;
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
	glGenVertexArrays(1, &start.VAO);
	glBindVertexArray(start.VAO);

	glGenBuffers(1, &start.Color);
	glBindBuffer(GL_ARRAY_BUFFER, start.Color);
	glBufferData(GL_ARRAY_BUFFER, start.vertexNum * 3 * sizeof(float), start.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &start.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, start.VBO);
	glBufferData(GL_ARRAY_BUFFER, start.vertexNum * 3 * sizeof(float), start.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &start.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, start.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, start.faceNum * 3 * sizeof(unsigned int), start.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &finish.VAO);
	glBindVertexArray(finish.VAO);

	glGenBuffers(1, &finish.Color);
	glBindBuffer(GL_ARRAY_BUFFER, finish.Color);
	glBufferData(GL_ARRAY_BUFFER, finish.vertexNum * 3 * sizeof(float), finish.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &finish.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, finish.VBO);
	glBufferData(GL_ARRAY_BUFFER, finish.vertexNum * 3 * sizeof(float), finish.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &finish.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, finish.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, finish.faceNum * 3 * sizeof(unsigned int), finish.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &player.VAO);
	glBindVertexArray(player.VAO);

	glGenBuffers(1, &player.Color);
	glBindBuffer(GL_ARRAY_BUFFER, player.Color);
	glBufferData(GL_ARRAY_BUFFER, player.vertexNum * 3 * sizeof(float), player.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &player.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, player.VBO);
	glBufferData(GL_ARRAY_BUFFER, player.vertexNum * 3 * sizeof(float), player.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &player.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, player.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, player.faceNum * 3 * sizeof(unsigned int), player.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------
	if (R) {
		for (int i = 0; i < blockNum; ++i) {
			glGenVertexArrays(1, &blocks[i].VAO);
			glBindVertexArray(blocks[i].VAO);

			glGenBuffers(1, &blocks[i].Color);
			glBindBuffer(GL_ARRAY_BUFFER, blocks[i].Color);
			glBufferData(GL_ARRAY_BUFFER, blocks[i].vertexNum * 3 * sizeof(float), blocks[i].vertexColor, GL_STATIC_DRAW);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);

			glGenBuffers(1, &blocks[i].VBO);
			glBindBuffer(GL_ARRAY_BUFFER, blocks[i].VBO);
			glBufferData(GL_ARRAY_BUFFER, blocks[i].vertexNum * 3 * sizeof(float), blocks[i].vertexData, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glGenBuffers(1, &blocks[i].EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, blocks[i].EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, blocks[i].faceNum * 3 * sizeof(unsigned int), blocks[i].vertexFace, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
		}
	}
	else {
		mountains = new structure[hor * ver];
		int i = 0;
		for (int z = 0; z < ver; ++z) {
			for (int x = 0; x < hor; ++x) {
				mountains[i].z = z;
				mountains[i].x = x;
				i++;
			}
		}
		for (int i = 0; i < hor * ver; ++i) {
			glGenVertexArrays(1, &mountains[i].VAO);
			glBindVertexArray(mountains[i].VAO);

			glGenBuffers(1, &mountains[i].Color);
			glBindBuffer(GL_ARRAY_BUFFER, mountains[i].Color);
			glBufferData(GL_ARRAY_BUFFER, mountains[i].vertexNum * 3 * sizeof(float), mountains[i].vertexColor, GL_STATIC_DRAW);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);

			glGenBuffers(1, &mountains[i].VBO);
			glBindBuffer(GL_ARRAY_BUFFER, mountains[i].VBO);
			glBufferData(GL_ARRAY_BUFFER, mountains[i].vertexNum * 3 * sizeof(float), mountains[i].vertexData, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glGenBuffers(1, &mountains[i].EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mountains[i].EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, mountains[i].faceNum * 3 * sizeof(unsigned int), mountains[i].vertexFace, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
		}
	}
}