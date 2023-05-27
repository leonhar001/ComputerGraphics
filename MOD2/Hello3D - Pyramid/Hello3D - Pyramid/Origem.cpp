#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

int setupGeometry();

const GLuint WIDTH = 720, HEIGHT = 720;

char rotateChar;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float deltaTime, lastFrame, lastX, lastY, yaw = -90, pitch;
float fov = 45.0f;
float zoom = 1.0;
bool firstMouse = true;

int main() {

	glfwInit();

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Tarefa M2", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	Shader shader("Hello3D.vs", "Hello3D.fs");

	GLuint VAO = setupGeometry();

	glUseProgram(shader.ID);

	GLint viewLoc = glGetUniformLocation(shader.ID, "view");
	GLint projLoc = glGetUniformLocation(shader.ID, "projection");

	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(10);

		float angle = (GLfloat) glfwGetTime();

		glm::mat4 model = glm::mat4(1);

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glm::mat4 view = glm::lookAt(cameraPos,
			cameraPos + cameraFront,
			cameraUp);

		glm::mat4 projection = glm::perspective(fov*zoom, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

		switch (rotateChar) {
			case 'X':
				model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
				break;

			case 'Y':
				model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
				break;

			case 'Z':
				model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
				break;
		}

		GLint modelLoc = glGetUniformLocation(shader.ID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 66);
		
		glDrawArrays(GL_POINTS, 0, 66);
		glBindVertexArray(0);
	

		glfwSwapBuffers(window);
	}
	glDeleteVertexArrays(1, &VAO);
	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	
	float cameraSpeed = 100.0f * deltaTime;

	switch (key) {

		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;

		case GLFW_KEY_X:
			rotateChar = 'X';
			break;

		case GLFW_KEY_Y:
			rotateChar = 'Y';
			break;

		case GLFW_KEY_Z:
			rotateChar = 'Z';
			break;

		case GLFW_KEY_W:
			cameraPos += cameraSpeed * cameraFront;
			break;

		case GLFW_KEY_S:
			cameraPos -= cameraSpeed * cameraFront;
			break;

		case GLFW_KEY_A:
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			break;

		case GLFW_KEY_D:
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			break;
		case GLFW_KEY_LEFT_BRACKET:
			zoom += 0.001;
			break;
		case GLFW_KEY_RIGHT_BRACKET:
			zoom -= 0.001;
			break;
		default:
			rotateChar = NULL;
	}
}

