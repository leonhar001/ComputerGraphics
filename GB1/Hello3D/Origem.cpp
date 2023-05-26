#include <iostream>
#include <string>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"

using namespace std;

struct Vertex {
	glm::vec3 position;
	glm::vec3 v_color;
};


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int loadSimpleOjb(string filePath, int& nVertices);

const GLuint WIDTH = 720, HEIGHT = 720;

char rotateChar;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float deltaTime, lastFrame, lastX, lastY, yaw = -90, pitch, fov = 45.0f;
bool firstClick = true;

vector <Vertex> vertices;
vector <GLuint> indices;
vector <glm::vec3> normals;
vector <glm::vec2> texCoords;

int main() {

	glfwInit();

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Tarefa Vivencial 1", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPos(window, WIDTH / 2, HEIGHT / 2);
	glfwSetCursorPosCallback(window, mouse_callback);

	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		cout << "Failed to initialize GLAD" << std::endl;
	}

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	Shader shader("Hello3D.vs", "Hello3D.fs");
	shader.use();

	int numVertices;
	GLuint VAO = loadSimpleOjb("../suzanneTri1.obj", numVertices);

	GLint viewLoc = glGetUniformLocation(shader.ID, "view");
	GLint projLoc = glGetUniformLocation(shader.ID, "projection");

	shader.setFloat("ka", 0.5);
	shader.setFloat("kd", 0.5);
	shader.setFloat("ks", 0.5);
	shader.setFloat("n", 10);

	shader.setVec3("lightPos", 0.0f, 15.0f, 0.0f);
	shader.setVec3("lightColor", 0.8f, 0.3f, 0.1f);

	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		float angle = (GLfloat) glfwGetTime();

		glm::mat4 model = glm::mat4(1);

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glm::mat4 view = glm::lookAt(cameraPos,
			cameraPos + cameraFront,
			cameraUp);

		shader.setVec3("cameraPos", cameraPos.x, cameraPos.y, cameraPos.z);

		glm::mat4 projection = glm::perspective(fov, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

		if (rotateChar == 'X') {
			model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
		}
		else if (rotateChar == 'Y') {
			model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		else if (rotateChar == 'Z') {
			model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
		}
		else if (rotateChar == 'F') {
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		}
		else if (rotateChar == 'G') {
			model = glm::rotate(model, glm::radians(90.0f) * 2, glm::vec3(1.0f, 0.0f, 0.0f));
		}
		else if (rotateChar == 'H') {
			model = glm::rotate(model, glm::radians(90.0f) * 3, glm::vec3(1.0f, 0.0f, 0.0f));
		}
		else if (rotateChar == 'J') {
			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		else if (rotateChar == 'K') {
			model = glm::rotate(model, glm::radians(90.0f) * 3, glm::vec3(0.0f, 1.0f, 0.0f));
		}

		GLint modelLoc = glGetUniformLocation(shader.ID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, numVertices);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &VAO);
	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	
	float cameraSpeed = 60.0f * deltaTime;

	if (key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	else if (key == GLFW_KEY_X) {
		rotateChar = 'X';
	}
	else if (key == GLFW_KEY_Y) {
		rotateChar = 'Y';
	}
	else if (key == GLFW_KEY_Z) {
		rotateChar = 'Z';
	}
	else if (key == GLFW_KEY_W) {
		cameraPos += cameraSpeed * cameraFront;
	}
	else if (key == GLFW_KEY_S) {
		cameraPos -= cameraSpeed * cameraFront;
	}
	else if (key == GLFW_KEY_A) {
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	else if (key == GLFW_KEY_D) {
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	else if (key == GLFW_KEY_F) {
		rotateChar = 'F';
	}
	else if (key == GLFW_KEY_G) {
		rotateChar = 'G';
	}
	else if (key == GLFW_KEY_H) {
		rotateChar = 'H';
	}
	else if (key == GLFW_KEY_J) {
		rotateChar = 'J';
	}
	else if (key == GLFW_KEY_K) {
		rotateChar = 'K';
	}
	else {
		rotateChar = '\0';
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstClick) {
		lastX = xpos;
		lastY = ypos;
		firstClick = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.03;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 90.0f)
		pitch = 90.0f;

	if (pitch < -90.0f)
		pitch = -90.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset*0.1;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}

int loadSimpleOjb(string filePath, int& numVertices) {

	ifstream inputFile;
	inputFile.open(filePath);

	if (!inputFile.is_open()) {
		cout << "Erro ao abrir arquivo no caminho " << filePath << endl;
		return -1;
	}

	char line[100];
	string sLine;
	vector<GLfloat> vertBuffer;

	while (!inputFile.eof()) {
		inputFile.getline(line, 100);
		sLine = line;

		string word;

		istringstream ssLine(line);

		ssLine >> word;

		if (word == "v") {
			Vertex v;
			ssLine >> v.position.x >> v.position.y >> v.position.z;
			v.v_color.r = 1.0;
			v.v_color.g = 1.0;
			v.v_color.b = 0.0;
			vertices.push_back(v);

		} else if (word == "vn") {
			glm::vec3 vn;
			ssLine >> vn.x >> vn.y >> vn.z;
			normals.push_back(vn);
		}
		else if (word == "vt") {
			glm::vec2 vt;
			ssLine >> vt.s >> vt.t;
			texCoords.push_back(vt);

		} else if (word == "f") {
			string tokens[3];

			for (int i = 0; i < 3; i++) {
				ssLine >> tokens[i];
				int pos = tokens[i].find("/");
				string token = tokens[i].substr(0, pos);
				int index = atoi(token.c_str()) - 1;
				indices.push_back(index);
				vertBuffer.push_back(vertices[index].position.x);
				vertBuffer.push_back(vertices[index].position.y);
				vertBuffer.push_back(vertices[index].position.z);
				vertBuffer.push_back(vertices[index].v_color.r);
				vertBuffer.push_back(vertices[index].v_color.g);
				vertBuffer.push_back(vertices[index].v_color.b);

				tokens[i] = tokens[i].substr(pos + 1);
				pos = tokens[i].find("/");
				token = tokens[i].substr(0, pos);
				int indexT = atoi(token.c_str()) - 1;
				vertBuffer.push_back(texCoords[indexT].s);
				vertBuffer.push_back(texCoords[indexT].t);

				tokens[i] = tokens[i].substr(pos + 1);
				token = tokens[i].substr(0, pos);
				int indexN = atoi(token.c_str()) - 1;
				vertBuffer.push_back(normals[indexN].x);
				vertBuffer.push_back(normals[indexN].y);
				vertBuffer.push_back(normals[indexN].z);
			}
		}
	}

	inputFile.close();

	numVertices = vertBuffer.size() / 11;

	GLuint VBO, VAO;

	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, vertBuffer.size() * sizeof(GLfloat), vertBuffer.data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBindVertexArray(VAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	return VAO;
}
