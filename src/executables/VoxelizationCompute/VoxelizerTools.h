#ifndef VOXELIZERTOOLS_H
#define VOXELIZERTOOLS_H

#include <Rendering/Shader.h>

/**
 * Everything needed to voxelize an object on the GPU
 */

class ComputeVoxelizer
{
private:
	ComputeShader* m_computeShader;
public:

	ComputeVoxelizer();
	~ComputeVoxelizer();
};

#endif
