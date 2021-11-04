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
GLuint VAO[3], VBO[3];
GLuint shaderProgram, WindowID;
bool P,Y,T,Fmax,O;

class structure {
public:
	GLuint VAO[6];
	GLuint VBO;
	GLuint Color;
	GLuint EBO[6];
	int vertexNum;
	int faceNum;
	float* vertexData;
	float* vertexColor;
	unsigned int* vertexFace;
	glm::mat4 trans{ 1.0f };
	float rotX[6]{}, rotY[6]{}, transX[6]{}, transY[6]{}, transZ[6]{}, scaleX[6]{ 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f }, scaleY[6]{ 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f }, scaleZ[6]{ 1.0f,1.0f,1.0f,1.0f,1.0f,1.0f };

	structure(const char* FileName) {
		VAO[0] = VAO[1] = VAO[2] = VAO[3] = VAO[4] = VAO[5] = VBO = EBO[0] = EBO[1] = EBO[2] = EBO[3] = EBO[4] = EBO[5] = vertexNum = faceNum = 0;
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

	~structure() {
		delete[] vertexData;
		delete[] vertexFace;
		delete[] vertexColor;
	}

	void mTranslate(float x, float y, float z) {
		unsigned int transformLocation = glGetUniformLocation(shaderProgram, "ModelTransform");
		trans = glm::translate(trans, glm::vec3(x, y, z));
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(trans));
	}
	void mRotate(float degree, float x, float y, float z) {
		unsigned int transformLocation = glGetUniformLocation(shaderProgram, "ModelTransform");
		trans = glm::rotate(trans, glm::radians(degree), glm::vec3(x, y, z));
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(trans));
	}
	void mScale(float x, float y, float z) {
		unsigned int transformLocation = glGetUniformLocation(shaderProgram, "ModelTransform");
		trans = glm::scale(trans, glm::vec3(x, y, z));
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(trans));
	}
};

float x[]{
	1,0,0,		1,0,0,
	-1,0,0,		1,0,0
};

float y[]{
	0,1,0,		0,1,0,
	0,-1,0,		0,1,0
};

float z[]{
	0,0,1,		0,0,1,
	0,0,-1,		0,0,1
};


structure box("Box.obj");
structure rectCone("RectCone.obj");

