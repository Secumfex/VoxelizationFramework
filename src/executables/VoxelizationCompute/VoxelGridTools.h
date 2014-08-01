#ifndef VOXELGRIDTOOLS_H
#define VOXELGRIDTOOLS_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Utility/DebugLog.h>
#include <Resources/Texture.h>

/**
 * Everything needed to create a suitable voxel grid thing on the GPU
 */
class VoxelGridGPU
{
public:
	VoxelGridGPU();
	~VoxelGridGPU();

	float width;
	float height;
	float depth;

	float cellSize;

	int resX;
	int resY;
	int resZ;

	glm::mat4 perspective;
	glm::mat4 view;
	glm::mat4 worldToVoxel;

	GLuint handle;

	Texture* texture;

	// provide some info and it will give you the suitable missing value
	void setUniformCellSizeFromResolutionAndMapping(float width, float height, float resX, float resY, float resZ);

	// void compute resolutions to map as specified
	void setDimensionFromMapping( float cellSize, float resX, float resY, float resZ = 32);

	// computeProjectionMatrix from member variables
	void computeProjectionMatrix();

	// compute world to voxel space from member variables
	void computeWorldToVoxel();

	// use this to set LookAt, to update worldToVoxel matrix aswell
	void setView( glm::mat4 view );

	// print variables
	void printInfo();
};

#endif
