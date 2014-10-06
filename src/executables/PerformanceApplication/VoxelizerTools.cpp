#include "VoxelizerTools.h"

#include "Utility/DebugLog.h"

ComputeVoxelizer::ComputeVoxelizer() {
	m_computeShader = 0;
}

ComputeVoxelizer::~ComputeVoxelizer() {
}

DispatchVoxelGridComputeShaderListener::DispatchVoxelGridComputeShaderListener(
		ComputeShader* computeShader, VoxelGridGPU* voxelGrid, int x, int y,
		int z)
	: DispatchComputeShaderListener(computeShader, x,y,z)
{
	p_voxelGrid = voxelGrid;
}

DispatchClearVoxelGridComputeShader::DispatchClearVoxelGridComputeShader(
		ComputeShader* computeShader, VoxelGridGPU* voxelGrid, int x, int y,
		int z)
	: DispatchVoxelGridComputeShaderListener(computeShader, voxelGrid, x,y,z)
{
}

void DispatchClearVoxelGridComputeShader::call() {
	// only thing that works for the f-ing laptop

//		glBindTexture(GL_TEXTURE_2D, p_voxelGrid->handle);
//
//		std::vector < GLuint > emptyData( voxelGridResolution * voxelGridResolution , 0);
//		glTexSubImage2D(
//				GL_TEXTURE_2D,	// target
//				0,				// level
//				0,				// xOffset
//				0,				// yOffset
//				voxelGridResolution, // width
//				voxelGridResolution, // height
//				GL_RED,			// format
//				GL_UNSIGNED_INT,// type
//				&emptyData[0] );// data
//
//		glBindTexture(GL_TEXTURE_2D, 0);

	p_computeShader->useProgram();

	// upload clear texture index binding
	glBindImageTexture(0,
	p_voxelGrid->handle,
	0,
	GL_FALSE,
	0,
	GL_WRITE_ONLY,						// only write
	GL_R32UI);							// 1 channel 32 bit unsigned int

	// set suitable amount of work groups
	m_num_groups_x = p_voxelGrid->resX / p_computeShader->getLocalGroupSizeX() + ( ( p_voxelGrid->resX % p_computeShader->getLocalGroupSizeX() == 0 ) ? 0 : 1 );
	m_num_groups_y = p_voxelGrid->resY / p_computeShader->getLocalGroupSizeY() + ( ( p_voxelGrid->resY % p_computeShader->getLocalGroupSizeY() == 0 ) ? 0 : 1 );
	m_num_groups_z = 1;

	// dispatch as usual
	DispatchComputeShaderListener::call();

	// barrier bla whatever
	glMemoryBarrier( GL_ALL_BARRIER_BITS );

	// unbind clear texture
	glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI );
}

DispatchMipmapVoxelGridComputeShader::DispatchMipmapVoxelGridComputeShader(
		ComputeShader* computeShader, VoxelGridGPU* voxelGrid, int x, int y,
		int z)
: DispatchVoxelGridComputeShaderListener(computeShader, voxelGrid, x,y,z)
{
}

