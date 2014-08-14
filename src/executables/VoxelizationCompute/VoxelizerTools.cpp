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

		if ( pixelsObject->getModel() )
		{
			// bind positions VBO to shader storage buffer
			glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, pixelsObject->getModel()->getPositionBufferHandle() );

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
		m_num_groups_x = numVertices / p_computeShader->getLocalGroupSizeX() + ( ( numVertices % p_computeShader->getLocalGroupSizeX() == 0 ) ? 0 : 1 );
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
		m_num_groups_x = numFaces / p_computeShader->getLocalGroupSizeX() + ( ( numVertices % p_computeShader->getLocalGroupSizeX() == 0 ) ? 0 : 1 );
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
