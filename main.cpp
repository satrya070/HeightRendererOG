#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>

int main()
{
	// init glfw
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW!" << std::endl;
		return -1;
	}

	// set openGL version: 4.0 core
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Heightmap Renderer", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// load openGL extension functions with GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD!" << std::endl;
		return -1;
	}

	// load map
	int cols, rows, nChannels;
	unsigned char *data = stbi_load("images/heightmap_sample.png", &cols, &rows, &nChannels, 0);

	// load all vertices with heighvalue pixels
	std::vector<float> vertices;
	float yScale = 0.25f, yShift = 16.0f;
	for (int r = 0; r < rows; r++)
	{
		for (int c = 0; c < cols; c++)
		{
			// memory index of the height pixel
			unsigned char* pixelOffset = data + (c + rows * r) * nChannels;
			// its value
			if (pixelOffset != nullptr)
			{
				unsigned char height = *pixelOffset;

				// sets origin to middel of (rows, cols)
				vertices.push_back(-rows / 2.0f + r);
				vertices.push_back(height * yScale - yShift);
				vertices.push_back(-cols / 2.0 + c);
			}
		}
	}
	stbi_image_free(data);

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
				indices.push_back(c + rows * (r + k));
			}
		}
	}
	std::cout << "loaded " << indices.size() << " indices" << std::endl;

	// VAO
	GLuint terrainVAO, terrainVBO, terrainEBO;
	glGenVertexArrays(1, &terrainVAO);
	glBindVertexArray(terrainVAO);
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), &vertices[0]);
	glBindVertexArray(0);

	glGenBuffers(1, &terrainVBO);
	glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &terrainEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	const unsigned int N_STRIPS = rows - 1;
	const unsigned int N_VERTS_PER_STRIP = rows * 2;

	// main loop
	while (!glfwWindowShouldClose(window))
	{
		// clear buffers
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// swap
		glfwSwapBuffers(window);
		glfwPollEvents();

		// render cube
		glBindVertexArray(terrainVAO);
		for (unsigned int strip = 0; strip << N_STRIPS; strip++)
		{
			glDrawElements(
				GL_TRIANGLE_STRIP,
				N_VERTS_PER_STRIP,
				GL_UNSIGNED_INT,
				(void*)(sizeof(unsigned int) * N_VERTS_PER_STRIP * strip)
			);
		}
	}

	glfwTerminate();

	//std::cout << "pointer: " << data;
	return 0;
}