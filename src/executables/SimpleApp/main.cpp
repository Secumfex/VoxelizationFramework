#include <Application/Application.h>

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include <Rendering/Shader.h>
#include <Rendering/FramebufferObject.h>
#include <Scene/Camera.h>
#include <Scene/RenderableNode.h>
#include <Utility/Updatable.h>

class RotatingNode : public Updatable, public RenderableNode
{
	private:
	glm::vec3 m_rotationAxis;
	float m_angle;

	public:
	RotatingNode(Node* parent = 0)
	{
		setParent(parent);
	}
	void update(float d_t = 0.1f)
	{
		rotate( glm::rotate (glm::mat4(1.0f), m_angle, m_rotationAxis ) );
	}
	void setRotationAxis(glm::vec3 rotationAxis)
	{
		m_rotationAxis = rotationAxis;
	}
	void setAngle(float angle)
	{
		m_angle = angle;
	}

};

class TestRenderPass : public RenderPass
{
protected:
	Camera* m_camera;
public:
	TestRenderPass(Shader* shader, FramebufferObject* fbo)
	{
		m_shader = shader;
		m_fbo = fbo;
		m_viewport = glm::vec4(0,0,800,600);

		if (fbo)
		{
			m_viewport.z = fbo->getWidth();
			m_viewport.w = fbo->getHeight();
		}
		m_camera = new Camera();
		m_camera->setPosition(glm::vec3(1.0f,3.0f,7.0f));
		m_camera->setCenter(glm::vec3(0.0f,2.0f,0.0f));
		m_camera->setProjectionMatrix(glm::perspective(60.0f, 4.0f / 3.0f, 0.1f, 100.0f));
	}

	void preRender()
	{
		glClearColor(0.1f,0.1f,0.1f,1.0f);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void uploadUniforms()
	{
		m_shader->uploadUniform(m_camera->getViewMatrix(),"uniformView");
		m_shader->uploadUniform(m_camera->getProjectionMatrix(), "uniformProjection");
	}
};

class ObjectLoadingApp : public Application
{
	void postInitialize()
	{
		DEBUGLOG->log("Loading some objects");
		DEBUGLOG->indent();
		
			DEBUGLOG->log("Loading cube dae file");
			DEBUGLOG->indent();
				std::vector< Object* > daeCube = m_resourceManager.loadObjectsFromFile(RESOURCES_PATH "/cube.dae");
				DEBUGLOG->log("Loading custom Texture for dae Cube");
				daeCube[0]->getMaterial()->setTexture("diffuseTexture", m_resourceManager.loadTexture("cvlogo.png", RESOURCES_PATH "/"));
			DEBUGLOG->outdent();

			DEBUGLOG->log("Loading cube obj file");
			DEBUGLOG->indent();
				std::vector< Object* > objCube =  m_resourceManager.loadObjectsFromFile( RESOURCES_PATH "/cube.obj" );
			DEBUGLOG->outdent();

			DEBUGLOG->log("Loading background dae file");
			DEBUGLOG->indent();
				std::vector< Object* > daeBackground=  m_resourceManager.loadObjectsFromFile( RESOURCES_PATH "/background.dae" );
			DEBUGLOG->outdent();

		DEBUGLOG->log("Loading some objects complete");
		DEBUGLOG->outdent();

		DEBUGLOG->log("Adding objects to a scene instance");
		DEBUGLOG->indent();
			Scene* scene = new Scene();
			scene->addObjects( daeCube );
			scene->addObjects( objCube );
			scene->addObjects( daeBackground );

			DEBUGLOG->log("Creating scene graph nodes");
			DEBUGLOG->indent();

				DEBUGLOG->log("Creating renderable node for cube 1");
				RenderableNode* cubeNode1 = new RenderableNode( scene->getSceneGraph()->getRootNode( ) );
				cubeNode1->rotate( glm::rotate (glm::mat4(1.0f), 60.0f, glm::vec3(0.0f,1.0f,0.0f) ) );
				cubeNode1->translate( glm::translate( glm::mat4(1.0f), glm::vec3(1.2f, 1.0f,-1.0f) ) );
				cubeNode1->setObject(daeCube[0]);

				DEBUGLOG->log("Creating node tree for cube 2");
				Node* positionNode = new Node(scene->getSceneGraph()->getRootNode());
				positionNode->translate( glm::translate(glm::mat4(1.0f),  glm::vec3(0.0f, 3.0f, 1.0f) ) );

				RotatingNode* yAxisRotationNode = new RotatingNode(positionNode);
				yAxisRotationNode->setAngle(0.005f);
				yAxisRotationNode->setRotationAxis( glm::vec3( 0.0f, 1.0f, 0.0f ) );

				RotatingNode* rotatingCubeNode = new RotatingNode(yAxisRotationNode);
				rotatingCubeNode->setRotationAxis(glm::vec3 ( 1.0f, 1.0f, 0.1f));
				rotatingCubeNode->setAngle(0.01f);
				rotatingCubeNode->setObject(objCube[0]);

				DEBUGLOG->log("Adding updatable rotation nodes of cube 2 to scene");
				scene->addUpdatable(yAxisRotationNode);
				scene->addUpdatable(rotatingCubeNode);

				DEBUGLOG->log("Creating renderable node for background");
				RenderableNode* backgroundNode = new RenderableNode(scene->getSceneGraph()->getRootNode());
				backgroundNode->setObject(daeBackground[0]);

			DEBUGLOG->outdent();

		DEBUGLOG->outdent();
		
		DEBUGLOG->log("Setting scene instance as active scene ");
		m_sceneManager.setActiveScene(scene);	

		DEBUGLOG->log("Configuring Rendering");
		DEBUGLOG->indent();
			DEBUGLOG->log("Compiling simple Shader");
			Shader* shader = new Shader(SHADERS_PATH "/myShader/phong.vert", SHADERS_PATH "/myShader/phong.frag");

			DEBUGLOG->log("Creating FramebufferObject");
			FramebufferObject* fbo = new FramebufferObject(800,600);
			fbo->addColorAttachments(1);

			DEBUGLOG->log("Creating Renderpass");
			TestRenderPass* renderPass = new TestRenderPass(shader, 0);
			renderPass->setViewport(0,0,800,600);

			DEBUGLOG->log("Adding Objects to Renderpass");
			renderPass->addRenderable(cubeNode1);
			renderPass->addRenderable(rotatingCubeNode);
			renderPass->addRenderable(backgroundNode);

			DEBUGLOG->log("Adding Renderpass to Application");
			m_renderManager.addRenderPass(renderPass);
		DEBUGLOG->outdent();
	}
};

int main(){

	ObjectLoadingApp myApp;

	myApp.configure();
	myApp.initialize();
	myApp.run();

	
	return 0;
}
