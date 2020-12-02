#include <gl/glew.h>
#include <GL/freeglut.h>
// GLM library - for matrix manipulation
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_aligned.hpp>
// Copy the GLM folder to the "include" folder of Visual C++
#include <fstream>
#include <iostream>
#include <string>

#pragma warning(disbale:4996)

GLuint VAO;		// ID for vertex array objects
GLuint VBO;		// ID for vertex buffer objects
GLuint program;


// function to load shaders
GLuint loadShaders(const std::string vertexShaderFile, const std::string fragmentShaderFile)
{
	GLint status;	// for checking compile and linking status

					// load vertex shader code from file
	std::string vertexShaderCode;	// to store shader code
	std::ifstream vertexShaderStream(vertexShaderFile, std::ios::in);	// open file stream

															// check whether file stream was successfully opened
	if (vertexShaderStream.is_open())
	{
		// read from stream line by line and append it to shader code
		std::string line = "";
		while (std::getline(vertexShaderStream, line))
			vertexShaderCode += line + "\n";

		vertexShaderStream.close();		// no longer need file stream
	}
	else
	{
		// output error message and exit
		std::cout << "Failed to open vertex shader file - " << vertexShaderFile << std::endl;
		exit(EXIT_FAILURE);
	}

	// load fragment shader code from file
	std::string fragmentShaderCode;	// to store shader code
	std::ifstream fragmentShaderStream(fragmentShaderFile, ios::in);	// open file stream

																// check whether file stream was successfully opened
	if (fragmentShaderStream.is_open())
	{
		// read from stream line by line and append it to shader code
		std::string line = "";
		while (std::getline(fragmentShaderStream, line))
			fragmentShaderCode += line + "\n";

		fragmentShaderStream.close();	// no longer need file stream
	}
	else
	{
		// output error message and exit
		std::cout << "Failed to open fragment shader file - " << fragmentShaderFile << std::endl;
		exit(EXIT_FAILURE);
	}

	// create shader objects
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// provide source code for shaders
	const GLchar* vShaderCode = vertexShaderCode.c_str();
	const GLchar* fShaderCode = fragmentShaderCode.c_str();
	glShaderSource(vertexShaderID, 1, &vShaderCode, NULL);
	glShaderSource(fragmentShaderID, 1, &fShaderCode, NULL);

	// compile vertex shader
	glCompileShader(vertexShaderID);

	// check compile status
	status = GL_FALSE;
	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE)
	{
		// output error message
		std::cout << "Failed to compile vertex shader - " << vertexShaderFile << std::endl;

		// output error information
		int infoLogLength;
		glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* errorMessage = new char[infoLogLength + 1];
		glGetShaderInfoLog(vertexShaderID, infoLogLength, NULL, errorMessage);
		std::cout << errorMessage << std::endl;
		delete[] errorMessage;

		exit(EXIT_FAILURE);
	}

	// compile fragment shader
	glCompileShader(fragmentShaderID);

	// check compile status
	status = GL_FALSE;
	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE)
	{
		// output error message
		std::cout << "Failed to compile fragment shader - " << fragmentShaderFile << std::endl;

		// output error information
		int infoLogLength;
		glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* errorMessage = new char[infoLogLength + 1];
		glGetShaderInfoLog(fragmentShaderID, infoLogLength, NULL, errorMessage);
		std::cout << errorMessage << std::endl;
		delete[] errorMessage;

		exit(EXIT_FAILURE);
	}

	// create program
	GLuint programID = glCreateProgram();

	// attach shaders to the program object
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);

	// flag shaders for deletion (will not be deleted until detached from program)
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	// link program object
	glLinkProgram(programID);

	// check link status
	status = GL_FALSE;
	glGetProgramiv(programID, GL_LINK_STATUS, &status);

	if (status == GL_FALSE)
	{
		// output error message
		std::cout << "Failed to link program object." << std::endl;

		// output error information
		int infoLogLength;
		glGetShaderiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* errorMessage = new char[infoLogLength + 1];
		glGetShaderInfoLog(programID, infoLogLength, NULL, errorMessage);
		std::cout << errorMessage << std::endl;
		delete[] errorMessage;

		exit(EXIT_FAILURE);
	}

	return programID;
}