void DispatchMipmapVoxelGridComputeShader::call() {
	p_computeShader->useProgram();

	// bind to access image size
	glBindTexture( GL_TEXTURE_2D, p_voxelGrid->handle );

	int current_mipmap_res;

	double totalExecutionTime = 0.0;

	for ( int i = 1; i <= p_voxelGrid->numMipmaps; i++ )
	{

		// bind voxel grid mipmap base level
		glBindImageTexture(
		0,									// texture image unit
		p_voxelGrid->handle,				// texture name
		i-1,                                // mipmap level
		GL_FALSE,							// layered
		0,									// layer
		GL_READ_ONLY,						// only read access
		GL_R32UI);							// 1 channel 32 bit unsigned int

		// bind voxel grid mipmap target level
		glBindImageTexture(
		1,									// texture image unit
		p_voxelGrid->handle,				// texture name
		i,                                  // mipmap level
		GL_FALSE,                           // layered
		0,                                  // layer
		GL_WRITE_ONLY,						// only write access
		GL_R32UI);							// 1 channel 32 bit unsigned int

		glGetTexLevelParameteriv( GL_TEXTURE_2D, i, GL_TEXTURE_WIDTH, &current_mipmap_res );
//			DEBUGLOG->log( "current mipmap target level size : ", current_mipmap_res );

		// make sure mipmap is written before proceeding
		glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );

		// set suitable amount of work groups
		m_num_groups_x = current_mipmap_res / p_computeShader->getLocalGroupSizeX() + ( ( current_mipmap_res % p_computeShader->getLocalGroupSizeX() == 0 ) ? 0 : 1 );
		m_num_groups_y = current_mipmap_res / p_computeShader->getLocalGroupSizeY() + ( ( current_mipmap_res % p_computeShader->getLocalGroupSizeY() == 0 ) ? 0 : 1 );
		m_num_groups_z = 1;

		// dispatch compute shader
		DispatchComputeShaderListener::call();

		// add this execution time to the total execution time
		if ( m_queryTime ) {
			totalExecutionTime += m_executionTime;
		}
	}

	if ( m_queryTime )
	{
				m_executionTime = totalExecutionTime;
	}

	// arbitrary barrier
	glMemoryBarrier( GL_ALL_BARRIER_BITS );

	// unbind textures
	glBindTexture( GL_TEXTURE_2D, 0);
	glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_ONLY,  GL_R32UI );
	glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI );
}

DispatchVoxelizeWithTexAtlasComputeShader::DispatchVoxelizeWithTexAtlasComputeShader(
		ComputeShader* computeShader,
		std::vector<std::pair<Object*, TexAtlas::TextureAtlas*> > objects,
		VoxelGridGPU* voxelGrid, Texture* bitMask, int x, int y, int z)
	: DispatchVoxelGridComputeShaderListener(computeShader, voxelGrid, x,y,z)
{
	m_objects = objects;
	p_bitMask = bitMask;
}

void DispatchVoxelizeWithTexAtlasComputeShader::call() 	{
	// use compute program
	p_computeShader->useProgram();

	// upload output texture
	glBindImageTexture(0,
	p_voxelGrid->handle,
	0,
	GL_FALSE,
	0,
	GL_READ_WRITE,						// allow both for atomic operations
	GL_R32UI);							// 1 channel 32 bit unsigned int to make sure OR-ing works

	// upload bit mask
	glBindImageTexture(1,
	p_bitMask->getTextureHandle(),
	0,
	GL_FALSE,
	0,
	GL_READ_ONLY,
	GL_R32UI
	);

	double totalExecutionTime = 0.0;

	// dispatch this shader once per object
	for ( unsigned int i = 0; i < m_objects.size(); i++)
	{
		Object* pixelsObject = m_objects[i].first;
		TexAtlas::TextureAtlas* textureAtlas = m_objects[i].second;

		int numVertices = 0;
		int numIndices = 0;
		if ( pixelsObject->getModel() )
		{
//			// bind positions VBO to shader storage buffer
//			glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, pixelsObject->getModel()->getPositionBufferHandle() );
//
//			numVertices = pixelsObject->getModel()->getNumVertices();



			// bind positions VBO to shader storage buffer
			glBindBufferBase(
					GL_SHADER_STORAGE_BUFFER,   // target
					0,							// target binding index
					pixelsObject->getModel()->getPositionBufferHandle() );	// buffer

			numVertices = pixelsObject->getModel()->getNumVertices();

			// bind index buffer
			glBindBufferBase(
					GL_SHADER_STORAGE_BUFFER,	// target
					1,							// target binding index
					pixelsObject->getModel()->getIndexBufferHandle() ); // buffer

			numIndices = pixelsObject->getModel()->getNumIndices();
			numVertices = pixelsObject->getModel()->getNumVertices();

		}
		else
		{
			continue;
		}

		if (textureAtlas)
		{
			textureAtlas->unbindFromActiveUnit();

			// bind textureAtlas
			textureAtlas->bindToTextureUnit(3);

			// upload uniform textureAtlas position
			if ( !p_computeShader->uploadUniform(3 , "uniformTextureAtlas" ) )
			{
				DEBUGLOG->log("ERROR : Failed to upload uniform texture atlas");
			}
		}

		// upload uniform voxel grid matrix
		p_computeShader->uploadUniform( p_voxelGrid->worldToVoxel, "uniformWorldToVoxel" );

		// upload uniform vertices amount
		p_computeShader->uploadUniform( numVertices, "uniformNumVertices");

		// set local group amount suitable for object size:
		m_num_groups_x = int ( ceil( (float)numVertices / (float)p_computeShader->getLocalGroupSizeX() ) );
		m_num_groups_y = 1;
		m_num_groups_z = 1;

		// dispatch as usual
		DispatchComputeShaderListener::call();

		glMemoryBarrier( GL_ALL_BARRIER_BITS );

		if ( m_queryTime )
		{
			totalExecutionTime += m_executionTime;
		}
	}

	if ( m_queryTime )
	{
		m_executionTime = totalExecutionTime;
	}

	// unbind image textures
	glBindImageTexture(0, 0, 0,
	GL_FALSE, 0,
	GL_READ_WRITE,
	GL_R32UI);

	glBindImageTexture(1, 0, 0,
	GL_FALSE, 0,
	GL_READ_ONLY,
	GL_R32UI);
}

