#ifndef TEXTUREATLAS_H
#define TEXTUREATLAS_H

#include <Rendering/Shader.h>
#include <Rendering/RenderPass.h>
#include <Rendering/CustomRenderPasses.h>
#include <Scene/RenderableNode.h>
#include <Utility/SubjectListenerPattern.h>

namespace TexAtlas
{
	Shader* getWriteWorldPositionTextureAtlasShader();

	/**
	 * This class represents a Texture Atlas, which is a special type of texture referencing an object
	 */
	class TextureAtlas : public Texture
	{
	protected:
		Object* p_object; 	// corresponding object
	public:
		TextureAtlas( Object* object = 0, GLuint TextureAtlasHandle = 0);
		~TextureAtlas();

		Object* 		getObject();
		void 			setObject( Object* object );
	};

	/**
	 * This class represents a Renderer for a Texture Atlas of a Model
	 * It consists of
	 * - a FBO with one texture to which the UV coordinates of the model will be written
	 * - a CameraRenderPass which will render only one object
	 * - a Camera which will be arbitrarily placed around the Object, to ensure all of it will be rendered
	 * - a RenderableNode pointer with an Object attached to it which will be rendered
	 */
	class TextureAtlasRenderPass : public RenderPass
	{
	protected:
		TextureAtlas*	m_textureAtlas;		// TextureAtlas to be rendered to

//		FramebufferObject* m_fbo;			// FBO to be rendered to
//
//		CameraRenderPass* m_renderPass;		// Renderpass to be used with this
//
//		Camera*			p_camera;			// arbitrary Camera to be used to render the Object ( should be the same as voxelization camera )

		RenderableNode* p_renderableNode;	// pointer to the object node to be rendered

		/**
		 * private methods used exclusively during configuration and rendering
		 */

//		void configureCamera();
		void configureRenderPass();
		void configureFramebufferObject();
		void configureTextureAtlas();

	public:
		/**
		 *	Create a Texture Atlas Renderer for an object
		 * @param renderableNode	to be rendered into a texture atlas
		 * @param width				of texture atlas
		 * @param height			of texture atlas
		 * @param camera			used to voxelize ( can be left out if texture atlas should be rendered, even if object is outside of voxelgrid )
		 */
		TextureAtlasRenderPass( RenderableNode* renderableNode, int width, int height, Camera* camera = 0 );
		virtual ~TextureAtlasRenderPass();

		RenderableNode* getRenderableNode();

		FramebufferObject* getFbo() ;
		void setFbo( FramebufferObject* fbo);
		CameraRenderPass* getRenderPass() ;
		void setRenderPass( CameraRenderPass* renderPass);
		TextureAtlas* getTextureAtlas();
		void setTextureAtlas( TextureAtlas* textureAtlas);
		Camera* getCamera() ;
		void setCamera( Camera* camera);
		void setRenderableNode( RenderableNode* renderableNode);
};


	/**
	 * This class represents a Generator of Vertices for a Texture Atlas
	 * It will produce a set of vertices from the provided Texture Atlas, one vertex per valid Atlas texel
	 */
	class TextureAtlasVertexGenerator : public Listener
	{
	protected:
		std::vector< glm::vec3 > m_vertexPositions;
		TextureAtlas* p_textureAtlas;

	public:
		TextureAtlasVertexGenerator( TextureAtlas* textureAtlas );
		virtual ~TextureAtlasVertexGenerator();

		std::vector< glm::vec3 >& getVertexPositions();

		void generateVertexPositions();

		void call();			// generate Vertex Positions if empty
	};
}

#endif
