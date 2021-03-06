#include "T4TApp.h"
#include "Launcher.h"
#include "MyNode.h"

Launcher::Launcher() : Project::Project("launcher") {

	app->addItem("rubberBand1", 1, "launcherBand");
	app->addItem("rubberBand2", 1, "launcherBand");

	_rubberBand = (RubberBand*) addElement(new RubberBand(this));
	setupMenu();

	_table = MyNode::create("table1");
	_table->loadData("res/common/", false);
	_table->setStatic(true);
	BoundingBox tableBox = _table->getBoundingBox(true);
	Vector3 tablePos(0, -tableBox.min.y, -tableBox.max.z);
	_table->setMyTranslation(tablePos);
	_tableHeight = tableBox.max.y - tableBox.min.y;
	//app->getPhysicsController()->createFixedConstraint(_table->getCollisionObject()->asRigidBody());
	_clampWidth = 8.0f;
	for(short i = 0; i < 2; i++) {
		_clamps[i] = MyNode::create("clamp");
		_clamps[i]->loadData("res/common/", false);
		_clamps[i]->setStatic(true);
		os.str("");
		os << "clamp" << (i+1);
		_clamps[i]->setId(os.str().c_str());
		BoundingBox clampBox = _clamps[i]->getBoundingBox(true);
		Vector3 clampPos((2*i-1) * _clampWidth/2, (tableBox.max.y - tableBox.min.y) - clampBox.min.y, -clampBox.max.z);
		_anchorPoints[i] = clampPos;
		clampPos -= tablePos;
		if(i == 0) clampPos.x -= clampBox.max.x;
		else clampPos.x -= clampBox.min.x;
		_clamps[i]->setMyTranslation(clampPos);
		_table->addChild(_clamps[i]);
		//clampPos.y += clampBox.min.y;
		//app->addConstraint(_table, _clamps[i], -1, "fixed", clampPos, Vector3::unitY(), true);
	}
	_table->addPhysics();
	_table->enablePhysics(false);
	
	_cevBox = BoundingBox::empty();
}

bool Launcher::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex) {
	Project::touchEvent(evt, x, y, contactIndex);
	if(app->_navMode < 0 && _subMode == 1 && !_launching) {
		switch(evt) {
			case Touch::TOUCH_PRESS: {
				break;
			} case Touch::TOUCH_MOVE: {
				if(!_touchPt._touching) break;
				//allow the user to pull back the rubber band				
				float stretch = fmax(0, fmin(3.0f, _touchPt.deltaPix().x / 300.0f));
				setStretch(stretch);
				break;
			} case Touch::TOUCH_RELEASE: {
				launch();
				break;
			}
		}
	}
	return true;
}

bool Launcher::setSubMode(short n) {
	bool changed = Project::setSubMode(n);
	switch(_subMode) {
		case 0: {
			for(short i = 0; i < 2; i++) _bandAnchors[i].reset();
			break;
		} case 1: {
			//show the table with the clamps
			app->setCameraEye(55, 0, M_PI/3);
			_scene->addNode(_table);
			_table->enablePhysics(true);
			app->_ground->setVisible(true);
			app->getPhysicsController()->setGravity(app->_gravity);
			//anchor the rubber band between the clamps
			Vector3 trans = _anchorPoints[0] + (_anchorPoints[1] - _anchorPoints[0]) / 2;
			_rootNode->enablePhysics(false);
			_rootNode->setMyTranslation(trans);
			for(short i = 0; i < 2; i++) {
				os.str("");
				os << _nodeId << "_rubberBand" << (1 + i*(_rubberBand->_numNodes-1));
				MyNode *end = dynamic_cast<MyNode*>(_rootNode->findNode(os.str().c_str()));
				Vector3 dir = Vector3::unitX() * (2*i-1);
				_bandAnchors[i] = ConstraintPtr(app->addConstraint(_clamps[i], end, -1, "socket", _anchorPoints[i], dir));
			}
			//place the crew exploration vehicle in front of the rubber band
			_cev = app->getProjectNode("buggy");
			if(_cev) {
				_cev->enablePhysics(false);
				_cev->placeRest();
				_cev->updateTransform();
				_scene->addNode(_cev);
				_cevBox = _cev->getBoundingBox(true);
			}
			setStretch(0);
			break;
		}
	}
	return changed;
}

