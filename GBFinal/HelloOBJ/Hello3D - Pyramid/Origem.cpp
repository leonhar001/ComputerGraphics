#include <iostream>
#include <string>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <random>
#include <algorithm>
#include "Bezier.h"

#include <iomanip>
struct Vertex {
	glm::vec3 position;
	glm::vec3 v_color;
};

struct Material {
	std::string name;
	float Ns;
	float Ka[3];
	float Ks[3];
	float Ke[3];
	float Ni;
	float d;
	int illum;
	std::string map_Kd;
};

struct Config {
	int width;
	int height;
	std::string controlPointsPath;
	int pointsPerSegment;
	std::string bolaOBJ;
	std::string cubeOBJ;
	std::string suzanneOBJ;
	std::string boxTexture;
	float ka;
	float kd;
	float ks;
	float ns;
	float lightPosition[3];
	float lightColor[3];
	float backGroundColor[3];
	float cameraPosition[3];
};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int loadTexture(string path);
void loadConfig(const std::string& filePath, Config& config);

int loadBoxVAO();
void drawBox(const Shader& shader, GLuint boxVAO, int boxVertices, GLuint texID);

std::vector<glm::vec3> readControlPointsFromFile(const std::string& filename);

int loadSimpleOBJ(string filePath, int& nVertices, glm::vec3 color);

vector<Material> loadMTL(string filePath);
string mtlLibPath = "";


char command;
bool stop = false;
bool keyPressed = false;
bool firstMouse = true;
bool isInsideBasket = false;
int score = 0;
string select = "suzanne";

float deltaTime, lastFrame, lastX, lastY, yaw = -90, pitch;
float fov = 45.0f;

vector <Vertex> vertices;
vector <GLuint> indices;
vector <glm::vec3> normals;
vector <glm::vec2> texCoords;

glm::vec3 cameraPos; 
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

void drawObject(const Shader& shader, GLuint VAO, int nVertices, GLuint texID);
bool checkCollision(float objectX, float objectY, float basketX, float basketY, float basketWidth, float basketHeight);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

