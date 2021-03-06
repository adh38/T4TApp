#include "T4TApp.h"
#include "Project.h"
#include "MyNode.h"

Project::Project(const char* id) : Mode::Mode(id) {

	_typeCount = -1;
	_scene = Scene::load("res/common/scene.gpb");
	_camera = _scene->getActiveCamera();
    _camera->setAspectRatio(app->getAspectRatio());

	_rootNode = MyNode::create(_id.c_str());
	_rootNode->_type = "root";
	_buildAnchor = NULL;

	_other = (Other*) addElement(new Other(this));	

	_currentElement = 0;
	_moveMode = 0;

	_subModes.push_back("build");
	_subModes.push_back("test");
}

void Project::setupMenu() {
	_numElements = _elements.size();

	//add a launch button
	_launchButton = app->addButton <Button> (_controls, "launch", "Launch");
	_launchButton->setHeight(40.0f);
	app->addListener(_launchButton, this);

	//add a button for each element to choose its item and edit it
	short i, j, n;
	_elementContainer = Container::create("elements", app->_theme->getStyle("hiddenContainer"), Layout::LAYOUT_VERTICAL);
	_elementContainer->setAutoWidth(true);
	_elementContainer->setHeight(_numElements * 50.0f);
	for(i = 0; i < _numElements; i++) {
		j = (i+1) % _numElements;
		Button *button = app->addButton <Button> (_elementContainer, _elements[j]->_id.c_str(), _elements[j]->_name.c_str());
	}
	_controls->addControl(_elementContainer);
	app->addListener(_elementContainer, this);
	
	//add a button for attaching general items
	//Button *button = app->addButton <Button> (_controls, "other", "Other");
	
	_moveContainer = Container::create("moveMode", app->_theme->getStyle("hiddenContainer"), Layout::LAYOUT_FLOW);
	_moveModes.push_back("translate");
	_moveModes.push_back("rotate");
	_moveModes.push_back("rotateFree");
	_moveModes.push_back("groundFace");
	n = _moveModes.size();
	for(i = 0; i < n; i++) {
		ImageControl *button = app->addButton <ImageControl> (_moveContainer, _moveModes[i].c_str(), _moveModes[i].c_str(),
			"imageSquare");
		button->setImage(MyNode::concat(3, "res/png/", _moveModes[i].c_str(), ".png"));
		button->setSize(50.0f, 50.0f);
	}
	_moveContainer->setAutoWidth(true);
	_moveContainer->setHeight(ceil(n / 3.0f) * 60.0f);
	_controls->addControl(_moveContainer);
	app->addListener(_moveContainer, this);

	//add a button for each action that any element has - we will enable them on the fly for the selected element
	_actionContainer = Container::create("actions", app->_theme->getStyle("hiddenContainer"), Layout::LAYOUT_FLOW);
	_numActions = 0;
	for(i = 0; i < _numElements; i++) {
		short numActions = _elements[i]->_actions.size();
		for(j = 0; j < numActions; j++) {
			const char *action = _elements[i]->_actions[j].c_str();
			if(_actionContainer->getControl(action) != NULL) continue;
			ImageControl *button = app->addButton <ImageControl> (_actionContainer, action, action, "imageSquare");
			button->setImage(MyNode::concat(3, "res/png/", action, ".png"));
			button->setSize(50.0f, 50.0f);
			_numActions++;
		}
	}
	_actionFilter = new MenuFilter(_actionContainer);
	_actionContainer->setAutoWidth(true);
	_actionContainer->setHeight(ceil(_numActions/3.0f) * 60.0f);
	_controls->addControl(_actionContainer);
	app->addListener(_actionContainer, this);

	_controls->setHeight(_controls->getHeight() + _elementContainer->getHeight() + _actionContainer->getHeight()
		+ _moveContainer->getHeight() + 150.0f);
}

