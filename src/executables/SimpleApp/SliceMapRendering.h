#include <Scene/Camera.h>
#include "CustomRenderPasses.h"
#include <Utility/DebugLog.h>

namespace SliceMap
{
	class SliceMapRenderPass : public CameraRenderPass
	{
	private:
		float m_nearFarDistance;
	public:
		SliceMapRenderPass(Shader* shader, FramebufferObject* fbo)
		{
			m_shader = shader;
			m_fbo = fbo;
			m_viewport = glm::vec4(0,0,800,600);
			if (fbo)
			{
				m_viewport.z = fbo->getWidth();
				m_viewport.w = fbo->getHeight();
			}
			m_camera = 0;
			m_nearFarDistance = -1.0f;
		}
		void setNearFarDistance( float nearFarDistance )
		{
			m_nearFarDistance = nearFarDistance;
		}
		~SliceMapRenderPass()
		{

		}

		void uploadUniforms()
		{
			if (m_nearFarDistance != -1.0f)
			{
				m_shader->uploadUniform(m_nearFarDistance, "uniformNearFarDistance");
			}
			CameraRenderPass::uploadUniforms();
		}
	};

	RenderPass* getSliceMapRenderPass()
	{
		/*Init shader & FBO*/
		DEBUGLOG->log("Creating Shader to construct Slice Map");
		Shader* sliceMapShader = new Shader(SHADERS_PATH "/slicemap/simpleVertex.vert", SHADERS_PATH "/slicemap/sliceMap.frag");

		DEBUGLOG->log("Creating Framebuffer with 3 Render Targets");
		FramebufferObject* fbo  = new FramebufferObject(512,512);
		fbo->addColorAttachments(3);	// to enable slice mapping into render targets

		/*Init Renderpass*/
		SliceMapRenderPass* sliceMapRenderPass = new SliceMapRenderPass(sliceMapShader, fbo);
		sliceMapRenderPass->setViewport(0,0,512,512);

		sliceMapRenderPass->setClearColor(0.0f, 0.0f, 0.0f, 1.0f);	// clear every channel to 0
		sliceMapRenderPass->addClearBit(GL_COLOR_BUFFER_BIT);		// enable clearing of color bits
		sliceMapRenderPass->addDisable(GL_DEPTH_TEST);				// disable depth testing to prevent fragments from being discarded

		/*Init Camera*/
		Camera* orthocam = new Camera();

		glm::mat4 ortho = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, -5.0f, 5.0f);
		sliceMapRenderPass->setNearFarDistance(10.0f);

		orthocam->setProjectionMatrix(ortho);
		sliceMapRenderPass->setCamera(orthocam);



		return sliceMapRenderPass;
	}
}
