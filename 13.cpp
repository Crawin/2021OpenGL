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

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid InitBuffer();

double RED = 1.0f, GREEN = 1.0f, BLUE = 1.0f;
GLuint VAO[13], VBO[3], EBO[10];
GLuint shaderProgram, WindowID;
int input;

float xyz[3][2][6]{
	-1,0,0,		0,0,0,
	1,0,0,		0,0,0,

	0,-1,0,		0,0,0,
	0,1,0,		0,0,0,

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

float triData[4][6]{
	0,0,-0.5,		1,0,0,
	-0.3,0,0.3,		0,0.7,0,
	0.3,0,0.3,		0,0,0.8,
	0,0.5,0,		1,1,1,
};

unsigned int rectIndex[6][6]{
	0,1,2,					//아래
	0,2,3,

	1,6,5,					// 왼쪽
	1,2,6,

	2,7,6,					// 뒤
	2,3,7,

	3,4,7,					// 오른쪽
	3,0,4,

	0,5,4,					// 앞
	0,1,5,

	6,4,5,					// 위
	6,7,4
};

unsigned int triIndex[4][3]{
	0,1,2,				// 밑
	1,2,3,				// 앞
	0,1,3,				// 왼
	0,3,2				// 오
};

void main(int argc, char** argv)								//--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);										// glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);				// 디스플레이 모드 설정
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
	glClear(GL_COLOR_BUFFER_BIT);								// 설정된 색으로 전체를 칠하기
	glm::mat4 RR(1.0f);
	unsigned int transformLocation = glGetUniformLocation(shaderProgram, "transform");
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(RR));
	glLineWidth(2);
	for (int i = 0; i < 3; ++i) {								// 축 긋기
		glBindVertexArray(VAO[i]);
		glDrawArrays(GL_LINES, 0, 2);
	}
	
	RR = glm::rotate(RR, glm::radians(30.0f), glm::vec3(1.0, 0.0, 0.0));
	RR = glm::rotate(RR, glm::radians(-30.0f), glm::vec3(0.0, 1.0, 0.0));
	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(RR));

	switch (input) {
	case '1':
		glBindVertexArray(VAO[3]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		break;
	case '2':
		glBindVertexArray(VAO[4]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		break;
	case '3':
		glBindVertexArray(VAO[5]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		break;
	case '4':
		glBindVertexArray(VAO[6]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		break;
	case '5':
		glBindVertexArray(VAO[7]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		break;
	case '6':
		glBindVertexArray(VAO[8]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		break;
	case '7':
		glBindVertexArray(VAO[9]);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
		break;
	case '8':
		glBindVertexArray(VAO[10]);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
		break;
	case '9':
		glBindVertexArray(VAO[11]);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
		break;
	case '0':
		glBindVertexArray(VAO[12]);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
		break;
	case 'A':
	case 'a':
		glBindVertexArray(VAO[5]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(VAO[7]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		break;
	case 'B':
	case 'b':
		glBindVertexArray(VAO[4]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(VAO[6]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		break;
	case 'C':
	case 'c':
		glBindVertexArray(VAO[3]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(VAO[8]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		break;
	case 'E':
	case 'e':
		glBindVertexArray(VAO[9]);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
		glBindVertexArray(VAO[10]);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
		break;
	case 'F':
	case 'f':
		glBindVertexArray(VAO[9]);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
		glBindVertexArray(VAO[11]);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
		break;
	case 'G':
	case 'g':
		glBindVertexArray(VAO[9]);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
		glBindVertexArray(VAO[12]);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
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
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'a':
	case 'A':
	case 'b':
	case 'B':
	case 'c':
	case 'C':
	case 'e':
	case 'E':
	case 'f':
	case 'F':
	case 'g':
	case 'G':
		input = key;
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

	for (int i = 3; i < 9; ++i) {
		glGenVertexArrays(1, &VAO[i]);
		glBindVertexArray(VAO[i]);
		// Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// Color
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		
		glGenBuffers(1, &EBO[i-3]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i-3]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectIndex[i-3]), rectIndex[i-3], GL_STATIC_DRAW);
	}

	glGenBuffers(1, &VBO[2]);		// 사면체
	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triData), triData, GL_STATIC_DRAW);

	for (int i = 9; i < 13; ++i) {
		glGenVertexArrays(1, &VAO[i]);
		glBindVertexArray(VAO[i]);
		// Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// Color
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glGenBuffers(1, &EBO[i - 3]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i - 3]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triIndex[i - 9]), triIndex[i - 9], GL_STATIC_DRAW);
	}
}