void DispatchVoxelizeComputeShader::call()
{
	// use compute program
	p_computeShader->useProgram();

	// unbind output texture
	p_voxelGrid->texture->unbindFromActiveUnit();

	// upload output texture
	glBindImageTexture(0,
	p_voxelGrid->handle,
	0,
	GL_FALSE,
	0,
	GL_READ_WRITE,						// allow both
	GL_R32UI);							// 1 channel 32 bit unsigned int to make sure OR-ing works

	// upload bit mask
	glBindImageTexture(1,
	p_bitMask->getTextureHandle(),
	0,
	GL_FALSE,
	0,
	GL_READ_ONLY,
	GL_R32UI
	);

	double totalExecutionTime = 0.0;

	// dispatch this shader once per object
	for ( unsigned int i = 0; i < m_objects.size(); i++)
	{
		Object* object = m_objects[i].first;
		RenderableNode* objectNode = m_objects[i].second;

		int numVertices = 0;
		int numIndices = 0;
		int numFaces = 0;

		if ( object->getModel() )
		{
			// bind positions VBO to shader storage buffer
			glBindBufferBase(
					GL_SHADER_STORAGE_BUFFER,   // target
					0,							// target binding index
					object->getModel()->getPositionBufferHandle() );	// buffer

			numVertices = object->getModel()->getNumVertices();

			// bind index buffer
			glBindBufferBase(
					GL_SHADER_STORAGE_BUFFER,	// target
					1,							// target binding index
					object->getModel()->getIndexBufferHandle() ); // buffer

			numIndices = object->getModel()->getNumIndices();
			numFaces = object->getModel()->getNumFaces();
		}
		else
		{
			continue;
		}

		glm::mat4 modelMatrix = glm::mat4(1.0f);
		if (objectNode)
		{
			modelMatrix = objectNode->getAccumulatedModelMatrix();
		}

		// upload uniform model matrix
		p_computeShader->uploadUniform( modelMatrix, "uniformModel");

		// upload voxel grid info
		p_computeShader->uploadUniform( p_voxelGrid->worldToVoxel, "uniformWorldToVoxel");

		// upload geometric info
		p_computeShader->uploadUniform( numVertices, "uniformNumVertices");
		p_computeShader->uploadUniform( numIndices,  "uniformNumIndices");
		p_computeShader->uploadUniform( numFaces,    "uniformNumFaces");

		// set local group amount suitable for object size:
		m_num_groups_x = numFaces / p_computeShader->getLocalGroupSizeX() + ( ( numFaces % p_computeShader->getLocalGroupSizeX() == 0 ) ? 0 : 1 );
		m_num_groups_y = 1;
		m_num_groups_z = 1;

		// dispatch as usual
		DispatchComputeShaderListener::call();

		glMemoryBarrier( GL_ALL_BARRIER_BITS );

		if ( m_queryTime )
		{
			totalExecutionTime += m_executionTime;
		}
	}

	if ( m_queryTime )
	{
		m_executionTime = totalExecutionTime;
	}

	// unbind image textures
	glBindImageTexture(0, 0, 0,
	GL_FALSE, 0,
	GL_READ_WRITE,
	GL_R32UI);
	glBindImageTexture(1, 0, 0,
	GL_FALSE, 0,
	GL_READ_ONLY,
	GL_R32UI);
}

