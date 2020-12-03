#include <gl/glew.h>
#include <gl/freeglut.h>
// GLM library - for matrix manipulation
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// Copy the GLM folder to the "include" folder of Visual C++
#include <fstream>
#include <iostream>
#include <string>
// library to read image files
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// global constants
#define WORLD_SIZE 33
#define MAX_HEIGHT 5.0
#define ROUGHNESS 1.5
#define VAO_SIZE 1
#define VBO_SIZE 1
#define TEXTURES 2

#pragma warning(disable:4996)

// --------------------------------------------------------------------------------
// Global variables
// --------------------------------------------------------------------------------

// OpenGL variables
GLuint VAO[VAO_SIZE];
GLuint VBO[VBO_SIZE];
GLuint program;

// terrain
struct terrain { glm::vec3 vertex; glm::vec3 normal; glm::vec2 tex_coord; };
const int QUADS_PER_DIMENSION = WORLD_SIZE - 1;
const int NUM_OF_VERTICES = QUADS_PER_DIMENSION * QUADS_PER_DIMENSION * 4;

float heightField[WORLD_SIZE][WORLD_SIZE];
glm::vec3 vertexCoord[WORLD_SIZE][WORLD_SIZE];
glm::vec3 vertexNormal[WORLD_SIZE][WORLD_SIZE];
struct terrain ground[NUM_OF_VERTICES];

// water
// vertex coord X Y Z, normal vector X Y Z, texture coord S T)
float water[] = {
	-WORLD_SIZE / 2.0f, 0.0f, -WORLD_SIZE / 2.0f,	0.0f, 1.0f, 0.0f,	0.0f, 1.0f,
	-WORLD_SIZE / 2.0f, 0.0f, +WORLD_SIZE / 2.0f,	0.0f, 1.0f, 0.0f,	0.0f, 0.0f,
	+WORLD_SIZE / 2.0f, 0.0f, +WORLD_SIZE / 2.0f,	0.0f, 1.0f, 0.0f,	1.0f, 0.0f,
	+WORLD_SIZE / 2.0f, 0.0f, -WORLD_SIZE / 2.0f,	0.0f, 1.0f, 0.0f,	1.0f, 1.0f,
};

// predefined matrix type from GLM
glm::mat4 model;
glm::mat4 view;
glm::mat4 proj;

// camera position and facing direction
GLfloat camX = 0.0f;
GLfloat camY = MAX_HEIGHT * 2.0f;
GLfloat camZ = WORLD_SIZE / 2.0f;
GLfloat dirX = 0.0f;
GLfloat dirY = MAX_HEIGHT / 2.0f;
GLfloat dirZ = 0.0f;

// textures
unsigned int textureID[TEXTURES];

// other options variables
int obj = 0;
bool odd = false;

// --------------------------------------------------------------------------------
// Function prototypes
// --------------------------------------------------------------------------------

GLuint loadShaders(const std::string, const std::string);
unsigned int loadTexture(unsigned int ID, char* file);
void init(void);
float randomize(float);
glm::vec3 calculateNormal(glm::vec3, glm::vec3, glm::vec3);
void generateTerrain(float, float, float, float);

// --------------------------------------------------------------------------------
// Functions
// --------------------------------------------------------------------------------

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
	std::ifstream fragmentShaderStream(fragmentShaderFile, std::ios::in);	// open file stream

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

// function to load textures
unsigned int loadTexture(unsigned int ID, const char* file) {
	unsigned int textureID;
	// generate 1 texture with ID of "textureID"
	glGenTextures(1, &textureID);
	// activate the texture unit first before binding texture
	glActiveTexture(GL_TEXTURE0 + ID);
	// bind textureID before use
	glBindTexture(GL_TEXTURE_2D, textureID);
	int width, height, nrChannels;
	unsigned char* data = stbi_load(file, &width, &height, &nrChannels, 0);

	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		// set texture behaviour
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	return textureID;
}

// function to generate a random value between [-d, +d]
float randomize(float d) {
	float randVal = ((float)rand()) / (float)RAND_MAX;
	return (randVal * 2 * d) - d;
}

// function to calculate normal vector based on vector cross-product
glm::vec3 calculateNormal(glm::vec3 A, glm::vec3 B, glm::vec3 C) {
	return glm::normalize(glm::cross(B - A, C - A));
}

