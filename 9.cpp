#include <iostream>
#include <gl/glew.h> // 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include "mouse_to_coordinate.h"
#include "FileToBuf.h"
#include <cmath>
#include <random>
using namespace std;

#define WinX 600
#define WinY 600

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();

GLuint shaderID, WindowID;
GLint width, height;
GLuint VAO, VBO;
int dotcnt = 0;
float theta = 1.0f, r = 0.01f, Xmouse,Ymouse;
bool bigtheta;

random_device rd;
default_random_engine dre(rd());
uniform_real_distribution <> urd(0, 10);

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid InitBuffer();
GLvoid Mouse(int button, int state, int xm, int ym);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Timer(int value);
float vertexData[358][6] = {
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
	glutMouseFunc(Mouse);
	glutMainLoop();												// 이벤트 처리 시작
}

GLvoid drawScene()												//--- 콜백 함수: 그리기 콜백 함수
{
	//--- 변경된 배경색 설정
	glClearColor(BRED, BGREEN, BBLUE, 1.0f);											// 바탕색을 변경
	glClear(GL_COLOR_BUFFER_BIT);								// 설정된 색으로 전체를 칠하기
	glUseProgram(shaderID);
	glPointSize(3);
	glBindVertexArray(VAO);
	glDrawArrays(GL_POINTS, 0, dotcnt);

	glutSwapBuffers();											// 화면에 출력하기
}
GLvoid Reshape(int w, int h)									//--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}

GLvoid Mouse(int button, int state, int xm, int ym) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		cout << "X:" << XM2C(WinX,xm) << ", Y:" << YM2C(WinY,ym) << endl;
		Xmouse = XM2C(WinX, xm);
		Ymouse = YM2C(WinY, ym);
		BRED = urd(dre) / 10.0f;
		BGREEN = urd(dre) / 10.0f;
		BBLUE = urd(dre) / 10.0f;
		//배경색 바꿨고
		dotcnt = 0;
		theta = 1.0f;
		r = 0.01f;
		bigtheta = false;
		glutTimerFunc(10, Timer, 0);


		InitBuffer();
	}

	glutPostRedisplay();
}

GLvoid Timer(int value) {	// 점 위치 계산 및 출력 타이머
	if (theta >= 18.84956) {
		bigtheta = true;
	}
	if (bigtheta) {
		vertexData[dotcnt][0] = 0.378 + Xmouse - r * cos(theta);			// X
		vertexData[dotcnt][1] = -r * sin(theta) + Ymouse;			// Y
		vertexData[dotcnt][2] = 0;						// Z
		vertexData[dotcnt][3] = 1.0f;	// R
		vertexData[dotcnt][4] = 1.0f;	// G
		vertexData[dotcnt++][5] = 1.0f;	// B
		theta -= 0.1;
		r -= 0.001;
	}
	else {
		vertexData[dotcnt][0] = r * cos(theta) + Xmouse;			// X
		vertexData[dotcnt][1] = r * sin(theta) + Ymouse;			// Y
		vertexData[dotcnt][2] = 0;						// Z
		vertexData[dotcnt][3] = 1.0f;	// R
		vertexData[dotcnt][4] = 1.0f;	// G
		vertexData[dotcnt++][5] = 1.0f;	// B
		theta += 0.1;
		r += 0.001;
	}
	InitBuffer();
	glutPostRedisplay();
	if (dotcnt < 358) {
		glutTimerFunc(10, Timer, 0);
	}
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

	// Vertex Array Object 생성
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
}