DispatchVoxelizeComputeShader::DispatchVoxelizeComputeShader(
		ComputeShader* computeShader,
		std::vector<std::pair<Object*, RenderableNode*> > objects,
		VoxelGridGPU* voxelGrid, Texture* bitMask, int x, int y, int z)
: DispatchVoxelGridComputeShaderListener(computeShader, voxelGrid, x,y,z)
{
	m_objects = objects;
	p_bitMask = bitMask;
}

VoxelizationManager::VoxelizationManager() {
	m_activeVoxelizationMethod = SLICEMAP;
	m_dispatchVoxelizeCompute = 0;
	m_dispatchVoxelizeTexAtlasCompute = 0;
	m_dispatchCountFullVoxels = 0;
	m_sliceMapRenderPass = 0;
	m_texAtlasSliceMapRenderPass = 0;
	m_activeCandidateObject = 0;
	m_activeVoxelGrid = 0;

	m_voxelizationMethods.push_back(SLICEMAP);
	m_voxelizationMethods.push_back(TEXATLAS);
	m_voxelizationMethods.push_back(COMPUTE);
	m_voxelizationMethods.push_back(COMPUTETEXATLAS);

	m_compute_time_mean = -1.0;
	m_computeTexAtlas_time_mean = -1.0;
	m_texAtlas_time_mean = -1.0;
	m_sliceMap_time_mean = -1.0;
	m_texAtlasUpdate_time_mean = -1.0;

	m_iterationCounter = 0;
	m_compute_times.resize(100,-1000.0);
	m_computeTexAtlas_times.resize(100,-1000.0);
	m_texAtlas_times.resize(100,-1000.0);
	m_texAtlasUpdate_times.resize(100,-10000.0);
	m_sliceMap_times.resize(100,-1000.0);

	m_startTime = 0.0;
	m_stopTime = 0.0;
	m_executionTime = 0.0;
	m_elapsedTime = 0.0;

	m_compute_voxels = 0;
	m_computeTexAtlas_voxels = 0;
	m_texAtlas_voxels = 0;
	m_sliceMap_voxels = 0;
}

VoxelizationManager::~VoxelizationManager() {
}