void main(int argc, char** argv)								//--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);										// glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);				// 디스플레이 모드 설정
	glutInitWindowPosition(0, 0);								// 윈도우의 위치 지정
	glutInitWindowSize(WinX, WinY);								// 윈도우의 크기 지정
	WindowID = glutCreateWindow("17");								// 윈도우 생성 (윈도우 이름)

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
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
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

	glm::mat4 Model(1.0f);
	int ModelLoc = glGetUniformLocation(shaderProgram, "ModelTransform");
	if (P) {
		Model = glm::scale(Model, glm::vec3(2.5f, 2.5f, 2.5f));
	}
	glUniformMatrix4fv(ModelLoc, 1, GL_FALSE, glm::value_ptr(Model));

	glm::vec3 camPos = glm::vec3(0.1f, 0.1f, 0.1f);									// 카메라 위치
	glm::vec3 camDir = -glm::normalize(camPos - glm::vec3(0.0f, 0.0f, 0.0f));		// 카메라가 보는 방향벡터
	glm::vec3 camUp = glm::cross(camDir, glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), camDir)));		// (고정식) 캠의 세로 평면과 수직을 이루는값을 노멀라이즈한 값을 캠의 방향벡터로 up구하기
	glm::mat4 View(1.0f);
	View = glm::lookAt(camPos, camDir, camUp);
	int ViewLoc = glGetUniformLocation(shaderProgram, "ViewTransform");
	glUniformMatrix4fv(ViewLoc, 1, GL_FALSE, glm::value_ptr(View));

	glm::mat4 Proj(1.0f);
	int ProjLoc = glGetUniformLocation(shaderProgram, "ProjectionTransform");
	if (P) {
		Proj = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 0.0f, 10.0f);
	}
	else {
		Proj = glm::perspective(glm::radians(45.0f), (float)WinX / (float)WinY, 0.1f, 50.0f);
	}

	Proj = glm::translate(Proj, glm::vec3(0.0, 0.0, -2.0));
	glUniformMatrix4fv(ProjLoc, 1, GL_FALSE, glm::value_ptr(Proj));

	glBindVertexArray(VAO[2]);			// y축
	glDrawArrays(GL_LINES, 0, 2);
	glUniformMatrix4fv(ModelLoc, 1, GL_FALSE, glm::value_ptr(Model));
	for (int i = 0; i < 2; ++i) {
		glBindVertexArray(VAO[i]);			// x z축
		glDrawArrays(GL_LINES, 0, 2);
	}

	if (P) {
		if (O) {
			rectCone.mScale(1.9, 1.9, 1.9);
			rectCone.mRotate(rectCone.rotY[0], 0, 1, 0);
			glBindVertexArray(rectCone.VAO[0]);									//아래
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			rectCone.mRotate(-rectCone.rotY[0], 0, 1, 0);
			rectCone.mScale(1 / 1.9, 1 / 1.9, 1 / 1.9);

			rectCone.mScale(1.9, 1.9, 1.9);
			rectCone.mRotate(rectCone.rotY[0], 0, 1, 0);
			rectCone.mTranslate(0, 0, -rectCone.vertexData[2]);
			rectCone.mRotate(-rectCone.rotX[1], 1, 0, 0);
			rectCone.mTranslate(0, 0, rectCone.vertexData[2]);
			glBindVertexArray(rectCone.VAO[1]);									// 뒤
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
			rectCone.mTranslate(0, 0, -rectCone.vertexData[2]);
			rectCone.mRotate(rectCone.rotX[1], 1, 0, 0);
			rectCone.mTranslate(0, 0, rectCone.vertexData[2]);
			rectCone.mRotate(-rectCone.rotY[0], 0, 1, 0);
			rectCone.mScale(1 / 1.9, 1 / 1.9, 1 / 1.9);

			rectCone.mScale(1.9, 1.9, 1.9);
			rectCone.mRotate(rectCone.rotY[0], 0, 1, 0);
			rectCone.mTranslate(-rectCone.vertexData[2], 0, 0);
			rectCone.mRotate(rectCone.rotX[1], 0, 0, 1);
			rectCone.mTranslate(rectCone.vertexData[2], 0, 0);
			glBindVertexArray(rectCone.VAO[2]);									// 왼
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
			rectCone.mTranslate(-rectCone.vertexData[2], 0, 0);
			rectCone.mRotate(-rectCone.rotX[1], 0, 0, 1);
			rectCone.mTranslate(rectCone.vertexData[2], 0, 0);
			rectCone.mRotate(-rectCone.rotY[0], 0, 1, 0);
			rectCone.mScale(1 / 1.9, 1 / 1.9, 1 / 1.9);

			rectCone.mScale(1.9, 1.9, 1.9);
			rectCone.mRotate(rectCone.rotY[0], 0, 1, 0);
			rectCone.mTranslate(0, 0, rectCone.vertexData[2]);
			rectCone.mRotate(rectCone.rotX[1], 1, 0, 0);
			rectCone.mTranslate(0, 0, -rectCone.vertexData[2]);
			glBindVertexArray(rectCone.VAO[3]);									// 앞
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
			rectCone.mTranslate(0, 0, rectCone.vertexData[2]);
			rectCone.mRotate(-rectCone.rotX[1], 1, 0, 0);
			rectCone.mTranslate(0, 0, -rectCone.vertexData[2]);
			rectCone.mRotate(-rectCone.rotY[0], 0, 1, 0);
			rectCone.mScale(1 / 1.9, 1 / 1.9, 1 / 1.9);

			rectCone.mScale(1.9, 1.9, 1.9);
			rectCone.mRotate(rectCone.rotY[0], 0, 1, 0);


			rectCone.mTranslate(rectCone.vertexData[2], 0, 0);
			rectCone.mRotate(-rectCone.rotX[1], 0, 0, 1);
			rectCone.mTranslate(-rectCone.vertexData[2], 0, 0);
			glBindVertexArray(rectCone.VAO[4]);									// 우
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
			rectCone.mTranslate(rectCone.vertexData[2], 0, 0);
			rectCone.mRotate(rectCone.rotX[1], 0, 0, 1);
			rectCone.mTranslate(-rectCone.vertexData[2], 0, 0);
			rectCone.mRotate(-rectCone.rotY[0], 0, 1, 0);
			rectCone.mScale(1 / 1.9, 1 / 1.9, 1 / 1.9);
		}
		else {
			box.mScale(2.5, 2.5, 2.5);
			box.mRotate(box.rotY[0], 0, 1, 0);
			glBindVertexArray(box.VAO[0]);									//아래
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			box.mRotate(-box.rotY[0], 0, 1, 0);
			box.mScale(1 / 2.5, 1 / 2.5, 1 / 2.5);

			box.mScale(2.5, 2.5, 2.5);
			box.mRotate(box.rotY[0], 0, 1, 0);
			box.mTranslate(0, 0.2, 0);
			box.mRotate(box.rotX[1], 1, 0, 0);
			box.mTranslate(0, -0.2, 0);
			glBindVertexArray(box.VAO[1]);									// 위
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			box.mTranslate(0, 0.2, 0);
			box.mRotate(-box.rotX[1], 1, 0, 0);
			box.mTranslate(0, -0.2, 0);
			box.mRotate(-box.rotY[0], 0, 1, 0);
			box.mScale(1 / 2.5, 1 / 2.5, 1 / 2.5);

			box.mScale(2.5, 2.5, 2.5);
			box.mRotate(box.rotY[0], 0, 1, 0);
			box.mTranslate(0, 0, 0.1);
			box.mRotate(box.rotX[2], 1, 0, 0);
			box.mTranslate(0, 0, -0.1);
			glBindVertexArray(box.VAO[2]);									// 앞면
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			box.mTranslate(0, 0, 0.1);
			box.mRotate(-box.rotX[2], 1, 0, 0);
			box.mTranslate(0, 0, -0.1);
			box.mRotate(-box.rotY[0], 0, 1, 0);
			box.mScale(1 / 2.5, 1 / 2.5, 1 / 2.5);

			box.mScale(2.5, 2.5, 2.5);
			box.mRotate(box.rotY[0], 0, 1, 0);
			box.mTranslate(0, box.transY[3], 0);
			glBindVertexArray(box.VAO[3]);									// 우측면
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			box.mTranslate(0, -box.transY[3], 0);
			box.mRotate(-box.rotY[0], 0, 1, 0);
			box.mScale(1 / 2.5, 1 / 2.5, 1 / 2.5);

			box.mScale(2.5, 2.5, 2.5);
			box.mRotate(box.rotY[0], 0, 1, 0);
			glBindVertexArray(box.VAO[4]);									// 뒷면
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			box.mRotate(-box.rotY[0], 0, 1, 0);
			box.mScale(1 / 2.5, 1 / 2.5, 1 / 2.5);

			box.mScale(2.5, 2.5, 2.5);
			box.mRotate(box.rotY[0], 0, 1, 0);
			box.mTranslate(0, box.transY[3], 0);
			glBindVertexArray(box.VAO[5]);									// 좌측면
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			box.mTranslate(0, -box.transY[3], 0);
			box.mRotate(-box.rotY[0], 0, 1, 0);
			box.mScale(1 / 2.5, 1 / 2.5, 1 / 2.5);
		}
	}
	else {
		if (O) {
			rectCone.mRotate(rectCone.rotY[0], 0, 1, 0);
			rectCone.mScale(1.5, 1.5, 1.5);
			glBindVertexArray(rectCone.VAO[0]);									//아래
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			rectCone.mScale(1 / 1.5, 1 / 1.5, 1 / 1.5);
			rectCone.mRotate(-rectCone.rotY[0], 0, 1, 0);

			rectCone.mRotate(rectCone.rotY[0], 0, 1, 0);
			rectCone.mScale(1.5, 1.5, 1.5);
			rectCone.mTranslate(0, 0, -rectCone.vertexData[2]);
			rectCone.mRotate(-rectCone.rotX[1], 1, 0, 0);
			rectCone.mTranslate(0, 0, rectCone.vertexData[2]);
			glBindVertexArray(rectCone.VAO[1]);									// 뒤
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
			rectCone.mTranslate(0, 0, -rectCone.vertexData[2]);
			rectCone.mRotate(rectCone.rotX[1], 1, 0, 0);
			rectCone.mTranslate(0, 0, rectCone.vertexData[2]);
			rectCone.mScale(1 / 1.5, 1 / 1.5, 1 / 1.5);
			rectCone.mRotate(-rectCone.rotY[0], 0, 1, 0);

			rectCone.mRotate(rectCone.rotY[0], 0, 1, 0);
			rectCone.mScale(1.5, 1.5, 1.5);
			rectCone.mTranslate(-rectCone.vertexData[2], 0, 0);
			rectCone.mRotate(rectCone.rotX[1], 0, 0, 1);
			rectCone.mTranslate(rectCone.vertexData[2], 0, 0);
			glBindVertexArray(rectCone.VAO[2]);									// 왼
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
			rectCone.mTranslate(-rectCone.vertexData[2], 0, 0);
			rectCone.mRotate(-rectCone.rotX[1], 0, 0, 1);
			rectCone.mTranslate(rectCone.vertexData[2], 0, 0);
			rectCone.mScale(1 / 1.5, 1 / 1.5, 1 / 1.5);
			rectCone.mRotate(-rectCone.rotY[0], 0, 1, 0);

			rectCone.mRotate(rectCone.rotY[0], 0, 1, 0);
			rectCone.mScale(1.5, 1.5, 1.5);
			rectCone.mTranslate(0, 0, rectCone.vertexData[2]);
			rectCone.mRotate(rectCone.rotX[1], 1, 0, 0);
			rectCone.mTranslate(0, 0, -rectCone.vertexData[2]);
			glBindVertexArray(rectCone.VAO[3]);									// 앞
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
			rectCone.mTranslate(0, 0, rectCone.vertexData[2]);
			rectCone.mRotate(-rectCone.rotX[1], 1, 0, 0);
			rectCone.mTranslate(0, 0, -rectCone.vertexData[2]);
			rectCone.mScale(1 / 1.5, 1 / 1.5, 1 / 1.5);
			rectCone.mRotate(-rectCone.rotY[0], 0, 1, 0);

			rectCone.mRotate(rectCone.rotY[0], 0, 1, 0);
			rectCone.mScale(1.5, 1.5, 1.5);
			rectCone.mTranslate(rectCone.vertexData[2], 0, 0);
			rectCone.mRotate(-rectCone.rotX[1], 0, 0, 1);
			rectCone.mTranslate(-rectCone.vertexData[2], 0, 0);
			glBindVertexArray(rectCone.VAO[4]);									// 우
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
			rectCone.mTranslate(rectCone.vertexData[2], 0, 0);
			rectCone.mRotate(rectCone.rotX[1], 0, 0, 1);
			rectCone.mTranslate(-rectCone.vertexData[2], 0, 0);
			rectCone.mScale(1 / 1.5, 1 / 1.5, 1 / 1.5);
			rectCone.mRotate(-rectCone.rotY[0], 0, 1, 0);
		}
		else {
			box.mRotate(box.rotY[0], 0, 1, 0);
			glBindVertexArray(box.VAO[0]);									//아래
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			box.mRotate(-box.rotY[0], 0, 1, 0);

			box.mRotate(box.rotY[0], 0, 1, 0);
			box.mTranslate(0, 0.2, 0);
			box.mRotate(box.rotX[1], 1, 0, 0);
			box.mTranslate(0, -0.2, 0);
			glBindVertexArray(box.VAO[1]);									// 위
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			box.mTranslate(0, 0.2, 0);
			box.mRotate(-box.rotX[1], 1, 0, 0);
			box.mTranslate(0, -0.2, 0);
			box.mRotate(-box.rotY[0], 0, 1, 0);

			box.mRotate(box.rotY[0], 0, 1, 0);
			box.mTranslate(0, 0, 0.1);
			box.mRotate(box.rotX[2], 1, 0, 0);
			box.mTranslate(0, 0, -0.1);
			glBindVertexArray(box.VAO[2]);									// 앞면
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			box.mTranslate(0, 0, 0.1);
			box.mRotate(-box.rotX[2], 1, 0, 0);
			box.mTranslate(0, 0, -0.1);
			box.mRotate(-box.rotY[0], 0, 1, 0);

			box.mRotate(box.rotY[0], 0, 1, 0);
			box.mTranslate(0, box.transY[3], 0);
			glBindVertexArray(box.VAO[3]);									// 우측면
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			box.mTranslate(0, -box.transY[3], 0);
			box.mRotate(-box.rotY[0], 0, 1, 0);

			box.mRotate(box.rotY[0], 0, 1, 0);
			glBindVertexArray(box.VAO[4]);									// 뒷면
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			box.mRotate(-box.rotY[0], 0, 1, 0);

			box.mRotate(box.rotY[0], 0, 1, 0);
			box.mTranslate(0, box.transY[3], 0);
			glBindVertexArray(box.VAO[5]);									// 좌측면
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			box.mTranslate(0, -box.transY[3], 0);
			box.mRotate(-box.rotY[0], 0, 1, 0);
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
	case 'Y':
	case 'y':
		Y = !Y;
		glutTimerFunc(10, Timer, 'Y');
		break;
	case 'T':
	case 't':
		O = false;
		T = !T;
		glutTimerFunc(10, Timer, 'T');
		break;
	case 'F':
		O = false;
		glutTimerFunc(10, Timer, 'F');
		break;
	case 'f':
		O = false;
		glutTimerFunc(10, Timer, 'f');
		break;
	case '1':
		O = false;
		glutTimerFunc(10, Timer, 1);
		break;
	case '2':
		O = false;
		glutTimerFunc(10, Timer, 2);
		break;
	case 'O':
		O = true;
		glutTimerFunc(10, Timer, 'O');
		break;
	case 'o':
		O = true;
		glutTimerFunc(10, Timer, 'o');
		break;
	case 'P':
		P = true;
		break;
	case 'p':
		P = false;
		break;
	}
	glutPostRedisplay();										//--- 배경색이 바뀔때마다 출력 콜백함수를 호출하여 화면을 refresh 한다
}
GLvoid SpecialKeyboard(int key, int x, int y) {
	glutPostRedisplay();
}
GLvoid Timer(int value) {
	switch (value) {
	case 'Y':
		for (int i = 0; i < 6; ++i) {
			box.rotY[i] += 2;
			rectCone.rotY[i] += 2;
		}
		if (Y) {
			glutTimerFunc(10, Timer, 'Y');
		}
		break;
	case 'T':
		if (T) {
			box.rotX[1] += 2;
			glutTimerFunc(10, Timer, 'T');
		}
		break;
	case 'F':
		if (box.rotX[2] < 90) {
			box.rotX[2] += 2;
			glutTimerFunc(10, Timer, 'F');
		}
		break;
	case 'f':
		if (box.rotX[2] > 0) {
			box.rotX[2] -= 2;
			glutTimerFunc(10, Timer, 'f');
		}
		break;
	case 1:
		if (box.transY[3] < 0.3) {
			box.transY[3] += 0.01;
			glutTimerFunc(10, Timer, 1);
		}
		break;
	case 2:
		if (box.transY[3] > 0.01) {
			box.transY[3] -= 0.01;
			glutTimerFunc(10, Timer, 2);
		}
		break;
	case 'O':
		if (rectCone.rotX[1] < 219) {
			rectCone.rotX[1] += 1;
			glutTimerFunc(10, Timer, 'O');
		}
		break;
	case 'o':
		if (rectCone.rotX[1] > 0) {
			rectCone.rotX[1] -= 1;
			glutTimerFunc(10, Timer, 'o');
		}
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
	glGenVertexArrays(1, &box.VAO[0]);		// 정육 아래면
	glBindVertexArray(box.VAO[0]);

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


	glGenBuffers(1, &box.EBO[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, box.EBO[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, box.faceNum * 3 * sizeof(unsigned int), box.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//---------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &box.VAO[1]);		// 정육 윗면
	glBindVertexArray(box.VAO[1]);

	glBindBuffer(GL_ARRAY_BUFFER, box.Color);
	glBufferData(GL_ARRAY_BUFFER, box.vertexNum * 3 * sizeof(float), box.vertexColor, GL_STATIC_DRAW);
	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // 0~
	glEnableVertexAttribArray(1);

	// VBO에 좌표를 통째로 넣고
	glBindBuffer(GL_ARRAY_BUFFER, box.VBO);
	glBufferData(GL_ARRAY_BUFFER, box.vertexNum * 3 * sizeof(float), box.vertexData, GL_STATIC_DRAW);
	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0 * sizeof(float))); // 0~
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &box.EBO[1]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, box.EBO[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (box.faceNum * 3 * sizeof(unsigned int)) - 6, &box.vertexFace[6], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//---------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &box.VAO[2]);		// 정육 앞면
	glBindVertexArray(box.VAO[2]);

	glBindBuffer(GL_ARRAY_BUFFER, box.Color);
	glBufferData(GL_ARRAY_BUFFER, box.vertexNum * 3 * sizeof(float), box.vertexColor, GL_STATIC_DRAW);
	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // 0~
	glEnableVertexAttribArray(1);

	// VBO에 좌표를 통째로 넣고
	glBindBuffer(GL_ARRAY_BUFFER, box.VBO);
	glBufferData(GL_ARRAY_BUFFER, box.vertexNum * 3 * sizeof(float), box.vertexData, GL_STATIC_DRAW);
	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0 * sizeof(float))); // 0~
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &box.EBO[2]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, box.EBO[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (box.faceNum * 3 * sizeof(unsigned int)) - 12, &box.vertexFace[12], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//---------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &box.VAO[3]);		// 정육 오른쪽면
	glBindVertexArray(box.VAO[3]);

	glBindBuffer(GL_ARRAY_BUFFER, box.Color);
	glBufferData(GL_ARRAY_BUFFER, box.vertexNum * 3 * sizeof(float), box.vertexColor, GL_STATIC_DRAW);
	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // 0~
	glEnableVertexAttribArray(1);

	// VBO에 좌표를 통째로 넣고
	glBindBuffer(GL_ARRAY_BUFFER, box.VBO);
	glBufferData(GL_ARRAY_BUFFER, box.vertexNum * 3 * sizeof(float), box.vertexData, GL_STATIC_DRAW);
	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0 * sizeof(float))); // 0~
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &box.EBO[3]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, box.EBO[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (box.faceNum * 3 * sizeof(unsigned int)) - 18, &box.vertexFace[18], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//---------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &box.VAO[4]);		// 정육 뒷면
	glBindVertexArray(box.VAO[4]);

	glBindBuffer(GL_ARRAY_BUFFER, box.Color);
	glBufferData(GL_ARRAY_BUFFER, box.vertexNum * 3 * sizeof(float), box.vertexColor, GL_STATIC_DRAW);
	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // 0~
	glEnableVertexAttribArray(1);

	// VBO에 좌표를 통째로 넣고
	glBindBuffer(GL_ARRAY_BUFFER, box.VBO);
	glBufferData(GL_ARRAY_BUFFER, box.vertexNum * 3 * sizeof(float), box.vertexData, GL_STATIC_DRAW);
	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0 * sizeof(float))); // 0~
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &box.EBO[4]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, box.EBO[4]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (box.faceNum * 3 * sizeof(unsigned int)) - 24, &box.vertexFace[24], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//---------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &box.VAO[5]);		// 정육 왼쪽면
	glBindVertexArray(box.VAO[5]);

	glBindBuffer(GL_ARRAY_BUFFER, box.Color);
	glBufferData(GL_ARRAY_BUFFER, box.vertexNum * 3 * sizeof(float), box.vertexColor, GL_STATIC_DRAW);
	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // 0~
	glEnableVertexAttribArray(1);

	// VBO에 좌표를 통째로 넣고
	glBindBuffer(GL_ARRAY_BUFFER, box.VBO);
	glBufferData(GL_ARRAY_BUFFER, box.vertexNum * 3 * sizeof(float), box.vertexData, GL_STATIC_DRAW);
	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0 * sizeof(float))); // 0~
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &box.EBO[5]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, box.EBO[5]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (box.faceNum * 3 * sizeof(unsigned int)) - 30, &box.vertexFace[30], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//---------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &rectCone.VAO[0]);		// 사각뿔 밑면
	glBindVertexArray(rectCone.VAO[0]);

	glGenBuffers(1, &rectCone.Color);
	glBindBuffer(GL_ARRAY_BUFFER, rectCone.Color);
	glBufferData(GL_ARRAY_BUFFER, rectCone.vertexNum * 3 * sizeof(float), rectCone.vertexColor, GL_STATIC_DRAW);
	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // 0~
	glEnableVertexAttribArray(1);

	// VBO에 좌표를 통째로 넣고
	glGenBuffers(1, &rectCone.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, rectCone.VBO);
	glBufferData(GL_ARRAY_BUFFER, rectCone.vertexNum * 3 * sizeof(float), rectCone.vertexData, GL_STATIC_DRAW);

	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // 0~
	glEnableVertexAttribArray(0);


	glGenBuffers(1, &rectCone.EBO[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectCone.EBO[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, rectCone.faceNum * 3 * sizeof(unsigned int), rectCone.vertexFace, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//---------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &rectCone.VAO[1]);		// 사각뿔 뒷면
	glBindVertexArray(rectCone.VAO[1]);

	glBindBuffer(GL_ARRAY_BUFFER, rectCone.Color);
	glBufferData(GL_ARRAY_BUFFER, rectCone.vertexNum * 3 * sizeof(float), rectCone.vertexColor, GL_STATIC_DRAW);
	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // 0~
	glEnableVertexAttribArray(1);

	// VBO에 좌표를 통째로 넣고
	glBindBuffer(GL_ARRAY_BUFFER, rectCone.VBO);
	glBufferData(GL_ARRAY_BUFFER, rectCone.vertexNum * 3 * sizeof(float), rectCone.vertexData, GL_STATIC_DRAW);
	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0 * sizeof(float))); // 0~
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &rectCone.EBO[1]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectCone.EBO[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (rectCone.faceNum * 3 * sizeof(unsigned int)) - 6, &rectCone.vertexFace[6], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//---------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &rectCone.VAO[2]);		// 사각뿔 좌측
	glBindVertexArray(rectCone.VAO[2]);

	glBindBuffer(GL_ARRAY_BUFFER, rectCone.Color);
	glBufferData(GL_ARRAY_BUFFER, rectCone.vertexNum * 3 * sizeof(float), rectCone.vertexColor, GL_STATIC_DRAW);
	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // 0~
	glEnableVertexAttribArray(1);

	// VBO에 좌표를 통째로 넣고
	glBindBuffer(GL_ARRAY_BUFFER, rectCone.VBO);
	glBufferData(GL_ARRAY_BUFFER, rectCone.vertexNum * 3 * sizeof(float), rectCone.vertexData, GL_STATIC_DRAW);
	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0 * sizeof(float))); // 0~
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &rectCone.EBO[2]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectCone.EBO[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (rectCone.faceNum * 3 * sizeof(unsigned int)) - 9, &rectCone.vertexFace[9], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//---------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &rectCone.VAO[3]);		// 사각뿔 정면
	glBindVertexArray(rectCone.VAO[3]);

	glBindBuffer(GL_ARRAY_BUFFER, rectCone.Color);
	glBufferData(GL_ARRAY_BUFFER, rectCone.vertexNum * 3 * sizeof(float), rectCone.vertexColor, GL_STATIC_DRAW);
	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // 0~
	glEnableVertexAttribArray(1);

	// VBO에 좌표를 통째로 넣고
	glBindBuffer(GL_ARRAY_BUFFER, rectCone.VBO);
	glBufferData(GL_ARRAY_BUFFER, rectCone.vertexNum * 3 * sizeof(float), rectCone.vertexData, GL_STATIC_DRAW);
	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0 * sizeof(float))); // 0~
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &rectCone.EBO[3]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectCone.EBO[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (rectCone.faceNum * 3 * sizeof(unsigned int)) - 12, &rectCone.vertexFace[12], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
	//---------------------------------------------------------------------------------------------------------------
	glGenVertexArrays(1, &rectCone.VAO[4]);		// 사각뿔 우측
	glBindVertexArray(rectCone.VAO[4]);

	glBindBuffer(GL_ARRAY_BUFFER, rectCone.Color);
	glBufferData(GL_ARRAY_BUFFER, rectCone.vertexNum * 3 * sizeof(float), rectCone.vertexColor, GL_STATIC_DRAW);
	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // 0~
	glEnableVertexAttribArray(1);

	// VBO에 좌표를 통째로 넣고
	glBindBuffer(GL_ARRAY_BUFFER, rectCone.VBO);
	glBufferData(GL_ARRAY_BUFFER, rectCone.vertexNum * 3 * sizeof(float), rectCone.vertexData, GL_STATIC_DRAW);
	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0 * sizeof(float))); // 0~
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &rectCone.EBO[4]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectCone.EBO[4]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (rectCone.faceNum * 3 * sizeof(unsigned int)) - 15, &rectCone.vertexFace[15], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(unsigned int), (void*)0);
}