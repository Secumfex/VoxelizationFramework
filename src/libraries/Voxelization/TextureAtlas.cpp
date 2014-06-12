#include <Voxelization/TextureAtlas.h>


Shader*	writeWorldPositionTextureAtlasShader = 0;	// globally accessible Shader to be used to write a texture atlas

Shader* TexAtlas::getWriteWorldPositionTextureAtlasShader() {
	if (!writeWorldPositionTextureAtlasShader)
	{
		writeWorldPositionTextureAtlasShader = new Shader(
				SHADERS_PATH "/textureAtlas/simpleUV.vert",
				SHADERS_PATH "/textureAtlas/worldPosition.frag");
	}
	else
	{
		return writeWorldPositionTextureAtlasShader;
	}
}

void TexAtlas::TextureAtlasRenderer::configureRenderPass(int width,
		int height) {
	// TODO make sure renderpass uses the provided Camera
	// TODO make sure renderpass clears with alpha 0.0

}

void TexAtlas::TextureAtlasRenderer::configureFramebufferObject(int width,
		int height) {
	// TODO make sure there is a render target texture
}

TexAtlas::TextureAtlasRenderer::TextureAtlasRenderer(
		RenderableNode* renderableNode, int width, int height, Camera* camera) {
	p_renderableNode = renderableNode;
	p_camera = camera;

	if (camera == 0)
		{
			configureCamera();
		}

	m_fbo = new FramebufferObject(width, height);
	configureFramebufferObject(width, height);

	m_renderPass = new CameraRenderPass( getWriteWorldPositionTextureAtlasShader(), m_fbo);
	configureRenderPass(width, height);

}

TexAtlas::TextureAtlasRenderer::~TextureAtlasRenderer() {
}

void TexAtlas::TextureAtlasRenderer::render() {
}

void TexAtlas::TextureAtlasRenderer::uploadUniforms(Shader* shader) {
}

void TexAtlas::TextureAtlasRenderer::configureCamera() {
	// TODO : retrieve object bbox and create corresponding frustum for camera
}

RenderableNode* TexAtlas::TextureAtlasRenderer::getRenderableNodePtr() {
	return p_renderableNode;
}