int main() {

	glfwInit();

	Config config;
	loadConfig("parameters.txt", config);

	const GLuint WIDTH = config.width, HEIGHT = config.height;
	cameraPos = glm::vec3(config.cameraPosition[0], config.cameraPosition[1], config.cameraPosition[2]);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Tarefa GB | Score: 0", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPos(window, WIDTH / 2, HEIGHT / 2);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);

	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		cout << "Failed to initialize GLAD" << std::endl;
	}

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	Shader shader("Phong.vs", "Phong.fs");
	shader.Use();

	int nVerticesSuz;
	int nVerticesCube;
	int nVerticesBola;
	vector<Material> mtlInfo;

	//POINTS AND CURVES
	std::vector<glm::vec3> controlPoints = readControlPointsFromFile(config.controlPointsPath);

	int pointsPerSegment = config.pointsPerSegment;
	Bezier bezier;
	bezier.setControlPoints(controlPoints);
	bezier.setShader(&shader);
	bezier.generateCurve(pointsPerSegment);

	bool forward = true;

	int nbCurvePoints = bezier.getNbCurvePoints();
	int i = 0;

	//POINTS AND CURVES

	GLuint bolaVAO = loadSimpleOBJ(config.bolaOBJ, nVerticesBola, glm::vec3(0, 0, 0));
	mtlInfo = loadMTL(mtlLibPath);
	GLuint bolaTexID = loadTexture(mtlInfo[0].map_Kd);

	GLuint cubeVAO = loadSimpleOBJ(config.cubeOBJ, nVerticesCube, glm::vec3(0, 0, 0));
	mtlInfo = loadMTL(mtlLibPath);
	GLuint cubeTexID = loadTexture(mtlInfo[0].map_Kd);

	GLuint suzVAO = loadSimpleOBJ(config.suzanneOBJ, nVerticesSuz, glm::vec3(0, 0, 0));
	mtlInfo = loadMTL(mtlLibPath);
	GLuint suzTexID = loadTexture(mtlInfo[0].map_Kd); 

	GLuint boxVAO = loadBoxVAO();
	GLuint boxTexId = loadTexture(config.boxTexture);

	GLint viewLoc = glGetUniformLocation(shader.ID, "view");
	GLint projLoc = glGetUniformLocation(shader.ID, "projection");

	shader.setFloat("ka", config.ka);
	shader.setFloat("kd", config.kd);
	shader.setFloat("ks", config.ks);
	shader.setFloat("n", config.ns);

	shader.setVec3("lightPos", config.lightPosition[0], config.lightPosition[1], config.lightPosition[2]);
	shader.setVec3("lightColor", config.lightColor[0], config.lightColor[1], config.lightColor[2]);

	glm::mat4 model2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));

	float basketX = 0.0f;
	float basketY = -0.5f;
	float basketWidth = 3.0f;
	float basketHeight = 3.0f;
	
	string novoTitulo;
	glEnable(GL_DEPTH_TEST);
	glClearColor(config.backGroundColor[0], config.backGroundColor[1], config.backGroundColor[2], 1.0f);
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		novoTitulo = "Tarefa GB | Score: " + to_string(score);


		glfwSetWindowTitle(window, novoTitulo.c_str());

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

		// MOVE OBJECT ON CURVE
		glm::vec3 pointOnCurve = bezier.getPointOnCurve(i);
		model = glm::translate(model, pointOnCurve);

		if (!stop) {
			if (i == 0) forward = true;

			if (forward)
				i = (i + 1) % nbCurvePoints;
			else
				i = (i - 1) % nbCurvePoints;

			if (i == 500) forward = false;
		}
		// MOVE OBJECT ON CURVE

		isInsideBasket = checkCollision(pointOnCurve[0], pointOnCurve[1], basketX, basketY, basketWidth, basketHeight);
		if (select == "suzanne") {
			switch (command) {
			case 'X':
				model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
				break;

			case 'Y':
				model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
				break;

			case 'Z':
				model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
				break;

			case '1':
				model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				break;

			case '2':
				model = glm::rotate(model, glm::radians(90.0f) * 2, glm::vec3(1.0f, 0.0f, 0.0f));
				break;

			case '3':
				model = glm::rotate(model, glm::radians(90.0f) * 3, glm::vec3(1.0f, 0.0f, 0.0f));
				break;

			case '4':
				model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				break;

			case '5':
				model = glm::rotate(model, glm::radians(90.0f) * 3, glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			}
		}

		GLint modelLoc = glGetUniformLocation(shader.ID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glBindTexture(GL_TEXTURE_2D, suzTexID);
		glActiveTexture(GL_TEXTURE0);

		glBindVertexArray(suzVAO);
		glDrawArrays(GL_TRIANGLES, 0, nVerticesSuz);

		model2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		model2 = glm::rotate(model2, angle*2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
		drawBox(shader, boxVAO, 36, boxTexId);

		model2 = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f));
		if (select == "bola") {
			switch (command) {
			case 'X':
				model2 = glm::rotate(model2, -angle, glm::vec3(1.0f, 0.0f, 0.0f));
				break;

			case 'Y':
				model2 = glm::rotate(model2, -angle, glm::vec3(0.0f, 1.0f, 0.0f));
				break;

			case 'Z':
				model2 = glm::rotate(model2, -angle, glm::vec3(0.0f, 0.0f, 1.0f));
				break;
			}
		}
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
		glBindTexture(GL_TEXTURE_2D, bolaTexID);
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(bolaVAO);
		glDrawArrays(GL_TRIANGLES, 0, nVerticesBola);

		model2 = glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 0.0f, 0.0f));
		if (select == "cubo") {
			switch (command) {
			case 'X':
				model2 = glm::rotate(model2, angle, glm::vec3(1.0f, 0.0f, 0.0f));
				break;

			case 'Y':
				model2 = glm::rotate(model2, angle, glm::vec3(0.0f, 1.0f, 0.0f));
				break;

			case 'Z':
				model2 = glm::rotate(model2, angle, glm::vec3(0.0f, 0.0f, 1.0f));
				break;
			}
		}
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
		glBindTexture(GL_TEXTURE_2D, cubeTexID);
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, nVerticesCube);

		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &suzVAO);
	glDeleteVertexArrays(1, &boxVAO);
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &bolaVAO);

	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	
	float cameraSpeed = 100.0f * deltaTime;

	if (action == GLFW_PRESS and key == GLFW_KEY_K) {
		if (stop) stop = false;
		else stop = true;
	}
	if (action == GLFW_PRESS and key == GLFW_KEY_B) {
		select = "bola";
	}
	if (action == GLFW_PRESS and key == GLFW_KEY_M) {
		select = "suzanne";
	}
	if (action == GLFW_PRESS and key == GLFW_KEY_C) {
		select = "cubo";
	}

	switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;

		case GLFW_KEY_X:
			command = 'X';
			break;

		case GLFW_KEY_Y:
			command = 'Y';
			break;

		case GLFW_KEY_Z:
			command = 'Z';
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

		case GLFW_KEY_1:
			command = '1';
			break;

		case GLFW_KEY_2:
			command = '2';
			break;

		case GLFW_KEY_3:
			command = '3';
			break;

		case GLFW_KEY_4:
			command = '4';
			break;

		case GLFW_KEY_5:
			command = '5';
			break;
		default:
			command = NULL;
		
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.09;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;

	if (pitch < -89.0f)
		pitch = -89.0f;

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

