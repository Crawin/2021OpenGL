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
#define WinX 600
#define WinY 600

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Timer(int value);
GLvoid SpecialKeyboard(int key, int x, int y);

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid InitBuffer();

double RED = 1.0f, GREEN = 1.0f, BLUE = 1.0f;
GLuint VAO, VBO;
GLuint shaderProgram, WindowID;
int input;
bool C;
float Ydegree = -30.0f;
int vertexNum = 0;
int faceNum = 0;
char keyboard;

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
	float rotX = 0.f;
	float rotY= 0.f;

	structure(const char* FileName) {
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

		std::random_device rd;
		std::default_random_engine dre(rd());
		std::uniform_real_distribution<>uid(0, 100);

		while (!feof(objFile)) {
			fscanf(objFile, "%s", count);
			if (count[0] == 'v' && count[1] == '\0') {
				fscanf(objFile, "%f %f %f", &vertexData[vertIndex], &vertexData[vertIndex + 1], &vertexData[vertIndex + 2]);
				vertexColor[vertIndex++] = uid(dre) / 100.f;
				vertexColor[vertIndex++] = uid(dre) / 100.f;
				vertexColor[vertIndex++] = uid(dre) / 100.f;
			}
			else if (count[0] == 'f' && count[1] == '\0') {
				fscanf(objFile, "%d %d %d", &vertexFace[faceIndex], &vertexFace[faceIndex + 1], &vertexFace[faceIndex + 2]);
				vertexFace[faceIndex++]--;
				vertexFace[faceIndex++]--;
				vertexFace[faceIndex++]--;
			}
			memset(count, '\0', sizeof(count)); // 배열 초기화
		}
		fclose(objFile);
	}

	void translate(float x, float y, float z) {
		unsigned int transformLocation = glGetUniformLocation(shaderProgram, "transform");
		trans = glm::translate(trans, glm::vec3(x, y, z));
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(trans));
	}
	void rotate(float degree, float x, float y, float z) {
		unsigned int transformLocation = glGetUniformLocation(shaderProgram, "transform");
		trans = glm::rotate(trans, glm::radians(degree), glm::vec3(x, y, z));
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(trans));
	}
};

float xyz[]{
	-1,0,0,		0,0,0,
	1,0,0,		0,0,0,

	0,-1,0,		0,0,0,
	0,1,0,		0,0,0,

	0,0,-1,		0,0,0,
	0,0,1,		0,0,0
};

structure box("Box.obj");
structure sphere("Sphere.obj");
structure torus("Torus.obj");
structure cone("Cone.obj");

void main(int argc, char** argv)								//--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);										// glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);				// 디스플레이 모드 설정
	glutInitWindowPosition(0, 0);								// 윈도우의 위치 지정
	glutInitWindowSize(WinX, WinY);								// 윈도우의 크기 지정
	WindowID = glutCreateWindow("13");								// 윈도우 생성 (윈도우 이름)

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
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	shaderProgram = make_shaderProgram();
	glutDisplayFunc(drawScene);									// 출력 콜백함수의 지정
	glutReshapeFunc(Reshape);									// 다시 그리기 콜백함수 지정
	glutKeyboardFunc(Keyboard);									// 키보드 입력 콜백함수 지정
	glutMainLoop();												// 이벤트 처리 시작
}