void VoxelizationManager::call() {

	// update node dimensions to fit into grid
	m_activeCandidateObject->m_node->setModelMatrix(glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, ( (float)m_activeVoxelGrid->resZ / (float)m_activeVoxelGrid->resY) ) ) );

	// voxelize
	switch (m_activeVoxelizationMethod)
	{
	case SLICEMAP:
		if ( m_sliceMapRenderPass != 0 )
		{
			// update voxelgrid
			m_sliceMapRenderPass->setFramebufferObject( m_activeVoxelGrid->fbo );
			m_sliceMapRenderPass->setViewport(0,0,m_activeVoxelGrid->resX, m_activeVoxelGrid->resY);

			// update voxelization camera
			m_sliceMapRenderPass->getCamera()->setProjectionMatrix( m_activeVoxelGrid->perspective );
//			m_sliceMapRenderPass->getCamera()->setPosition(0.0,0.0,m_activeVoxelGrid->depth/2.0f);

			// update voxelization object
			m_sliceMapRenderPass->clearRenderables();
			m_sliceMapRenderPass->addRenderable( m_activeCandidateObject->m_node );

			startTime();	// begin time query
			m_sliceMapRenderPass->render();
			stopTime();		// end time query
			m_sliceMap_times[m_iterationCounter] = m_executionTime; // update time
		}
		break;
	case COMPUTE:
		if ( m_dispatchVoxelizeCompute != 0 )
		{
			//update voxelgrid
			m_dispatchVoxelizeCompute->p_voxelGrid = m_activeVoxelGrid;

			// update voxelization object
			m_dispatchVoxelizeCompute->m_objects.clear();
			std::pair<Object*, RenderableNode*> voxelizeObject(m_activeCandidateObject->m_object, m_activeCandidateObject->m_node);
			m_dispatchVoxelizeCompute->m_objects.push_back(voxelizeObject);
			startTime();	// begin time query
			m_dispatchVoxelizeCompute->call();
			stopTime();		// end time query
			m_compute_times[m_iterationCounter] = m_executionTime; // update time
		}
		break;
	case COMPUTETEXATLAS:
		if ( m_dispatchVoxelizeTexAtlasCompute != 0)
		{
			// update voxelgrid
			m_dispatchVoxelizeTexAtlasCompute->p_voxelGrid = m_activeVoxelGrid;

			// update texture atlas
			startTime();
			m_activeCandidateObject->m_texAtlasRenderPass->render(); // update texatlas
			stopTime();
			m_texAtlasUpdate_times[m_iterationCounter] = m_executionTime;

			// update voxelization object
			m_dispatchVoxelizeTexAtlasCompute->m_objects.clear();
			std::pair<Object*, TexAtlas::TextureAtlas*> voxelizeObject(m_activeCandidateObject->m_atlasObject, m_activeCandidateObject->m_atlas);
			m_dispatchVoxelizeTexAtlasCompute->m_objects.push_back(voxelizeObject);

			startTime(); // begin time query
			m_dispatchVoxelizeTexAtlasCompute->call();
			stopTime();		// end time query
			m_computeTexAtlas_times[m_iterationCounter] = m_executionTime; // update time
		}
		break;
	case TEXATLAS:
		if ( m_texAtlasSliceMapRenderPass != 0)
		{
			// update voxelgrid fbo
			m_texAtlasSliceMapRenderPass->setFramebufferObject( m_activeVoxelGrid->fbo );
			m_texAtlasSliceMapRenderPass->setViewport(0,0,m_activeVoxelGrid->resX, m_activeVoxelGrid->resY);

			// update voxelgrid
			m_texAtlasSliceMapRenderPass->getCamera()->setProjectionMatrix( m_activeVoxelGrid->perspective );


			// update texture atlas
			startTime();
			m_activeCandidateObject->m_texAtlasRenderPass->render(); // update texatlas
			stopTime();
			m_texAtlasUpdate_times[m_iterationCounter] = m_executionTime;

			// update voxelization object
			m_texAtlasSliceMapRenderPass->clearRenderables();
			m_texAtlasSliceMapRenderPass->addRenderable( m_activeCandidateObject->m_atlasObject ); // atlas vertices object

			startTime();
			m_texAtlasSliceMapRenderPass->render(); // render slicemap
			stopTime();
			m_texAtlas_times[m_iterationCounter] = m_executionTime;
		}
		break;
	}

	m_iterationCounter++;
	if ( m_iterationCounter >= 100 )
	{
		m_iterationCounter = 0;

		// update mean times
		double sumCompute, sumComputeTexAtlas, sumTexAtlasUpdate, sumTexAtlas,
				sumSliceMap = 0.0;
		for (unsigned int i = 0; i < 100; i++) {
			sumCompute += m_compute_times[i];
			sumComputeTexAtlas += m_computeTexAtlas_times[i];
			sumTexAtlas += m_texAtlas_times[i];
			sumTexAtlasUpdate += m_texAtlasUpdate_times[i];
			sumSliceMap += m_sliceMap_times[i];
		}

		m_compute_time_mean = sumCompute / 100.0;
		m_sliceMap_time_mean = sumSliceMap / 100.0;
		m_texAtlas_time_mean = sumTexAtlas / 100.0;
		m_computeTexAtlas_time_mean = sumComputeTexAtlas / 100.0;
		m_texAtlasUpdate_time_mean = sumTexAtlasUpdate / 100.0;

	}



	// count voxels
	if (m_dispatchCountFullVoxels != 0) {
		m_dispatchCountFullVoxels->p_voxelGrid = m_activeVoxelGrid;

		m_dispatchCountFullVoxels->call();
		switch (m_activeVoxelizationMethod) {
		case SLICEMAP:
			m_sliceMap_voxels = m_dispatchCountFullVoxels->m_amount;
			break;
		case COMPUTETEXATLAS:
			m_computeTexAtlas_voxels = m_dispatchCountFullVoxels->m_amount;
			break;
		case COMPUTE:
			m_compute_voxels = m_dispatchCountFullVoxels->m_amount;
			break;
		case TEXATLAS:
			m_texAtlas_voxels = m_dispatchCountFullVoxels->m_amount;
			break;
		}
	}
}

