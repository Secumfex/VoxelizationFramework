#ifndef VOXELIZERTOOLS_H
#define VOXELIZERTOOLS_H

#include "VoxelGridTools.h"

#include <Voxelization/TextureAtlas.h>
#include <Rendering/Shader.h>
#include <Misc/MiscListeners.h>
#include <Scene/RenderableNode.h>


/**
 * Everything needed to voxelize an object on the GPU
 */
// TODO what is it supposed to know, anyway?
class ComputeVoxelizer
{
private:
	ComputeShader* m_computeShader;
public:

	ComputeVoxelizer();
	~ComputeVoxelizer();
};

/**********************************************
 * DISPATCHCOMPUTESHADER LISTENER CLASSES
 **********************************************/

/**
 * A Dispatch ComputeShader Listener with a reference to a voxel grid
 */
class DispatchVoxelGridComputeShaderListener : public DispatchComputeShaderListener
{
protected:
	VoxelGridGPU* p_voxelGrid;
public:
	DispatchVoxelGridComputeShaderListener( ComputeShader* computeShader, VoxelGridGPU* voxelGrid, int x = 0, int y = 0, int z = 0  );
};

/**
 * Clear voxel grid texture with 0
 */
class DispatchClearVoxelGridComputeShader : public DispatchVoxelGridComputeShaderListener
{
public:
	DispatchClearVoxelGridComputeShader(ComputeShader* computeShader, VoxelGridGPU* voxelGrid, int x = 0, int y = 0, int z = 0 );
	void call();
};

class DispatchMipmapVoxelGridComputeShader : public DispatchVoxelGridComputeShaderListener
{
public:
	DispatchMipmapVoxelGridComputeShader(ComputeShader* computeShader, VoxelGridGPU* voxelGrid, int x = 0, int y = 0, int z = 0 );
	void call();
};

/**
 * Voxelize scene
 */
class DispatchVoxelizeComputeShader : public DispatchVoxelGridComputeShaderListener
{
protected:
	std::vector<std::pair<Object*, RenderableNode* > > m_objects;
	Texture* p_bitMask;
public:
	DispatchVoxelizeComputeShader(ComputeShader* computeShader, std::vector< std::pair<Object*, RenderableNode*> > objects,
			VoxelGridGPU* voxelGrid, Texture* bitMask,
			int x= 0, int y= 0, int z = 0 );
	void call();
};

/**
 * Voxelize scene via tex atlas
 */
class DispatchVoxelizeWithTexAtlasComputeShader : public DispatchVoxelGridComputeShaderListener
{
protected:
	std::vector<std::pair<Object*, TexAtlas::TextureAtlas* > > m_objects;
	Texture* p_bitMask;
public:
	DispatchVoxelizeWithTexAtlasComputeShader(ComputeShader* computeShader, std::vector< std::pair<Object*, TexAtlas::TextureAtlas*> > objects, VoxelGridGPU* voxelGrid, Texture* bitMask, int x= 0, int y= 0, int z = 0 );
	void call();
};

#endif
