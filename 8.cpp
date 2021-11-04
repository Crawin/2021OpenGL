#include <iostream>
#include <gl/glew.h> // 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include "mouse_to_coordinate.h"
#include "FileToBuf.h"
#include <cmath>
using namespace std;

#define WinX 600
#define WinY 600

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();

GLuint shaderID, WindowID;
GLint width, height;
GLuint VAO[6], VBO;
bool M, movetype[6], A, B, S;
int timerspeed = 100;

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid InitBuffer();
GLvoid Mouse(int button, int state, int xm, int ym);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Timer(int value);
float vertexData[6][3][6] = {
};

double BRED = 1.0f, BGREEN = 1.0f, BBLUE = 1.0f;
void main(int argc, char** argv)								//--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);										// glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);				// 디스플레이 모드 설정
	glutInitWindowPosition(0, 0);								// 윈도우의 위치 지정
	glutInitWindowSize(WinX, WinY);								// 윈도우의 크기 지정
	WindowID = glutCreateWindow("3-8");								// 윈도우 생성 (윈도우 이름)

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
	shaderID = make_shaderProgram();
	glutDisplayFunc(drawScene);									// 출력 콜백함수의 지정
	glutReshapeFunc(Reshape);									// 다시 그리기 콜백함수 지정
	//for (int i = 0; i < 4; ++i) {
	//	glutTimerFunc(100, TimerFuction, i);
	//}
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);
	glutMainLoop();												// 이벤트 처리 시작
}

GLvoid drawScene()												//--- 콜백 함수: 그리기 콜백 함수
{
	//--- 변경된 배경색 설정
	glClearColor(BRED, BGREEN, BBLUE, 1.0f);											// 바탕색을 변경
	glClear(GL_COLOR_BUFFER_BIT);								// 설정된 색으로 전체를 칠하기
	glUseProgram(shaderID);
	for (int i = 0; i < 6; ++i) {
		glBindVertexArray(VAO[i]);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
	glutSwapBuffers();											// 화면에 출력하기
}
GLvoid Reshape(int w, int h)									//--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}

GLvoid Mouse(int button, int state, int xm, int ym) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		// 마지막 수 0 x 값, 1 y 값, 2 z 값인데 z 는 이차원이라 항상 0
		// 마우스 위치에 첫번째 삼각형 만들어주고
		if (!S) {
			vertexData[0][0][0] = XM2C(WinX, xm);
			vertexData[0][0][1] = YM2C(WinY, ym) + 0.05f * 4.0f / 3.0f;
			vertexData[0][1][0] = XM2C(WinX, xm) - 0.05f;
			vertexData[0][1][1] = YM2C(WinY, ym) - 0.05f * 2.0f / 3.0f;
			vertexData[0][2][0] = XM2C(WinX, xm) + 0.05f;
			vertexData[0][2][1] = YM2C(WinY, ym) - 0.05f * 2.0f / 3.0f;
			// 그 이후 삼각형은 첫번째 삼각형을 평행이동 y값을 일정하므로 x 값만 평행이동
			for (int i = 1; i < 6; ++i) {
				for (int j = 0; j < 3; ++j) {
					vertexData[i][j][0] = vertexData[0][j][0] + 0.05f * 3 * i;
					vertexData[i][j][1] = vertexData[0][j][1];
				}
			}
			InitBuffer();
		}
	}
	

	
	glutPostRedisplay();
}

GLvoid Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'M':
	case 'm':
		if (!M) { 
			M = true;
			S = true;
			glutTimerFunc(timerspeed, Timer, 0);
		}
		break;
	case 'N':
	case 'n':
		for (int i = 0; i < 6; ++i) {
			movetype[i] = !movetype[i];
		}
		break;
	case '=':
		timerspeed -= 10;
		if (timerspeed <= 0) {
			timerspeed += 10;
		}
		break;
	case '-':
		timerspeed += 10;
		break;
	case 'A':
	case 'a':
		if (!A) {
			A = true;
			B = false;
			for (int i = 0; i < 6; ++i) {
				for (int j = 0; j < 3; ++j) {
					if (i % 2 == 0) {		// 0, 2, 4 는 오른쪽으로 절반씩 x 평행이동
						if (movetype[i]) {
							vertexData[i][j][0] -= (0.05f / 2.0f);
						}
						else {
							vertexData[i][j][0] += (0.05f / 2.0f);
						}
					}
					else {
						if (movetype[i]) {
							vertexData[i][j][0] += (0.05f / 2.0f);
						}
						else {
							vertexData[i][j][0] -= (0.05f / 2.0f);
						}
					}
				}
			}
		}
		break;
	case 'B':
	case 'b':
		if (A) {
			if (!B) {
				B = true;
				A = false;
				for (int i = 0; i < 6; ++i) {
					for (int j = 0; j < 3; ++j) {
						if (i % 2 == 0) {		// 0, 2, 4 는 오른쪽으로 절반씩 x 평행이동
							if (movetype[i]) {
								vertexData[i][j][0] += (0.05f / 2.0f);
							}
							else {
								vertexData[i][j][0] -= (0.05f / 2.0f);
							}
						}
						else {
							if (movetype[i]) {
								vertexData[i][j][0] -= (0.05f / 2.0f);
							}
							else {
								vertexData[i][j][0] += (0.05f / 2.0f);
							}
						}
					}
				}
			}
		}
		break;
	case 'S':
	case 's':
		S = false;
		M = false;
		break;
	case 'Q':
	case 'q':
		glutDestroyWindow(WindowID);
		break;
	}
}

GLvoid Timer(int value) {
	switch (value) {
	case 0:								// 이동 타이머
		for (int i = 0; i < 6; ++i) {
			for (int j = 0; j < 3; ++j) {
				if (movetype[i]) {				// 참이면 오른쪽 이동
					vertexData[i][j][0] += 0.01f;
				}
				else {
					vertexData[i][j][0] -= 0.01f;
				}
			}
			if (vertexData[i][1][0] <= -1.0) {	// 왼쪽 넘어가면
				movetype[i] = true;
				for (int j = 0; j < 3; ++j) {	// y 값 평행이동
					vertexData[i][j][1] -= 0.05f * 2;
				}
			}
			if (vertexData[i][2][0] >= 1.0) {	// 오른쪽 넘어가면
				movetype[i] = false;
				for (int j = 0; j < 3; ++j) {	// y 값 평행이동
					vertexData[i][j][1] -= 0.05f * 2;
				}
			}
		}
		if (S) {
			glutTimerFunc(timerspeed, Timer, 0);
		}
		break;
	}
	InitBuffer();
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
		cerr << "EROOR: vertex shader error\n" << errorlog << endl;
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
		cerr << "ERROR: fragment shader error\n" << errorlog << endl;
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
		cerr << "ERROR: shader program 연결 실패\n" << errorlog << endl;
		return false;
	}

	glUseProgram(ShaderProgramID);

	return ShaderProgramID;
}

GLvoid InitBuffer() {
	// VBO 하나를 통째로 넣고
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	for (int i = 0; i < 6; ++i) {
		// Vertex Array Object 생성
		glGenVertexArrays(1, &VAO[i]);
		glBindVertexArray(VAO[i]);
		// Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(18 * i * sizeof(float)));
		glEnableVertexAttribArray(0);
		// Color
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)((18 * i + 3) * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
}