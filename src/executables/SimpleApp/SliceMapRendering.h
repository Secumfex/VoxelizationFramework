#include <Scene/Camera.h>
#include <Rendering/RenderPass.h>
#include <Utility/DebugLog.h>

namespace SliceMap
{
	RenderPass* getSliceMapRenderPass()
	{
		/*Init shader & FBO*/
		DEBUGLOG->log("Creating Shader to construct Slice Map");
		Shader* sliceMapShader = new Shader(SHADERS_PATH "/slicemap/simpleVertex.vert", SHADERS_PATH "/slicemap/sliceMap.frag");
		DEBUGLOG->log("Creating Framebuffer with 3 Render Targets")
				FramebufferObject* fbo  = new FramebufferObject(512,512);
		fbo->addColorAttachments(3);	// to enable flace

		/*Init Renderpass*/
		TestRenderPass* sliceMapRenderPass = new TestRenderPass(sliceMapShader, fbo);
		sliceMapRenderPass->setViewport(0,0,512,512);

		/*Init Camera*/
		Camera* orthocam = new Camera();
		glm::mat4 ortho = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, -5.0f, 5.0f);
		orthocam->setProjectionMatrix(ortho);
		sliceMapRenderPass->setCamera(orthocam);


		return sliceMapRenderPass;
	}
}
