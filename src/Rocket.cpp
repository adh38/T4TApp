#include "T4TApp.h"
#include "Rocket.h"
#include "MyNode.h"

Rocket::Rocket() : Project::Project("rocket") {

	app->_rocket = _rootNode;

	app->addItem("straw", 2, "general", "straw");
	app->addItem("balloon_sphere", 2, "general", "balloon");
	app->addItem("balloon_long", 2, "general", "balloon");

	_straw = (Straw*) addElement(new Straw(this, "straw", "Straw"));
	_balloons = (Balloon*) addElement(new Balloon(this, "balloons", "Balloon", _straw));
	setupMenu();
	app->addListener(_controls, this);
}

void Rocket::setActive(bool active) {
	Project::setActive(active);
}

bool Rocket::setSubMode(short mode) {
	bool changed = Project::setSubMode(mode);
	switch(mode) {
		case 0: { //build
			break;
		} case 1: { //test
			//move the straw back to the starting point
			Vector3 start(0, 0, -5);
			app->setCameraEye(30, 0, M_PI/12);
			app->getPhysicsController()->setGravity(app->_gravity);
			_straw->_constraint->setEnabled(false);
			_rootNode->enablePhysics(false);
			_rootNode->setMyTranslation(start);
			_rootNode->enablePhysics(true);
			_straw->_constraint->setEnabled(true);
			_launching = true;
			//dangle the lunar buggy from the center of the straw with a socket constraint
			MyNode *buggy = app->getProjectNode("buggy");
			if(buggy && buggy->getScene() != _scene && buggy->getChildCount() > 0) {
				MyNode *straw = _straw->getNode();
				buggy->enablePhysics(false);
				buggy->placeRest();
				buggy->updateTransform();
				straw->updateTransform();
				BoundingBox strawBox = straw->getBoundingBox(true);
				BoundingBox buggyBox = buggy->getBoundingBox(true);
				Vector3 trans = straw->getTranslationWorld();
				trans.y += strawBox.min.y - buggyBox.max.y - 1.5f;
				buggy->setMyTranslation(trans);
				buggy->enablePhysics(true);
				MyNode *body = dynamic_cast<MyNode*>(buggy->getFirstChild());
				Vector3 joint = trans, dir = Vector3::unitY();
				joint.y += buggyBox.max.y;
				for(short i = 0; i < 2; i++) {
					Vector3 springJoint(joint.x, joint.y, joint.z + (2*i-1) * _strawLength/3);
					PhysicsSpringConstraint *constraint = 
						(PhysicsSpringConstraint*) app->addConstraint(straw, body, -1, "spring", springJoint, dir, true);
					constraint->setLinearStrengthZ(0.1f);
				}
			}
			break;
		}
	}
	return changed;
}

void Rocket::update() {
	if(!_launching) return;
	//deflate each balloon by a fixed percentage
	_launching = false;
	short n = _balloons->_nodes.size();
	MyNode *straw = _straw->getNode();
	for(short i = 0; i < n; i++) {
		MyNode *balloon = _balloons->_nodes[i].get(), *anchor = dynamic_cast<MyNode*>(balloon->getParent());
		float scale = balloon->getScaleX();
		if(scale > 0.2f) {
			_launching = true;
			scale *= 0.985f;
			balloon->setScale(scale);
			//adjust it so it is still tangent to the straw
			Vector3 trans = anchor->getTranslationWorld() - straw->getTranslationWorld(), strawAxis;
			straw->getWorldMatrix().transformVector(Vector3::unitZ(), &strawAxis);
			strawAxis.normalize();
			trans -= strawAxis * trans.dot(strawAxis);
			trans = trans.normalize() * (scale * _balloons->_balloonRadius[i] - _balloons->_anchorRadius[i]);
			balloon->setTranslation(trans);	
			//apply the air pressure force
			anchor->getCollisionObject()->asRigidBody()->applyForce(Vector3(0, 0, 40) * scale);
		}
	}
}

void Rocket::controlEvent(Control *control, EventType evt) {
	Project::controlEvent(control, evt);
	const char *id = control->getId();
}

bool Rocket::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex) {
	Project::touchEvent(evt, x, y, contactIndex);
	switch(evt) {
		case Touch::TOUCH_PRESS: {
			break;
		} case Touch::TOUCH_MOVE: {
			break;
		} case Touch::TOUCH_RELEASE: {
			break;
		}
	}
	return true;
}

