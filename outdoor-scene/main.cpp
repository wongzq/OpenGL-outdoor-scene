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
#define WORLD_SIZE 65
#define MAX_HEIGHT 5.0
#define ROUGHNESS 1.5
#define VAO_SIZE 1

#pragma warning(disable:4996)

// --------------------------------------------------------------------------------
// Global variables
// --------------------------------------------------------------------------------

// OpenGL variables
enum Object { VBO_WATER, VBO_TERRAIN, VBO_SKY, VBO_SIZE };
GLuint VAO[VAO_SIZE];
GLuint VBO[VBO_SIZE];
GLuint program;

// terrain
struct terrain { glm::vec3 vertex; glm::vec3 normal; glm::vec2 tex_coord; };
const int QUADS_PER_DIMENSION = WORLD_SIZE - 1;
const int VERTICES = QUADS_PER_DIMENSION * QUADS_PER_DIMENSION * 4;

float heightField[WORLD_SIZE][WORLD_SIZE];
glm::vec3 vertexCoord[WORLD_SIZE][WORLD_SIZE];
glm::vec3 vertexNormal[WORLD_SIZE][WORLD_SIZE];
struct terrain ground[VERTICES];

// water
// vertex coord X Y Z, normal vector X Y Z, texture coord S T)
float water[] = {
	-WORLD_SIZE / 2.0f, 0.0f, -WORLD_SIZE / 2.0f,	0.0f, 1.0f, 0.0f,	0.0f, 0.0f,
	-WORLD_SIZE / 2.0f, 0.0f, +WORLD_SIZE / 2.0f,	0.0f, 1.0f, 0.0f,	0.0f, 1.0f,
	+WORLD_SIZE / 2.0f, 0.0f, +WORLD_SIZE / 2.0f,	0.0f, 1.0f, 0.0f,	1.0f, 1.0f,
	+WORLD_SIZE / 2.0f, 0.0f, -WORLD_SIZE / 2.0f,	0.0f, 1.0f, 0.0f,	1.0f, 0.0f,
};

// sky
glm::vec3 skyColor = glm::vec3(3.0f, 3.0f, 3.0f);
// vertex coord X Y Z, normal vector X Y Z
float sky[] = {
	-WORLD_SIZE, +WORLD_SIZE / 1.50f, -WORLD_SIZE / 2.0f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f,
	-WORLD_SIZE, -WORLD_SIZE / 15.0f, -WORLD_SIZE / 2.0f,	0.0f, 0.0f, 1.0f,	0.0f, 1.0f,
	+WORLD_SIZE, -WORLD_SIZE / 15.0f, -WORLD_SIZE / 2.0f,	0.0f, 0.0f, 1.0f,	1.0f, 1.0f,
	+WORLD_SIZE, +WORLD_SIZE / 1.50f, -WORLD_SIZE / 2.0f,	0.0f, 0.0f, 1.0f,	1.0f, 0.0f,
};

// predefined matrix type from GLM
glm::mat4 model;
glm::mat4 view;
glm::mat4 proj;

// camera position and facing direction
GLfloat camX = 0.0f;
GLfloat camY = MAX_HEIGHT * 1.0f;
GLfloat camZ = WORLD_SIZE / 2.0f;
GLfloat dirX = 0.0f;
GLfloat dirY = MAX_HEIGHT / 2.0f;
GLfloat dirZ = 0.0f;

// light
glm::vec3 sunlightPos = { 0, WORLD_SIZE, 0 };
glm::vec3 sunlightColor = { 10.0f, 10.0f, 10.0f };

// textures
enum Texture { WATER, GRASS, FOREST, SAND, EARTH, SKY, TEXTURES };
unsigned int textureID[TEXTURES];
unsigned int groundTexture = 1;

// frames-per-second (FPS)
int counter = 0, s_time = 0, e_time = 0;
float curFPS;
char curFPSstr[50] = "0.0";

// other options variables
int obj = 0;
int ripple = 0;
bool antiAliasing = false;
bool showMenu = true;
int curTextLoc, startTextLoc;

// --------------------------------------------------------------------------------
// Function prototypes
// --------------------------------------------------------------------------------

GLuint loadShaders(const std::string, const std::string);
unsigned int loadTexture(unsigned int ID, char* file);
float randomize(double);
glm::vec3 calculateNormal(glm::vec3, glm::vec3, glm::vec3);
void generateTerrain(float, float, float, float);
void init(void);
void drawWater(void);
void drawGround(void);
void drawSky(void);
void drawLight(void);
int textLoc(void);
void drawText(int, int, char*);
void drawMenu(void);
void display(void);
void update(int);
void updateFPS(int);
void specialKey(int, int, int);
void keyboardKey(unsigned char, int, int);
int main(int, char**);

