#include "T4TApp.h"
#include "Buggy.h"
#include "MyNode.h"

Buggy::Buggy() : Project::Project("buggy") {

	app->addItem("axle1", 2, "general", "axle");
	app->addItem("wheel1", 2, "general", "wheel");
	app->addItem("wheel2", 2, "general", "wheel");

	_body = addElement(new Body(this));
	_frontAxle = addElement(new Axle(this, _body, "frontAxle", "Front Axle"));
	_rearAxle = addElement(new Axle(this, _body, "rearAxle", "Rear Axle"));
	_frontWheels = addElement(new Wheels(this, _frontAxle, "frontWheels", "Front Wheels"));
	_rearWheels = addElement(new Wheels(this, _rearAxle, "rearWheels", "Rear Wheels"));
	setupMenu();

	_ramp = MyNode::create("buggyRamp");
	_ramp->loadData("res/common/", false);
	_ramp->setTranslation(0, 2.5f, -7.5f);
	_ramp->setStatic(true);
	_ramp->addPhysics();
	_ramp->setVisible(false);
	_scene->addNode(_ramp);
}

void Buggy::setupMenu() {
	Project::setupMenu();
}

void Buggy::setActive(bool active) {
	Project::setActive(active);
	_ramp->setVisible(false);
}

bool Buggy::setSubMode(short mode) {
	bool changed = Project::setSubMode(mode);
	switch(_subMode) {
		case 0: { //build
			break;
		} case 1: { //test
			_rootNode->enablePhysics(false);
			//Vector3 trans(0, 6, 0);
			//_rootNode->setMyTranslation(trans);
			setRampHeight(1);
			app->getPhysicsController()->setGravity(app->_gravity);
			app->setCameraEye(30, 0, M_PI/12);
			app->_ground->setVisible(true);
			_ramp->setVisible(true);
			break;
		}
	}
}

void Buggy::setRampHeight(float scale) {
	//cout << "scaling ramp to " << scale << endl;
	_rampSlope = scale * 5.0f / 15.0f;
	_ramp->removePhysics();
	_ramp->setScaleY(scale);
	_ramp->setTranslationY(scale * 2.5f);
	_ramp->updateTransform();
	_ramp->addPhysics();
	//position the buggy near the top of the ramp
	_rootNode->updateTransform();
	BoundingBox box = _rootNode->getBoundingBox(true);
	Vector3 normal(0, 1, _rampSlope), trans(0, 0, -15 - box.min.z);
	normal.normalize();
	trans.y = -trans.z * _rampSlope;
	trans -= normal * box.min.y;
	_rootNode->setMyTranslation(trans);
	Quaternion rot(Vector3::unitX(), atan2(scale * 5.0f, 15.0f));
	_rootNode->setMyRotation(rot);
}

void Buggy::launch() {
	_rootNode->enablePhysics(true);
	app->getPhysicsController()->setGravity(app->_gravity);
	_rootNode->setActivation(DISABLE_DEACTIVATION);
}

void Buggy::controlEvent(Control *control, EventType evt) {
	Project::controlEvent(control, evt);
	const char *id = control->getId();
}

bool Buggy::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex) {
	Project::touchEvent(evt, x, y, contactIndex);
	if(_subMode == 1 && !_launching) {
		if(isTouching() && getTouchNode() == _ramp) {
			float scale = _ramp->_baseScale.y * (1.0f - _touchPt.deltaPix().y / 400.0f);
			scale = fmin(4.0f, fmax(0.2f, scale));
			setRampHeight(scale);
		}
	}
	return true;
}

Buggy::Body::Body(Project *project) : Project::Element(project, NULL, "body", "Body") {
	_filter = "body";
}

Buggy::Axle::Axle(Project *project, Element *parent, const char *id, const char *name)
  : Project::Element(project, parent, id, name) {
	_filter = "axle";
}

void Buggy::Axle::placeNode(short n) {
	Vector3 point = _project->getTouchPoint(_project->getLastTouchEvent());
	//z-axis of the cylinder should be along the buggy's x-axis
	Quaternion rot(Vector3::unitY(), M_PI/2);
	_nodes[n]->setMyRotation(rot);
	point.x = 0;
	_nodes[n]->setMyTranslation(point);
}

void Buggy::Axle::addPhysics(short n) {
	Element::addPhysics(n);
	app->getPhysicsController()->setConstraintNoCollide();
	MyNode *node = getNode(), *parent = _parent->getNode();
	app->addConstraint(parent, node, -1, "fixed", node->getTranslationWorld(), Vector3::unitX(), true);
}

Buggy::Wheels::Wheels(Project *project, Element *parent, const char *id, const char *name)
  : Project::Element(project, parent, id, name) {
	_numNodes = 2;
	_filter = "wheel";
}

void Buggy::Wheels::placeNode(short n) {
	Vector3 point = _project->getTouchPoint(_project->getLastTouchEvent());
	//position determines how far we are along the axle
	float dir = n == 0 ? 1 : -1;
	Vector3 parent = _parent->getNode()->getTranslationWorld();
	Quaternion rot(Vector3::unitY(), M_PI/2);
	_nodes[n]->setMyRotation(rot);
	Vector3 trans(dir * point.x, parent.y, parent.z);
	_nodes[n]->setMyTranslation(trans);
}

void Buggy::Wheels::addPhysics(short n) {
	Element::addPhysics(n);
	MyNode *node = getNode(n);
	app->getPhysicsController()->setConstraintNoCollide();
	app->addConstraint(_parent->getNode(), node, -1, "hinge", node->getTranslationWorld(), Vector3::unitX(), true);
}