CandidateObject::CandidateObject() {
	m_object = 0;
	m_texAtlasRenderPass = 0;
	m_atlas = 0;
	m_atlasObject = 0;
	m_node = 0;
}

CandidateObject::~CandidateObject() {
}

#include <Utility/UtilityListeners.h>

Listener* VoxelizationManager::getSwitchThroughCandidatesListener() {
	return new SwitchThroughValuesListener<CandidateObject* >(&m_activeCandidateObject, m_candidateObjects );
}

Listener* VoxelizationManager::getSwitchThroughVoxelGridsListener() {
	return new SwitchThroughValuesListener<VoxelGridGPU* >(&m_activeVoxelGrid, m_voxelGrids);
}

class UpdateDebugGeometrySizeListener : public Listener
{
public:
	VoxelizationManager* p_voxelizationManager;
	Node* p_debugGeometryNode;

	UpdateDebugGeometrySizeListener(Node* debugGeometry, VoxelizationManager* voxelizationManager)
	{
		p_voxelizationManager = voxelizationManager;
		p_debugGeometryNode = debugGeometry;
	}

	void call()
	{
		VoxelGridGPU* activeVoxelGrid = p_voxelizationManager->m_activeVoxelGrid;
		if ( activeVoxelGrid != 0 && p_debugGeometryNode != 0)
		{
			p_debugGeometryNode->setModelMatrix( glm::scale(glm::mat4(1.0),glm::vec3(
					activeVoxelGrid->width,
					activeVoxelGrid->height,
					activeVoxelGrid->depth
					)));
		}
	}
};

Listener* VoxelizationManager::getUpdateDebugGeometrySizeListener(
		Node* debugGeometry) {
	return new UpdateDebugGeometrySizeListener(debugGeometry, this);
}

class UpdateDispatchVoxelGridComputeShaderListener : public Listener
{
public:
DispatchVoxelGridComputeShaderListener* p_dispatcher;
VoxelizationManager* p_voxelizationManager;
	UpdateDispatchVoxelGridComputeShaderListener(DispatchVoxelGridComputeShaderListener* dispatcher, VoxelizationManager* voxelizationManager)
{
		p_dispatcher = dispatcher;
		p_voxelizationManager = voxelizationManager;
}
	void call()
	{
		p_dispatcher->p_voxelGrid = p_voxelizationManager->m_activeVoxelGrid;
	}
};

Listener* VoxelizationManager::getUpdateDispatchVoxelGridComputeShaderListener(
		DispatchVoxelGridComputeShaderListener* dispatcher) {
	return new UpdateDispatchVoxelGridComputeShaderListener(dispatcher, this);
}

Listener* VoxelizationManager::getUpdateVoxelgridReferenceListener(
		VoxelGridGPU** voxelgridPtr) {
	return new CopyValueListener<VoxelGridGPU*>(voxelgridPtr, &m_activeVoxelGrid);
}

class UpdateWorldToVoxelListener : public Listener
{
public:
	VoxelizationManager* p_voxelizationManager;
	glm::mat4** p_worldToVoxel;

	UpdateWorldToVoxelListener(glm::mat4** worldToVoxel, VoxelizationManager* voxelizationManager)
	{
		p_voxelizationManager = voxelizationManager;
		p_worldToVoxel = worldToVoxel;
	}

