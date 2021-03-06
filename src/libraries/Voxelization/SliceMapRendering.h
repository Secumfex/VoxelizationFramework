#ifndef SLICEMAPRENDERING_H
#define SLICEMAPRENDERING_H

#include <Scene/Camera.h>
#include <Rendering/CustomRenderPasses.h>
#include <Utility/DebugLog.h>



namespace SliceMap
{
	class SliceMapRenderPass : public CameraRenderPass
	{
	private:
		Texture* m_bitMask;
		int m_numSliceMaps; 	// number of slice maps ( render targets to be used as slice maps )
	public:
		SliceMapRenderPass(Shader* shader, FramebufferObject* fbo);

		~SliceMapRenderPass();

		void preRender();

		void postRender();

		void enableStates();

		void restoreStates();

		void uploadUniforms();

		void setBitMask(Texture* bitMask);
	};

	/**
	 * Compute a 8 bit mask which simply holds information about which depth value corresponds with which RGBA bit combination
	 * @return bit mask
	 */
	Texture* get8BitRGBAMask();

	/**
	 * Compute a 32 bit mask which simply holds information about which depth value corresponds with which 32bit uint value
	 * @return bit mask
	 */
	Texture* get32BitUintMask();

	/**
	 * Compute a 32 bit mask which simply holds information about which depth value corresponds with which 32bit uint value
	 * which sets all lower bits to 1, primarily used for XORing
	 * @return bit mask
	 */
	Texture* get32BitUintXORMask();

	enum ShaderType{ BITMASK_MULTIPLETARGETS, BITMASK_SINGLETARGET, COMPUTATION };

	/**
	 *
	 * @param width grid width
	 * @param height grid height
	 * @param depth grid depth
	 * @param resX grid resolution in X dimension
	 * @param resY grid resolution in Y dimension
	 * @param numSliceMaps grid resolution in Z dimension ( interpreted as factor of 32 )
	 * @param vertexShader to be used with slice mapping fragment shader
	 * @return a slice map render pass consisting of a framebuffer object of given dimensions, voxelizing the given grid volume
	 */
	SliceMapRenderPass* getSliceMapRenderPass(float width, float height, float depth, int resX, int resY, int numSliceMaps = 3, ShaderType shaderType = BITMASK_MULTIPLETARGETS, std::string vertexShader = std::string ( SHADERS_PATH "/slicemap/simpleVertex.vert" ));


	SliceMapRenderPass* getSliceMapRenderPassR32UI(FramebufferObject* fbo, glm::mat4 perspective, ShaderType shaderType, std::string vertexShader = std::string ( SHADERS_PATH "/slicemap/simpleVertex.vert" ));
}

#endif
