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
void calLoop(float r);

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid InitBuffer();

double RED = 0.7f, GREEN = 0.7f, BLUE = 0.7f;
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
	float rotX{}, rotY{}, transX{}, transY{}, transZ{}, scaleX{ 1.0f }, scaleY{ 1.0f }, scaleZ{ 1.0f }, theta{}, r{};

	structure(const char* FileName,float R,float G,float B,float degree,float radius) {
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
		theta = degree;
		r = radius;
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

structure Sun("Sphere.obj", 1.0f, 0.0f, 0.0f, 0, 1);
structure First("Sphere.obj", 0.0f, 1.0f, 0.0f, 0, 1);
structure one("Sphere.obj", 0.0f, 0.5f, 0.0f, 0, 0.5);
structure Second("Sphere.obj", 0.0f, 0.0f, 1.0f, 30, 1);
structure two("Sphere.obj", 0.0f, 0.0f, 0.5f, 0, 0.5);
structure Third("Sphere.obj", 0.8f, 0.8f, 0.8f, 90, 1);
structure three("Sphere.obj", 0.3f, 0.3f, 0.3f, 0, 0.5);

GLuint VAO[6], VBO[6];
float cycle[3600][6]{};
float rotY{};
float horizontal, vertical, front;

void main(int argc, char** argv)								//--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);										// glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);				// 디스플레이 모드 설정
	glutInitWindowPosition(0, 0);								// 윈도우의 위치 지정
	glutInitWindowSize(WinX, WinY);								// 윈도우의 크기 지정
	WindowID = glutCreateWindow("18");								// 윈도우 생성 (윈도우 이름)

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)									// glew 초기화
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";
	calLoop(1);
	InitBuffer();
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	shaderProgram = make_shaderProgram();
	glutDisplayFunc(drawScene);									// 출력 콜백함수의 지정
	glutReshapeFunc(Reshape);									// 다시 그리기 콜백함수 지정
	glutKeyboardFunc(Keyboard);									// 키보드 입력 콜백함수 지정
	glutTimerFunc(10, Timer, 0);
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
	glm::vec3 camPos = glm::vec3(0.0f, 0.4f, 1.0f);							// 카메라 위치
	glm::vec3 camDir = -glm::normalize(camPos - glm::vec3(0.0f, 0.0f, 0.0f));		// 카메라가 보는 방향벡터
	glm::vec3 camUp = glm::cross(camDir, glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), camDir)));		// (고정식) 캠의 세로 평면과 수직을 이루는값을 노멀라이즈한 값을 캠의 방향벡터로 up구하기
	View = glm::lookAt(camPos, camDir, camUp);
	int ViewLoc = glGetUniformLocation(shaderProgram, "ViewTransform");
	glUniformMatrix4fv(ViewLoc, 1, GL_FALSE, glm::value_ptr(View));

	glm::mat4 Proj(1.0f);
	int ProjLoc = glGetUniformLocation(shaderProgram, "ProjectionTransform");
	Proj = glm::perspective(glm::radians(45.0f), (float)WinX / (float)WinY, 1.5f, 5.0f);
	Proj = glm::translate(Proj, glm::vec3(0.0, 0.0, -2.0));
	glUniformMatrix4fv(ProjLoc, 1, GL_FALSE, glm::value_ptr(Proj));

	glLineWidth(2);
	Model = glm::rotate(Model, glm::radians(rotY), glm::vec3(0, 1, 0));
	Model = glm::translate(Model, glm::vec3(horizontal, vertical, front));
	glUniformMatrix4fv(ModelLoc, 1, GL_FALSE, glm::value_ptr(Model));
	glBindVertexArray(VAO[0]);																			// XY 평면
	glDrawArrays(GL_LINE_STRIP, 0, 3600);
	Model = glm::mat4{ 1.0 };

	Model = glm::rotate(Model, glm::radians(rotY), glm::vec3(0, 1, 0));
	Model = glm::translate(Model, glm::vec3(horizontal, vertical, front));

	Model = glm::rotate(Model, glm::radians(45.0f), glm::vec3(0, 0, 1));
	glUniformMatrix4fv(ModelLoc, 1, GL_FALSE, glm::value_ptr(Model));
	glBindVertexArray(VAO[1]);																			// 오른쪽 으로 기울어진 
	glDrawArrays(GL_LINE_STRIP, 0, 3600);
	Model = glm::mat4{ 1.0 };

	Model = glm::rotate(Model, glm::radians(rotY), glm::vec3(0, 1, 0));
	Model = glm::translate(Model, glm::vec3(horizontal, vertical, front));

	Model = glm::rotate(Model, glm::radians(-45.0f), glm::vec3(0, 0, 1));
	glUniformMatrix4fv(ModelLoc, 1, GL_FALSE, glm::value_ptr(Model));
	glBindVertexArray(VAO[2]);																			// 왼쪽으로 기울어진
	glDrawArrays(GL_LINE_STRIP, 0, 3600);
	Model = glm::mat4{ 1.0 };

	Model = glm::rotate(Model, glm::radians(rotY), glm::vec3(0, 1, 0));
	Model = glm::translate(Model, glm::vec3(horizontal, vertical, front));

	Model = glm::translate(Model, glm::vec3(First.transX, First.transY, First.transZ));
	Model = glm::scale(Model, glm::vec3(0.5, 0.5, 0.5));
	glUniformMatrix4fv(ModelLoc, 1, GL_FALSE, glm::value_ptr(Model));
	glBindVertexArray(VAO[3]);																			// xy 평면 작은애
	glDrawArrays(GL_LINE_STRIP, 0, 3600);
	Model = glm::mat4{ 1.0 };

	Model = glm::rotate(Model, glm::radians(rotY), glm::vec3(0, 1, 0));
	Model = glm::translate(Model, glm::vec3(horizontal, vertical, front));

	Model = glm::translate(Model, glm::vec3(Second.transX, Second.transY, Second.transZ));
	Model = glm::scale(Model, glm::vec3(0.5, 0.5, 0.5));
	glUniformMatrix4fv(ModelLoc, 1, GL_FALSE, glm::value_ptr(Model));
	glBindVertexArray(VAO[4]);																			// 오른쪽으로 기울어진 작은애
	glDrawArrays(GL_LINE_STRIP, 0, 3600);
	Model = glm::mat4{ 1.0 };

	Model = glm::rotate(Model, glm::radians(rotY), glm::vec3(0, 1, 0));
	Model = glm::translate(Model, glm::vec3(horizontal, vertical, front));

	Model = glm::translate(Model, glm::vec3(Third.transX, Third.transY, Third.transZ));
	Model = glm::scale(Model, glm::vec3(0.5, 0.5, 0.5));
	glUniformMatrix4fv(ModelLoc, 1, GL_FALSE, glm::value_ptr(Model));
	glBindVertexArray(VAO[5]);																			// 왼쪽으로 기울어진 작은애
	glDrawArrays(GL_LINE_STRIP, 0, 3600);
	Model = glm::mat4{ 1.0 };
	glUniformMatrix4fv(ModelLoc, 1, GL_FALSE, glm::value_ptr(Model));

	glLineWidth(1);
	Sun.Rotate(rotY, 0, 1, 0);
	Sun.Translate(horizontal, vertical, front);

	glBindVertexArray(Sun.VAO);
	glDrawElements(GL_TRIANGLES, Sun.faceNum * 3, GL_UNSIGNED_INT, 0);
	Sun.transReset();

	First.Rotate(rotY, 0, 1, 0);
	First.Translate(horizontal, vertical, front);

	First.Translate(First.transX, First.transY, First.transZ);
	First.Scale(0.5, 0.5, 0.5);
	glBindVertexArray(First.VAO);
	glDrawElements(GL_TRIANGLES, First.faceNum * 3, GL_UNSIGNED_INT, 0);
	First.transReset();

	one.Rotate(rotY, 0, 1, 0);
	one.Translate(horizontal, vertical, front);

	one.Translate(First.transX,First.transY,First.transZ);
	one.Translate(one.r * cos(glm::radians(one.theta)), 0, one.r * sin(glm::radians(one.theta)));
	one.Scale(0.2, 0.2, 0.2);		// 크기줄이고
	glBindVertexArray(one.VAO);
	glDrawElements(GL_TRIANGLES, one.faceNum * 3, GL_UNSIGNED_INT, 0);
	one.transReset();

	Second.Rotate(rotY, 0, 1, 0);
	Second.Translate(horizontal, vertical, front);
	
	Second.Translate(Second.transX, Second.transY, Second.transZ);
	Second.Scale(0.5, 0.5, 0.5);
	glBindVertexArray(Second.VAO);
	glDrawElements(GL_TRIANGLES, Second.faceNum * 3, GL_UNSIGNED_INT, 0);
	Second.transReset();

	two.Rotate(rotY, 0, 1, 0);
	two.Translate(horizontal, vertical, front);

	two.Translate(two.transX, two.transY, two.transZ);
	two.Scale(0.2, 0.2, 0.2);
	glBindVertexArray(two.VAO);
	glDrawElements(GL_TRIANGLES, two.faceNum * 3, GL_UNSIGNED_INT, 0);
	two.transReset();

	Third.Rotate(rotY, 0, 1, 0);
	Third.Translate(horizontal, vertical, front);

	Third.Translate(Third.transX, Third.transY, Third.transZ);
	Third.Scale(0.5, 0.5, 0.5);
	glBindVertexArray(Third.VAO);
	glDrawElements(GL_TRIANGLES, Third.faceNum * 3, GL_UNSIGNED_INT, 0);
	Third.transReset();

	three.Rotate(rotY, 0, 1, 0);
	three.Translate(horizontal, vertical, front);

	three.Translate(three.transX, three.transY, three.transZ);
	three.Scale(0.2, 0.2, 0.2);
	glBindVertexArray(three.VAO);
	glDrawElements(GL_TRIANGLES, three.faceNum * 3, GL_UNSIGNED_INT, 0);
	three.transReset();

	glutSwapBuffers();											// 화면에 출력하기
}
GLvoid Reshape(int w, int h)									//--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}
GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'M':
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case 'm':
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case 'w':
		vertical += 0.1;
		break;
	case 'a':
		horizontal += 0.1;
		break;
	case 's':
		vertical -= 0.1;
		break;
	case 'd':
		horizontal -= 0.1;
		break;
	case 'z':
		front += 0.1;
		break;
	case 'x':
		front -= 0.1;
		break;
	case 'Y':
		glutTimerFunc(10, Timer, 'Y');
		break;
	case 'y':
		glutTimerFunc(10, Timer, 'y');
		break;
	}
	glutPostRedisplay();										//--- 배경색이 바뀔때마다 출력 콜백함수를 호출하여 화면을 refresh 한다
}