	void call()
	{
		VoxelGridGPU* activeVoxelGrid = p_voxelizationManager->m_activeVoxelGrid;
		if ( activeVoxelGrid != 0 && p_worldToVoxel != 0)
		{
			*p_worldToVoxel = &activeVoxelGrid->worldToVoxel;
		}
	}
};

Listener* VoxelizationManager::getUpdateVoxelgridWorldToVoxelReferenceListener(glm::mat4** matPtr)
{
	return new UpdateWorldToVoxelListener(matPtr, this);
}

class PrintCurrentVoxelGridInfoListener : public Listener
{
public:
	VoxelizationManager* p_voxelizationManager;
	PrintCurrentVoxelGridInfoListener(VoxelizationManager* voxelizationManager)
	{
		p_voxelizationManager = voxelizationManager;
	}

	virtual void call()
	{
		p_voxelizationManager->m_activeVoxelGrid->printInfo();
	}
};

Listener* VoxelizationManager::getSwitchThroughVoxelizaionMethodsListener() {
	return new SwitchThroughValuesListener<VoxelizationMethod>(&m_activeVoxelizationMethod, m_voxelizationMethods);
}

Listener* VoxelizationManager::getPrintCurrentVoxelGridInfoListener()
{
	return new PrintCurrentVoxelGridInfoListener(this);
}

class PrintCurrentVoxelizationMethodListener : public Listener
{
public:
	VoxelizationManager* p_voxelizationManager;
	PrintCurrentVoxelizationMethodListener(VoxelizationManager* voxelizationManager)
	{
		p_voxelizationManager = voxelizationManager;
	}

	virtual void call()
	{
		switch (p_voxelizationManager->m_activeVoxelizationMethod)
		{
		case VoxelizationManager::COMPUTE:
			DEBUGLOG->log("active voxelization method : COMPUTE");
			break;
		case VoxelizationManager::SLICEMAP:
			DEBUGLOG->log("active voxelization method : SLICE MAP");
			break;
		case VoxelizationManager::TEXATLAS:
			DEBUGLOG->log("active voxelization method : TEXTURE ATLAS");
			break;
		case VoxelizationManager::COMPUTETEXATLAS:
			DEBUGLOG->log("active voxelization method : COMPUTE TEXTURE ATLAS");
			break;

		}
	}
};

void VoxelizationManager::startTime() {

	glGenQueries(1, &m_queryID[0] );
	glBeginQuery(GL_TIME_ELAPSED, m_queryID[0]);

//	// generate OpenGL query objects
//	glGenQueries(2, &m_queryID[0] );
//
//	// request current time-stamp ( before GL command)
//	glQueryCounter(m_queryID[0], GL_TIMESTAMP);
}

void VoxelizationManager::stopTime() {

	glEndQuery(GL_TIME_ELAPSED);

	unsigned int stopTimerAvailable = 0;
	while (!stopTimerAvailable )
	{
	    glGetQueryObjectuiv(m_queryID[0],
	    		GL_QUERY_RESULT_AVAILABLE,
	    		&stopTimerAvailable);
	}

	// retrieve query results
	glGetQueryObjectui64v(m_queryID[0], GL_QUERY_RESULT, &m_elapsedTime);
	m_executionTime = (m_elapsedTime) / 1000000.0;

//	DEBUGLOG->log("Elapsed Time on GPU : ", m_executionTime );


	//	// request current time-stamp ( after GL command returns )
//	glQueryCounter(m_queryID[1], GL_TIMESTAMP);
//
//	// wait for query to become available ( when command finished )
//	unsigned int stopTimerAvailable = 0;
//	while (!stopTimerAvailable )
//	{
//	    glGetQueryObjectuiv(m_queryID[1],
//	    		GL_QUERY_RESULT_AVAILABLE,
//	    		&stopTimerAvailable);
//	}
//
//	// retrieve query results
//	glGetQueryObjectui64v(m_queryID[0], GL_QUERY_RESULT, &m_startTime);
//	glGetQueryObjectui64v(m_queryID[1], GL_QUERY_RESULT, &m_stopTime);
//
//	// compute execution time
//	m_executionTime = (m_stopTime - m_startTime) / 1000000.0;

}