// function to generate terrain data and store in array of "ground"
void generateTerrain(float UL, float LL, float LR, float UR) {
	int i = WORLD_SIZE;
	int midX, midZ;
	float d = MAX_HEIGHT;
	// initial height at the 4 corners of the terrain
	const int first = 0, last = WORLD_SIZE - 1;
	heightField[first][first] = UL;
	heightField[first][last] = LL;
	heightField[last][last] = LR;
	heightField[last][first] = UR;

	// calculate height of ground - using midpoint displacement algorithm
	while (i > 1) {
		for (int x = 0; x < WORLD_SIZE; x += i) {
			for (int z = 0; z < WORLD_SIZE; z += i) {
				// square step
				midX = x + i / 2;
				midZ = z + i / 2;

				const float x0z0 = heightField[x][z];
				const float xiz0 = heightField[x + i][z];
				const float x0zi = heightField[x][z + i];
				const float xizi = heightField[x + i][z + i];
				const float xMidzMid = heightField[midX][midZ];

				heightField[midX][midZ] = (x0z0 + x0zi + xizi + xiz0) / 4;

				// diamond steps
				heightField[x][midZ] = (x0z0 + x0zi + xMidzMid) / 3 + randomize(d);
				heightField[x + i][midZ] = (xizi + xiz0 + xMidzMid) / 3 + randomize(d);
				heightField[midX][z] = (x0z0 + xiz0 + xMidzMid) / 3 + randomize(d);
				heightField[midX][z + i] = (x0zi + xizi + xMidzMid) / 3 + randomize(d);
			}
		}
		i /= 2;
		d *= glm::pow(2, -ROUGHNESS);
	}

	// calculate normal vector
	const float halfSize = (WORLD_SIZE - 1) / 2.0f;
	i = 0;
	for (int x = 0; x < WORLD_SIZE; x++) {
		for (int z = 0; z < WORLD_SIZE; z++) {
			vertexCoord[x][z] = glm::vec3(x - halfSize, heightField[x][z], z - halfSize);
		}
	}

	// calculate normal vector for each vertex of terrain
	for (int x = 0; x < WORLD_SIZE; x++) {
		for (int z = 0; z < WORLD_SIZE; z++) {
			// P = Plus 1, M = Minus 1
			const glm::vec3 x0z0 = vertexCoord[x][z];
			const glm::vec3 xPz0 = vertexCoord[x + 1][z];
			const glm::vec3 x0zP = vertexCoord[x][z + 1];
			const glm::vec3 xPzP = vertexCoord[x + 1][z + 1];
			const glm::vec3 xMz0 = vertexCoord[x - 1][z];
			const glm::vec3 x0zM = vertexCoord[x][z - 1];

			vertexNormal[x][z] =
				x == 0 && z == 0
				? calculateNormal(x0z0, x0zP, xPz0)
				: x == WORLD_SIZE - 1 && z == 0
				? calculateNormal(x0z0, xMz0, x0zP)
				: x == 0 && z == WORLD_SIZE - 1
				? calculateNormal(x0z0, xPz0, x0zM)
				: x == WORLD_SIZE - 1 && z == WORLD_SIZE - 1
				? calculateNormal(x0z0, x0zM, xMz0)
				: x == 0
				? glm::normalize((calculateNormal(x0z0, xPzP, x0zM) + calculateNormal(x0z0, x0zP, xPzP)) / 2.0f)
				: x == WORLD_SIZE - 1
				? glm::normalize((calculateNormal(x0z0, x0zM, xMz0) + calculateNormal(x0z0, xMz0, x0zP)) / 2.0f)
				: z == 0
				? glm::normalize((calculateNormal(x0z0, xMz0, x0zP) + calculateNormal(x0z0, x0zP, xPz0)) / 2.0f)
				: z == WORLD_SIZE - 1
				? glm::normalize((calculateNormal(x0z0, x0zM, xMz0) + calculateNormal(x0z0, xPz0, x0zM)) / 2.0f)
				: glm::normalize((
					calculateNormal(x0z0, xMz0, x0zP) +
					calculateNormal(x0z0, x0zP, xPz0) +
					calculateNormal(x0z0, xPz0, x0zM) +
					calculateNormal(x0z0, x0zM, xMz0))
					/ 4.0f);
		}
	}

	// fill in data to "ground" array
	i = 0;
	for (int x = 0; x < WORLD_SIZE - 1; x++) {
		for (int z = 0; z < WORLD_SIZE - 1; z++) {
			ground[i].vertex = glm::vec3(x - halfSize, heightField[x][z], z - halfSize);
			ground[i].normal = vertexNormal[x][z];
			ground[i++].tex_coord = glm::vec2(0.0, 1.0);

			ground[i].vertex = glm::vec3(x - halfSize, heightField[x][z + 1], z + 1 - halfSize);
			ground[i].normal = vertexNormal[x][z + 1];
			ground[i++].tex_coord = glm::vec2(0.0, 0.0);

			ground[i].vertex = glm::vec3(x + 1 - halfSize, heightField[x + 1][z + 1], z + 1 - halfSize);
			ground[i].normal = vertexNormal[x + 1][z + 1];
			ground[i++].tex_coord = glm::vec2(1.0, 0.0);

			ground[i].vertex = glm::vec3(x + 1 - halfSize, heightField[x + 1][z], z - halfSize);
			ground[i].normal = vertexNormal[x + 1][z];
			ground[i++].tex_coord = glm::vec2(1.0, 1.0);
		}
	}
}

// function to initialize the program
void init(void) {
	generateTerrain(6.9, 3.1, 2.5, 3.0);

	glGenVertexArrays(VAO_SIZE, VAO);
	glBindVertexArray(VAO[0]);

	// terrain
	glGenBuffers(VBO_SIZE, VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ground), ground, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// water
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(water), water, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);

	// program
	program = loadShaders("vertexShader.glsl", "fragmentShader.glsl");
	glUseProgram(program);

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.33, 0.33, 0.33, 1.0);

	// projection matrix (fov, aspect, near, far)
	proj = glm::perspective(glm::radians(45.0f), 1.8f, 0.1f, 200.0f);
	unsigned int projLoc = glGetUniformLocation(program, "proj");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

	// model matrix
	model = glm::mat4(1.0f);
	unsigned int modelLoc = glGetUniformLocation(program, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// texture
	textureID[0] = loadTexture(0, "textures/grass.jpg");
	textureID[1] = loadTexture(1, "textures/water.jpg");
	srand(0);
	glutFullScreen();
}
