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

GLuint shaderID;
GLint width, height;
GLuint VAO[4], VBO;
bool filled = true, sizeplus;
float sizecnt = 5.0f;
float trisize = sizecnt / 3.0f;
int tricnt = 0;

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int mx, int my);
GLvoid InitBuffer();

float vertexData[4][3][6] = {
	-0.7,0.3,0.0,		0.7,0.0,0.0,
	-0.3,0.3,0.0,		0.7,0.0,0.0,
	-0.5,0.7,0.0,		0.7,0.0,0.0,

	0.3,0.3,0.0,		0.0,0.7,0.0,
	0.7,0.3,0.0,		0.0,0.7,0.0,
	0.5,0.7,0.0,		0.0,0.7,0.0,

	-0.7,-0.7,0.0,		0.0,0.0,0.7,
	-0.3,-0.7,0.0,		0.0,0.0,0.7,
	-0.5,-0.3,0.0,		0.0,0.0,0.7,

	0.3,-0.7,0.0,		0.5,0.5,0.5,
	0.7,-0.7,0.0,		0.5,0.5,0.5,
	0.5,-0.3,0.0,		0.5,0.5,0.5
};

double BRED = 1.0f, BGREEN = 1.0f, BBLUE = 1.0f;
void main(int argc, char** argv)								//--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);										// glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);				// 디스플레이 모드 설정
	glutInitWindowPosition(0, 0);								// 윈도우의 위치 지정
	glutInitWindowSize(WinX, WinY);								// 윈도우의 크기 지정
	glutCreateWindow("3-1");								// 윈도우 생성 (윈도우 이름)

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
	glutKeyboardFunc(Keyboard);									// 키보드 입력 콜백함수 지정
	glutMouseFunc(Mouse);
	glutMainLoop();												// 이벤트 처리 시작
}

GLvoid drawScene()												//--- 콜백 함수: 그리기 콜백 함수
{
	//--- 변경된 배경색 설정
	glClearColor(BRED, BGREEN, BBLUE, 1.0f);											// 바탕색을 변경
	glClear(GL_COLOR_BUFFER_BIT);								// 설정된 색으로 전체를 칠하기
	glUseProgram(shaderID);
	for (int i = 0; i < 4; ++i) {
		glBindVertexArray(VAO[i]);
		if (filled) {
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
		else {
			glDrawArrays(GL_LINE_LOOP, 0, 3);
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
	case 'A':
	case 'a':
		filled = true;
		break;
	case 'B':
	case 'b':
		filled = false;
		break;
	}
	glutPostRedisplay();										//--- 배경색이 바뀔때마다 출력 콜백함수를 호출하여 화면을 refresh 한다
}
GLvoid Mouse(int button, int state, int mx, int my) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (sizecnt == 3) {
			sizeplus = true;
		}
		else if (sizecnt == 6) {
			sizeplus = false;
		}
		if (sizeplus) {
			trisize = ++sizecnt / 30.0f;		// 사이즈부터 설정해주고
		}
		else {
			trisize = --sizecnt / 30.0f;
		}
		vertexData[tricnt][0][0] = XM2C(WinX, mx) - (sqrt(3) * trisize);
		vertexData[tricnt][0][1] = YM2C(WinY, my) - trisize;
		vertexData[tricnt][1][0] = XM2C(WinX, mx) + (sqrt(3) * trisize);
		vertexData[tricnt][1][1] = YM2C(WinY, my) - trisize;
		vertexData[tricnt][2][0] = XM2C(WinX, mx);
		vertexData[tricnt][2][1] = YM2C(WinY, my) + (trisize * 2);
		glBindVertexArray(VAO[tricnt]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
		/*switch (tricnt) {
		case 0:
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
			break;
		case 1:
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(18 * sizeof(float)));
			break;
		case 2:
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(36 * sizeof(float)));
			break;
		case 3:
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(54 * sizeof(float)));
			break;
		}*/
		
		if (++tricnt == 4) {
			tricnt = 0;
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
	// Vertex Array Object 생성
	glGenVertexArrays(1, &VAO[0]);
	glBindVertexArray(VAO[0]);
	// VBO 하나를 통째로 넣고
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Vertex Array Object 생성
	glGenVertexArrays(1, &VAO[1]);
	glBindVertexArray(VAO[1]);
	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(18 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(21 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Vertex Array Object 생성
	glGenVertexArrays(1, &VAO[2]);
	glBindVertexArray(VAO[2]);
	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(36 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(39 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Vertex Array Object 생성
	glGenVertexArrays(1, &VAO[3]);
	glBindVertexArray(VAO[3]);
	// Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(54 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(57 * sizeof(float)));
	glEnableVertexAttribArray(1);
}