GLvoid Timer(int value) {
	switch (value) {
	case 'Y':
		rotY -= 1.5;
		glutTimerFunc(10, Timer, 'Y');
		break;
	case 'y':
		rotY += 1.5;
		glutTimerFunc(10, Timer, 'y');
		break;
	default:
		First.transX = First.r * cos(glm::radians(First.theta));
		First.transZ = First.r * sin(glm::radians(First.theta));
		First.theta += 1;
		one.transX = one.r * cos(glm::radians(one.theta));
		one.transZ = one.r * sin(glm::radians(one.theta));
		one.theta += 0.5;

		Second.transX = Second.r * glm::cos(glm::radians(Second.theta)) * glm::cos(glm::radians(45.0f));
		Second.transY = Second.r * glm::cos(glm::radians(Second.theta)) * glm::sin(glm::radians(45.0f));
		Second.transZ = Second.r * glm::sin(glm::radians(Second.theta));
		Second.theta += 2;

		two.transX = two.r * cos(glm::radians(two.theta)) + Second.transX;
		two.transY = Second.transY;
		two.transZ = two.r * sin(glm::radians(two.theta)) + Second.transZ;
		two.theta += 1.2;

		Third.transX = Third.r * glm::cos(glm::radians(Third.theta)) * glm::cos(glm::radians(-45.0f));
		Third.transY = Third.r * glm::cos(glm::radians(Third.theta)) * glm::sin(glm::radians(-45.0f));
		Third.transZ = Third.r * glm::sin(glm::radians(Third.theta));
		Third.theta += 3;

		three.transX = three.r * cos(glm::radians(three.theta)) + Third.transX;
		three.transY = Third.transY;
		three.transZ = three.r * sin(glm::radians(three.theta))+Third.transZ;
		three.theta += 0.8;
		glutTimerFunc(10, Timer, 0);
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
	for (int i = 0; i < 6; ++i) {
		glGenVertexArrays(1, &VAO[i]);
		glBindVertexArray(VAO[i]);

		glGenBuffers(1, &VBO[i]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cycle), cycle, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
	//-------------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &Sun.VAO);
	glBindVertexArray(Sun.VAO);

	glGenBuffers(1, &Sun.Color);
	glBindBuffer(GL_ARRAY_BUFFER, Sun.Color);
	glBufferData(GL_ARRAY_BUFFER, Sun.vertexNum * 3 * sizeof(float), Sun.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &Sun.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, Sun.VBO);
	glBufferData(GL_ARRAY_BUFFER, Sun.vertexNum * 3 * sizeof(float), Sun.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &Sun.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Sun.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, Sun.faceNum * 3 * sizeof(unsigned int), Sun.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &First.VAO);
	glBindVertexArray(First.VAO);

	glGenBuffers(1, &First.Color);
	glBindBuffer(GL_ARRAY_BUFFER, First.Color);
	glBufferData(GL_ARRAY_BUFFER, First.vertexNum * 3 * sizeof(float), First.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &First.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, First.VBO);
	glBufferData(GL_ARRAY_BUFFER, First.vertexNum * 3 * sizeof(float), First.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &First.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, First.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, First.faceNum * 3 * sizeof(unsigned int), First.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &one.VAO);
	glBindVertexArray(one.VAO);

	glGenBuffers(1, &one.Color);
	glBindBuffer(GL_ARRAY_BUFFER, one.Color);
	glBufferData(GL_ARRAY_BUFFER, one.vertexNum * 3 * sizeof(float), one.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &one.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, one.VBO);
	glBufferData(GL_ARRAY_BUFFER, one.vertexNum * 3 * sizeof(float), one.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &one.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, one.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, one.faceNum * 3 * sizeof(unsigned int), one.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &Second.VAO);
	glBindVertexArray(Second.VAO);

	glGenBuffers(1, &Second.Color);
	glBindBuffer(GL_ARRAY_BUFFER, Second.Color);
	glBufferData(GL_ARRAY_BUFFER, Second.vertexNum * 3 * sizeof(float), Second.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &Second.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, Second.VBO);
	glBufferData(GL_ARRAY_BUFFER, Second.vertexNum * 3 * sizeof(float), Second.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &Second.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Second.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, Second.faceNum * 3 * sizeof(unsigned int), Second.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &two.VAO);
	glBindVertexArray(two.VAO);

	glGenBuffers(1, &two.Color);
	glBindBuffer(GL_ARRAY_BUFFER, two.Color);
	glBufferData(GL_ARRAY_BUFFER, two.vertexNum * 3 * sizeof(float), two.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &two.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, two.VBO);
	glBufferData(GL_ARRAY_BUFFER, two.vertexNum * 3 * sizeof(float), two.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	glGenBuffers(1, &two.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, two.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, two.faceNum * 3 * sizeof(unsigned int), two.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &Third.VAO);
	glBindVertexArray(Third.VAO);

	glGenBuffers(1, &Third.Color);
	glBindBuffer(GL_ARRAY_BUFFER, Third.Color);
	glBufferData(GL_ARRAY_BUFFER, Third.vertexNum * 3 * sizeof(float), Third.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &Third.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, Third.VBO);
	glBufferData(GL_ARRAY_BUFFER, Third.vertexNum * 3 * sizeof(float), Third.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &Third.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Third.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, Third.faceNum * 3 * sizeof(unsigned int), Third.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//-------------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &three.VAO);
	glBindVertexArray(three.VAO);

	glGenBuffers(1, &three.Color);
	glBindBuffer(GL_ARRAY_BUFFER, three.Color);
	glBufferData(GL_ARRAY_BUFFER, three.vertexNum * 3 * sizeof(float), three.vertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &three.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, three.VBO);
	glBufferData(GL_ARRAY_BUFFER, three.vertexNum * 3 * sizeof(float), three.vertexData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &three.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, three.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, three.faceNum * 3 * sizeof(unsigned int), three.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
}

void calLoop(float r) {
	int dotcnt = 0;
		for (float theta = 0; theta < 360; theta += 0.1) {
			cycle[dotcnt][0] = r * glm::cos(glm::radians(theta));	//	X
			cycle[dotcnt][1] = 0;									//	Y
			cycle[dotcnt][2] = r * glm::sin(glm::radians(theta));	//	Z
			cycle[dotcnt][3] = 0;									//	R
			cycle[dotcnt][4] = 0;									//	G
			cycle[dotcnt++][5] = 0;									//	B
		}
}