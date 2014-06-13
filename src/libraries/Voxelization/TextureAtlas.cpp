#include <Voxelization/TextureAtlas.h>

#include <Utility/DebugLog.h>

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

void TexAtlas::TextureAtlasRenderPass::configureRenderPass() {
	// make sure renderpass uses the provided Camera
	// make sure renderpass clears with alpha 0.0
	// make sure renderpass only renders the provided node

//	setCamera( p_camera );					// set camera ( TODO : is this even necessary ? )
	setClearColor( 0.0f, 0.0f ,0.0f, 0.0f);	// clear with alpha = 0
	addClearBit( GL_COLOR_BUFFER_BIT);		// clear on every frame

	addRenderable( p_renderableNode );
}

void TexAtlas::TextureAtlasRenderPass::configureFramebufferObject() {
	// make sure there is a render target texture
	m_fbo->addColorAttachments(1);
}

TexAtlas::TextureAtlasRenderPass::TextureAtlasRenderPass(
		RenderableNode* renderableNode, int width, int height, Camera* camera)
  : RenderPass(getWriteWorldPositionTextureAtlasShader(), new FramebufferObject(width,height))
{
	p_renderableNode = renderableNode;

	//	p_camera = camera;
//
//	if (camera == 0)
//	{
//		p_camera = new Camera();
//		configureCamera();
//	}

//	m_fbo = new FramebufferObject(width, height);
	configureFramebufferObject();

	configureRenderPass();

	m_textureAtlas = new TextureAtlas();
	configureTextureAtlas();
}

TexAtlas::TextureAtlasRenderPass::~TextureAtlasRenderPass() {
	m_fbo->~FramebufferObject();
}

//void TexAtlas::TextureAtlasRenderPass::configureCamera() {
//	// TODO : retrieve object bbox and create corresponding frustum for camera
//}

void TexAtlas::TextureAtlasRenderPass::configureTextureAtlas() {
	m_textureAtlas->setObject( p_renderableNode->getObject() );
	m_textureAtlas->setTextureHandle( m_fbo->getColorAttachmentTextureHandle( GL_COLOR_ATTACHMENT0 ) );
}

 FramebufferObject* TexAtlas::TextureAtlasRenderPass::getFbo()  {
	return m_fbo;
}

void TexAtlas::TextureAtlasRenderPass::setFbo( FramebufferObject* fbo) {
	m_fbo = fbo;
}

TexAtlas::TextureAtlas* TexAtlas::TextureAtlasRenderPass::getTextureAtlas() {
	return m_textureAtlas;
}

void TexAtlas::TextureAtlasRenderPass::setTextureAtlas( TextureAtlas* textureAtlas) {
	m_textureAtlas = textureAtlas;
}

void TexAtlas::TextureAtlasRenderPass::setRenderableNode(
		 RenderableNode* renderableNode) {
	p_renderableNode = renderableNode;
}

RenderableNode* TexAtlas::TextureAtlasRenderPass::getRenderableNode() {
	return p_renderableNode;
}

TexAtlas::TextureAtlas::TextureAtlas(Object* object, GLuint textureAtlasHandle) {
	p_object = object;
	setTextureHandle( textureAtlasHandle );
}

TexAtlas::TextureAtlas::~TextureAtlas() {
}

void TexAtlas::TextureAtlas::setObject(Object* object) {
	p_object = object;
}

Object* TexAtlas::TextureAtlas::getObject() {
	return p_object;
}

TexAtlas::TextureAtlasVertexGenerator::TextureAtlasVertexGenerator(
		TextureAtlas* textureAtlas) {
	p_textureAtlas = textureAtlas;
	m_pixelsObject = 0;
}

TexAtlas::TextureAtlasVertexGenerator::~TextureAtlasVertexGenerator() {
}

std::vector<glm::vec3>& TexAtlas::TextureAtlasVertexGenerator::getVertexPositions() {
	return m_vertexPositions;
}