// --------------------------------------------------------------------------------
// Functions
// --------------------------------------------------------------------------------

// function to load shaders
GLuint loadShaders(const std::string vShaderFile, const std::string fShaderFile) {
	GLint status;	// to check compile and linking status

	// VERTEX SHADER
	// load vertex shader code from file
	std::string vShaderCodeStr;
	std::ifstream vShaderStream(vShaderFile, std::ios::in);
	if (vShaderStream.is_open()) {
		// read from stream line by line and append it to shader code
		std::string line = "";
		while (std::getline(vShaderStream, line))
			vShaderCodeStr += line + "\n";
		vShaderStream.close();
	}
	else {
		// output error message and exit
		std::cout << "Failed to open vertex shader file - " << vShaderFile << std::endl;
		exit(EXIT_FAILURE);
	}

	// FRAGMENT SHADER
	// load fragment shader code from file
	std::string fShaderCodeStr;
	std::ifstream fShaderStream(fShaderFile, std::ios::in);
	if (fShaderStream.is_open()) {
		// read from stream line by line and append it to shader code
		std::string line = "";
		while (std::getline(fShaderStream, line))
			fShaderCodeStr += line + "\n";
		fShaderStream.close();
	}
	else {
		// output error message and exit
		std::cout << "Failed to open fragment shader file - " << fShaderFile << std::endl;
		exit(EXIT_FAILURE);
	}

	// compile vertex shader
	GLuint vShaderID = glCreateShader(GL_VERTEX_SHADER);
	const GLchar* vShaderCode = vShaderCodeStr.c_str();
	glShaderSource(vShaderID, 1, &vShaderCode, NULL);

	status = GL_FALSE;
	glCompileShader(vShaderID);
	glGetShaderiv(vShaderID, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE) {
		std::cout << "Failed to compile vertex shader - " << vShaderFile << std::endl;
		int infoLogLength;
		glGetShaderiv(vShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* errorMsg = new char[static_cast<__int64>(infoLogLength) + 1];
		glGetShaderInfoLog(vShaderID, infoLogLength, NULL, errorMsg);
		std::cout << errorMsg << std::endl;
		delete[] errorMsg;
		exit(EXIT_FAILURE);
	}

	// compile fragment shader
	GLuint fShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	const GLchar* fShaderCode = fShaderCodeStr.c_str();
	glShaderSource(fShaderID, 1, &fShaderCode, NULL);

	status = GL_FALSE;
	glCompileShader(fShaderID);
	glGetShaderiv(fShaderID, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE) {
		std::cout << "Failed to compile fragment shader - " << fShaderFile << std::endl;
		int infoLogLength;
		glGetShaderiv(fShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* errorMsg = new char[static_cast<__int64>(infoLogLength) + 1];
		glGetShaderInfoLog(fShaderID, infoLogLength, NULL, errorMsg);
		std::cout << errorMsg << std::endl;
		delete[] errorMsg;
		exit(EXIT_FAILURE);
	}

	// create program
	GLuint programID = glCreateProgram();
	// attach shaders to program object
	glAttachShader(programID, vShaderID);
	glAttachShader(programID, fShaderID);
	// link program object
	glLinkProgram(programID);

	// check link status
	status = GL_FALSE;
	glGetProgramiv(programID, GL_LINK_STATUS, &status);

	if (status == GL_FALSE) {
		std::cout << "Failed to link program object." << std::endl;
		int infoLogLength;
		glGetShaderiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* errorMsg = new char[static_cast<__int64>(infoLogLength) + 1];
		glGetShaderInfoLog(programID, infoLogLength, NULL, errorMsg);
		std::cout << errorMsg << std::endl;
		delete[] errorMsg;
		exit(EXIT_FAILURE);
	}

	return programID;
}

// function to load textures
unsigned int loadTexture(unsigned int ID, char* file) {
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
float randomize(double d) {
	float randVal = ((float)rand()) / (float)RAND_MAX;
	return (randVal * 2 * (float)d) - (float)d;
}

// function to calculate normal vector based on vector cross-product
glm::vec3 calculateNormal(glm::vec3 A, glm::vec3 B, glm::vec3 C) {
	return glm::normalize(glm::cross(B - A, C - A));
}

// function to generate terrain data and store in array of "ground"
void generateTerrain(float UL, float LL, float LR, float UR) {
	int i = WORLD_SIZE;
	int midX, midZ;
	double d = MAX_HEIGHT;
	// initial height at the 4 corners of the terrain
	const int first = 0;
	const int last = WORLD_SIZE - 1;
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
				const float xiz0 = x + i < WORLD_SIZE ? heightField[x + i][z] : x0z0;
				const float x0zi = z + i < WORLD_SIZE ? heightField[x][z + i] : x0z0;
				const float xizi = (x + i < WORLD_SIZE) && (z + i < WORLD_SIZE) ? heightField[x + i][z + i] : x0z0;
				const float xMidzMid = (x0z0 + x0zi + xizi + xiz0) / 4 + randomize(d);

				// diamond steps
				heightField[midX][midZ] = xMidzMid;
				heightField[x][midZ] = (x0z0 + x0zi + xMidzMid) / 3 + randomize(d);
				heightField[midX][z] = (x0z0 + xiz0 + xMidzMid) / 3 + randomize(d);
				if (x + i < WORLD_SIZE) heightField[x + i][midZ] = (float)((xizi + xiz0 + xMidzMid) / 3 + randomize(d));
				if (z + i < WORLD_SIZE) heightField[midX][z + i] = (float)((x0zi + xizi + xMidzMid) / 3 + randomize(d));
			}
		}
		i /= 2;
		d *= glm::pow(2, -ROUGHNESS);
	}

	// calculate normal vector
	const float halfSize = (WORLD_SIZE - 1) / 2.0f;
	for (int x = 0; x < WORLD_SIZE; x++)
		for (int z = 0; z < WORLD_SIZE; z++)
			vertexCoord[x][z] = glm::vec3(x - halfSize, heightField[x][z], z - halfSize);

	// calculate normal vector for each vertex of terrain
	for (int x = 0; x < WORLD_SIZE; x++) {
		for (int z = 0; z < WORLD_SIZE; z++) {
			// P = Plus 1, M = Minus 1
			const glm::vec3 x0z0 = vertexCoord[x][z];
			const glm::vec3 xPz0 = vertexCoord[x + 1][z];
			const glm::vec3 x0zP = vertexCoord[x][z + 1];
			const glm::vec3 xPzP = vertexCoord[x + 1][z + 1];
			const glm::vec3 xMz0 = x - 1 >= 0 ? vertexCoord[x - 1][z] : x0z0;
			const glm::vec3 x0zM = z - 1 >= 0 ? vertexCoord[x][z - 1] : x0z0;

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
					calculateNormal(x0z0, x0zM, xMz0)) / 4.0f);
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
	generateTerrain(5.0f, 1.0f, -5.0f, 5.0f);

	glGenVertexArrays(VAO_SIZE, VAO);
	glBindVertexArray(VAO[0]);

	glGenBuffers(VBO_SIZE, VBO);

	// terrain
	glBindBuffer(GL_ARRAY_BUFFER, VBO[Object::VBO_WATER]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(water), water, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// water
	glBindBuffer(GL_ARRAY_BUFFER, VBO[Object::VBO_TERRAIN]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ground), ground, GL_STATIC_DRAW);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);

	//// sky
	glBindBuffer(GL_ARRAY_BUFFER, VBO[Object::VBO_SKY]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sky), sky, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(8, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(6);
	glEnableVertexAttribArray(7);
	glEnableVertexAttribArray(8);

	// program
	program = loadShaders("vertexShader.glsl", "fragmentShader.glsl");
	glUseProgram(program);

	glEnable(GL_DEPTH_TEST);
	glClearColor((GLclampf)0.3, (GLclampf)0.3, (GLclampf)0.3, (GLclampf)1.0);

	// projection matrix (fov, aspect, near, far)
	proj = glm::perspective(glm::radians(45.0f), 1.8f, 0.1f, 200.0f);
	unsigned int projLoc = glGetUniformLocation(program, "proj");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

	// model matrix
	model = glm::mat4(1.0f);
	unsigned int modelLoc = glGetUniformLocation(program, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// texture
	char texWater[50] = "textures/water.jpg";
	char texGround1[50] = "textures/grass.jpg";
	char texGround2[50] = "textures/forest.jpg";
	char texGround3[50] = "textures/sand.jpg";
	char texGround4[50] = "textures/earth.jpg";
	char texSky[50] = "textures/sky.jpg";
	textureID[Texture::WATER] = loadTexture(Texture::WATER, texWater);
	textureID[Texture::GRASS] = loadTexture(Texture::GRASS, texGround1);
	textureID[Texture::FOREST] = loadTexture(Texture::FOREST, texGround2);
	textureID[Texture::SAND] = loadTexture(Texture::SAND, texGround3);
	textureID[Texture::EARTH] = loadTexture(Texture::EARTH, texGround4);
	textureID[Texture::SKY] = loadTexture(Texture::SKY, texSky);

	glutFullScreen();
}

// function to draw water
void drawWater(void) {
	unsigned int objLoc = glGetUniformLocation(program, "obj");
	obj = 1;
	glUniform1i(objLoc, obj);

	unsigned int vColorLoc = glGetUniformLocation(program, "vColor");
	glUniform3fv(vColorLoc, 1, glm::value_ptr(glm::vec3(0.3, 0.3, 0.8)));

	unsigned int ourTextureLoc = glGetUniformLocation(program, "ourTexture");
	glUniform1i(ourTextureLoc, Texture::WATER);

	glDrawArrays(GL_QUADS, 0, 4);
}

// function to draw ground
void drawGround(void) {
	unsigned int objLoc = glGetUniformLocation(program, "obj");
	obj = 2;
	glUniform1i(objLoc, obj);

	unsigned int vColorLoc = glGetUniformLocation(program, "vColor");
	glUniform3fv(vColorLoc, 1, glm::value_ptr(glm::vec3(0.8, 0.8, 0.8)));

	unsigned int ourTextureLoc = glGetUniformLocation(program, "ourTexture");
	glUniform1i(ourTextureLoc, groundTexture);

	glDrawArrays(GL_QUADS, 0, VERTICES);
}

// function to draw sky
void drawSky(void) {
	unsigned int objLoc = glGetUniformLocation(program, "obj");
	obj = 3;
	glUniform1i(objLoc, obj);

	unsigned int vColorLoc = glGetUniformLocation(program, "vColor");
	glUniform3fv(vColorLoc, 1, glm::value_ptr(skyColor));

	unsigned int ourTextureLoc = glGetUniformLocation(program, "ourTexture");
	glUniform1i(ourTextureLoc, Texture::SKY);

	glDrawArrays(GL_QUADS, 0, 4);
}

// function to draw light
void drawLight(void) {
	// pass light position vector to fragment shader for light calculation
	unsigned int lightPosLoc = glGetUniformLocation(program, "sunlightPos");
	glUniform3fv(lightPosLoc, 1, glm::value_ptr(sunlightPos));

	// pass light color to fragment shader for light calculation
	unsigned int lightColorLoc = glGetUniformLocation(program, "sunlightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(sunlightColor));
}

// function to draw text
void drawText(int x, int y, char* string) {
	glRasterPos2d(x, y);
	glColor3f(1.0, 1.0, 1.0);
	glutBitmapString(GLUT_BITMAP_8_BY_13, (const unsigned char*)string);
}

// function to automatically calculate text location in menu
int textLoc(void) {
	return curTextLoc -= 20;
}

// function to draw help menu
void drawMenu(void) {
	glUseProgram(0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, 1280.0, 0.0, 720.0);

	startTextLoc = 150;
	curTextLoc = startTextLoc;

	sprintf(curFPSstr, "%f", curFPS);

	if (showMenu) {
		drawText(30, textLoc(), (char*)" 1 2 3 4 : Change ground texture");

		antiAliasing
			? drawText(30, textLoc(), (char*)" A       : Anti-aliasing is ON")
			: drawText(30, textLoc(), (char*)" A       : Anti-aliasing is OFF");

		drawText(30, textLoc(), (char*)" Q       : Exit");
	}
	drawText(30, 60, (char*)" H       : Help Menu");
	drawText(30, 30, (char*)" Current FPS: ");
	drawText(150, 30, (char*)curFPSstr);
	glUseProgram(program);
}

// function to display
void display(void) {
	counter++;

	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);

	// toggle full-scene anti-aliasing
	antiAliasing ? glEnable(GL_MULTISAMPLE) : glDisable(GL_MULTISAMPLE);

	// view martrix - glm::lookAt(camera position, direction, up vector)
	view = glm::lookAt(glm::vec3(camX, camY, camZ), glm::vec3(dirX, dirY, dirZ), glm::vec3(0.0, 1.0, 0.0));
	unsigned int viewLoc = glGetUniformLocation(program, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	// pass camera position to fragment shader for light calculation
	unsigned int viewPosLoc = glGetUniformLocation(program, "viewPos");
	glUniform3fv(viewPosLoc, 1, glm::value_ptr(glm::vec3(camX, camY, camZ)));

	// draw background
	drawSky();
	drawWater();
	drawGround();

	// draw animals

	// draw light
	drawLight();

	// draw menu
	drawMenu();

	glutSwapBuffers();
}

// function to update
void update(int n) {
	// alternate water texture coordinate to create the illusion of motion
	if (ripple == 0) {
		water[6] = 0.0f;
		water[7] = 0.0f;
		water[14] = 0.0f;
		water[15] = 1.0f;
		water[22] = 1.0f;
		water[23] = 1.0f;
		water[30] = 1.0f;
		water[31] = 0.0f;
	}
	else if (ripple == 1) {
		water[6] = 1.0f;
		water[7] = 1.0f;
		water[14] = 1.0f;
		water[15] = 0.0f;
		water[22] = 0.0f;
		water[23] = 0.0f;
		water[30] = 0.0f;
		water[31] = 1.0f;
	}
	else if (ripple == 2) {
		water[6] = 0.0f;
		water[7] = 1.0f;
		water[14] = 0.0f;
		water[15] = 0.0f;
		water[22] = 1.0f;
		water[23] = 0.0f;
		water[30] = 1.0f;
		water[31] = 1.0f;
	}
	else if (ripple == 3) {
		water[6] = 1.0f;
		water[7] = 0.0f;
		water[14] = 1.0f;
		water[15] = 1.0f;
		water[22] = 0.0f;
		water[23] = 1.0f;
		water[30] = 0.0f;
		water[31] = 0.0f;
	}

	glBindBuffer(GL_ARRAY_BUFFER, VBO[Object::VBO_WATER]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(water), water);
	ripple = (++ripple) % 4;

	// change sunlight position and color
	float newSunlightX = sunlightPos[0] - 1.0f;
	float ratio = 1.0f - abs(newSunlightX / WORLD_SIZE);
	float newSunlightColor = 10.0f * ratio;
	sunlightPos[0] = newSunlightX <= -WORLD_SIZE ? WORLD_SIZE : newSunlightX;
	sunlightColor = glm::vec3(newSunlightColor, newSunlightColor, newSunlightColor);

	glutTimerFunc(500, update, 0);
}

// function to calculate FPS
void updateFPS(int n) {
	s_time = glutGet(GLUT_ELAPSED_TIME);

	if (s_time - e_time >= 1000) {
		curFPS = counter * 1000.0f / (s_time - e_time);
		e_time = s_time;
		counter = 0;
	}
	glutTimerFunc(5, updateFPS, 0);
}

// function to detect special keys
void specialKey(int key, int mouseX, int mouseY) {
	// change camera position and ensure camera always "points" toward the front
	switch (key) {
	case GLUT_KEY_LEFT:
		if (camX >= -WORLD_SIZE / 10.0f) {
			camX -= 0.5;
			dirX -= 0.5;
		}
		break;

	case GLUT_KEY_RIGHT:
		if (camX <= WORLD_SIZE / 10.0f) {
			camX += 0.5;
			dirX += 0.5;
		}
		break;

	case GLUT_KEY_UP:
		if (camZ >= 0.0f) {
			camZ -= 0.5;
			dirZ -= 0.5;
		}
		break;

	case GLUT_KEY_DOWN:
		if (camZ <= WORLD_SIZE / 2.0f) {
			camZ += 0.5;
			dirZ += 0.5;
		}
		break;

	case GLUT_KEY_PAGE_UP:
		if (camY <= 10.0) {
			camY += 0.5;
			dirY += 0.5;
		}
		break;

	case GLUT_KEY_PAGE_DOWN:
		if (camY >= 2.0) {
			camY -= 0.5;
			dirY -= 0.5;
		}
		break;

	case GLUT_KEY_F4:
		exit(0);

	default:
		break;
	}

}

// function to detect keys
void keyboardKey(unsigned char key, int mouseX, int mouseY) {
	switch (key) {
	case '1':
		groundTexture = Texture::GRASS;
		break;
	case '2':
		groundTexture = Texture::FOREST;
		break;
	case '3':
		groundTexture = Texture::SAND;
		break;
	case '4':
		groundTexture = Texture::EARTH;
		break;

	case 'a':
	case 'A':
		antiAliasing = !antiAliasing;
		break;
	case 'q':
	case 'Q':
		exit(0);
		break;
	case 'h':
	case 'H':
		showMenu = !showMenu;
		break;
	default:
		break;
	}
}

// function to run main program
int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(1280, 720);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Outdoor Scene");
	glewInit();

	init();

	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutSpecialFunc(specialKey);
	glutKeyboardFunc(keyboardKey);
	glutTimerFunc(500, update, 0);
	glutTimerFunc(5, updateFPS, 0);
	glutMainLoop();

	return 0;
}