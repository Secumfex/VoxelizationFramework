#ifndef CUSTOMRENDERPASSES_H
#define CUSTOMRENDERPASSES_H

#include <Rendering/RenderPass.h>
#include <Scene/Camera.h>

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

#endif
