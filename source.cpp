#include "clObjects.h"
#include "shaderHelper.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <cstdlib>
#include <mutex>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>

static const size_t width = 1904;
static const size_t height = 1071;

#define WINDOW_TITLE "OpenCL Render"
#define MATRIX_HEIGHT 100000
#define MATRIX_WIDTH 5

// computing sizes at compile time
constexpr size_t planets_size_full = MATRIX_HEIGHT * MATRIX_WIDTH * sizeof(float);
constexpr size_t planets_size_points = MATRIX_HEIGHT * 2 * sizeof(float);

#define X 1
#define Y 2

//float planets[MATRIX_HEIGHT][MATRIX_WIDTH];
float* planets;

clObjects clObjs;
uint8_t oldFps = 0;
GLFWwindow* window;
GLuint planets_vbo;

__forceinline void setupOpenCL()
{
	srand(500);

	planets = new float[MATRIX_HEIGHT * MATRIX_WIDTH];

	for (int i = 0; i < MATRIX_HEIGHT; ++i)
	{
		planets[(i * MATRIX_WIDTH) + 0] = (float(rand() % 999) + 1.0f); // mass
		planets[(i * MATRIX_WIDTH) + 1] = ((float(rand() % width / 4)) * 2) + float(width / 4); // x
		planets[(i * MATRIX_WIDTH) + 2] = ((float(rand() % height / 4)) * 2) + float(height / 4); // y
		planets[(i * MATRIX_WIDTH) + 3] = 0; // dx
		planets[(i * MATRIX_WIDTH) + 4] = 0; // dy
	}

	cl::Platform platform = cl::Platform::getDefault();

	cl_context_properties contextProperties[] = 
	{
		CL_GL_CONTEXT_KHR, (cl_context_properties)glfwGetWGLContext(window),
		CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
		0
	};

	clObjs.context = cl::Context(cl::Device::getDefault(), contextProperties);

	// The kernel
	cl::Program::Sources sources;

	// raw kernel string
	const std::string deviceData = readFile("device.cl");

	// adding kernel string to the source data structure
	sources.push_back({ deviceData.c_str(), deviceData.length() });

	// setting up program for building runnable kernel objects
	clObjs.program = cl::Program(clObjs.context, sources);

	if (clObjs.program.build({ cl::Device::getDefault() }) != CL_SUCCESS)
	{
		std::cout << "error: " << clObjs.program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(cl::Device::getDefault()) << std::endl;
	}
	
	clObjs.inBuffer = cl::Buffer(clObjs.context, CL_MEM_READ_WRITE, planets_size_full);
	clObjs.outBuffer = cl::Buffer(clObjs.context, CL_MEM_READ_WRITE, planets_size_points);

	// buffer initialisation
	clObjs.queue = cl::CommandQueue(clObjs.context, cl::Device::getDefault());
	clObjs.queue.enqueueWriteBuffer(clObjs.inBuffer, CL_TRUE, 0, planets_size_full, planets);

	clObjs.physicsKernel = cl::Kernel(clObjs.program, "planetCalc");
	clObjs.physicsKernel.setArg(0, clObjs.inBuffer);
	clObjs.physicsKernel.setArg(1, clObjs.outBuffer);

	delete[] planets;
	planets = new float[MATRIX_HEIGHT * 2];
}

__forceinline void configureOpenGL()
{
	// create vbo and use variable id to store vbo id
	glGenBuffers(1, &planets_vbo);

	// make the new vbo active
	glBindBuffer(GL_ARRAY_BUFFER, planets_vbo);

	gluOrtho2D(0, width, 0, height);
}

__forceinline void manageTitle(std::chrono::steady_clock::time_point start)
{
	const uint8_t fps = std::round(1 / (
		std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start)
		).count());

	if (((fps >= oldFps - 2) && (fps <= oldFps + 2)) || !oldFps)
	{
		oldFps = fps;
		glfwSetWindowTitle(window, (WINDOW_TITLE + std::string(" FPS: ") + std::to_string(fps)).c_str());
	}
}

__forceinline void updatePlanetVBO()
{
	glBufferData(GL_ARRAY_BUFFER, planets_size_points, planets, GL_STATIC_DRAW);
	glVertexPointer(2, GL_FLOAT, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, planets_vbo);
}

__forceinline void runOpenCL()
{
	clObjs.queue.enqueueNDRangeKernel(clObjs.physicsKernel, cl::NullRange, cl::NDRange(MATRIX_HEIGHT), cl::NullRange);
	clObjs.queue.finish();
	clObjs.queue.enqueueReadBuffer(clObjs.outBuffer, CL_TRUE, 0, planets_size_points, planets);
}

__forceinline void render()
{
	glClear(GL_COLOR_BUFFER_BIT);

	updatePlanetVBO();
	glDrawArrays(GL_POINTS, 0, MATRIX_HEIGHT);

	glFlush();
}

void setup()
{
	// initialising glfw (wrapper for platform code)
	glfwInit();

	// creating the window
	window = glfwCreateWindow(width, height, WINDOW_TITLE, NULL, NULL);

	if (!window)
	{
		// if window couldn't be created - terminate the program
		glfwTerminate();
		exit(-1);
	}

	// set the current OpenGL context to be the window
	glfwMakeContextCurrent(window);

	// initialising function pointers (glew.h)
	initOpenGL();

	// configuring openGL settings
	configureOpenGL();

	// compiling/preparing the OpenCL compute shaders and setting up the queue
	setupOpenCL();
}


int main(int argc, char** argv)
{
	// sets up the program (OpenCL and OpenGL)
	setup();

	// initial running of OpenCL do get data
	runOpenCL();

	updatePlanetVBO();

	glEnableClientState(GL_VERTEX_ARRAY);

	// main program loop
	while (!glfwWindowShouldClose(window))
	{	
		auto start = std::chrono::high_resolution_clock::now();

		// processing data on gpu with opencl
		runOpenCL();

		// OpenGL render code
		render();

		// swaping front and back buffers
		glfwSwapBuffers(window);

		// set title to fps
		manageTitle(start);

		// checking for events
		glfwPollEvents();
	}

	// terminate on close
	glfwTerminate();

	delete[] planets;
	return 0;
}