void Project::controlEvent(Control *control, EventType evt) {
	Mode::controlEvent(control, evt);
	const char *id = control->getId();
	cout << "project control " << id << endl;
	Element *element = getEl();

	if(_numElements > 0 && _elementContainer->getControl(id) == control) {
		for(short i = 0; i < _elements.size(); i++) if(_elements[i]->_id.compare(id) == 0) {
			cout << "current element now " << i << endl;
			setCurrentElement(i);
			break;
		}
//	} else if(strcmp(id, "other") == 0) {
//		setCurrentElement(-1);
//		app->promptItem();
	} else if(strncmp(id, "comp_", 5) == 0) {
		app->_componentMenu->setVisible(false);
		if(element) element->setNode(id+5);
		else _currentNodeId = id+5;
	} else if(_moveContainer->getControl(id) == control) {
		short n = _moveModes.size(), i;
		for(i = 0; i < n; i++) {
			if(_moveModes[i].compare(id) == 0) _moveMode = i;
		}
	} else if(_numActions > 0 && _actionContainer->getControl(id) == control) {
		if(element) element->doAction(id);
		if(strcmp(id, "delete") == 0) deleteSelected();
	} else if(control == _launchButton) {
		launch();
	} else if(strcmp(id, "translate") == 0) {
		_moveMode = 0;
	} else if(strlen(id) == 7 && strncmp(id, "rotate", 6) == 0) {
		_moveMode = 1;
		_moveAxis = (short)(id[6] - 88); //char 7 is 'X', 'Y', or 'Z'
		app->setNavMode(-1);
	} else if(strcmp(id, "finishElement") == 0) {
		promptNextElement();
	}
}

bool Project::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex) {
	MyNode *touchNode = getTouchNode(); //get the current node first in case we release and thus set it to null
	Mode::touchEvent(evt, x, y, contactIndex);
	if(touchNode == NULL) touchNode = getTouchNode();
	if(app->_navMode >= 0) return false;
	if(touchNode != NULL && touchNode->_element != NULL && touchNode->_element->_project == this) {
		//see if we are placing a node on its parent
		Element *current = getEl();
		if(evt == Touch::TOUCH_PRESS && current && current->_currentNodeId
		  && (current->_isOther || current->_parent == touchNode->_element)) {
			current->addNode();
		}
		//otherwise just trigger whatever node we clicked
		else {
			if(evt == Touch::TOUCH_PRESS) {
				short n = _elements.size(), i;
				for(i = 0; i < n; i++) if(_elements[i].get() == touchNode->_element) setCurrentElement(i);
			}
			getEl()->touchEvent(evt, x, y, contactIndex);
		}
	}
}

void Project::deleteSelected() {
	if(_selectedNode == NULL) return;
	Element *el = _selectedNode->_element;
	if(el == NULL) return;
	short n = el->_nodes.size(), i;
	for(i = 0; i < n; i++) {
		if(el->_nodes[i].get() == _selectedNode) {
			el->deleteNode(i);
			break;
		}
	}
}

Project::Element* Project::getEl(short n) {
	if(n < 0) n = _currentElement;
	if(n > _elements.size()) return NULL;
	return _elements[n].get();
}

MyNode* Project::getNode(short n) {
	if(n < 0) n = _currentElement;
	if(_elements[n]->_nodes.empty()) return NULL;
	return _elements[n]->_nodes.back().get();
}

void Project::addNode() {
	if(_currentNodeId == NULL || getTouchNode() == NULL) return;
	MyNode *node = app->duplicateModelNode(_currentNodeId), *parent = getTouchNode();
	node->_project = this;
	Vector3 point = getTouchPoint(), normal = getTouchNormal();
	node->attachTo(parent, point, normal);
	node->addPhysics();
	app->addConstraint(parent, node, -1, "fixed", point, normal, true);
	_currentNodeId = NULL;
}

void Project::finish() {
	for(short i = 0; i < _elements.size(); i++) {
		MyNode *node = getNode(i);
		if(_elements[i]->_static) { //we didn't make it static before now, to allow user to adjust position
			node->removePhysics();
			node->setStatic(true);
			node->addPhysics();
		} else { //add gravity to this node
			node->getCollisionObject()->asRigidBody()->setGravity(app->getPhysicsController()->getGravity());
		}
	}
	app->_scene->addNode(_rootNode);
	setActive(false);
}

Project::Element* Project::addElement(Element *element) {
	_elements.push_back(std::shared_ptr<Element>(element));
	return element;
}

void Project::addPhysics() {
	short i, n = _elements.size();
	for(i = 0; i < n; i++) {
		short numNodes = _elements[i]->_nodes.size(), j;
		for(j = 0; j < numNodes; j++) {
			_elements[i]->addPhysics(j);
		}
	}
}

