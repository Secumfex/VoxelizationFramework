#include "VoxelGridTools.h"

VoxelGridGPU::VoxelGridGPU() {
	handle = 0;
	resY = 0;
	resX = 0;
	resZ = 0;
	width = 0;
	height = 0;
	depth = 0;
	cellSize = 0;
	numMipmaps = 0;
	perspective = glm::mat4(1.0f);
	view = glm::lookAt( glm::vec3(0,0,0), glm::vec3(0,0,-1), glm::vec3(0,1,0) );
	texture = 0;
	worldToVoxel  = glm::mat4(1.0f);
	worldToVoxelParam  = glm::mat4(1.0f);
	voxelToVoxelParam = glm::mat4(1.0f);
}

VoxelGridGPU::~VoxelGridGPU() {
}

void VoxelGridGPU::setUniformCellSizeFromResolutionAndMapping(float width,
		float height, int resX, int resY, int resZ) {

	if ( ( width / resX )  != ( height / resY) )
	{
		DEBUGLOG->log( "Resolution and dimensions are in an unequal ratio. Cannot compute uniform cell size");
		return;
	}

	this->width = width;
	this->height = height;
	this->resX = resX;
	this->resY = resY;
	this->resZ = resZ;

	this->cellSize = ( width / resX );

	this->depth = this->cellSize * resZ;

	computeProjectionMatrix();

	computeWorldToVoxel();

	printInfo();
}

void VoxelGridGPU::setDimensionFromMapping(float cellSize, float resX,
		float resY, float resZ) {

	if ( cellSize <= 0.0f )
	{
		DEBUGLOG->log( "Invalid cell size : ", cellSize);
		return;
	}

	this->cellSize = cellSize;
	this->resX = resX;
	this->resY = resY;
	this->resZ = resZ;

	this->width = resX * cellSize;
	this->height = resY * cellSize;
	this->depth = resZ * cellSize;

	computeProjectionMatrix();

	computeWorldToVoxel();

	printInfo();
}

void VoxelGridGPU::computeProjectionMatrix() {
	float hw = width / 2.0f;
	float hh = height / 2.0f;
	float hd = depth / 2.0f;

	this->perspective = glm::ortho (
			-hw,hw,
			-hh,hh,
			-hd,hd );
}

void VoxelGridGPU::computeWorldToVoxel() {
	// view to bottom left origin
	float hw = width / 2.0f;
	float hh = height / 2.0f;
	float hd = depth / 2.0f;
	worldToVoxel =
			glm::scale(     glm::mat4(1.0f), glm::vec3 (1.0f / cellSize, 1.0f / cellSize, 1.0f /  cellSize ) ) *
			glm::translate( glm::mat4(1.0f), glm::vec3(hw, hh, hd)  ) *
			view *
			glm::mat4(1.0f);

	voxelToVoxelParam = glm::scale( glm::mat4( 1.0f ), glm::vec3( 1.0f / (float) resX, 1.0f / (float) resY, 1.0f/ (float) resZ) );

	worldToVoxelParam =	voxelToVoxelParam * worldToVoxel;

	}

void VoxelGridGPU::setView( glm::mat4 view ) {
	this->view = view;

	computeWorldToVoxel();
}

void VoxelGridGPU::printInfo() {
	DEBUGLOG->log("resX     : ", resX);
	DEBUGLOG->log("resY     : ", resY);
	DEBUGLOG->log("resZ     : ", resZ);
	DEBUGLOG->log("width    : ", width);
	DEBUGLOG->log("height   : ", height);
	DEBUGLOG->log("depth    : ", depth);
	DEBUGLOG->log("cellSize : ", cellSize);

}