int setupGeometry() {
	GLfloat vertices[] = {
		//CUBO 1
		// Base
		-0.5, -0.5, -0.5, 0.0, 0.0, 1.0,
		-0.5, -0.5, 0.5, 0.0, 0.0, 1.0,
		0.5, -0.5, -0.5, 0.0, 0.0, 1.0,

		-0.5, -0.5, 0.5, 0.0, 0.0, 1.0,
		0.5, -0.5, 0.5, 0.0, 0.0, 1.0,
		0.5, -0.5, -0.5, 0.0, 0.0, 1.0,

		// Topo
		-0.5, 0.5, -0.5, 0.0, 0.0, 1.0,
		-0.5, 0.5, 0.5, 0.0, 0.0, 1.0,
		0.5, 0.5, -0.5, 0.0, 0.0, 1.0,

		-0.5, 0.5, 0.5, 0.0, 0.0, 1.0,
		0.5, 0.5, 0.5, 0.0, 0.0, 1.0,
		0.5, 0.5, -0.5, 0.0, 0.0, 1.0,

		// Frente
		-0.5, -0.5, 0.5, 0.0, 0.0, 1.0,
		-0.5, 0.5, 0.5, 0.0, 0.0, 1.0,
		0.5, -0.5, 0.5, 0.0, 0.0, 1.0,

		-0.5, 0.5, 0.5, 0.0, 0.0, 1.0,
		0.5, 0.5, 0.5, 0.0, 0.0, 1.0,
		0.5, -0.5, 0.5, 0.0, 0.0, 1.0,

		// Trás
		-0.5, -0.5, -0.5, 0.0, 0.0, 1.0,
		-0.5, 0.5, -0.5, 0.0, 0.0, 1.0,
		0.5, -0.5, -0.5, 0.0, 0.0, 1.0,

		-0.5, 0.5, -0.5, 0.0, 0.0, 1.0,
		0.5, 0.5, -0.5, 0.0, 0.0, 1.0,
		0.5, -0.5, -0.5, 0.0, 0.0, 1.0,

		// Esquerda
		-0.5, -0.5, 0.5, 0.0, 0.0, 1.0,
		-0.5, 0.5, 0.5, 0.0, 0.0, 1.0,
		-0.5, -0.5, -0.5, 0.0, 0.0, 1.0,

		-0.5, 0.5, 0.5, 0.0, 0.0, 1.0,
		-0.5, 0.5, -0.5, 0.0, 0.0, 1.0,
		-0.5, -0.5, -0.5, 0.0, 0.0, 1.0,

		// Direita
		0.5, -0.5, 0.5, 0.0, 0.0, 0.0,
		0.5, 0.5, 0.5, 0.0, 0.0, 0.0,
		0.5, -0.5, -0.5, 0.0, 0.0, 0.0,

		0.5, 0.5, 0.5, 0.0, 0.0, 0.0,
		0.5, 0.5, -0.5, 0.0, 0.0, 0.0,
		0.5, -0.5, -0.5, 0.0, 0.0, 0.0,

		// CUBO 2
		// Base
		0.5, -0.5, -0.5, 0.0, 0.0, 0.0,
		0.5, -0.5, 0.5, 0.0, 0.0, 0.0,
		1.5, -0.5, -0.5, 0.0, 0.0, 0.0,

		0.5, -0.5, 0.5, 0.0, 0.0, 0.0,
		1.5, -0.5, 0.5, 0.0, 0.0, 0.0,
		1.5, -0.5, -0.5, 0.0, 0.0, 0.0,

		// Topo
		0.5, 0.5, -0.5, 0.0, 0.0, 0.0,
		0.5, 0.5, 0.5, 0.0, 0.0, 0.0,
		1.5, 0.5, -0.5, 0.0, 0.0, 0.0,

		0.5, 0.5, 0.5, 0.0, 0.0, 0.0,
		1.5, 0.5, 0.5, 0.0, 0.0, 0.0,
		1.5, 0.5, -0.5, 0.0, 0.0, 0.0,

		// Frente
		0.5, -0.5, 0.5, 0.0, 0.0, 0.0,
		0.5, 0.5, 0.5, 0.0, 0.0, 0.0,
		1.5, -0.5, 0.5, 0.0, 0.0, 0.0,

		0.5, 0.5, 0.5, 0.0, 0.0, 0.0,
		1.5, 0.5, 0.5, 0.0, 0.0, 0.0,
		1.5, -0.5, 0.5, 0.0, 0.0, 0.0,

		// Trás
		0.5, -0.5, -0.5, 0.0, 0.0, 0.0,
		0.5, 0.5, -0.5, 0.0, 0.0, 0.0,
		1.5, -0.5, -0.5, 0.0, 0.0, 0.0,

		0.5, 0.5, -0.5, 0.0, 0.0, 0.0,
		1.5, 0.5, -0.5, 0.0, 0.0, 0.0,
		1.5, -0.5, -0.5, 0.0, 0.0, 0.0,

		// Direita
		1.5, -0.5, 0.5, 0.0, 0.0, 0.0,
		1.5, 0.5, 0.5, 0.0, 0.0, 0.0,
		1.5, -0.5, -0.5, 0.0, 0.0, 0.0,

		1.5, 0.5, 0.5, 0.0, 0.0, 0.0,
		1.5, 0.5, -0.5, 0.0, 0.0, 0.0,
		1.5, -0.5, -0.5, 0.0, 0.0, 0.0,
	};

	GLuint VBO, VAO;

	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);
 
	glBindVertexArray(VAO);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	return VAO;
}