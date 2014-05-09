#include <Rendering/RenderPass.h>
#include <Scene/Camera.h>

class TestRenderPass : public RenderPass
{
protected:
	Camera* m_camera;
public:
	TestRenderPass(Shader* shader, FramebufferObject* fbo);
	~TestRenderPass();

	void setCamera(Camera* camera);

	Camera* getCamera();

	void preRender();

	void uploadUniforms();

};
