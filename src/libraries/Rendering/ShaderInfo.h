#ifndef SHADERINFO_H
#define SHADERINFO_H

#include "Rendering/Shader.h"
#include "Utility/DebugLog.h"

namespace ShaderInfo
{
	// compute shader related information
	static int MAX_COMPUTE_WORK_GROUP_COUNT[3];
	static int MAX_COMPUTE_WORK_GROUP_SIZE[3];
	static int MAX_COMPUTE_WORK_GROUP_INVOCATIONS;
	static int MAX_COMPUTE_SHARED_MEMORY_SIZE;

	void get();
	void printGlobalInfo();
	void print( Shader* shader );
	void print( ComputeShader* shader );
}

#endif
