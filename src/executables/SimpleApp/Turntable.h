#ifndef TURNTABLE_H
#define TURNTABLE_H

#include <Utility/Updatable.h>
#include <Scene/Node.h>
#include <Input/InputManager.h>

class Turntable : public Updatable
{
protected:
	Node* p_node;	// target node to be turned
	InputManager* p_inputManager; // inputManager to read mouse movement from
	bool m_dragActive; 	// boolean whether dragging is active
	float m_sensitivity;
public:
	Turntable(Node* node, InputManager* inputManager);
	virtual ~Turntable();

	void update(float d_t);
	void setDragActive(bool drag);
	bool getDragActive();

	void setSensitivity(float sensitivity);
	void setNode(Node* node);
	void setInputManager(InputManager* inputManager);

	void dragBy(float phi, float theta);

	class ToggleTurntableDragListener : public Listener
	{
	private:
		Turntable* p_turntable;
	public:
		ToggleTurntableDragListener(Turntable* turntable);
		void call();
	};
};


#endif
