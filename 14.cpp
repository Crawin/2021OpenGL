#include <iostream>
#include <gl/glew.h> // 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include "FileToBuf.h"

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
GLuint VAO[14], VBO[3], EBO[12];
GLuint shaderProgram, WindowID;
int input;
bool H,W,X,Y,S;
float Xdegree = 30.0f;
float Ydegree = -30.0f;
float MoveX = 0, MoveY = 0, MoveZ = 0;

float xyz[3][2][6]{
	-1,0,0.8,		0,0,0,
	1,0,0.8,		0,0,0,

	0,-1,0.8,		0,0,0,
	0,1,0.8,		0,0,0,

	0,0,-1,		0,0,0,
	0,0,1,		0,0,0
};

float rectData[8][6]{
	0.3,-0.3,0.3,		1,0,0,
	-0.3,-0.3,0.3,		0,1,0,
	-0.3,-0.3,-0.3,		0,0,1,
	0.3,-0.3,-0.3,		1,1,0,

	0.3,0.3,0.3,		1,0,1,
	-0.3,0.3,0.3,		0,1,1,
	-0.3,0.3,-0.3,		1,1,1,
	0.3,0.3,-0.3,		0,0,0
};

float hornData[5][6]{
	0.3,-0.2,0.3,		1,0,0,
	-0.3,-0.2,0.3,		0,0.7,0,
	-0.3,-0.2,-0.3,		0,0,0.8,
	0.3,-0.2,-0.3,		1,0,0.8,
	0,0.3,0,			0.8,0.8,0.8
};

unsigned int rectIndex[6][6]{
	0,4,5,					// 앞
	0,5,1,

	2,6,7,					// 뒤
	2,7,3,

	1,5,6,					// 왼쪽
	1,6,2,

	3,7,4,					// 오른쪽
	3,4,0,

	0,1,2,					//아래
	0,2,3,

	6,5,4,					// 위
	6,4,7
};

unsigned int hornIndex[6][3]{
	0,1,2,
	0,2,3,				// 밑

	4,2,1,				// 왼
	4,0,3,				// 오
	4,3,2,				// 뒤
	4,1,0				// 앞
};

void main(int argc, char** argv)								//--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);										// glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA |GLUT_DEPTH);				// 디스플레이 모드 설정
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
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(RR));
	glLineWidth(2);
	for (int i = 0; i < 3; ++i) {								// 축 긋기
		glBindVertexArray(VAO[i]);
		glDrawArrays(GL_LINES, 0, 2);
	}

	RR = glm::rotate(RR, glm::radians(Xdegree), glm::vec3(1.0, 0.0, 0.0));
	RR = glm::rotate(RR, glm::radians(Ydegree), glm::vec3(0.0, 1.0, 0.0));
	RR = glm::translate(RR, glm::vec3(MoveX, MoveY, MoveZ));
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(RR));
	switch (input) {
	case 'C':
	case 'c':
		for (int i = 3; i < 9; ++i) {
			glBindVertexArray(VAO[i]);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
		break;
	case 'P':
	case 'p':
		glBindVertexArray(VAO[9]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		for (int i = 10; i < 14; ++i) {
			glBindVertexArray(VAO[i]);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
		}
		break;
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
	case 'C':
	case 'c':
	case 'P':
	case 'p':
		input = key;
		break;
	case 'H':
	case 'h':
		if (H) {
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			std::cout << "disable" << std::endl;
		}
		else {
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			std::cout << "enable" << std::endl;
		}
		H = !H;
		break;
	case 'W':
	case 'w':
		if (W) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		W = !W;
		break;
	case 'X':
	case 'x':
		if (X) {
			glutTimerFunc(10, Timer, 1);
		}
		else {
			glutTimerFunc(10, Timer, 0);
		}
		X = !X;
		S = false;
		break;
	case 'Y':
	case 'y':
		if (Y) {
			glutTimerFunc(10, Timer, 3);
		}
		else {
			glutTimerFunc(10, Timer, 2);
		}
		Y = !Y;
		S = false;
		break;
	case 'S':
	case 's':
		S = true;
		Xdegree = 30.0f;
		Ydegree = -30.0f;
		MoveX = MoveY = MoveZ = 0;
		break;
	}
	glutPostRedisplay();										//--- 배경색이 바뀔때마다 출력 콜백함수를 호출하여 화면을 refresh 한다
}

GLvoid Timer(int value) {
	switch (value) {
	case 0:
		Xdegree += 0.5;
		if (X && !S)
			glutTimerFunc(10, Timer, 0);
		break;
	case 1:
		Xdegree -= 0.5;
		if (!X && !S)
			glutTimerFunc(10, Timer, 1);
		break;
	case 2:
		Ydegree += 0.5;
		if (Y && !S)
			glutTimerFunc(10, Timer, 2);
		break;
	case 3:
		Ydegree -= 0.5;
		if (!Y && !S)
			glutTimerFunc(10, Timer, 3);
		break;
	}
	glutPostRedisplay();
}

GLvoid SpecialKeyboard(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_LEFT:
		MoveX -= 0.05;
		break;
	case GLUT_KEY_RIGHT:
		MoveX += 0.05;
		break;
	case GLUT_KEY_UP:
		MoveY += 0.05;
		break;
	case GLUT_KEY_DOWN:
		MoveY -= 0.05;
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
	// VBO에 좌표를 통째로 넣고
	glGenBuffers(1, &VBO[0]);		// 축
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);

	glBufferData(GL_ARRAY_BUFFER, sizeof(xyz), xyz, GL_STATIC_DRAW);

	for (int i = 0; i < 3; ++i) {
		glGenVertexArrays(1, &VAO[i]);
		glBindVertexArray(VAO[i]);
		// Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(12 * i * sizeof(float)));
		glEnableVertexAttribArray(0);
		// Color
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)((12 * i + 3) * sizeof(float)));
		glEnableVertexAttribArray(1);
	}

	glGenBuffers(1, &VBO[1]);		// 정육면체
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectData), rectData, GL_STATIC_DRAW);

	for (int i = 3; i < 9; ++i) {		//3,4,5,6,7,8 정육면체
		glGenVertexArrays(1, &VAO[i]);
		glBindVertexArray(VAO[i]);
		// Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// Color
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glGenBuffers(1, &EBO[i - 3]);	//0,1,2,3,4,5
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i - 3]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectIndex[i - 3]), rectIndex[i - 3], GL_STATIC_DRAW);
	}

	glGenBuffers(1, &VBO[2]);		// 사각뿔
	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(hornData), hornData, GL_STATIC_DRAW);

	for (int i = 9; i < 14; ++i) {		// 9,10,11,12,13 사각뿔
		glGenVertexArrays(1, &VAO[i]);
		glBindVertexArray(VAO[i]);
		// Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// Color
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		if (i == 9) {					// ebo 6 
			glGenBuffers(1, &EBO[i - 3]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i - 3]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * sizeof(hornIndex[0]), hornIndex[0], GL_STATIC_DRAW);
		}
		else {							// ebo 7 8 9 10 , 10 11 12 13
			glGenBuffers(1, &EBO[i - 3]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i - 3]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(hornIndex[i - 8]), hornIndex[i - 8], GL_STATIC_DRAW);
		}
	}
}