vector<Material> loadMTL(string filepath) {
	vector<Material> materials;
	ifstream file(filepath);

	if (!file.is_open()) {
		cout << "Failed to open MTL file: " << filepath << std::endl;
		return materials;
	}

	Material currentMaterial;
	string line;

	while (std::getline(file, line)) {
		istringstream iss(line);
		string prefix;
		iss >> prefix;

		if (prefix == "newmtl") {
			if (!currentMaterial.name.empty()) {
				materials.push_back(currentMaterial);
			}
			iss >> currentMaterial.name;
		}
		else if (prefix == "Ns") {
			iss >> currentMaterial.Ns;
		}
		else if (prefix == "Ka") {
			iss >> currentMaterial.Ka[0] >> currentMaterial.Ka[1] >> currentMaterial.Ka[2];
		}
		else if (prefix == "Ks") {
			iss >> currentMaterial.Ks[0] >> currentMaterial.Ks[1] >> currentMaterial.Ks[2];
		}
		else if (prefix == "Ke") {
			iss >> currentMaterial.Ke[0] >> currentMaterial.Ke[1] >> currentMaterial.Ke[2];
		}
		else if (prefix == "Ni") {
			iss >> currentMaterial.Ni;
		}
		else if (prefix == "d") {
			iss >> currentMaterial.d;
		}
		else if (prefix == "illum") {
			iss >> currentMaterial.illum;
		}
		else if (prefix == "map_Kd") {
			iss >> currentMaterial.map_Kd;
		}
	}

	if (!currentMaterial.name.empty()) {
		materials.push_back(currentMaterial);
	}

	file.close();

	return materials;
}

int loadTexture(string path)
{
	GLuint texID;

	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

	if (data)
	{
		if (nrChannels == 3) //jpg, bmp
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else //png
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}

int loadSimpleOBJ(string filepath, int& nVerts, glm::vec3 color)
{
	vector <glm::vec3> vertices;
	vector <GLuint> indices;
	vector <glm::vec2> texCoords;
	vector <glm::vec3> normals;
	vector <GLfloat> vbuffer;

	ifstream inputFile;
	inputFile.open(filepath.c_str());
	if (inputFile.is_open())
	{
		char line[100];
		string sline;

		while (!inputFile.eof())
		{
			inputFile.getline(line, 100);
			sline = line;

			string word;

			istringstream ssline(line);
			ssline >> word;
			if (word == "mtllib") 
			{
				//LOAD MTL FILE ADDRESS
				ssline >> mtlLibPath;
				cout << mtlLibPath << std::endl;
			}
			if (word == "v")
			{
				glm::vec3 v;
				ssline >> v.x >> v.y >> v.z;


				vertices.push_back(v);
			}
			if (word == "vt")
			{
				glm::vec2 vt;
				ssline >> vt.s >> vt.t;

				texCoords.push_back(vt);
			}
			if (word == "vn")
			{
				glm::vec3 vn;
				ssline >> vn.x >> vn.y >> vn.z;
				normals.push_back(vn);
			}
			if (word == "f")
			{
				string tokens[3];

				ssline >> tokens[0] >> tokens[1] >> tokens[2];

				for (int i = 0; i < 3; i++)
				{
					int pos = tokens[i].find("/");
					string token = tokens[i].substr(0, pos);
					int index = atoi(token.c_str()) - 1;
					indices.push_back(index);

					vbuffer.push_back(vertices[index].x);
					vbuffer.push_back(vertices[index].y);
					vbuffer.push_back(vertices[index].z);
					vbuffer.push_back(color.r);
					vbuffer.push_back(color.g);
					vbuffer.push_back(color.b);

					tokens[i] = tokens[i].substr(pos + 1);
					pos = tokens[i].find("/");
					token = tokens[i].substr(0, pos);
					index = atoi(token.c_str()) - 1;

					if (index >= 0 && index < texCoords.size())
					{
						vbuffer.push_back(texCoords[index].s);
						vbuffer.push_back(texCoords[index].t);
					}
					else
					{
						// Use valores padrão para coordenadas de textura caso não sejam fornecidas
						vbuffer.push_back(0.0f);
						vbuffer.push_back(0.0f);
					}

					tokens[i] = tokens[i].substr(pos + 1);
					index = atoi(tokens[i].c_str()) - 1;

					if (index >= 0 && index < normals.size())
					{
						vbuffer.push_back(normals[index].x);
						vbuffer.push_back(normals[index].y);
						vbuffer.push_back(normals[index].z);
					}
					else
					{
						// Use valores padrão para normais caso não sejam fornecidas
						vbuffer.push_back(0.0f);
						vbuffer.push_back(0.0f);
						vbuffer.push_back(0.0f);
					}
				}
			}
		}
	}
	else
	{
		cout << "Problema ao encontrar o arquivo " << filepath << endl;
	}
	inputFile.close();
	GLuint VBO, VAO;
	nVerts = vbuffer.size() / 11; // 3 pos + 3 cor + 3 normal + 2 texcoord

	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, vbuffer.size() * sizeof(GLfloat), vbuffer.data(), GL_STATIC_DRAW);
	glGenVertexArrays(1, &VAO);

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

void drawObject(const Shader& shader, GLuint VAO, int nVertices, GLuint texID)
{
	//TEXTURE
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);
	//TEXTURE

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, nVertices);
	glBindVertexArray(0);

	//TEXTURE
	glBindVertexArray(0); //unbind - desconecta
	glBindTexture(GL_TEXTURE_2D, 0); //unbind da textura
	//TEXTURE
}