void Project::setActive(bool active) {
	Mode::setActive(active);
	if(active) {
		//determine the count of this component type based on the highest index for this element in the scene or in saved files
		_typeCount = 0;
		do {
			std::stringstream ss;
			ss << _id << ++_typeCount;
			_nodeId = ss.str();
		} while(app->_scene->findNode(_nodeId.c_str()) != NULL
			|| FileSystem::fileExists(("res/common/" + _nodeId + ".node").c_str()));
		_rootNode->setId(_nodeId.c_str());
		_scene->addNode(_rootNode);
		app->_ground->setVisible(false);
		app->_componentMenu->setState(Control::FOCUS);
		app->filterItemMenu();
		app->removeListener(app->_componentMenu, app);
		app->addListener(app->_componentMenu, this);
		Control *finish = _controls->getControl("finishElement");
		if(finish) finish->setEnabled(true);
		app->getPhysicsController()->setGravity(Vector3::zero());
		_inSequence = true;
		//determine the next element needing to be added
		short e;
		for(e = 1; e < _numElements && _elements[e]->getNode(); e++);
		_currentElement = e-1;
		if(e < _numElements) promptNextElement();
	}else {
		if(_buildAnchor) _buildAnchor->setEnabled(false);
		_rootNode->enablePhysics(false);
		app->_componentMenu->setState(Control::NORMAL);
		app->filterItemMenu();
		app->removeListener(app->_componentMenu, this);
		app->addListener(app->_componentMenu, app);
		app->getPhysicsController()->setGravity(app->_gravity);
	}
}

bool Project::setSubMode(short mode) {
	bool building = _subMode == 0, changed = Mode::setSubMode(mode);
	if(_subMode == 0) app->_ground->setVisible(false);
	if(building) {
		if(changed) _rootNode->setRest();
	} else _rootNode->placeRest();
	if(_buildAnchor.get() != nullptr) _buildAnchor->setEnabled(_subMode == 0);
	switch(_subMode) {
		case 0: { //build
			app->setCameraEye(30, -M_PI/3, M_PI/12);
			break;
		} case 1: { //place in test position
			app->setCameraEye(40, 0, M_PI/12);
			break;
		}
	}
	_launchButton->setEnabled(_subMode == 1);
	return changed;
}

void Project::setCurrentElement(short n) {
	_currentElement = n;
	if(_currentElement >= 0) {
		Element *element = getEl();
		std::vector<std::string> &actions = element->_actions;
		_actionFilter->filterAll(true);
		for(short i = 0; i < actions.size(); i++) {
			_actionFilter->filter(actions[i].c_str(), false);
		}
	}
}

void Project::promptNextElement() {
	if(_currentElement < (short)_elements.size()-1) setCurrentElement(_currentElement+1);
	else _inSequence = false;
	if(!_inSequence) return;
	app->promptItem(getEl()->_filter);
}

void Project::launch() {
	_launching = true;
	_launchButton->setEnabled(false);
}

Project::Element::Element(Project *project, Element *parent, const char *id, const char *name, bool multiple)
  : _project(project), _id(id), _name(name), _numNodes(1), _currentNodeId(NULL), _multiple(multiple), _touchInd(-1),
    _isOther(false) {
	app = (T4TApp*) Game::getInstance();
  	if(name == NULL) _name = _id;
	setParent(parent);
	_plane.set(Vector3::unitX(), 0); //usually keep things symmetric wrt yz-plane
	setMovable(false, false, false, -1);
	setRotable(false, false, false);
	for(short i = 0; i < 3; i++) setLimits(i, -MyNode::inf(), MyNode::inf());
	if(_multiple) {
		addAction("add");
		addAction("delete");
	}
}

void Project::Element::setParent(Element *parent) {
	_parent = parent;
	if(parent) parent->addChild(this);
}

void Project::Element::addChild(Element *element) {
	if(std::find(_children.begin(), _children.end(), element) == _children.end()) {
		_children.push_back(element);
	}
}

void Project::Element::setMovable(bool x, bool y, bool z, short ref) {
	_movable[0] = x;
	_movable[1] = y;
	_movable[2] = z;
	_moveRef = ref;
}

void Project::Element::setRotable(bool x, bool y, bool z) {
	_rotable[0] = x;
	_rotable[1] = y;
	_rotable[2] = z;
}

void Project::Element::setLimits(short axis, float lower, float upper) {
	_limits[axis][0] = lower;
	_limits[axis][1] = upper;
}

void Project::Element::setPlane(const Plane &plane) {
	_plane = plane;
}

void Project::Element::applyLimits(Vector3 &translation) {
	short i;
	for(i = 0; i < 3; i++) {
		if(!_movable[i]) MyNode::sv(translation, i, 0);
		else {
			Vector3 ref = translation, delta = Vector3::zero();
			if(_moveRef >= 0) {
				ref = translation - _project->getNode(_moveRef)->getTranslationWorld();
				delta = translation - ref;
			}
			float val = MyNode::gv(ref, i);
			if(_limits[i][0] > -MyNode::inf() && val < _limits[i][0]) {
				MyNode::sv(ref, i, _limits[i][0]);
			}
			else if(_limits[i][1] < MyNode::inf() && val > _limits[i][1]) {
				MyNode::sv(ref, i, _limits[i][1]);
			}
			translation = ref + delta;
		}
	}
}

