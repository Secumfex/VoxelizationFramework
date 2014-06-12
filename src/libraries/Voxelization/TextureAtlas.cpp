#include <Voxelization/TextureAtlas.h>


Shader*	writeWorldPositionTextureAtlasShader = 0;	// globally accessible Shader to be used to write a texture atlas

Shader* TexAtlas::getWriteWorldPositionTextureAtlasShader() {
	if (!writeWorldPositionTextureAtlasShader)
	{
		writeWorldPositionTextureAtlasShader = new Shader(
				SHADERS_PATH "/textureAtlas/simpleUV.vert",
				SHADERS_PATH "/textureAtlas/worldPosition.frag");
	}
	return writeWorldPositionTextureAtlasShader;
}

void TexAtlas::TextureAtlasRenderer::configureRenderPass() {
	// make sure renderpass uses the provided Camera
	// make sure renderpass clears with alpha 0.0
	// make sure renderpass only renders the provided node

	m_renderPass->setCamera( p_camera );					// set camera ( TODO : is this even necessary ? )
	m_renderPass->setClearColor( 0.0f, 0.0f ,0.0f, 0.0f);	// clear with alpha = 0
	m_renderPass->addClearBit( GL_COLOR_BUFFER_BIT);		// clear on every frame

	m_renderPass->addRenderable( p_renderableNode );
}

void TexAtlas::TextureAtlasRenderer::configureFramebufferObject() {
	// make sure there is a render target texture
	m_fbo->addColorAttachments(1);
}

TexAtlas::TextureAtlasRenderer::TextureAtlasRenderer(
		RenderableNode* renderableNode, int width, int height, Camera* camera) {
	p_renderableNode = renderableNode;
	p_camera = camera;

	if (camera == 0)
	{
		p_camera = new Camera();
		configureCamera();
	}

	m_fbo = new FramebufferObject(width, height);
	configureFramebufferObject();

	m_renderPass = new CameraRenderPass( getWriteWorldPositionTextureAtlasShader(), m_fbo);
	configureRenderPass();

	m_textureAtlas = new TextureAtlas();
	configureTextureAtlas();
}

TexAtlas::TextureAtlasRenderer::~TextureAtlasRenderer() {
	m_fbo->~FramebufferObject();
	m_renderPass->~RenderPass();
}

void TexAtlas::TextureAtlasRenderer::render() {
	m_renderPass->render();	// just tell the CameraRenderPass to render dat object
}

void TexAtlas::TextureAtlasRenderer::uploadUniforms(Shader* shader) {
	// TODO : is there anything to do? don't think so...
}

void TexAtlas::TextureAtlasRenderer::configureCamera() {
	// TODO : retrieve object bbox and create corresponding frustum for camera
}

void TexAtlas::TextureAtlasRenderer::configureTextureAtlas() {
	m_textureAtlas->setObject( p_renderableNode->getObject() );
	m_textureAtlas->setTextureHandle( m_fbo->getColorAttachmentTextureHandle( GL_COLOR_ATTACHMENT0 ) );
}

 FramebufferObject* TexAtlas::TextureAtlasRenderer::getFbo()  {
	return m_fbo;
}

void TexAtlas::TextureAtlasRenderer::setFbo( FramebufferObject* fbo) {
	m_fbo = fbo;
}

 CameraRenderPass* TexAtlas::TextureAtlasRenderer::getRenderPass()  {
	return m_renderPass;
}

void TexAtlas::TextureAtlasRenderer::setRenderPass( CameraRenderPass* renderPass) {
	m_renderPass = renderPass;
}

TexAtlas::TextureAtlas* TexAtlas::TextureAtlasRenderer::getTextureAtlas() {
	return m_textureAtlas;
}

void TexAtlas::TextureAtlasRenderer::setTextureAtlas( TextureAtlas* textureAtlas) {
	m_textureAtlas = textureAtlas;
}

 Camera* TexAtlas::TextureAtlasRenderer::getCamera()  {
	return p_camera;
}

void TexAtlas::TextureAtlasRenderer::setCamera( Camera* camera) {
	p_camera = camera;
}

void TexAtlas::TextureAtlasRenderer::setRenderableNode(
		 RenderableNode* renderableNode) {
	p_renderableNode = renderableNode;
}

RenderableNode* TexAtlas::TextureAtlasRenderer::getRenderableNode() {
	return p_renderableNode;
}

TexAtlas::TextureAtlas::TextureAtlas(Object* object, GLuint textureAtlasHandle) {
	p_object = object;
	setTextureHandle( textureAtlasHandle );
}

TexAtlas::TextureAtlas::~TextureAtlas() {
}

Object* TexAtlas::TextureAtlas::getObject() {
	return p_object;
}

TexAtlas::TextureAtlasVertexGenerator::TextureAtlasVertexGenerator(
		TextureAtlas* textureAtlas) {
	p_textureAtlas = textureAtlas;
}

TexAtlas::TextureAtlasVertexGenerator::~TextureAtlasVertexGenerator() {
}

std::vector<glm::vec3>& TexAtlas::TextureAtlasVertexGenerator::getVertexPositions() {
	return m_vertexPositions;
}

void TexAtlas::TextureAtlas::setObject(Object* object) {
	p_object = object;
}

void TexAtlas::TextureAtlasVertexGenerator::generateVertexPositions() {
	// TODO : read texture ( using glGet... or something )
	// for every valid texel ( alpha == 1.0f )
		// push back glm::vec3 at this x,y coordinate ( set z = 0.0f )
}
