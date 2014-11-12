#ifndef SATELLITE_H_
#define SATELLITE_H_

#include "Project.h"

class Buggy : public Project {
public:
	class Body : public Project::Element {
		public:
		Body(Project *project);
	};

	class Instrument : public Project::Element {
		public:
		float _mass;
		bool _folding;

		Instrument(Project *project, Element *parent)
		bool touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex);
	};
	
	Button *_launchButton;
	float _maxRadius, _maxLength;

	Satellite();
	void setupMenu();
	void setActive(bool active);
	bool setSubMode(short mode);
	void controlEvent(Control *control, EventType evt);
	bool touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex);
};

#endif