void Project::Element::addAction(const char *action) {
	_actions.push_back(action);
}

void Project::Element::removeAction(const char *action) {
	std::vector<std::string>::iterator it = std::find(_actions.begin(), _actions.end(), action);
	if(it != _actions.end()) _actions.erase(it);
}

void Project::Element::doAction(const char *action) {
	if(strcmp(action, "add") == 0) {
		app->promptItem(_filter);
	}
}

void Project::Element::setNode(const char *id) {
	_currentNodeId = id;
	if(_parent == NULL && !_isOther) { //auto-place this node
		addNode();
	}
}

void Project::Element::addNode() {
	bool append = _multiple || _nodes.empty();
	short offset = _multiple ? _nodes.size() / _numNodes : 0;
	for(short i = 0; i < _numNodes; i++) {
		MyNode *node = app->duplicateModelNode(_currentNodeId);
		node->_project = _project;
		node->_element = this;
		std::ostringstream os;
		short count = 0;
		if(_numNodes > 1 || _multiple) do {
			os.str("");
			os << _project->_nodeId << "_" << _id << ++count;
		} while (_project->_scene->findNode(os.str().c_str()) != NULL);
		else os << _project->_nodeId << "_" << _id;
		node->setId(os.str().c_str());
		if(append) _nodes.push_back(std::shared_ptr<MyNode>(node));
		else _nodes[i] = std::shared_ptr<MyNode>(node);
		placeNode(offset + i);
		addPhysics(offset + i);
	}
	_currentNodeId = NULL;
	_project->promptNextElement();
}

void Project::Element::placeNode(short n) {
	MyNode *node = _nodes[n].get();
	Vector3 point = _project->getTouchPoint(_project->getLastTouchEvent()),
	  normal = _project->getTouchNormal(_project->getLastTouchEvent());
	if(_parent == NULL && !_isOther) {
		node->setTranslation(0, 0, 0);
	} else {
		MyNode *parent = _isOther ? _project->getTouchNode(_project->getLastTouchEvent()) : _parent->getNode();
		if(parent && parent != node) {
			cout << "attaching to " << parent->getId() << " at " << app->pv(point) << endl;
			node->attachTo(parent, point, normal);
		}
	}
}

void Project::Element::addPhysics(short n) {
	MyNode *node = _nodes[n].get();
	node->addPhysics();
	if(_parent == NULL && !_isOther) {
		if(n == 0) _project->_buildAnchor = ConstraintPtr(app->getPhysicsController()->createFixedConstraint(
		  node->getCollisionObject()->asRigidBody()));
		_project->_rootNode->addChild(node);
	}
}

void Project::Element::deleteNode(short n) {
	MyNode *node = _nodes[n].get();
	node->removeMe();
	_nodes.erase(_nodes.begin() + n);
}

short Project::Element::getNodeCount() {
	return _nodes.size();
}

MyNode* Project::Element::getNode(short n) {
	return _nodes.size() > n ? _nodes[n].get() : NULL;
}