void TexAtlas::TextureAtlasVertexGenerator::generateVertexPositions() {

	// retrieve width and height
	GLint width = 0;
	GLint height = 0;

	p_textureAtlas->bindToTextureUnit( 0 );
	glActiveTexture( p_textureAtlas->getActiveUnit() );

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0,GL_TEXTURE_HEIGHT, &height);

	// retrieve the image and save as an array
	GLubyte* pixels = new GLubyte[ width * height * 4];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	DEBUGLOG->log("TEXTURE ATLAS VERTEX GENERATOR : retrieved width : ", width );
	DEBUGLOG->log("TEXTURE ATLAS VERTEX GENERATOR : retrieved height: ", height );

	// alternative : retrieve the image as an PBO
	// create a PBO with GL_STREAM_READ
	// Bind it for pack
	// glMapBuffer to access

	// iterate over texture
	for ( int y = 0; y < height; y++ )
	{
		for( int x = 0; x < width; x++)
		{
			// valid texture atlas pixel : alpha is != 0.0f
			int pixelIndex = ( ( (y *  width) * 4 ) + (x * 4) );
			if ( pixels[ pixelIndex + 3 ] != 0.0f )
			{
				m_vertexPositions.push_back( glm::vec3( x / ( (float) width ), y / ( (float) height ), 0.0f) );	// push back corresponding pixel coordinates
			}
		}
	}
}

void TexAtlas::TextureAtlasVertexGenerator::generateVertexArrayObject()
{
	Model *pixels = new Model;
	Material *mat = new Material();

	mat->setTexture("diffuseTexture", p_textureAtlas);

	GLuint vertexArrayHandle;

	glGenVertexArrays(1, &vertexArrayHandle);
	glBindVertexArray(vertexArrayHandle);

	std::vector <GLint> indices;
	std::vector <GLfloat> vertices;
	std::vector <GLfloat> uvCoords;

	 // buffer vertices
	for( unsigned int i = 0; i < m_vertexPositions.size(); i++)
	{
		for (unsigned int c = 0; c < 3; c++)
		{
			vertices.push_back( m_vertexPositions[i][c] );
			if ( c < 2 )
			{
				uvCoords.push_back( m_vertexPositions[i][c] );
			}
			indices.push_back(i);
		}
	}

	GLuint indexBufferHandle;
	glGenBuffers(1, &indexBufferHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferHandle);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * indices.size(), &indices[0], GL_STATIC_DRAW);

	// buffer vertices
	GLuint vertexBufferHandle;
	glGenBuffers(1, &vertexBufferHandle);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferHandle);

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// buffer uv coords
	GLuint uvBufferHandle;
	glGenBuffers(1, &uvBufferHandle);
	glBindBuffer(GL_ARRAY_BUFFER, uvBufferHandle);

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * uvCoords.size(), &uvCoords[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	pixels->setVAOHandle(vertexArrayHandle);
	pixels->setNumIndices( indices.size() );
	pixels->setNumVertices( vertices.size() );
	pixels->setNumFaces(0);

	m_pixelsObject = new Object(pixels,mat);
	m_pixelsObject->setRenderMode( GL_POINTS );
}

void TexAtlas::TextureAtlasVertexGenerator::call()
{
	if ( m_vertexPositions.empty() )
	{
		DEBUGLOG->log("TEXTURE ATLAS VERTEX GENERATOR : generating Vertex Positions from Texture Atlas");
		DEBUGLOG->indent();
			generateVertexPositions();
			DEBUGLOG->log("TEXTURE ATLAS VERTEX GENERATOR : generated Vertices :", m_vertexPositions.size( ) );
		DEBUGLOG->outdent();

		DEBUGLOG->log("TEXTURE ATLAS VERTEX GENERATOR : generating Vertex Array Object");
		DEBUGLOG->indent();
			generateVertexArrayObject();
		DEBUGLOG->outdent();


		detach();
		DEBUGLOG->log("TEXTURE ATLAS VERTEX GENERATOR : detached from subject");
	}
}

Object* TexAtlas::TextureAtlasVertexGenerator::getPixelsObject() {
	return m_pixelsObject;
}

void TexAtlas::TextureAtlasVertexGenerator::setPixelsObject(Object* pixelsObject) {
	m_pixelsObject = pixelsObject;
}
