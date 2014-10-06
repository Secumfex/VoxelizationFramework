#ifndef VOXELIZERTOOLS_H
#define VOXELIZERTOOLS_H

#include "VoxelGridTools.h"

#include <Voxelization/TextureAtlas.h>
#include <Voxelization/SliceMapRendering.h>
#include <Rendering/Shader.h>
#include <Misc/MiscListeners.h>
#include <Utility/SubjectListenerPattern.h>
#include <Scene/RenderableNode.h>

class DispatchVoxelizeComputeShader;
class DispatchVoxelizeWithTexAtlasComputeShader;
class DispatchVoxelGridComputeShaderListener;
class DispatchCountFullVoxelsComputeShader;


// all that is needed to voxelize with whatever method
class CandidateObject{
public:
	RenderableNode* m_node;			// scene graph node
	Object* m_object;				// object
	TexAtlas::TextureAtlas* m_atlas;// texture atlas
	TexAtlas::TextureAtlasRenderPass* 	m_texAtlasRenderPass; // texture atlas render pass
	Object* m_atlasObject;			// texture atlas vertices object

	CandidateObject();
	virtual ~CandidateObject();
};

/**
 * knows a voxelizer for every voxelization method
 */
class VoxelizationManager : public Listener{
public:
	// possible methods to voxelize
	enum VoxelizationMethod {
		SLICEMAP,
		TEXATLAS,
		COMPUTE,
		COMPUTETEXATLAS,
	};

	// times
	unsigned int m_queryID[2];
	GLuint64 m_elapsedTime;
	GLuint64 m_startTime;
	GLuint64 m_stopTime;
	double m_executionTime;

	// current time
	double m_compute_time_mean;
	double m_computeTexAtlas_time_mean;
	double m_texAtlas_time_mean;
	double m_sliceMap_time_mean;

	double m_texAtlasUpdate_time_mean;

	std::vector<double> m_compute_times;
	std::vector<double> m_texAtlas_times;
	std::vector<double> m_texAtlasUpdate_times;
	std::vector<double> m_sliceMap_times;
	std::vector<double> m_computeTexAtlas_times;
	int m_iterationCounter;


	// filled voxels
	unsigned int m_compute_voxels;
	unsigned int m_computeTexAtlas_voxels;
	unsigned int m_texAtlas_voxels;
	unsigned int m_sliceMap_voxels;

	// Voxelizers
	DispatchVoxelizeComputeShader* 				m_dispatchVoxelizeCompute;
	DispatchVoxelizeWithTexAtlasComputeShader* 	m_dispatchVoxelizeTexAtlasCompute;
	SliceMap::SliceMapRenderPass*				m_sliceMapRenderPass;
//	TexAtlas::TextureAtlasRenderPass* 			m_texAtlasRenderPass;
	SliceMap::SliceMapRenderPass*				m_texAtlasSliceMapRenderPass;

	// count voxels dispatcher
	DispatchCountFullVoxelsComputeShader*       m_dispatchCountFullVoxels;

	VoxelizationMethod 							m_activeVoxelizationMethod;
	CandidateObject*							m_activeCandidateObject;
	VoxelGridGPU*								m_activeVoxelGrid;

	std::vector<CandidateObject*>  				m_candidateObjects;
	std::vector<VoxelGridGPU*>					m_voxelGrids;
	std::vector<VoxelizationMethod>				m_voxelizationMethods;

	VoxelizationManager();
	virtual ~VoxelizationManager();

	virtual void call();

	void startTime();
	void stopTime();

	// silly listeners
	Listener* getSwitchThroughCandidatesListener();
	Listener* getSwitchThroughVoxelGridsListener();
	Listener* getSwitchThroughVoxelizaionMethodsListener();
	Listener* getUpdateDebugGeometrySizeListener(Node* debugGeometry);

	Listener* getUpdateDispatchVoxelGridComputeShaderListener(DispatchVoxelGridComputeShaderListener* dispatcher);
	Listener* getUpdateVoxelgridReferenceListener(VoxelGridGPU** texture);
	Listener* getUpdateVoxelgridWorldToVoxelReferenceListener(glm::mat4** matPtr);

	Listener* getPrintCurrentVoxelGridInfoListener();
	Listener* getPrintCurrentVoxelizationMethodListener();
	Listener* getPrintVoxelizationTimesListener();
	Listener* getPrintVoxelizationFillAmountListener();
};

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
public:
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

/**
 * Count all full voxels of a voxel grid
 */
class DispatchCountFullVoxelsComputeShader : public DispatchVoxelGridComputeShaderListener
{
public:
	GLuint m_atomicCounterHandle;
	unsigned int m_amount;
public:
	DispatchCountFullVoxelsComputeShader (ComputeShader* computeShader, VoxelGridGPU* voxelGrid, GLuint atomicCounterHandle, int x = 0, int y = 0, int z = 0 );
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
public:
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
public:
	std::vector<std::pair<Object*, TexAtlas::TextureAtlas* > > m_objects;
	Texture* p_bitMask;
public:
	DispatchVoxelizeWithTexAtlasComputeShader(ComputeShader* computeShader, std::vector< std::pair<Object*, TexAtlas::TextureAtlas*> > objects, VoxelGridGPU* voxelGrid, Texture* bitMask, int x= 0, int y= 0, int z = 0 );
	void call();
};

#endif
