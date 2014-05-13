#include <Scene/Camera.h>
#include "CustomRenderPasses.h"
#include <Utility/DebugLog.h>

namespace SliceMap
{
	class SliceMapRenderPass : public CameraRenderPass
	{
	private:
		Texture* m_bitMask;
		int m_numSliceMaps; 	// number of slice maps ( render targets to be used as slice maps )
	public:
		SliceMapRenderPass(Shader* shader, FramebufferObject* fbo)
		{
			m_shader = shader;
			m_fbo = fbo;
			m_viewport = glm::vec4(0,0,800,600);
			m_numSliceMaps = 1;
			if (fbo)
			{
				m_viewport.z = fbo->getWidth();
				m_viewport.w = fbo->getHeight();
				m_numSliceMaps = fbo->getNumColorAttachments();
			}
			m_camera = 0;
			m_bitMask = 0;

		}

		~SliceMapRenderPass()
		{
		}

		void preRender()
		{
			if (m_bitMask)
			{
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_1D, m_bitMask->getTextureHandle());
				glActiveTexture(GL_TEXTURE0);
			}
			RenderPass::preRender();
		}
		void postRender()
		{
			if (m_bitMask)
			{
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_1D, 0);
				glActiveTexture(GL_TEXTURE0);
			}
			RenderPass::postRender();
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
			m_shader->uploadUniform(5, "uniformBitMask");
			CameraRenderPass::uploadUniforms();

			m_shader->uploadUniform(m_numSliceMaps, "uniformNumSliceMaps");
		}
		void setBitMask(Texture* bitMask)
		{
			m_bitMask = bitMask;
		}
	};

	/**
	 * Compute a 8 bit mask which simply holds information about which depth value corresponds with which RGBA bit combination
	 * @return bit mask
	 */
	Texture* get8BitMask()
	{
		Texture* bitMask = new Texture();
		GLuint bitMaskHandle;
										// z =   0    ,    1     ,     2    ,     3    ,     4     ,     5     ,    6      ,      7, ...
		unsigned char bitMaskData[32][4] = { {1,0,0,0}, {2,0,0,0}, {4,0,0,0}, {8,0,0,0}, {16,0,0,0}, {32,0,0,0}, {64,0,0,0}, {128,0,0,0},
										     {0,1,0,0}, {0,2,0,0}, {0,4,0,0}, {0,8,0,0}, {0,16,0,0}, {0,32,0,0}, {0,64,0,0}, {0,128,0,0},
										     {0,0,1,0}, {0,0,2,0}, {0,0,4,0}, {0,0,8,0}, {0,0,16,0}, {0,0,32,0}, {0,0,64,0}, {0,0,128,0},
										     {0,0,0,1}, {0,2,0,0}, {0,0,0,4}, {0,0,0,8}, {0,0,0,16}, {0,0,0,32}, {0,0,0,64}, {0,0,0,128},
		};

		glGenTextures(1, &bitMaskHandle);
		glBindTexture(GL_TEXTURE_1D, bitMaskHandle);
		glTexImage1D( GL_TEXTURE_1D, 0, GL_RGBA, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, &bitMaskData);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_1D, 0);

		bitMask->setTextureHandle(bitMaskHandle);
		return bitMask;
	}

	enum ShaderType{ BITMASK_MULTIPLETARGETS, BITMASK_SINGLETARGET, COMPUTATION };

	/**
	 *
	 * @param width grid width
	 * @param height grid height
	 * @param depth grid depth
	 * @param resX grid resolution in X dimension
	 * @param resY grid resolution in Y dimension
	 * @param numSliceMaps grid resolution in Z dimension ( interpreted as factor of 32 )
	 * @return a slice map render pass consisting of a framebuffer object of given dimensions, voxelizing the given grid volume
	 */
	SliceMapRenderPass* getSliceMapRenderPass(float width, float height, float depth, int resX, int resY, int numSliceMaps = 3, ShaderType shaderType = BITMASK_MULTIPLETARGETS)
	{
		/*Init shader & FBO*/
		DEBUGLOG->log("Creating Shader to construct Slice Map");

		std::string vertexShader = std::string ( SHADERS_PATH "/slicemap/simpleVertex.vert" );
		std::string fragmentShader;

		switch (shaderType)
		{
		case BITMASK_SINGLETARGET:
			fragmentShader = std::string ( SHADERS_PATH "/slicemap/sliceMap.frag" );
			break;
		case BITMASK_MULTIPLETARGETS:
			fragmentShader = std::string ( SHADERS_PATH "/slicemap/sliceMapMultipleTargets.frag" );
			break;
		case COMPUTATION:
			fragmentShader = std::string ( SHADERS_PATH "/slicemap/sliceMapWithComputation.frag" );
			break;
		}

		Shader* sliceMapShader = new Shader( vertexShader, fragmentShader);

		DEBUGLOG->log("Creating Framebuffer with render target amount:", numSliceMaps);
		FramebufferObject* fbo  = new FramebufferObject(resX,resY);
		fbo->addColorAttachments(numSliceMaps);	// to enable slice mapping into render targets

		/*Init Renderpass*/
		SliceMapRenderPass* sliceMapRenderPass = new SliceMapRenderPass(sliceMapShader, fbo);
		sliceMapRenderPass->setViewport(0,0,resX,resY);

		sliceMapRenderPass->setClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// clear every channel to 0
		sliceMapRenderPass->addClearBit(GL_COLOR_BUFFER_BIT);		// enable clearing of color bits
		sliceMapRenderPass->addClearBit(GL_DEPTH_BUFFER_BIT);		// enable clearing of depth bits
		sliceMapRenderPass->addDisable(GL_DEPTH_TEST);				// disable depth testing to prevent fragments from being discarded
		sliceMapRenderPass->addEnable(GL_COLOR_LOGIC_OP);			// enable logic operations to be able to use OR operations

		/*init bit mask*/
		Texture* bitMask = get8BitMask();
		sliceMapRenderPass->setBitMask(bitMask);

		/*Init Camera*/
		Camera* orthocam = new Camera();
		glm::mat4 ortho = glm::ortho( - width * 0.5f , width * 0.5f , - height * 0.5f , height * 0.5f , 0.0f, depth);
//		glm::mat4 persp = glm::perspective(45.0f, 1.0f, 0.1f, 100.f);
		orthocam->setProjectionMatrix(ortho);
		sliceMapRenderPass->setCamera(orthocam);

		return sliceMapRenderPass;
	}
}