GLvoid drawScene()												//--- 콜백 함수: 그리기 콜백 함수
{
	glUseProgram(shaderProgram);
	//--- 변경된 배경색 설정
	glClearColor(RED, GREEN, BLUE, 1.0f);											// 바탕색을 변경
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);								// 설정된 색으로 전체를 칠하기
	glm::mat4 RR(1.0f);
	unsigned int transformLocation = glGetUniformLocation(shaderProgram, "transform");
	RR = glm::rotate(RR, glm::radians(30.0f), glm::vec3(1.0, 0.0, 0.0));
	RR = glm::rotate(RR, glm::radians(Ydegree), glm::vec3(0.0, 1.0, 0.0));
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(RR));
	glLineWidth(2);
	glBindVertexArray(VAO);			// 축
	glDrawArrays(GL_LINES, 0, 6);

	if (C) {
		cone.rotate(30, 1, 0, 0);
		cone.rotate(Ydegree, 0, 1, 0);
		cone.rotate(cone.rotX, 1, 0, 0);
		cone.translate(-0.5, 0, 0);
		cone.rotate(cone.rotY, 0, 1, 0);
		glBindVertexArray(cone.VAO);
		glDrawElements(GL_TRIANGLES, cone.faceNum * 3, GL_UNSIGNED_INT, 0);
		cone.rotate(-cone.rotY, 0, 1, 0);
		cone.translate(0.5, 0, 0);
		cone.rotate(-cone.rotX, 1, 0, 0);
		cone.rotate(-Ydegree, 0, 1, 0);
		cone.rotate(-30, 1, 0, 0);

		torus.rotate(30, 1, 0, 0);
		torus.rotate(Ydegree, 0, 1, 0);
		torus.rotate(torus.rotX, 1, 0, 0);
		torus.translate(0.5, 0, 0);
		torus.rotate(torus.rotY, 0, 1, 0);
		glBindVertexArray(torus.VAO);
		glDrawElements(GL_TRIANGLES, torus.faceNum * 3, GL_UNSIGNED_INT, 0);
		torus.rotate(-torus.rotY, 0, 1, 0);
		torus.translate(-0.5, 0, 0);
		torus.rotate(-torus.rotX, 1, 0, 0);
		torus.rotate(-Ydegree, 0, 1, 0);
		torus.rotate(-30, 1, 0, 0);
	}
	else {
		box.rotate(30, 1, 0, 0);
		box.rotate(Ydegree, 0, 1, 0);
		box.rotate(box.rotX, 1, 0, 0);
		box.translate(-0.5, 0, 0);
		box.rotate(box.rotY, 0, 1, 0);
		glBindVertexArray(box.VAO);
		glDrawElements(GL_TRIANGLES, box.faceNum * 3, GL_UNSIGNED_INT, 0);
		box.rotate(-box.rotY, 0, 1, 0);
		box.translate(0.5, 0, 0);
		box.rotate(-box.rotX, 1, 0, 0);
		box.rotate(-Ydegree, 0, 1, 0);
		box.rotate(-30, 1, 0, 0);

		sphere.rotate(30, 1, 0, 0);
		sphere.rotate(Ydegree, 0, 1, 0);
		sphere.rotate(sphere.rotX, 1, 0, 0);
		sphere.translate(0.5, 0, 0);
		sphere.rotate(sphere.rotY, 0, 1, 0);
		glBindVertexArray(sphere.VAO);
		glDrawElements(GL_TRIANGLES, sphere.faceNum * 3, GL_UNSIGNED_INT, 0);
		sphere.rotate(-sphere.rotY, 0, 1, 0);
		sphere.translate(-0.5, 0, 0);
		sphere.rotate(-sphere.rotX, 1, 0, 0);
		sphere.rotate(-Ydegree, 0, 1, 0);
		sphere.rotate(-30, 1, 0, 0);
	}
	glutSwapBuffers();											// 화면에 출력하기
}
GLvoid Reshape(int w, int h)									//--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}
GLvoid Keyboard(unsigned char key, int x, int y)
{
	keyboard = key;
	switch (key) {
	case 'X':
		box.rotX += 3;
		cone.rotX += 3;
		break;
	case 'x':
		box.rotX -= 3;
		cone.rotX -= 3;
		break;
	case 'Y':
		box.rotY += 3;
		cone.rotY += 3;
		break;
	case 'y':
		box.rotY -= 3;
		cone.rotY -= 3;
		break;
	case 'A':
		sphere.rotX += 3;
		torus.rotX += 3;
		break;
	case 'a':
		sphere.rotX -= 3;
		torus.rotX -= 3;
		break;
	case 'B':
		sphere.rotY += 3;
		torus.rotY += 3;
		break;
	case 'b':
		sphere.rotY -= 3;
		torus.rotY -= 3;
		break;
	case 'R':
		Ydegree += 1;
		break;
	case 'r':
		Ydegree -= 1;
		break;
	case 'C':
	case 'c':
		C = !C;
	case 'S':
	case 's':
		box.rotX = 0;
		box.rotY = 0;
		sphere.rotX = 0;
		sphere.rotY = 0;
		cone.rotX = 0;
		cone.rotY = 0;
		torus.rotX = 0;
		torus.rotY = 0;
		break;
	}
	glutPostRedisplay();										//--- 배경색이 바뀔때마다 출력 콜백함수를 호출하여 화면을 refresh 한다
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
	// VBO에 좌표를 통째로 넣고
	glGenBuffers(1, &VBO);		// 축
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(xyz), xyz, GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//---------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &box.VAO);		// 정육
	glBindVertexArray(box.VAO);

	glGenBuffers(1, &box.Color);
	glBindBuffer(GL_ARRAY_BUFFER, box.Color);
	glBufferData(GL_ARRAY_BUFFER, box.vertexNum * 3 * sizeof(float), box.vertexColor, GL_STATIC_DRAW);
	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // 0~
	glEnableVertexAttribArray(1);

	// VBO에 좌표를 통째로 넣고
	glGenBuffers(1, &box.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, box.VBO);
	glBufferData(GL_ARRAY_BUFFER, box.vertexNum * 3 * sizeof(float), box.vertexData, GL_STATIC_DRAW);

	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // 0~
	glEnableVertexAttribArray(0);


	glGenBuffers(1, &box.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, box.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, box.faceNum * 3 * sizeof(unsigned int), box.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//---------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &sphere.VAO);		// 구
	glBindVertexArray(sphere.VAO);

	glGenBuffers(1, &sphere.Color);
	glBindBuffer(GL_ARRAY_BUFFER, sphere.Color);
	glBufferData(GL_ARRAY_BUFFER, sphere.vertexNum * 3 * sizeof(float), sphere.vertexColor, GL_STATIC_DRAW);
	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // 0~
	glEnableVertexAttribArray(1);

	// VBO에 좌표를 통째로 넣고
	glGenBuffers(1, &sphere.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, sphere.VBO);
	glBufferData(GL_ARRAY_BUFFER, sphere.vertexNum * 3 * sizeof(float), sphere.vertexData, GL_STATIC_DRAW);

	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // 0~
	glEnableVertexAttribArray(0);


	glGenBuffers(1, &sphere.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.faceNum * 3 * sizeof(unsigned int), sphere.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//---------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &cone.VAO);		// 뿔
	glBindVertexArray(cone.VAO);

	glGenBuffers(1, &cone.Color);
	glBindBuffer(GL_ARRAY_BUFFER, cone.Color);
	glBufferData(GL_ARRAY_BUFFER, cone.vertexNum * 3 * sizeof(float), cone.vertexColor, GL_STATIC_DRAW);
	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // 0~
	glEnableVertexAttribArray(1);

	// VBO에 좌표를 통째로 넣고
	glGenBuffers(1, &cone.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, cone.VBO);
	glBufferData(GL_ARRAY_BUFFER, cone.vertexNum * 3 * sizeof(float), cone.vertexData, GL_STATIC_DRAW);

	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // 0~
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &cone.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cone.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cone.faceNum * 3 * sizeof(unsigned int), cone.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//---------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &torus.VAO);		// 도넛
	glBindVertexArray(torus.VAO);

	glGenBuffers(1, &torus.Color);
	glBindBuffer(GL_ARRAY_BUFFER, torus.Color);
	glBufferData(GL_ARRAY_BUFFER, torus.vertexNum * 3 * sizeof(float), torus.vertexColor, GL_STATIC_DRAW);
	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // 0~
	glEnableVertexAttribArray(1);

	// VBO에 좌표를 통째로 넣고
	glGenBuffers(1, &torus.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, torus.VBO);
	glBufferData(GL_ARRAY_BUFFER, torus.vertexNum * 3 * sizeof(float), torus.vertexData, GL_STATIC_DRAW);

	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // 0~
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &torus.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, torus.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, torus.faceNum * 3 * sizeof(unsigned int), torus.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//---------------------------------------------------------------------------------------------------------------
}

//GLvoid InitBuffer() {
//	// VBO에 좌표를 통째로 넣고
//	glGenBuffers(1, &VBO[0]);		// 축
//	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(xyz), xyz, GL_STATIC_DRAW);
//
//	glGenVertexArrays(1, &VAO[0]);
//	glBindVertexArray(VAO[0]);
//	// Position
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
//	glEnableVertexAttribArray(0);
//	// Color
//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
//	glEnableVertexAttribArray(1);
//
//	// VBO에 좌표를 통째로 넣고
//	glGenBuffers(1, &VBO[1]);		// 정육면체
//	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
//	glBufferData(GL_ARRAY_BUFFER, vertexNum * 3 * sizeof(vertexData), vertexData, GL_STATIC_DRAW);
//
//	glGenVertexArrays(1, &VAO[1]);
//	glBindVertexArray(VAO[1]);
//	// Position
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//	glEnableVertexAttribArray(0);
//
//	glGenBuffers(1, &EBO[0]);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceNum * 3 * sizeof(vertexIndex), vertexIndex, GL_STATIC_DRAW);
//
//	glGenVertexArrays(1, &VAO[2]); // 구
//	glBindVertexArray(VAO[2]);
//	// Position
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(3 * 8 * sizeof(float)));
//	glEnableVertexAttribArray(0);
//
//	glGenBuffers(1, &EBO[1]);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceNum * 3 * sizeof(vertexIndex), vertexIndex, GL_STATIC_DRAW);
//}

//void ReadObj(const char* FileName)
//{
//	//--- 1. 전체 버텍스 개수 및 삼각형 개수 세기
//	FILE* objFile = fopen(FileName, "r");
//	char count[100];
//	while (!feof(objFile)) {
//		fscanf(objFile, "%s", count);
//		if (count[0] == 'v' && count[1] == '\0')
//			vertexNum += 1;
//		else if (count[0] == 'f' && count[1] == '\0')
//			faceNum += 1;
//		memset(count, '\0', sizeof(count)); // 배열 초기화
//	}
//	//--- 2. 메모리 할당
//	rectdata = new float[vertexNum * 3];
//	rectIndex = new unsigned int[faceNum * 3];
//	int vertIndex = 0;
//	int faceIndex = 0;
//	fseek(objFile, 0, SEEK_SET);
//	//--- 3. 할당된 메모리에 각 버텍스, 페이스 정보 입력
//	while (!feof(objFile)) {
//		fscanf(objFile, "%s", count);
//		if (count[0] == 'v' && count[1] == '\0') {
//			fscanf(objFile, "%f %f %f", &rectdata[vertIndex], &rectdata[vertIndex+1], &rectdata[vertIndex+2]);
//			vertIndex += 3;
//		}
//		else if (count[0] == 'f' && count[1] == '\0') {
//			fscanf(objFile, "%d %d %d", &rectIndex[faceIndex], &rectIndex[faceIndex+1], &rectIndex[faceIndex+2]);
//			rectIndex[faceIndex]--;
//			rectIndex[faceIndex + 1]--;
//			rectIndex[faceIndex + 2]--;
//			faceIndex += 3;
//		}
//		memset(count, '\0', sizeof(count)); // 배열 초기화
//	}
//	fclose(objFile);
//}

//void ReadObj(const char* FileName)
//{
//	//--- 1. 전체 버텍스 개수 및 삼각형 개수 세기
//	FILE* objFile = fopen(FileName, "r");
//	char count[100];
//	while (!feof(objFile)) {
//		fscanf(objFile, "%s", count);
//		if (count[0] == 'v' && count[1] == '\0')
//			vertexNum += 1;
//		else if (count[0] == 'f' && count[1] == '\0')
//			faceNum += 1;
//		memset(count, '\0', sizeof(count)); // 배열 초기화
//	}
//	//--- 2. 메모리 할당
//	vertexData = new glm::vec3[vertexNum];
//	vertexIndex = new unsigned int[faceNum * 3];
//	int vertIndex = 0;
//	int faceIndex = 0;
//	fseek(objFile, 0, SEEK_SET);
//	//--- 3. 할당된 메모리에 각 버텍스, 페이스 정보 입력
//	while (!feof(objFile)) {
//		fscanf(objFile, "%s", count);
//		if (count[0] == 'v' && count[1] == '\0') {
//			fscanf(objFile, "%f %f %f", &vertexData[vertIndex].x, &vertexData[vertIndex].y, &vertexData[vertIndex].z);
//			//vertexData[vertIndex].r = 0;
//			//vertexData[vertIndex].g = 0;
//			//vertexData[vertIndex].b = 0;
//			vertIndex += 1;
//		}
//		else if (count[0] == 'f' && count[1] == '\0') {
//			fscanf(objFile, "%d %d %d", &vertexIndex[faceIndex], &vertexIndex[faceIndex+1], &vertexIndex[faceIndex+2]);
//			vertexIndex[faceIndex]--;
//			vertexIndex[faceIndex+1]--;
//			vertexIndex[faceIndex+2]--;
//			faceIndex += 3;
//		}
//		memset(count, '\0', sizeof(count)); // 배열 초기화
//	}
//	fclose(objFile);