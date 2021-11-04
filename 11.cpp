#include <iostream>
#include <gl/glew.h> // 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include "mouse_to_coordinate.h"
#include "FileToBuf.h"
using namespace std;

#define WinX 600
#define WinY 600

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();

GLuint shaderID, WindowID;
GLint width, height;
GLuint VAO[7], VBO, EBO;

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid InitBuffer();
GLvoid Mouse(int button, int state, int xm, int ym);
GLvoid Motion(int x, int y);
float vertexData[] = {
	-1,0,0,			0,0,0,
	1,0,0,			0,0,0,			// x축

	0,-1,0,			0,0,0,
	0,1,0,			0,0,0,			// y축

	-0.6,0.6,0,		1.0,0.0,0.0,	// left	0		24
	-0.6,-0.6,0,	0.0,1.0,0.0,	// left	1		30

	-0.6,-0.6,0,	0.0,1.0,0.0,	// bottom	1	36
	0.6,-0.6,0,		0.0,0.0,1.0,	// bottom	2	42
	
	0.6,-0.6,0,		0.0,0.0,1.0,	// right	2	48
	0.6,0.6,0,		0.0,0.0,0.0,	// right	3	54

	0.6,0.6,0,		0.0,0.0,0.0,	// top	3		60
	-0.6,0.6,0,		1.0,0.0,0.0		// top	0		66
};
typedef struct {
	float firstx;
	float firsty;
	float secondx;
	float secondy;
}Point;

int click = 10;
float CPx, CPy;
Point p[4];
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
	glutMotionFunc(Motion);
	glutMainLoop();												// 이벤트 처리 시작
}

GLvoid drawScene()												//--- 콜백 함수: 그리기 콜백 함수
{
	//--- 변경된 배경색 설정
	glClearColor(BRED, BGREEN, BBLUE, 1.0f);											// 바탕색을 변경
	glClear(GL_COLOR_BUFFER_BIT);								// 설정된 색으로 전체를 칠하기
	glLineWidth(2);
	for (int i = 0; i < 7; ++i) {
		glBindVertexArray(VAO[i]);								// x ,y draw
		glDrawArrays(GL_LINES, 0, 2);
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
		if (Cx >= (vertexData[24] - 0.05) && Cx <= (vertexData[24] + 0.05) && Cy >= (vertexData[25] - 0.05) && Cy <= (vertexData[25] + 0.05)) {
			// 좌상단 클릭되면
			click = 0;
		}
		else if (Cx >= (vertexData[36] - 0.05) && Cx <= (vertexData[36] + 0.05) && Cy >= (vertexData[37] - 0.05) && Cy <= (vertexData[37] + 0.05)) {
			// 좌하단 클릭되면
			click = 1;
		}
		else if (Cx >= (vertexData[48] - 0.05) && Cx <= (vertexData[48] + 0.05) && Cy >= (vertexData[49] - 0.05) && Cy <= (vertexData[49] + 0.05)) {
			// 우하단 클릭되면
			click = 2;
		}
		else if (Cx >= (vertexData[60] - 0.05) && Cx <= (vertexData[60] + 0.05) && Cy >= (vertexData[61] - 0.05) && Cy <= (vertexData[61] + 0.05)) {
			// 우상단 클릭되면
			click = 3;
		}
		else if ((Cx >= vertexData[24] && Cx <= vertexData[60] && Cy >= vertexData[31] && Cy <= vertexData[25]) || Cx >= vertexData[30] && Cx <= vertexData[42] && Cy >= vertexData[43] && Cy <= vertexData[55]) {
			// 사각형 안에 있으면
			click = 4;
			CPx = Cx;
			CPy = Cy;
			p[0].firstx = vertexData[24];
			p[0].firsty = vertexData[25];
			p[0].secondx = vertexData[66];
			p[0].secondy = vertexData[67];

			p[1].firstx = vertexData[30];
			p[1].firsty = vertexData[31];
			p[1].secondx = vertexData[36];
			p[1].secondy = vertexData[37];

			p[2].firstx = vertexData[42];
			p[2].firsty = vertexData[43];
			p[2].secondx = vertexData[48];
			p[2].secondy = vertexData[49];

			p[3].firstx = vertexData[54];
			p[3].firsty = vertexData[55];
			p[3].secondx = vertexData[60];
			p[3].secondy = vertexData[61];
		}
	}

	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		click = 10;
	}
	glutPostRedisplay();
}

GLvoid Motion(int x, int y){
	float Cx = XM2C(WinX, x);
	float Cy = YM2C(WinY, y);
	switch (click) {
	case 0:
		vertexData[24] = Cx;
		vertexData[25] = Cy;
		vertexData[66] = Cx;
		vertexData[67] = Cy;
		break;
	case 1:
		vertexData[30] = Cx;
		vertexData[31] = Cy;
		vertexData[36] = Cx;
		vertexData[37] = Cy;
		break;
	case 2:
		vertexData[42] = Cx;
		vertexData[43] = Cy;
		vertexData[48] = Cx;
		vertexData[49] = Cy;
		break;
	case 3:
		vertexData[54] = Cx;
		vertexData[55] = Cy;
		vertexData[60] = Cx;
		vertexData[61] = Cy;
		break;
	case 4: {
		float differX = CPx - Cx;
		float differY = CPy - Cy;
		vertexData[24] = p[0].firstx - differX;
		vertexData[66] = p[0].secondx - differX;
		vertexData[25] = p[0].firsty - differY;
		vertexData[67] = p[0].secondy - differY;

		vertexData[30] = p[1].firstx - differX;
		vertexData[36] = p[1].secondx - differX;
		vertexData[31] = p[1].firsty - differY;
		vertexData[37] = p[1].secondy - differY;

		vertexData[42] = p[2].firstx - differX;
		vertexData[48] = p[2].secondx - differX;
		vertexData[43] = p[2].firsty - differY;
		vertexData[49] = p[2].secondy - differY;

		vertexData[54] = p[3].firstx - differX;
		vertexData[60] = p[3].secondx - differX;
		vertexData[55] = p[3].firsty - differY;
		vertexData[61] = p[3].secondy - differY;
		break;
	}
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

	for (int i = 0; i < 7; ++i) {
		glGenVertexArrays(1, &VAO[i]);
		glBindVertexArray(VAO[i]);
		// Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)((12 * i) * sizeof(float)));
		glEnableVertexAttribArray(0);
		// Color
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)((12 * i + 3) * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
}