GLfloat boxVertices[] = {
	// Posições          // Coordenadas de textura
	// Frente
	-1.5f, -1.5f,  1.5f,  0.0f, 0.0f,
	 1.5f, -1.5f,  1.5f,  1.0f, 0.0f,
	 1.5f,  1.5f,  1.5f,  1.0f, 1.0f,
	 1.5f,  1.5f,  1.5f,  1.0f, 1.0f,
	-1.5f,  1.5f,  1.5f,  0.0f, 1.0f,
	-1.5f, -1.5f,  1.5f,  0.0f, 0.0f,

	// Lado direito
	 1.5f, -1.5f,  1.5f,  0.0f, 0.0f,
	 1.5f, -1.5f, -1.5f,  1.0f, 0.0f,
	 1.5f,  1.5f, -1.5f,  1.0f, 1.0f,
	 1.5f,  1.5f, -1.5f,  1.0f, 1.0f,
	 1.5f,  1.5f,  1.5f,  0.0f, 1.0f,
	 1.5f, -1.5f,  1.5f,  0.0f, 0.0f,

	 // Parte de trás
	 -1.5f, -1.5f, -1.5f,  0.0f, 0.0f,
	  1.5f, -1.5f, -1.5f,  1.0f, 0.0f,
	  1.5f,  1.5f, -1.5f,  1.0f, 1.0f,
	  1.5f,  1.5f, -1.5f,  1.0f, 1.0f,
	 -1.5f,  1.5f, -1.5f,  0.0f, 1.0f,
	 -1.5f, -1.5f, -1.5f,  0.0f, 0.0f,

	 // Lado esquerdo
	 -1.5f, -1.5f, -1.5f,  0.0f, 0.0f,
	 -1.5f, -1.5f,  1.5f,  1.0f, 0.0f,
	 -1.5f,  1.5f,  1.5f,  1.0f, 1.0f,
	 -1.5f,  1.5f,  1.5f,  1.0f, 1.0f,
	 -1.5f,  1.5f, -1.5f,  0.0f, 1.0f,
	 -1.5f, -1.5f, -1.5f,  0.0f, 0.0f,

};

int loadBoxVAO() {
	GLuint boxVAO;
	glGenVertexArrays(1, &boxVAO);
	glBindVertexArray(boxVAO);

	// Criação do VBO da caixa sem a tampa
	GLuint boxVBO;
	glGenBuffers(1, &boxVBO);
	glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(boxVertices), boxVertices, GL_STATIC_DRAW);

	// Configuração dos atributos de vértice da caixa sem a tampa
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	// Desvincula o VAO da caixa sem a tampa
	glBindVertexArray(0);

	return boxVAO;
}

