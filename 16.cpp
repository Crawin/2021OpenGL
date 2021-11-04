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
GLvoid SpecialKeyboard(int key, int x, int y);

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid InitBuffer();

double RED = 1.0f, GREEN = 1.0f, BLUE = 1.0f;
GLuint VAO[4], VBO[4];
GLuint shaderProgram, WindowID;
int input;
bool T, S, R, PR;
float Ydegree = -30.0f, centerscale = 1.0f, centerXY = 0, theta = 0.f, r = 0.01f;
int vertexNum = 0;
int faceNum = 0;
int dotcnt = 0;
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
	float rotX = 0.f, rotY = 0.f, transX = 0.f, transY = 0.f, transZ = 0.f, scaleX = 1.0f, scaleY = 1.0f, scaleZ = 1.0f;

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
	void scale(float x,float y,float z) {
		unsigned int transformLocation = glGetUniformLocation(shaderProgram, "transform");
		trans = glm::scale(trans,glm::vec3(x,y,z));
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(trans));
	}
};

float x[]{
	-1,0,0,		1,0,0,
	1,0,0,		1,0,0
};

float y[]{
	0,-1,0,		0,1,0,
	0,1,0,		0,1,0
};

float z[]{
	0,0,-1,		0,0,1,
	0,0,1,		0,0,1
};