void Launcher::setStretch(float stretch) {
	if(_cevBox.isEmpty()) return;
	float D = stretch + _cevBox.max.z - _cevBox.min.z;
	short n = _rubberBand->_numNodes, i, segment;
	float d1 = _cevBox.max.x - _cevBox.min.x, margin = (_clampWidth - d1) / 2, d2 = sqrt(margin*margin + D*D);
	float linkLength = _clampWidth / n;
	float total = d1 + 2*d2 - linkLength, delta = total / (n - 1), distance, remainder;
	Vector3 points[4];
	for(i = 0; i < 4; i++) points[i] = _anchorPoints[0];
	points[1].x += margin;
	points[1].z -= D;
	points[2].x += margin + d1;
	points[2].z -= D;
	points[3].x += _clampWidth;
	for(distance = linkLength/2, i = 0; i < n; distance += delta, i++) {
		if(distance < d2) {
			segment = 0;
			remainder = distance;
		} else if(distance < d1+d2) {
			segment = 1;
			remainder = distance - d2;
		} else {
			segment = 2;
			remainder = distance - (d1+d2);
		}
		os.str("");
		os << _nodeId << "_rubberBand" << (i+1);
		MyNode *node = dynamic_cast<MyNode*>(_rootNode->findNode(os.str().c_str()));
		Vector3 dir = points[segment+1] - points[segment];
		dir.normalize();
		Vector3 trans = points[segment] + dir * remainder;
		Quaternion rot = MyNode::getVectorRotation(Vector3::unitZ(), dir);
		node->setMyTranslation(trans);
		node->setMyRotation(rot);
	}
	Vector3 cevPos(0, _tableHeight - _cevBox.min.y, -_cevBox.max.z - stretch);
	_cev->setMyTranslation(cevPos);
}

void Launcher::launch() {
	Project::launch();
	_cev->enablePhysics(true);
	_rootNode->enablePhysics(true);
}

Launcher::RubberBand::RubberBand(Project *project)
  : Project::Element::Element(project, NULL, "rubberBand", "Rubber Band") {
  	_numNodes = 15;
	_filter = "launcherBand";
	_links.resize(_numNodes-1);
}

void Launcher::RubberBand::placeNode(short n) {
	//each node is one link - place it at the appropriate location in the chain
	Launcher *launcher = (Launcher*)_project;
	float spacing = launcher->_anchorPoints[1].x - launcher->_anchorPoints[0].x,
	  distance = launcher->_anchorPoints[0].x + spacing * (n+0.5f) / _numNodes;
	Vector3 pos(distance, 0, 0);
	_nodes[n]->setMyTranslation(pos);
	Quaternion rot(Vector3::unitY(), M_PI/2);
	_nodes[n]->setMyRotation(rot);
	_nodes[n]->_radius = spacing * 0.4f / _numNodes;
}

void Launcher::RubberBand::addPhysics(short n) {
	Project::Element::addPhysics(n);
	if(n == 0) return;
	MyNode *prev = _nodes[n-1].get(), *node = _nodes[n].get();
	Vector3 pos1 = prev->getTranslationWorld(), pos2 = node->getTranslationWorld();
	Vector3 joint = (pos1 + pos2) / 2, dir = pos2 - pos1;
	dir.normalize();
	app->getPhysicsController()->setConstraintNoCollide();
	PhysicsSpringConstraint *spring = (PhysicsSpringConstraint*) app->addConstraint(prev, node, -1, "spring", joint, dir);
	spring->setLinearStrengthX(1.0f);
	spring->setEnabled(false);
	_links[n-1] = ConstraintPtr(spring);
}


