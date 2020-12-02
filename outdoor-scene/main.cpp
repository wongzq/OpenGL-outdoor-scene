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