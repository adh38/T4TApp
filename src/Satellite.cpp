#include "T4TApp.h"
#include "Satellite.h"
#include "MyNode.h"

Satellite::Satellite() : Project::Project("satellite") {

	app->addItem("solarPanel", 1, "instrument");
	app->addItem("heatSensor", 1, "instrument");
	app->addItem("cameraProbe", 1, "instrument");
	app->addItem("gravityProbe", 1, "instrument");

	_body = (Body*) addElement(new Body(this));
	_instruments = (Instrument*) addElement(new Instrument(this, _body));
	setupMenu();

	_maxRadius = 10.0f;
	_maxLength = 10.0f;
}

bool Satellite::setSubMode(short mode) {
	bool changed = Project::setSubMode(mode);
	switch(_subMode) {
		case 0: { //build
			break;
		} case 1: { //test
			//check that it fits in the cylinder
			std::vector<MyNode*> nodes = _rootNode->getAllNodes();
			short i, j, n = nodes.size(), nv;
			float radius, maxRadius = 0, minZ = 1e6, maxZ = -1e6;
			Matrix m;
			_rootNode->getWorldMatrix().invert(&m);
			Vector3 vec;
			for(i = 0; i < n; i++) {
				MyNode *node = nodes[i];
				nv = node->nv();
				for(j = 0; j < nv; j++) {
					vec = node->_worldVertices[j];
					m.transformPoint(&vec);
					radius = sqrt(vec.x*vec.x + vec.y*vec.y);
					if(vec.z < minZ) minZ = vec.z;
					if(vec.z > maxZ) maxZ = vec.z;
					if(radius > maxRadius) {
						maxRadius = radius;
					}
				}
			}
			if(maxRadius > _maxRadius || (maxZ - minZ) > _maxLength) {
				app->message("Satellite does not fit in tube");
				_launchButton->setEnabled(false);
				break;
			}
			//check that the mass is within the limit
			if(_instruments->getTotalMass() > _instruments->_maxMass) {
				app->message("Instruments are more than 60kg");
				_launchButton->setEnabled(false);
				break;
			}
			//place it at a height of 1m
			app->getPhysicsController()->setGravity(Vector3::zero());
			_rootNode->enablePhysics(false);
			_rootNode->placeRest();
			Vector3 trans(0, 10, 0);
			_rootNode->setMyTranslation(trans);
			app->_ground->setVisible(true);
			break;
		}
	}
	return changed;
}

void Satellite::launch() {
	Project::launch();
	_rootNode->enablePhysics(true);
	app->getPhysicsController()->setGravity(app->_gravity);
	_rootNode->setActivation(DISABLE_DEACTIVATION);
}

Satellite::Body::Body(Project *project) : Project::Element::Element(project, NULL, "body", "Body") {
	_filter = "body";
}

Satellite::Instrument::Instrument(Project *project, Element *parent)
  : Project::Element::Element(project, parent, "instrument", "Instrument", true) {
	_filter = "instrument";
	_addingPanels = 0;
	_maxMass = 60.0f;
}

void Satellite::Instrument::setNode(const char *id) {
	float mass = getMass(id), total = getTotalMass();
	short numPanels = getPanelsNeeded(id);
	if(total + mass + 1.0f*numPanels > _maxMass) {
		app->message("Can't add that instrument - mass would be too high");
		return;
	}
	_addingPanels = numPanels;
	Project::Element::setNode(id);
}

void Satellite::Instrument::addNode(const Vector3 &position) {
	Project::Element::addNode(position);
	if(_addingPanels > 0) {
		_currentNodeId = "solarPanel";
		std::ostringstream os;
		os << "Click to add " << _addingPanels << " more solar panels to power your instrument";
		app->message(os.str().c_str());
		_addingPanels--;
	} else app->message(NULL);
}

void Satellite::Instrument::placeNode(const Vector3 &position, short n) {
	MyNode *body = _parent->getNode(), *node = _nodes[n].get();
	body->updateTransform();
	node->updateTransform();
	BoundingBox box = node->getBoundingBox(true);
	Vector3 normal = Vector3::unitY();
	short f = body->pt2Face(position);
	if(f >= 0) normal = body->_faces[f].getNormal();
	Quaternion rot = MyNode::getVectorRotation(Vector3::unitZ(), normal);
	node->setMyRotation(rot);
	node->setMyTranslation(position - normal * box.min.z);
}

void Satellite::Instrument::addPhysics(short n) {
	//_project->_scene->addNode(_nodes[n].get());
	Project::Element::addPhysics(n);
	MyNode *body = _parent->getNode(), *node = _nodes[n].get();
	body->addChild(node);
	Vector3 joint, dir;
	app->getPhysicsController()->setConstraintNoCollide();
	app->addConstraint(body, node, -1, "fixed", joint, dir, true);
}

bool Satellite::Instrument::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex) {
	Project::Element::touchEvent(evt, x, y, contactIndex);
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

float Satellite::Instrument::getMass(const char *type) {
	if(strcmp(type, "heatSensor") == 0) {
		return 10.0f;
	} else if(strcmp(type, "gravityProbe") == 0) {
		return 20.0f;
	} else if(strcmp(type, "cameraProbe") == 0) {
		return 30.0f;
	} else if(strcmp(type, "solarPanel") == 0) {
		return 1.0f;
	}
}

short Satellite::Instrument::getPanelsNeeded(const char *type) {
	if(strcmp(type, "heatSensor") == 0) {
		return 3;
	} else if(strcmp(type, "gravityProbe") == 0) {
		return 2;
	} else if(strcmp(type, "cameraProbe") == 0) {
		return 1;
	} else if(strcmp(type, "solarPanel") == 0) {
		return 0;
	}
}

float Satellite::Instrument::getTotalMass() {
	short i, n = _nodes.size();
	float mass = 0;
	for(i = 0; i < n; i++) {
		mass += getMass(_nodes[i]->_type.c_str());
	}
	return mass;
}


