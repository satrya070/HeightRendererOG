#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include "Shader.h"
#include "camera.h"
#include <algorithm>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
//void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

const unsigned int WIDTH = 1600;
const unsigned int HEIGHT = 1200;
const unsigned int NUM_PATCH_PTS = 4;

Camera camera(
	glm::vec3(67.f, 627.f, 169.f),
	glm::vec3(0.0f, 1.0f, 0.0f),
	-128.1f,
	-42.4f);
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool glfw_cursor_normal = false;
static float CameraMovementSpeed = 150.f;

int main()
{
	// init glfw
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW!" << std::endl;
		return -1;
	}

	camera.MovementSpeed = CameraMovementSpeed;
	stbi_set_flip_vertically_on_load(1);

	// set openGL version: 4.0 core
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Heightmap Renderer", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	//glfwSetKeyCall(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// load openGL extension functions with GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD!" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// load shader text files
	Shader HeightShader(
		"vertex_shader.txt", "fragment_shader.txt", "tesselation_control_shader.txt", "tesselation_evaluation_shader.txt"
	);

	// load all vertices with heighvalue pixels
	//std::vector<float> vertices;
	/*float yScale = 64.0f / 256.f, yShift = 16.0f;
	for (int r = 0; r < rows; r++)
	{
		for (int c = 0; c < cols; c++)
		{
			// memory index of the height pixel
			unsigned char* pixelOffset = data + (c + rows * r) * nChannels;
			// its value
			if (pixelOffset != nullptr)
			{
				unsigned char heightVal = pixelOffset[0];

				// sets origin to middel of (rows, cols)
				vertices.push_back(-rows / 2.0f + (rows * r / (float)rows)); // X
				//vertices.push_back((int)heightVal * yScale - yShift); // Y
				vertices.push_back(heightVal);
				vertices.push_back(-cols / 2.0f + (cols * c / (float)cols)); // Z
			}
		}
	}
	stbi_image_free(data);

	std::cout << "loaded: " << vertices.size() / 3 << " vertices" << std::endl;

	// EBO indices
	std::vector<unsigned int> indices;
	for (unsigned int r = 0; r < rows - 1; r++)
	{
		for (unsigned int c = 0; c < cols; c++)
		{
			// go back between top 
			for (unsigned int k = 0; k < 2; k++)
			{
				// picks the top c, then the one below
				indices.push_back(c + cols * (r + k));
			}
		}
	}
	std::cout << "loaded " << indices.size() << " indices" << std::endl;
	*/

	std::vector<float> vertices;

	// load the heightmap as texture
	unsigned int texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	// wrapping params
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// filtering params
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image
	int width, height, channels;
	unsigned char* data;
	data = stbi_load("images/heightmap_2.png", &width, &height, &channels, STBI_rgb_alpha);
	if (data) {
		//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		std::cout << width << ", " << height << std::endl;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		HeightShader.setInt("heightMap", 0);
		std::cout << "Heightmap dimension: (" << width << ", " << height << ")." << std::endl;
	}
	else
	{
		std::cout << "Failed to load" << std::endl;
	}
	stbi_image_free(data);

	// generate all coordinates for all patches
	unsigned int rez = 20;
	for (unsigned i = 0; i < rez; i++)
	{
		for (unsigned int j = 0; j < rez; j++)
		{
			// patch top left
			vertices.push_back(-width / 2.0f + width * i / (float)rez); // v.x
			vertices.push_back(0.0f); // v.y
			vertices.push_back(-height / 2.0f + height * j / (float)rez); // v.z
			vertices.push_back(i / (float)rez); // u
			vertices.push_back(j / (float)rez); // v

			// patch top right
			vertices.push_back(-width / 2.0f + width * (i+1) / (float)rez); // v.x
			vertices.push_back(0.0f); // v.y
			vertices.push_back(-height / 2.0f + height * j / (float)rez); // v.z
			vertices.push_back((i + 1) / (float)rez);
			vertices.push_back(j / (float)rez);

			// patch bottom left
			vertices.push_back(-width / 2.0f + width * i / (float)rez); // v.x
			vertices.push_back(0.0f); // v.y
			vertices.push_back(-height / 2.0f + height * (j+1) / (float)rez); // v.z
			vertices.push_back(i / (float)rez); // u
			vertices.push_back((j+1) / (float)rez); // v

			// patch bottom right
			vertices.push_back(-width / 2.0f + width * (i+1) / (float)rez); // v.x
			vertices.push_back(0.0f); // v.y
			vertices.push_back(-height / 2.0f + height * (j+1) / (float)rez); // v.z
			vertices.push_back((i + 1) / (float)rez);
			vertices.push_back((j + 1) / (float)rez);
		}
	}
	std::cout << "Loaded: " << rez * rez << " patches of 4 control points each" << std::endl;
	std::cout << "Processing " << rez * rez * 4 << " vertices in the vertex shader" << ::std::endl;

	// VAO
	GLuint terrainVAO, terrainVBO, terrainEBO;
	glGenVertexArrays(1, &terrainVAO);
	glBindVertexArray(terrainVAO);

	glGenBuffers(1, &terrainVBO);
	glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);


	glPatchParameteri(GL_PATCH_VERTICES, NUM_PATCH_PTS);

	//glGenBuffers(1, &terrainEBO);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);

	/* ------ Triangle VAO ---------- */
	/*float triangleVertices[] = {
		-500.f, -500.f, 0.f,
		500.f, -500.f, 0.f,
		-500.f, 500.f, 0.f
	};
	unsigned int triangleVAO, triangleVBO;
	glGenVertexArrays(1, &triangleVAO);
	glBindVertexArray(triangleVAO);

	glGenBuffers(1, &triangleVBO);
	glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);*/
	/* ------------------------------ */

	/*
	const unsigned int N_STRIPS = rows - 1;
	const unsigned int N_VERTS_PER_STRIP = cols * 2;

	std::cout << "number of strips: " << N_STRIPS << std::endl;
	std::cout << "number of triangles: " << N_VERTS_PER_STRIP << std::endl;
	
	auto [minIt, maxIt] = std::minmax_element(vertices.begin(), vertices.end());
	float minF = *minIt;
	float maxF = *maxIt;
	std::cout << "min value: " << minF << ", max value: " << maxF << std::endl;
	*/

	//-----init IMGUI------------
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	//
	ImGui::StyleColorsDark();
	// bind renderer
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// main loop
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		// clear buffers
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// activate shader before drawing and uniforms
		HeightShader.use();

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), ((float)WIDTH / (float)HEIGHT), 0.1f, 100000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);
		HeightShader.setMat4("projection", projection);
		HeightShader.setMat4("view", view);
		HeightShader.setMat4("model", model);

		// render heightmap
		glBindVertexArray(terrainVAO);
		glDrawArrays(GL_PATCHES, 0, NUM_PATCH_PTS* rez* rez);

		
		/*for (int strip = 0; strip < N_STRIPS; strip++)
		{
			glDrawElements(
				GL_TRIANGLE_STRIP,
				N_VERTS_PER_STRIP,
				GL_UNSIGNED_INT,
				(void*)(sizeof(unsigned int) * N_VERTS_PER_STRIP * strip)
			);
		}*/

		//glBindVertexArray(triangleVAO);
		//glDrawArrays(GL_TRIANGLES, 0, 3);

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Settings");
		ImGui::Text("This is a demo window.");
		if (ImGui::Button("Click Me"))
			printf("Camera speed: %g!\n", camera.MovementSpeed);

		ImGui::SliderFloat("Camera Movement Speed", &CameraMovementSpeed, 100.f, 200.f);
		if (camera.MovementSpeed != CameraMovementSpeed)
			camera.MovementSpeed = CameraMovementSpeed;

		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// swap
		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	// delete all used sources
	glDeleteVertexArrays(1, &terrainVAO);
	glDeleteBuffers(1, &terrainVBO);
	glDeleteBuffers(1, &terrainEBO);

	glfwTerminate();

	//std::cout << "pointer: " << data;
	return 0;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		if (!glfw_cursor_normal) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			glfw_cursor_normal = true;
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfw_cursor_normal = false;
		}


	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

/*void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_SPACE:
			useWireframe = 1 - useWireframe;
			break;
		case GLFW_KEY_G:
			displayGrayscale = 1 - displayGrayscale;
			break;
		default:
			break;
		}
	}
}*/

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}