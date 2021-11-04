#include <iostream>
#include <gl/glew.h> // 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include "mouse_to_coordinate.h"
#include "FileToBuf.h"
#include <random>
using namespace std;

#define WinX 600
#define WinY 600
#define RectSize 0.08

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();

GLuint shaderID, WindowID;
GLint width, height;
GLuint VAO[101], VBO[2], EBO;

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid InitBuffer();
GLvoid Mouse(int button, int state, int xm, int ym);
GLvoid Motion(int x, int y);
float rectData[100][4][6] = {};
float eraserData[4][6] = {};
unsigned int rectindex[] = {
	0,1,2,
	0,2,3
};
random_device rd;
default_random_engine dre(rd());
uniform_real_distribution <> urd{ -100,100 };

bool erase;
double BRED = 1.0f, BGREEN = 1.0f, BBLUE = 1.0f;
void main(int argc, char** argv)								//--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);										// glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);				// 디스플레이 모드 설정
	glutInitWindowPosition(0, 0);								// 윈도우의 위치 지정
	glutInitWindowSize(WinX, WinY);								// 윈도우의 크기 지정
	WindowID = glutCreateWindow("3-12");								// 윈도우 생성 (윈도우 이름)

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)									// glew 초기화
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";
	for (int i = 0; i < 100; ++i) {								// 사각형의 x값을 정하면 나머진 크기 0.08로
		rectData[i][0][0] = ((int)urd(dre) % 92) / 100.0;
		rectData[i][0][1] = ((int)urd(dre) % 92) / 100.0;
		rectData[i][0][3] = urd(dre) / 100.0;
		rectData[i][0][4] = urd(dre) / 100.0;
		rectData[i][0][5] = urd(dre) / 100.0;					// 색상 통일

		rectData[i][1][0] = rectData[i][0][0];
		rectData[i][1][1] = rectData[i][0][1] - RectSize;
		rectData[i][1][3] = rectData[i][0][3];
		rectData[i][1][4] = rectData[i][0][4];
		rectData[i][1][5] = rectData[i][0][5];					// 색상 통일

		rectData[i][2][0] = rectData[i][0][0] + RectSize;
		rectData[i][2][1] = rectData[i][0][1] - RectSize;
		rectData[i][2][3] = rectData[i][0][3];
		rectData[i][2][4] = rectData[i][0][4];
		rectData[i][2][5] = rectData[i][0][5];					// 색상 통일

		rectData[i][3][0] = rectData[i][0][0] + RectSize;
		rectData[i][3][1] = rectData[i][0][1];
		rectData[i][3][3] = rectData[i][0][3];
		rectData[i][3][4] = rectData[i][0][4];
		rectData[i][3][5] = rectData[i][0][5];					// 색상 통일
	}
	InitBuffer();
	shaderID = make_shaderProgram();
	glutDisplayFunc(drawScene);									// 출력 콜백함수의 지정
	glutReshapeFunc(Reshape);									// 다시 그리기 콜백함수 지정
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	glutMainLoop();												// 이벤트 처리 시작
}

GLvoid drawScene()												//--- 콜백 함수: 그리기 콜백 함수
{
	//--- 변경된 배경색 설정
	glClearColor(BRED, BGREEN, BBLUE, 1.0f);											// 바탕색을 변경
	glClear(GL_COLOR_BUFFER_BIT);								// 설정된 색으로 전체를 칠하기
	for (int i = 0; i < 101; ++i) {
		glBindVertexArray(VAO[i]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
	glutSwapBuffers();											// 화면에 출력하기
}
GLvoid Reshape(int w, int h)									//--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}

GLvoid Mouse(int button, int state, int xm, int ym) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		float Cx = XM2C(WinX, xm);
		float Cy = YM2C(WinY, ym);
		cout << "X:" << Cx << ", Y:" << Cy << endl;
		erase = true;
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		eraserData[0][0] = 0;
		eraserData[0][1] = 0;

		eraserData[1][0] = 0;
		eraserData[1][1] = 0;

		eraserData[2][0] = 0;
		eraserData[2][1] = 0;

		eraserData[3][0] = 0;
		eraserData[3][1] = 0;
		erase = false;
	}
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectData), rectData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(eraserData), eraserData, GL_STATIC_DRAW);
	glutPostRedisplay();
}

GLvoid Motion(int xm, int ym) {
	eraserData[0][0] = XM2C(WinX, xm) - RectSize;
	eraserData[0][1] = YM2C(WinY, ym) + RectSize;

	eraserData[1][0] = eraserData[0][0];
	eraserData[1][1] = eraserData[0][1] - RectSize * 2;

	eraserData[2][0] = eraserData[0][0] + RectSize * 2;
	eraserData[2][1] = eraserData[0][1] - RectSize * 2;

	eraserData[3][0] = eraserData[0][0] + RectSize * 2;
	eraserData[3][1] = eraserData[0][1];

	for (int i = 0; i < 100; ++i) {
		if (rectData[i][0][0] + RectSize >= eraserData[0][0] && rectData[i][0][0] + RectSize <= eraserData[3][0] && rectData[i][1][1] + RectSize >= eraserData[1][1] && rectData[i][1][1] + RectSize <= eraserData[0][1]) {
			for (int j = 0; j < 4; ++j) {
				for (int k = 0; k < 1; ++k) {
					rectData[i][j][k] = 0;
				}
			}
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectData), rectData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(eraserData), eraserData, GL_STATIC_DRAW);
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
	glGenBuffers(1, &VBO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);

	glBufferData(GL_ARRAY_BUFFER, sizeof(rectData), rectData, GL_STATIC_DRAW);

	for (int i = 0; i < 100; ++i) {
		glGenVertexArrays(1, &VAO[i]);
		glBindVertexArray(VAO[i]);
		// Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)((24 * i) * sizeof(float)));
		glEnableVertexAttribArray(0);
		// Color
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)((24 * i + 3) * sizeof(float)));
		glEnableVertexAttribArray(1);

		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectindex), rectindex, GL_STATIC_DRAW);
	}

	glGenBuffers(1, &VBO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(eraserData), eraserData, GL_STATIC_DRAW);
	glGenVertexArrays(1, &VAO[100]);
	glBindVertexArray(VAO[100]);
	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectindex), rectindex, GL_STATIC_DRAW);

}