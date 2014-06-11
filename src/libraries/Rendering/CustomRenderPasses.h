#ifndef CUSTOMRENDERPASSES_H
#define CUSTOMRENDERPASSES_H

#include <Rendering/RenderPass.h>
#include <Scene/Camera.h>
#include <Resources/Texture.h>

/// a Renderpass from a Camera object, uploading a view and perspective matrix
/// aswell as a bunch of other Camera uniforms : position, direction
class CameraRenderPass : public RenderPass
{
protected:
	Camera* m_camera;
public:
	CameraRenderPass();
	CameraRenderPass(Shader* shader, FramebufferObject* fbo = 0);
	virtual ~CameraRenderPass();

	void setCamera(Camera* camera);

	Camera* getCamera();

	virtual void uploadUniforms();
};

/// a Renderpass of a screen filling triangle, diabling GL_DEPTH_TEST on default
/// also possible to add textures to upload to uniforms with automatic units
class TriangleRenderPass : public RenderPass
{
protected:
	Renderable* m_screenFillingTriangle;
	std::vector< std::pair< Texture*, std::string > > m_uniformTextures;
public:
	TriangleRenderPass(Shader* shader, FramebufferObject* fbo, Renderable* screenFillingTriangle);
	void addUniformTexture(Texture* texture, std::string uniformTarget);
	virtual void uploadUniforms();
};

#endif
