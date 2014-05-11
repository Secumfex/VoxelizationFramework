#include <Scene/Camera.h>
#include "CustomRenderPasses.h"
#include <Utility/DebugLog.h>

namespace SliceMap
{
	class SliceMapRenderPass : public CameraRenderPass
	{
	private:
		float m_zNear;
		float m_zFar;
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
		}
		void setZNear( float zNear )
		{
			m_zNear= zNear;
		}
		void setZFar(float zFar)
		{
			m_zFar = zFar;
		}

		~SliceMapRenderPass()
		{

		}

		void enableStates()
		{
			// set logical operation to OR
			glLogicOp(GL_OR);
			RenderPass::enableStates();
		}

		void restoreStates()
		{
			// restore default logical operation
			glLogicOp(GL_COPY);
			RenderPass::restoreStates();
		}

		void uploadUniforms()
		{
			m_shader->uploadUniform(m_zNear, "uniformZNear");
			m_shader->uploadUniform(m_zFar, "uniformZFar");
			CameraRenderPass::uploadUniforms();
		}
	};

	SliceMapRenderPass* getSliceMapRenderPass()
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

		sliceMapRenderPass->setClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// clear every channel to 0
		sliceMapRenderPass->addClearBit(GL_COLOR_BUFFER_BIT);		// enable clearing of color bits
		sliceMapRenderPass->addClearBit(GL_DEPTH_BUFFER_BIT);		// enable clearing of depth bits
		sliceMapRenderPass->addDisable(GL_DEPTH_TEST);				// disable depth testing to prevent fragments from being discarded
		sliceMapRenderPass->addEnable(GL_COLOR_LOGIC_OP);			// enable logic operations to be able to use OR operations

		/*Init Camera*/
		Camera* orthocam = new Camera();
		glm::mat4 ortho = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.0f, 10.0f);
		orthocam->setProjectionMatrix(ortho);

		sliceMapRenderPass->setCamera(orthocam);
		sliceMapRenderPass->setZNear(0.0f);
		sliceMapRenderPass->setZFar(10.0f);

		orthocam->setPosition(2.5f, 2.5f, 2.5f);
		return sliceMapRenderPass;
	}
}