void drawBox(const Shader& shader, GLuint boxVAO, int boxVertices, GLuint texID)
{
	// Ativa a textura
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);

	// Desenha a caixa com a textura
	glBindVertexArray(boxVAO);
	glDrawArrays(GL_TRIANGLES, 0, boxVertices);
	glBindVertexArray(0);

	// Desativa a textura
	glBindTexture(GL_TEXTURE_2D, 0);
}

std::vector<glm::vec3> readControlPointsFromFile(const std::string& filename)
{
	std::vector<glm::vec3> controlPoints;

	std::ifstream file(filename);
	if (!file)
	{
		std::cerr << "Falha ao abrir o arquivo " << filename << std::endl;
		return controlPoints;
	}

	std::string line;
	while (std::getline(file, line))
	{
		if (line.find("Point: (") != std::string::npos)
		{
			// Extrair os valores das coordenadas
			size_t startPos = line.find("(") + 1;
			size_t endPos = line.find(")");
			std::string pointStr = line.substr(startPos, endPos - startPos);

			std::stringstream ss(pointStr);
			glm::vec3 point;
			char comma;
			ss >> point.x >> comma >> point.y >> comma >> point.z;
			controlPoints.push_back(point);
		}
	}

	file.close();
	return controlPoints;
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		if (isInsideBasket) {
			score++;
			glClearColor(0.0f, 0.5f, 0.0f, 1.0f);
			std::cout << "Score: " << score << std::endl;
		}
		else {
			score--;
			glClearColor(0.5f, 0.0f, 0.0f, 1.0f);
			std::cout << "Score: " << score << std::endl;
		}

	}
}

bool checkCollision(float objectX, float objectY, float basketX, float basketY, float basketWidth, float basketHeight) {
	// Verificar se a posição do objeto está dentro dos limites da "cesta"
	if (objectX >= basketX && objectX <= basketX + basketWidth && objectY >= basketY && objectY <= basketY + basketHeight) {
		return true;
	}
	else {
		return false;
	}
	
}

void loadConfig(const std::string& filePath, Config& config) {
	std::ifstream file(filePath);
	if (!file.is_open()) {
		std::cerr << "Erro ao abrir o arquivo de configuração: " << filePath << std::endl;
		return;
	}

	std::string line;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string key;
		std::string value;
		if (iss >> key >> value) {
			if (key == "width") {
				config.width = std::stoi(value);
			}
			else if (key == "height") {
				config.height = std::stoi(value);
			}
			else if (key == "controlPointsPath") {
				config.controlPointsPath = value;
			}
			else if (key == "pointsPerSegment") {
				config.pointsPerSegment = std::stoi(value);
			}
			else if (key == "bolaOBJ") {
				config.bolaOBJ = value;
			}
			else if (key == "cubeOBJ") {
				config.cubeOBJ = value;
			}
			else if (key == "suzanneOBJ") {
				config.suzanneOBJ = value;
			}
			else if (key == "boxTexture") {
				config.boxTexture = value;
			}
			else if (key == "ka") {
				config.ka = std::stof(value);
			}
			else if (key == "kd") {
				config.kd = std::stof(value);
			}
			else if (key == "ks") {
				config.ks = std::stof(value);
			}
			else if (key == "ns") {
				config.ns = std::stof(value);
			}
			else if (line.find("lightPosition") != std::string::npos) {
				std::string positions = line.substr(line.find(" ") + 1);
				std::stringstream ss(positions);
				for (int i = 0; i < 3; ++i) {
					ss >> config.lightPosition[i];
				}
			}
			else if (line.find("lightColor") != std::string::npos) {
				std::string colors = line.substr(line.find(" ") + 1);
				std::stringstream ss(colors);
				for (int i = 0; i < 3; ++i) {
					ss >> config.lightColor[i];
				}
			}
			else if (line.find("backGroundColor") != std::string::npos) {
				std::string colors = line.substr(line.find(" ") + 1);
				std::stringstream ss(colors);
				for (int i = 0; i < 3; ++i) {
					ss >> config.backGroundColor[i];
				}
			}
			else if (line.find("cameraPosition") != std::string::npos) {
				std::string positions = line.substr(line.find(" ") + 1);
				std::stringstream ss(positions);
				for (int i = 0; i < 3; ++i) {
					ss >> config.cameraPosition[i];
				}
			}
		}
	}

	file.close();
}