float cycle[358][6] = {
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
	glutSpecialFunc(SpecialKeyboard);
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
	RR = glm::scale(RR, glm::vec3(centerscale, centerscale, centerscale));
	RR = glm::rotate(RR, glm::radians(30.0f), glm::vec3(1.0, 0.0, 0.0));
	RR = glm::rotate(RR, glm::radians(Ydegree), glm::vec3(0.0, 1.0, 0.0));
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(RR));
	glBindVertexArray(VAO[2]);			// y축
	glDrawArrays(GL_LINES, 0, 2);
	RR = glm::translate(RR, glm::vec3(0, centerXY, 0));
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(RR));
	for (int i = 0; i < 2; ++i) {
		glBindVertexArray(VAO[i]);			// x z축
		glDrawArrays(GL_LINES, 0, 2);
	}
	glBindVertexArray(VAO[3]);
	glDrawArrays(GL_LINE_STRIP, 0, dotcnt);
	if (R) {
		if (T) {
			cone.rotate(30, 1, 0, 0);
			cone.rotate(Ydegree, 0, 1, 0);
			cone.scale(centerscale, centerscale, centerscale);
			cone.translate(cone.transX, cone.transY, cone.transZ);
			cone.translate(r * cos(theta), 0, r * sin(theta));
			cone.scale(cone.scaleX, cone.scaleY, cone.scaleZ);
			cone.translate(0, centerXY, 0);
			cone.rotate(cone.rotY, 0, 1, 0);
			cone.rotate(cone.rotX, 1, 0, 0);
			cone.translate(-cone.transX, -cone.transY, -cone.transZ);
			glBindVertexArray(cone.VAO);
			glDrawElements(GL_TRIANGLES, cone.faceNum * 3, GL_UNSIGNED_INT, 0);
			cone.translate(cone.transX, cone.transY, cone.transZ);
			cone.rotate(-cone.rotX, 1, 0, 0);
			cone.rotate(-cone.rotY, 0, 1, 0);
			cone.translate(0, -centerXY, 0);
			cone.scale(1 / cone.scaleX, 1 / cone.scaleY, 1 / cone.scaleZ);
			cone.translate(-(r * cos(theta)), 0, -(r * sin(theta)));
			cone.translate(-cone.transX, -cone.transY, -cone.transZ);
			cone.scale(1 / centerscale, 1 / centerscale, 1 / centerscale);
			cone.rotate(-Ydegree, 0, 1, 0);
			cone.rotate(-30, 1, 0, 0);

			torus.rotate(30, 1, 0, 0);
			torus.rotate(Ydegree, 0, 1, 0);
			torus.scale(centerscale, centerscale, centerscale);
			torus.translate(torus.transX, torus.transY, torus.transZ);
			torus.translate(0.5, 0, 0);
			torus.scale(torus.scaleX, torus.scaleY, torus.scaleZ);
			torus.translate(0, centerXY, 0);
			torus.rotate(torus.rotY, 0, 1, 0);
			torus.rotate(torus.rotX, 1, 0, 0);
			glBindVertexArray(torus.VAO);
			glDrawElements(GL_TRIANGLES, torus.faceNum * 3, GL_UNSIGNED_INT, 0);
			torus.rotate(-torus.rotX, 1, 0, 0);
			torus.rotate(-torus.rotY, 0, 1, 0);
			torus.translate(0, -centerXY, 0);
			torus.scale(1 / torus.scaleX, 1 / torus.scaleY, 1 / torus.scaleZ);
			torus.translate(-0.5, 0, 0);
			torus.translate(-torus.transX, -torus.transY, -torus.transZ);
			torus.scale(1 / centerscale, 1 / centerscale, 1 / centerscale);
			torus.rotate(-Ydegree, 0, 1, 0);
			torus.rotate(-30, 1, 0, 0);
		}
		else {
			box.rotate(30, 1, 0, 0);
			box.rotate(Ydegree, 0, 1, 0);
			box.scale(centerscale, centerscale, centerscale);
			box.translate(box.transX, box.transY, box.transZ);
			box.translate(r * cos(theta), 0, r * sin(theta));
			box.scale(box.scaleX, box.scaleY, box.scaleZ);
			box.translate(0, centerXY, 0);
			box.rotate(box.rotY, 0, 1, 0);
			box.rotate(box.rotX, 1, 0, 0);
			box.translate(-box.transX, -box.transY, -box.transZ);
			glBindVertexArray(box.VAO);
			glDrawElements(GL_TRIANGLES, box.faceNum * 3, GL_UNSIGNED_INT, 0);
			box.translate(box.transX, box.transY, box.transZ);
			box.rotate(-box.rotX, 1, 0, 0);
			box.rotate(-box.rotY, 0, 1, 0);
			box.translate(0, -centerXY, 0);
			box.scale(1 / box.scaleX, 1 / box.scaleY, 1 / box.scaleZ);
			box.translate(-(r * cos(theta)), 0, -(r * sin(theta)));
			box.translate(-box.transX, -box.transY, -box.transZ);
			box.scale(1 / centerscale, 1 / centerscale, 1 / centerscale);
			box.rotate(-Ydegree, 0, 1, 0);
			box.rotate(-30, 1, 0, 0);

			sphere.rotate(30, 1, 0, 0);
			sphere.rotate(Ydegree, 0, 1, 0);
			sphere.scale(centerscale, centerscale, centerscale);
			sphere.translate(sphere.transX, sphere.transY, sphere.transZ);
			sphere.translate(0.5, 0, 0);
			sphere.scale(sphere.scaleX, sphere.scaleY, sphere.scaleZ);
			sphere.translate(0, centerXY, 0);
			sphere.rotate(sphere.rotY, 0, 1, 0);
			sphere.rotate(sphere.rotX, 1, 0, 0);
			glBindVertexArray(sphere.VAO);
			glDrawElements(GL_TRIANGLES, sphere.faceNum * 3, GL_UNSIGNED_INT, 0);
			sphere.rotate(-sphere.rotX, 1, 0, 0);
			sphere.rotate(-sphere.rotY, 0, 1, 0);
			sphere.translate(0, -centerXY, 0);
			sphere.scale(1 / sphere.scaleX, 1 / sphere.scaleY, 1 / sphere.scaleZ);
			sphere.translate(-0.5, 0, 0);
			sphere.translate(-sphere.transX, -sphere.transY, -sphere.transZ);
			sphere.scale(1 / centerscale, 1 / centerscale, 1 / centerscale);
			sphere.rotate(-Ydegree, 0, 1, 0);
			sphere.rotate(-30, 1, 0, 0);
		}
	}
	else {
			if (T) {
			cone.rotate(30, 1, 0, 0);
			cone.rotate(Ydegree, 0, 1, 0);
			cone.scale(centerscale, centerscale, centerscale);
			cone.translate(cone.transX, cone.transY, cone.transZ);
			cone.translate(-0.5, 0, 0);
			cone.scale(cone.scaleX, cone.scaleY, cone.scaleZ);
			cone.translate(0, centerXY, 0);
			cone.rotate(cone.rotY, 0, 1, 0);
			cone.rotate(cone.rotX, 1, 0, 0);
			glBindVertexArray(cone.VAO);
			glDrawElements(GL_TRIANGLES, cone.faceNum * 3, GL_UNSIGNED_INT, 0);
			cone.rotate(-cone.rotX, 1, 0, 0);
			cone.rotate(-cone.rotY, 0, 1, 0);
			cone.translate(0, -centerXY, 0);
			cone.scale(1 / cone.scaleX, 1 / cone.scaleY, 1 / cone.scaleZ);
			cone.translate(0.5, 0, 0);
			cone.translate(-cone.transX, -cone.transY, -cone.transZ);
			cone.scale(1 / centerscale, 1 / centerscale, 1 / centerscale);
			cone.rotate(-Ydegree, 0, 1, 0);
			cone.rotate(-30, 1, 0, 0);

			torus.rotate(30, 1, 0, 0);
			torus.rotate(Ydegree, 0, 1, 0);
			torus.scale(centerscale, centerscale, centerscale);
			torus.translate(torus.transX, torus.transY, torus.transZ);
			torus.translate(0.5, 0, 0);
			torus.scale(torus.scaleX, torus.scaleY, torus.scaleZ);
			torus.translate(0, centerXY, 0);
			torus.rotate(torus.rotY, 0, 1, 0);
			torus.rotate(torus.rotX, 1, 0, 0);
			glBindVertexArray(torus.VAO);
			glDrawElements(GL_TRIANGLES, torus.faceNum * 3, GL_UNSIGNED_INT, 0);
			torus.rotate(-torus.rotX, 1, 0, 0);
			torus.rotate(-torus.rotY, 0, 1, 0);
			torus.translate(0, -centerXY, 0);
			torus.scale(1 / torus.scaleX, 1 / torus.scaleY, 1 / torus.scaleZ);
			torus.translate(-0.5, 0, 0);
			torus.translate(-torus.transX, -torus.transY, -torus.transZ);
			torus.scale(1 / centerscale, 1 / centerscale, 1 / centerscale);
			torus.rotate(-Ydegree, 0, 1, 0);
			torus.rotate(-30, 1, 0, 0);
		}
		else {
			box.rotate(30, 1, 0, 0);
			box.rotate(Ydegree, 0, 1, 0);
			box.scale(centerscale, centerscale, centerscale);
			box.translate(box.transX, box.transY, box.transZ);
			box.translate(-0.5, 0, 0);
			box.scale(box.scaleX, box.scaleY, box.scaleZ);
			box.translate(0, centerXY, 0);
			box.rotate(box.rotY, 0, 1, 0);
			box.rotate(box.rotX, 1, 0, 0);
			glBindVertexArray(box.VAO);
			glDrawElements(GL_TRIANGLES, box.faceNum * 3, GL_UNSIGNED_INT, 0);
			box.rotate(-box.rotX, 1, 0, 0);
			box.rotate(-box.rotY, 0, 1, 0);
			box.translate(0, -centerXY, 0);
			box.scale(1 / box.scaleX, 1 / box.scaleY, 1 / box.scaleZ);
			box.translate(0.5, 0, 0);
			box.translate(-box.transX, -box.transY, -box.transZ);
			box.scale(1 / centerscale, 1 / centerscale, 1 / centerscale);
			box.rotate(-Ydegree, 0, 1, 0);
			box.rotate(-30, 1, 0, 0);

			sphere.rotate(30, 1, 0, 0);
			sphere.rotate(Ydegree, 0, 1, 0);
			sphere.scale(centerscale, centerscale, centerscale);
			sphere.translate(sphere.transX, sphere.transY, sphere.transZ);
			sphere.translate(0.5, 0, 0);
			sphere.scale(sphere.scaleX, sphere.scaleY, sphere.scaleZ);
			sphere.translate(0, centerXY, 0);
			sphere.rotate(sphere.rotY, 0, 1, 0);
			sphere.rotate(sphere.rotX, 1, 0, 0);
			glBindVertexArray(sphere.VAO);
			glDrawElements(GL_TRIANGLES, sphere.faceNum * 3, GL_UNSIGNED_INT, 0);
			sphere.rotate(-sphere.rotX, 1, 0, 0);
			sphere.rotate(-sphere.rotY, 0, 1, 0);
			sphere.translate(0, -centerXY, 0);
			sphere.scale(1 / sphere.scaleX, 1 / sphere.scaleY, 1 / sphere.scaleZ);
			sphere.translate(-0.5, 0, 0);
			sphere.translate(-sphere.transX, -sphere.transY, -sphere.transZ);
			sphere.scale(1 / centerscale, 1 / centerscale, 1 / centerscale);
			sphere.rotate(-Ydegree, 0, 1, 0);
			sphere.rotate(-30, 1, 0, 0);
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
	keyboard = key;
	switch (key) {
	case 'F':
		S = false;
		glutTimerFunc(10, Timer, 'F');
		break;
	case 'f':
		S = false;
		glutTimerFunc(10, Timer, 'f');
		break;
	case 'V':
		S = false;
		glutTimerFunc(10, Timer, 'V');
		break;
	case 'v':
		S = false;
		glutTimerFunc(10, Timer, 'v');
		break;
	case 'H':
		S = false;
		glutTimerFunc(10, Timer, 'H');
		break;
	case 'h':
		S = false;
		glutTimerFunc(10, Timer, 'h');
		break;
	case 'N':
		S = false;
		glutTimerFunc(10, Timer, 'N');
		break;
	case 'n':
		S = false;
		glutTimerFunc(10, Timer, 'n');
		break;
	case 'Y':
		S = false;
		glutTimerFunc(10, Timer, 'Y');
		break;
	case 'y':
		S = false;
		glutTimerFunc(10, Timer, 'y');
		break;
	case 'T':
	case 't':
		T = !T;
	case 'C':
	case 'c':
		PR = false;
		S = true;
		R = false;
		dotcnt = 0;
		theta = 1.0f;
		r = 0.01f;
		Ydegree = -30.0f;
		centerscale = 1.0f;
		centerXY = 0;
		box.rotX = box.rotY = box.transX = box.transY = box.transZ = sphere.rotX = sphere.rotY = sphere.transX = sphere.transY = sphere.transZ = cone.rotX = cone.rotY = cone.transX = cone.transY = cone.transZ = torus.rotX = torus.rotY = torus.transX = torus.transY = torus.transZ = 0;
		box.scaleX = box.scaleY = box.scaleZ = sphere.scaleX = sphere.scaleY = sphere.scaleZ = cone.scaleX = cone.scaleY = cone.scaleZ = torus.scaleX = torus.scaleY = torus.scaleZ = 1.0f;
		break;
	case 'W':
		box.transX += 0.05;
		cone.transX += 0.05;
		break;
	case 'w':
		box.transX -= 0.05;
		cone.transX -= 0.05;
		break;
	case 'A':
		box.transY += 0.05;
		cone.transY += 0.05;
		break;
	case 'a':
		box.transY -= 0.05;
		cone.transY -= 0.05;
		break;
	case 'D':
		box.transZ += 0.05;
		cone.transZ += 0.05;
		break;
	case 'd':
		box.transZ -= 0.05;
		cone.transZ -= 0.05;
		break;
	case 'I':
		sphere.transX += 0.05;
		torus.transX += 0.05;
		break;
	case 'i':
		sphere.transX -= 0.05;
		torus.transX -= 0.05;
		break;
	case 'J':
		sphere.transY += 0.05;
		torus.transY += 0.05;
		break;
	case 'j':
		sphere.transY -= 0.05;
		torus.transY -= 0.05;
		break;
	case 'L':
		sphere.transZ += 0.05;
		torus.transZ += 0.05;
		break;
	case 'l':
		sphere.transZ -= 0.05;
		torus.transZ -= 0.05;
		break;
	case ']':
		box.scaleX += 0.1;
		box.scaleY += 0.1;
		box.scaleZ += 0.1;
		cone.scaleX += 0.1;
		cone.scaleY += 0.1;
		cone.scaleZ += 0.1;
		break;
	case '[':
		box.scaleX -= 0.1;
		box.scaleY -= 0.1;
		box.scaleZ -= 0.1;
		cone.scaleX -= 0.1;
		cone.scaleY -= 0.1;
		cone.scaleZ -= 0.1;
		break;
	case '=':
		torus.scaleX += 0.1;
		torus.scaleY += 0.1;
		torus.scaleZ += 0.1;
		sphere.scaleX += 0.1;
		sphere.scaleY += 0.1;
		sphere.scaleZ += 0.1;
		break;
	case '-':
		torus.scaleX -= 0.1;
		torus.scaleY -= 0.1;
		torus.scaleZ -= 0.1;
		sphere.scaleX -= 0.1;
		sphere.scaleY -= 0.1;
		sphere.scaleZ -= 0.1;
		break;
	case 'G':
		centerscale += 0.1;
		break;
	case 'g':
		centerscale -= 0.1;
		break;
	case 'r':
	case 'R':
		if (!PR) {
			PR = true;
			R = true;
			S = false;
			dotcnt = 0;
			theta = 1.0f;
			r = 0.01f;
			glutTimerFunc(10, Timer, 'R');
		}
		break;
	}
	glutPostRedisplay();										//--- 배경색이 바뀔때마다 출력 콜백함수를 호출하여 화면을 refresh 한다
}
GLvoid SpecialKeyboard(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		centerXY += 0.05;
		break;
	case GLUT_KEY_DOWN:
		centerXY -= 0.05;
		break;
	}
	glutPostRedisplay();
}
GLvoid Timer(int value) {
	if (!S) {
		switch (value) {
		case 'F':
			box.rotX += 3;
			cone.rotX += 3;
			glutTimerFunc(10, Timer, 'F');
			break;
		case 'f':
			box.rotX -= 3;
			cone.rotX -= 3;
			glutTimerFunc(10, Timer, 'f');
			break;
		case 'V':
			box.rotY += 3;
			cone.rotY += 3;
			glutTimerFunc(10, Timer, 'V');
			break;
		case 'v':
			box.rotY -= 3;
			cone.rotY -= 3;
			glutTimerFunc(10, Timer, 'v');
			break;
		case 'H':
			sphere.rotX += 3;
			torus.rotX += 3;
			glutTimerFunc(10, Timer, 'H');
			break;
		case 'h':
			sphere.rotX -= 3;
			torus.rotX -= 3;
			glutTimerFunc(10, Timer, 'h');
			break;
		case 'N':
			sphere.rotY += 3;
			torus.rotY += 3;
			glutTimerFunc(10, Timer, 'N');
			break;
		case 'n':
			sphere.rotY -= 3;
			torus.rotY -= 3;
			glutTimerFunc(10, Timer, 'n');
			break;
		case 'Y':
			Ydegree += 1;
			glutTimerFunc(10, Timer, 'Y');
			break;
		case 'y':
			Ydegree -= 1;
			glutTimerFunc(10, Timer, 'y');
			break;
		case 'R':
			cycle[dotcnt][0] = r * cos(theta);		// x
			cycle[dotcnt][2] = r * sin(theta);		// z
			theta += 0.1;
			r += 0.002;
			dotcnt++;
			glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(cycle), cycle, GL_STATIC_DRAW);
			glBindVertexArray(VAO[3]);
			// Position
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			// Color
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
			if (dotcnt < 358) {
				glutTimerFunc(10, Timer, 'R');
			}
			else {
				PR = false;
			}
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
	// VBO에 좌표를 넣고
	glGenBuffers(1, &VBO[0]);		// x축
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(x), x, GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO[0]);
	glBindVertexArray(VAO[0]);
	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// VBO에 좌표를 넣고
	glGenBuffers(1, &VBO[1]);		// z축
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(z), z, GL_STATIC_DRAW);
	glGenVertexArrays(1, &VAO[1]);
	glBindVertexArray(VAO[1]);
	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// VBO에 좌표를 통째로 넣고
	glGenBuffers(1, &VBO[2]);		// y축
	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(y), y, GL_STATIC_DRAW);
	glGenVertexArrays(1, &VAO[2]);
	glBindVertexArray(VAO[2]);
	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//---------------------------------------------------------------------------------------------------------------
	// VBO에 좌표를 통째로 넣고
	glGenBuffers(1, &VBO[3]);		// XZ 평면 회오리
	glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cycle), cycle, GL_STATIC_DRAW);
	glGenVertexArrays(1, &VAO[3]);
	glBindVertexArray(VAO[3]);
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