bool Project::Element::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex) {
	_planeTouch.set(evt, x, y, _plane);
	if(evt == Touch::TOUCH_PRESS) {
		//identify which of my nodes was touched
		_touchInd = -1;
		for(short i = 0; i < _nodes.size(); i++) {
			_nodes[i]->setBase();
			if(_nodes[i].get() == _project->getTouchNode(evt)) _touchInd = i;
		}
	}
	MyNode *node = NULL, *parent = NULL;
	if(_touchInd >= 0) node = _nodes[_touchInd].get();
	if(node) parent = dynamic_cast<MyNode*>(node->getParent());
	//move the node as needed
	if(node && parent && _project->_moveMode >= 0) {
		switch(_project->_moveMode) {
			case 0: { //translate
				switch(evt) {
					case Touch::TOUCH_PRESS: {
						//treat it as if the user clicked on the point where this node is attached to its parent
						parent->updateTransform();
						parent->updateCamera();
						Vector3 point = node->getAnchorPoint();
						_project->_touchPt.set(evt, x, y, point);
						cout << "touched " << node->getId() << ", currently at " << app->pv(point) << endl;
						node->enablePhysics(false);
						break;
					} case Touch::TOUCH_MOVE: {
						_project->_touchPt.set(evt, x, y, parent);
						cout << "moving to " << app->pv(_project->_touchPt.getPoint(evt)) << endl;
						placeNode(_touchInd);
						break;
					} case Touch::TOUCH_RELEASE: {
						addPhysics(_touchInd);
						node->enablePhysics(true);
						break;
					}
				}
				break;
			} case 1: { //rotate
				switch(evt) {
					case Touch::TOUCH_PRESS: {
						node->enablePhysics(false);
						break;
					} case Touch::TOUCH_MOVE: {
						float deltaAngle = _project->_touchPt.deltaPix().x * 2 * M_PI / 400.0f;
						Quaternion rot(node->getJointNormal(), deltaAngle);
						node->baseRotate(rot);
						break;
					} case Touch::TOUCH_RELEASE: {
						addPhysics(_touchInd);
						node->enablePhysics(true);
						break;
					}
				}
				break;
			} case 2: { //free rotate
				switch(evt) {
					case Touch::TOUCH_PRESS: {
						node->enablePhysics(false);
						_project->_jointBase = _project->getTouchPoint() - node->getAnchorPoint();
						break;
					} case Touch::TOUCH_MOVE: {
						//try to keep the point that was touched under the mouse pointer while rotating about the joint
						//if ray is v0 + k*v, and free radius is R, then |v0 + k*v| = R
						// => v0^2 + 2*k*v*v0 + k^2*v^2 = R^2 => k = [-v*v0 +/- sqrt((v*v0)^2 - v^2*(v0^2 - R^2))] / v^2
						Vector3 origin = node->getAnchorPoint();
						Vector3 v0 = _project->_ray.getOrigin() - origin, v = _project->_ray.getDirection(), joint;
						float R = _project->_jointBase.length(), v_v0 = v.dot(v0), v_v = v.lengthSquared();
						float det = v_v0*v_v0 - v_v * (v0.lengthSquared() - R*R), k;
						if(det >= 0) {
							k = (-v_v0 - sqrt(det)) / v_v;
							joint = v0 + v * k;
						} else { //if the mouse pointer is too far out...
							Vector3 normal = node->getJointNormal();
							//gracefully decline the joint vector to the parent's surface
							Plane plane(normal, -normal.dot(origin));
							//take the joint where the determinant would be zero (joint would be perp. to camera ray)
							float k1 = -v_v0 / v_v;
							//and the joint that lies on the parent surface tangent plane
							float k2 = _project->_ray.intersects(plane);
							//and vary between them according to the determinant
							if(k2 != Ray::INTERSECTS_NONE) {
								k = k1 + (k2-k1) * fmin(-det, 1);
								joint = v0 + v * k;
							} else joint = _project->_camera->getNode()->getTranslationWorld() - origin;
						}
						Quaternion rot = MyNode::getVectorRotation(_project->_jointBase, joint);
						//cout << "rotating to " << app->pq(rot) << " about " << app->pv(origin) << endl;
						node->baseRotate(rot, &origin);
						break;
					} case Touch::TOUCH_RELEASE: {
						addPhysics(_touchInd);
						node->enablePhysics(true);
						break;
					}
				}
				break;
			} case 3: { //ground face
				switch(evt) {
					case Touch::TOUCH_PRESS: {
						break;
					} case Touch::TOUCH_MOVE: {
						break;
					} case Touch::TOUCH_RELEASE: {
						if(node->getChildCount() > 0) break;
						short f = node->pt2Face(_project->_touchPt.getPoint(evt));
						if(f < 0) break;
						node->enablePhysics(false);
						Vector3 joint = node->getAnchorPoint(), normal = node->getJointNormal();
						Plane plane(normal, -joint.dot(normal));
						node->rotateFaceToPlane(f, plane);
						addPhysics(_touchInd);
						node->enablePhysics(true);
						break;
					}
				}
				break;
			}
		}
	}
	if(evt == Touch::TOUCH_RELEASE) {
		_touchInd = -1;
	}
}

Project::Other::Other(Project *project) : Project::Element::Element(project, NULL, "other", "Other", true) {
	_isOther = true;
	_filter = "general";
}

void Project::Other::addPhysics(short n) {
	Project::Element::addPhysics(n);
	MyNode *node = _nodes[n].get(), *parent = dynamic_cast<MyNode*>(node->getParent());
	if(!parent) parent = _project->getTouchNode();
	app->getPhysicsController()->setConstraintNoCollide();
	app->addConstraint(parent, node, node->_constraintId, "fixed", node->_parentOffset, node->_parentAxis, true);
}



