#pragma once
#include <CL/cl.hpp>

namespace
{
	struct clObjects
	{
		cl::Context context;
		cl::Program program;
		cl::Buffer inBuffer;
		cl::Buffer outBuffer;
		cl::BufferGL glBuffer;
		cl::Kernel physicsKernel;
		cl::Kernel renderKernel;
		cl::CommandQueue queue;
	};
}