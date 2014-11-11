#ifndef ROCKET_H_
#define ROCKET_H_

#include "Project.h"

class Rocket : public Project
{
public:
	class Straw : public Project::Element {
		public:
		PhysicsGenericConstraint *_constraint;

		Straw(Project *project, const char *id, const char *name);
		void addPhysics(short n);
		bool touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex);
	};
	
	class Balloon : public Project::Element {
		public:
		std::vector<float> _balloonRadius, _anchorRadius;

		Balloon(Project *project, const char *id, const char *name, Element *parent);
		void placeNode(const Vector3 &position, short n);
		void addPhysics(short n);
	};

	Straw *_straw;
	Balloon *_balloons;
	float _strawRadius, _strawLength, _originalStrawLength;
	bool _launching;

	Rocket();
	void setActive(bool active);
	bool setSubMode(short mode);
	void update();
	void controlEvent(Control *control, EventType evt);
	bool touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex);
};

#endif
