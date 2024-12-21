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
	unsigned char *data = stbi_load("images/heightmap_sample.png", &rows, &cols, &nChannels, 0);

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
				std::cout << "pixelOffset: " << sizeof(pixelOffset);
				unsigned char height = pixelOffset[0];

				// sets origin to middel of (rows, cols)
				vertices.push_back(-rows / 2.0f + r);
				vertices.push_back(height * yScale - yShift);
				vertices.push_back(-cols / 2.0 + c);
			}
		}
	}

	//printf("%d", data);
	std::cout << "pointer: " << static_cast<int>(data[7]) << nChannels << std::endl ;

	// main loop
	while (!glfwWindowShouldClose(window))
	{
		// clear buffers
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// swap
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	//std::cout << "pointer: " << data;
	return 0;
}