Listener* VoxelizationManager::getPrintCurrentVoxelizationMethodListener()
{
	return new PrintCurrentVoxelizationMethodListener(this);
}

Listener* VoxelizationManager::getPrintVoxelizationTimesListener() {
	SubjectListener* subjectListener = new SubjectListener();
	subjectListener->addListener(new DebugPrintListener(                        "-----------------------------------------------"));
	subjectListener->addListener(new DebugPrintDoubleListener(&m_sliceMap_time_mean, "Voxelization Times (ms) : Slice Map        : "));
	subjectListener->addListener(new DebugPrintDoubleListener(&m_texAtlas_time_mean, "Voxelization Times (ms) : Texture Atlas    : "));
	subjectListener->addListener(new DebugPrintDoubleListener(&m_compute_time_mean,  "Voxelization Times (ms) : Compute          : "));
	subjectListener->addListener(new DebugPrintDoubleListener(&m_computeTexAtlas_time_mean, "Voxelization Times (ms) : Compute TexAtlas : "));
	subjectListener->addListener(new DebugPrintDoubleListener(&m_texAtlasUpdate_time_mean, "Voxelization Times (ms) : Update TexAtlas  : "));
	subjectListener->addListener(new DebugPrintListener(                        "-----------------------------------------------"));

	return subjectListener;

}

DispatchCountFullVoxelsComputeShader::DispatchCountFullVoxelsComputeShader(
		ComputeShader* computeShader, VoxelGridGPU* voxelGrid,
		GLuint atomicCounterHandle, int x, int y, int z)
	: DispatchVoxelGridComputeShaderListener(computeShader, voxelGrid, x,y,z)
{
	m_atomicCounterHandle = atomicCounterHandle;
	m_amount = 0;
}

void DispatchCountFullVoxelsComputeShader::call() {
	p_computeShader->useProgram();

	// reset atomic counters
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, m_atomicCounterHandle);
	GLuint atomicCounterValues[3] = {0,0,0};
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint) * 3, atomicCounterValues);

	// upload clear texture index binding
	glBindImageTexture(0,
	p_voxelGrid->handle,
	0,
	GL_FALSE,
	0,
	GL_READ_ONLY,						// only read
	GL_R32UI);							// 1 channel 32 bit unsigned int

	// set suitable amount of work groups
	m_num_groups_x = int( ceil( (float)p_voxelGrid->resX / (float)p_computeShader->getLocalGroupSizeX() ) );
	m_num_groups_y = int( ceil( (float)p_voxelGrid->resY / (float)p_computeShader->getLocalGroupSizeY() ) );
	m_num_groups_z = 1;

	// dispatch as usual
	DispatchComputeShaderListener::call();

	// barrier bla whatever
	glMemoryBarrier( GL_ALL_BARRIER_BITS );

	// retrieve counter values
	glGetBufferSubData( GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint) * 3, atomicCounterValues);

	// retrieve counter value
	m_amount = atomicCounterValues[0];

	// release Atomic Counter Buffer binding point
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, 0);

	// unbind clear texture
	glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI );
}

Listener* VoxelizationManager::getPrintVoxelizationFillAmountListener() {
	SubjectListener* subjectListener = new SubjectListener();
	subjectListener->addListener(new DebugPrintListener(                        "-----------------------------------------------"));
	subjectListener->addListener(new DebugPrintValueListener<unsigned int>(&m_sliceMap_voxels, "Voxelization fill amount: Slice Map        : "));
	subjectListener->addListener(new DebugPrintValueListener<unsigned int>(&m_texAtlas_voxels, "Voxelization fill amount: Texture Atlas    : "));
	subjectListener->addListener(new DebugPrintValueListener<unsigned int>(&m_compute_voxels,  "Voxelization fill amount: Compute          : "));
	subjectListener->addListener(new DebugPrintValueListener<unsigned int>(&m_computeTexAtlas_voxels, "Voxelization fill amount: Compute TexAtlas : "));
	subjectListener->addListener(new DebugPrintListener(                        "-----------------------------------------------"));

	return subjectListener;

}