Rocket::Straw::Straw(Project *project, const char *id, const char *name)
  : Project::Element::Element(project, id, name) {
  	_filter = "straw";
}
  
void Rocket::Straw::addPhysics(short n) {

	BoundingBox box = getNode()->getModel()->getMesh()->getBoundingBox();
	Rocket *rocket = (Rocket*)_project;
	rocket->_strawLength = box.max.z - box.min.z;
	rocket->_originalStrawLength = rocket->_strawLength;
	rocket->_strawRadius = (box.max.x - box.min.x) / 2;

	Quaternion rot = Quaternion::identity();
	float angle = atan2(rocket->_strawRadius * 2, rocket->_strawLength);
	Vector3 trans = Vector3::zero(), linearLow(0, 0, -5), linearHigh(0, 0, 5), angularLow(0, -2*M_PI, -2*M_PI),
	  angularHigh(angle, 2*M_PI, 2*M_PI);
	MyNode *node = getNode();
	node->addPhysics();
	_constraint = app->getPhysicsController()->createGenericConstraint(node->getCollisionObject()->asRigidBody(), rot, trans);
	_constraint->setLinearLowerLimit(linearLow);
	_constraint->setLinearUpperLimit(linearHigh);
	_constraint->setAngularLowerLimit(angularLow);
	_constraint->setAngularUpperLimit(angularHigh);
}

bool Rocket::Straw::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex) {
	Project::Element::touchEvent(evt, x, y, contactIndex);
	switch(evt) {
		case Touch::TOUCH_PRESS: {
			Rocket *rocket = (Rocket*)_project;
			MyNode *node = getNode();
			if(rocket->_touchNode == node) {
				Vector3 trans = rocket->_touchPoint - node->getTranslationWorld();
				Matrix straw = node->getWorldMatrix(), strawInv;
				straw.invert(&strawInv);
				strawInv.transformVector(&trans);
				float scale = ((rocket->_strawLength/2 - trans.z) / rocket->_strawLength) * node->getScaleZ();
				node->setScaleZ(scale);
				rocket->_strawLength = rocket->_originalStrawLength * scale;
			}
		}
	}
}

Rocket::Balloon::Balloon(Project *project, const char *id, const char *name, Element *parent)
  : Project::Element::Element(project, id, name, parent) {
  	_filter = "balloon";
}
  
void Rocket::Balloon::placeNode(const Vector3 &position, short n) {
	//get the unit direction vector 
	Rocket *rocket = (Rocket*)_project;
	MyNode *straw = rocket->_straw->getNode();
	Vector3 trans = position - straw->getTranslationWorld(), strawAxis;
	straw->getWorldMatrix().transformVector(Vector3::unitZ(), &strawAxis);
	strawAxis.normalize();
	trans -= strawAxis * trans.dot(strawAxis);
	trans.normalize();
	//constrain the balloon so it is fixed to the straw
	MyNode *balloon = _nodes.back().get(), *anchor = MyNode::create(MyNode::concat(2, "anchor_", _currentNodeId));
	BoundingBox box = balloon->getModel()->getMesh()->getBoundingBox();
	float balloonRadius = (box.max.x - box.min.x) / 2;
	float anchorRadius = balloonRadius * 0.5f; //best fit to the balloon shape as it deflates?
	if(_balloonRadius.size() <= n) {
		_balloonRadius.resize(n+1);
		_anchorRadius.resize(n+1);
	}
	_balloonRadius[n] = balloonRadius;
	_anchorRadius[n] = anchorRadius;
	anchor->setTranslation(position + trans * anchorRadius);
	anchor->setRotation(straw->getRotation());
	anchor->_objType = "sphere";
	anchor->_radius = anchorRadius;
	anchor->_mass = 0.5f;
	balloon->_objType = "none";
	anchor->addChild(balloon);
	_project->_scene->addNode(anchor);
	balloon->setTranslation(trans * (balloonRadius - anchorRadius));
}

void Rocket::Balloon::addPhysics(short n) {
	short i = 0, numNodes = _nodes.size();
	MyNode *straw = ((Rocket*)_project)->_straw->getNode();
	for(i = 0; i < numNodes; i++) {
		if(n < 0 || i == n) {
			MyNode *balloon = _nodes[i].get(), *anchor = dynamic_cast<MyNode*>(balloon->getParent());
			anchor->addPhysics(false);
			app->addConstraint(straw, anchor, -1, "fixed", Vector3::zero(), Vector3::zero(), true);
		}
	}
}

