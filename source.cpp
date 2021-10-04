// the OpenGL wrangler library for
// later versions of OpenGL
#include <GL/glew.h>

// GLFW3 graphics library
// exposing some native stuff to 
// make OpenCL OpenGL interop work
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_EXPOSE_NATIVE_WIN32

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

// standard libraries
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <cstdlib>

// width and height need to be saved in memory
// for OpenGL calls.
// -----------------------------------------
// I could make them macros
// and then assign local variables the values
// but that seems to complex for a 0.00000001ms
// performance gain.
// -----------------------------------------
static constexpr size_t width = 1904;
static constexpr size_t height = 1071;

// using __forceinline only with compatible compiler
#ifdef _MSC_VER
#define INLINE __forceinline
#else
#define INLINE static inline
#endif

// some useful macros
#define WINDOW_TITLE "OpenCL Render"
#define SIZE 5000
#define G 0.000006743f

// planet data
struct Planet
{
	float mass, x, y, dx, dy;
};

// global variables
uint8_t oldFps = 0;
GLFWwindow* window;
GLuint planets_vbo;
Planet planets[SIZE];

INLINE void initOpenGL()
{
	// wrangling OpenGL functions
	const GLint GlewInitResult = glewInit();

	// checking if wrangle was
	if (GLEW_OK != GlewInitResult)
	{
		// if this failed then the program can't run so print error and exit
		std::cerr << "GLEW ERROR: " << glewGetErrorString(GlewInitResult) << "\t\n";
		exit(EXIT_FAILURE);
	}
}

INLINE void setupPlanets()
{
	// seeding future rand() calls
	srand(50);

	// setting initial values
	for (int i = 0; i < SIZE; ++i)
	{
		planets[i].mass = (float(rand() % 999) + 1.0f);
		planets[i].x = ((float(rand() % width / 4)) * 2) + float(width / 4);
		planets[i].y = ((float(rand() % height / 4)) * 2) + float(height / 4);
		planets[i].dx = 0;
		planets[i].dy = 0;
	}
}

INLINE void configureOpenGL()
{
	// setting matrix
	glMatrixMode(GL_PROJECTION);

	// setting up how values are normalized
	glOrtho(0, width, height, 0, -1, 1);
}

INLINE void manageTitle(std::chrono::steady_clock::time_point start)
{
	// calculating frame rate
	uint8_t fps = std::round(1 / (
		std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start)
		).count());

	// if framerate change is small enough don't bother updating title
	if (((fps >= oldFps - 2) && (fps <= oldFps + 2)) || !oldFps)
	{
		oldFps = fps;
		glfwSetWindowTitle(window, (WINDOW_TITLE + std::string(" FPS: ") + std::to_string(fps)).c_str());
	}
}

INLINE void calculatePlanetsPositions()
{
	// Get the index of the current element to be processed
	for (int i = 0; i < SIZE; ++i)
	{
		float fx = 0, fy = 0;
		for (int j = 0; j < SIZE; ++j)
		{
			if (j == i) continue; // not comparing planet to itself
			float F, rx, ry;

			// calculating distance
			rx = planets[i].x - planets[j].x;
			ry = planets[i].y - planets[j].y;

			// gravity calculation -> G * M0 * M1 / R^2
			if ((unsigned int(rx) | unsigned int(ry)) == 0) F = (planets[i].mass * planets[j].mass * G);
			else F = (planets[i].mass * planets[j].mass * G) / ((rx * rx) + (ry * ry));

			// doing 0 checks so no NaNs show up
			if (rx != 0) fx += -F * rx;
			if (ry != 0) fy += -F * ry;
		}

		// calculating rate of change
		planets[i].dx += fx / planets[i].mass;
		planets[i].dy += fy / planets[i].mass;

		// setting coordinates
		planets[i].x += planets[i].dx;
		planets[i].y += planets[i].dy;
	}
}

INLINE void render()
{
	// clear screen to avoid onioning
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_POINTS);
	for (int i = 0; i < SIZE; ++i)
	{
		glVertex2f(planets[i].x, planets[i].y);
	}
	glEnd();

	// flush changes
	glFlush();
}

void setup()
{
	// setting initial planets values
	setupPlanets();

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

	// configuring OpenGL settings
	configureOpenGL();
}


int main(int argc, char** argv)
{
	// sets up the program (OpenCL and OpenGL)
	setup();

	// main program loop
	while (!glfwWindowShouldClose(window))
	{	
		auto start = std::chrono::high_resolution_clock::now();

		// calculating planets positions
		calculatePlanetsPositions();

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
	return 0;
}