#include "T4TApp.h"
#include "MyNode.h"

void T4TApp::generateModels() {
	generateModel("box", "box", 4.0f, 2.0f, 6.0f);
	generateModel("sphere", "sphere", 1.0f, 10);
	generateModel("cylinder", "cylinder", 1.0f, 4.0f, 20);
	generateModel("halfpipe", "halfpipe", 1.0f, 12.0f, 0.2f, 20);
	generateModel("gear_basic", "gear", 0.6f, 0.9f, 1.2f, 5);
	generateModel("stringLink", "cylinder", 0.1f, 1.0f, 5);
	
	//BEST projects
	generateModel("straw", "cylinder", 0.25f, 5.0f, 8);
	generateModel("balloon_sphere", "sphere", 1.5f, 10);
	generateModel("balloon_long", "cylinder", 0.5f, 3.0f, 20);
	generateModel("axle1", "cylinder", 0.15f, 6.0f, 8);
	generateModel("wheel1", "cylinder", 1.2f, 0.5f, 20);
	generateModel("wheel2", "box", 2.5f, 2.5f, 0.5f);
	generateModel("buggyRamp", "wedge", 15.0f, 15.0f, 5.0f);
	generateModel("gravityProbe", "cylinder", 0.4f, 0.4f, 8);
	generateModel("heatSensor", "cylinder", 0.3f, 0.6f, 8);
	generateModel("cameraProbe", "box", 1.0f, 1.0f, 1.0f);
	generateModel("solarPanel", "box", 2.0f, 3.0f, 0.3f);
	generateModel("hatch1", "box", 3.5f, 3.5f, 0.3f);
	generateModel("hatch2", "box", 3.0f, 4.0f, 0.4f);
	
	//robot
	MyNode *robot[6];
	robot[0] = generateModel("robot_torso", "box", 2.0f, 3.0f, 1.5f);
	robot[1] = generateModel("robot_head", "box", 1.0f, 1.0f, 1.0f);
	robot[2] = generateModel("robot_arm1", "box", 0.7f, 2.5f, 0.7f);
	robot[3] = generateModel("robot_arm2", "box", 0.7f, 2.5f, 0.7f);
	robot[4] = generateModel("robot_leg1", "box", 1.0f, 2.0f, 1.0f);
	robot[5] = generateModel("robot_leg2", "box", 1.0f, 2.0f, 1.0f);
	short i;
	for(i = 1; i < 6; i++) robot[0]->addChild(robot[i]);
	//arms and legs should move wrt top of their model
	for(i = 2; i < 4; i++) robot[i]->shiftModel(0, -1.25f, 0);
	for(i = 4; i < 6; i++) robot[i]->shiftModel(0, -1.0f, 0);
	//no physics - the Robot class just adds a capsule on the root node
	for(i = 0; i < 6; i++) robot[i]->_objType = "none";
	robot[0]->setTranslation(0, 3.5f, 0);
	robot[1]->setTranslation(0, 2.0f, 0);
	robot[2]->setTranslation(-1.5f, 1.25f, 0);
	robot[3]->setTranslation(1.5f, 1.25f, 0);
	robot[4]->setTranslation(-0.6f, -1.5f, 0);
	robot[5]->setTranslation(0.6f, -1.5f, 0);
	MyNode *_robot = MyNode::create("robot");
	_robot->_type = "root";
	_robot->addChild(robot[0]);
	_robot->writeData("res/common/");
	
	loadObj("res/common/unnamed.obj");
}

MyNode* T4TApp::generateModel(const char *id, const char *type, ...) {
	va_list arguments;
	va_start(arguments, type);
	MyNode *node = MyNode::create(id);
	node->_type = type;
	node->_objType = "mesh";
	node->_mass = 10.0f;
	short nv, nf, ne, i, j, k, m, n;
	Vector3 vertex;
	std::vector<unsigned short> face;
	std::vector<std::vector<unsigned short> > triangles;
	MyNode::ConvexHull *hull;

	if(strcmp(type, "sphere") == 0) {
		float radius = (float)va_arg(arguments, double);
		int segments = va_arg(arguments, int);
		float theta, phi;
		node->_objType = "sphere";
		node->addVertex(0, 0, -radius);
		for(i = 1; i <= segments-1; i++) {
			phi = (i - segments/2) * M_PI/segments;
			for(j = 0; j < segments; j++) {
				theta = j * 2*M_PI / segments;
				node->addVertex(radius * cos(phi) * cos(theta), radius * cos(phi) * sin(theta), radius * sin(phi));
			}
		}
		node->addVertex(0, 0, radius);
		for(i = 0; i < segments; i++) {
			node->addFace(3, 1+i, 0, 1+(i+1)%segments);
			m = 1 + (segments-2)*segments;
			node->addFace(3, m+i, m+(i+1)%segments, m+segments);
		}
		for(i = 0; i < segments-2; i++) {
			m = 1 + i*segments;
			for(j = 0; j < segments; j++) {
				node->addFace(4, m+j, m+(j+1)%segments, m+segments+(j+1)%segments, m+segments+j);
			}
		}
		node->setOneHull();
	}
	else if(strcmp(type, "cylinder") == 0) {
		float radius = (float)va_arg(arguments, double);
		float height = (float)va_arg(arguments, double);
		int segments = va_arg(arguments, int);
		float angle;
		for(i = 0; i < segments; i++) {
			angle = i * 2*M_PI / segments;
			node->addVertex(radius * cos(angle), radius * sin(angle), height/2);
			node->addVertex(radius * cos(angle), radius * sin(angle), -height/2);
		}
		for(i = 0; i < segments; i++) {
			node->addFace(4, i*2, i*2+1, (i*2+3)%(2*segments), (i*2+2)%(2*segments));
		}
		std::vector<unsigned short> face(segments);
		std::vector<std::vector<unsigned short> > triangles;
		for(i = 0; i < 2; i++) {
			for(j = 0; j < segments; j++) face[j] = i == 0 ? 2*j : 2*segments-1 - 2*j;
			triangles.clear();
			node->addFace(face);
		}
		node->setOneHull();
	}
	else if(strcmp(type, "halfpipe") == 0) {
		float radius = (float)va_arg(arguments, double);
		float height = (float)va_arg(arguments, double);
		float thickness = (float)va_arg(arguments, double);
		int segments = va_arg(arguments, int);
		float angle, innerRadius = radius - thickness, outerRadius = radius;
		for(i = 0; i <= segments/2; i++) {
			angle = i * M_PI / (segments/2);
			node->addVertex(innerRadius * cos(angle), innerRadius * sin(angle), height/2);
			node->addVertex(innerRadius * cos(angle), innerRadius * sin(angle), -height/2);
			node->addVertex(outerRadius * cos(angle), outerRadius * sin(angle), height/2);
			node->addVertex(outerRadius * cos(angle), outerRadius * sin(angle), -height/2);
		}
		for(i = 0; i < segments/2; i++) {
			node->addFace(4, i*4, i*4+4, i*4+5, i*4+1);
			node->addFace(4, i*4+2, i*4+3, i*4+7, i*4+6);
			node->addFace(4, i*4+4, i*4, i*4+2, i*4+6);
			node->addFace(4, i*4+1, i*4+5, i*4+7, i*4+3);
		}
		node->addFace(4, 0, 1, 3, 2);
		m = (segments/2) * 4;
		node->addFace(4, m+1, m, m+2, m+3);
		
		for(i = 0; i < segments/2; i++) {
			hull = new MyNode::ConvexHull(node);
			for(j = 0; j < 8; j++) hull->addVertex(node->_vertices[i*4 + j]);
			hull->addFace(4, 0, 4, 5, 1);
			hull->addFace(4, 2, 3, 7, 6);
			hull->addFace(4, 4, 0, 2, 6);
			hull->addFace(4, 1, 5, 7, 3);
			hull->addFace(4, 0, 1, 3, 2);
			hull->addFace(4, 4, 6, 7, 5);
			node->_hulls.push_back(hull);
		}
	}
	else if(strcmp(type, "box") == 0) {
		float length = (float)va_arg(arguments, double);
		float height = (float)va_arg(arguments, double);
		float width = (float)va_arg(arguments, double);
		node->_objType = "box";
		node->_vertices.resize(8);
		for(i = 0; i < 2; i++) {
			for(j = 0; j < 2; j++) {
				for(k = 0; k < 2; k++) {
					node->_vertices[i*4 + j*2 + k].set((2*k-1) * length/2, (2*j-1) * height/2, (2*i-1) * width/2);
				}
			}
		}
		node->addFace(4, 2, 3, 1, 0);
		node->addFace(4, 4, 5, 7, 6);
		node->addFace(4, 1, 5, 4, 0);
		node->addFace(4, 2, 6, 7, 3);
		node->addFace(4, 4, 6, 2, 0);
		node->addFace(4, 1, 3, 7, 5);
		node->setOneHull();
	}
	else if(strcmp(type, "wedge") == 0) {
		float length = (float)va_arg(arguments, double);
		float width = (float)va_arg(arguments, double);
		float height = (float)va_arg(arguments, double);
		node->_vertices.resize(6);
		for(i = 0; i < 3; i++) {
			for(j = 0; j < 2; j++) {
				node->_vertices[i*2 + j].set((2*j-1) * width/2, (i/2) * height, (2*(i%2)-1) * length/2);
			}
		}
		node->addFace(4, 0, 1, 3, 2);
		node->addFace(4, 0, 4, 5, 1);
		node->addFace(4, 2, 3, 5, 4);
		node->addFace(3, 0, 2, 4);
		node->addFace(3, 1, 5, 3);
		node->setOneHull();
	}
	else if(strcmp(type, "gear") == 0) {
		float innerRadius = (float)va_arg(arguments, double);
		float outerRadius = (float)va_arg(arguments, double);
		float width = (float)va_arg(arguments, double);
		int teeth = va_arg(arguments, int);
		nv = teeth * 8;
		float angle, dAngle = 2*M_PI / teeth, gearWidth = innerRadius * sin(dAngle/2);
		Matrix rot;
		//vertices
		for(n = 0; n < teeth; n++) {
			angle = n * dAngle;
			Matrix::createRotation(Vector3(0, 0, 1), -angle, &rot);
			for(i = 0; i < 2; i++) {
				for(j = 0; j < 2; j++) {
					for(k = 0; k < 2; k++) {
						vertex.set(0, innerRadius, -width/2);
						if(i == 1) vertex.z += width;
						if(j == 1) vertex.y = outerRadius;
						if(k == 1) vertex.x += gearWidth;
						rot.transformPoint(&vertex);
						node->_vertices.push_back(vertex);
					}
				}
			}
		}
		//faces
		for(i = 0; i < 2; i++) {
			face.clear();
			for(j = 0; j < teeth; j++) {
				face.push_back(8 * j + 4*i);
				face.push_back(8 * j + 1 + 4*i);
			}
			node->addFace(face, i == 1);
		}
		for(n = 0; n < teeth; n++) {
			for(i = 0; i < 2; i++) {
				//tooth sides
				face.clear();
				triangles.clear();
				face.push_back(0);
				face.push_back(1);
				face.push_back(3);
				face.push_back(2);
				for(j = 0; j < 4; j++) face[j] += n*8 + i*4;
				node->addFace(face, i == 0);
				//tooth front/back
				face.clear();
				triangles.clear();
				face.push_back(0);
				face.push_back(2);
				face.push_back(6);
				face.push_back(4);
				for(j = 0; j < 4; j++) face[j] += n*8 + i;
				node->addFace(face, i == 0);
			}
			//tooth top
			face.clear();
			triangles.clear();
			face.push_back(2);
			face.push_back(6);
			face.push_back(7);
			face.push_back(3);
			for(j = 0; j < 4; j++) face[j] += n*8;
			node->addFace(face);
			//tooth connector
			face.clear();
			triangles.clear();
			face.push_back(1);
			face.push_back(5);
			face.push_back(12);
			face.push_back(8);
			for(j = 0; j < 4; j++) face[j] = (face[j] + n*8) % nv;
			node->addFace(face);
		}
		//convex hulls
		for(n = 0; n < teeth; n++) {
			hull = new MyNode::ConvexHull(node);
			for(i = 0; i < 8; i++) hull->addVertex(node->_vertices[i + n*8]);
			node->_hulls.push_back(hull);
		}
		hull = new MyNode::ConvexHull(node);
		for(n = 0; n < teeth; n++) {
			hull->addVertex(node->_vertices[0 + n*8]);
			hull->addVertex(node->_vertices[1 + n*8]);
			hull->addVertex(node->_vertices[4 + n*8]);
			hull->addVertex(node->_vertices[5 + n*8]);
		}
		node->_hulls.push_back(hull);
	}
	node->writeData("res/common/");
	return node;
}

void T4TApp::loadObj(const char *filename) {
	std::unique_ptr<Stream> stream(FileSystem::open(filename));
	stream->rewind();

	char *str, line[2048], *label = (char*)malloc(100*sizeof(char)), *nodeName = (char*)malloc(100*sizeof(char)),
	  *token = (char*)malloc(100*sizeof(char));
	short i, j, k, m, n, ind, vOffset = 0;
    float x, y, z, w;
    Vector3 scale, vertex;
    std::vector<unsigned short> face;
    bool reverseFace = false;
	MyNode *node = MyNode::create("node");
	while(!stream->eof()) {
		strcpy(label, "");
		str = stream->readLine(line, 2048);
		std::istringstream in(str);
		in >> label;
		if(strcmp(label, "s") == 0) {
			in >> x >> y >> z;
			scale.set(x, y, z);
			cout << "Scale: " << x << ", " << y << ", " << z << endl;
		} else if(strcmp(label, "reverse") == 0) {
			in >> m;
			reverseFace = m > 0;
		} else if(strcmp(label, "v") == 0) { //vertex
			in >> x >> y >> z;
			node->addVertex(x, y, z);
		} else if(strcmp(label, "f") == 0) { //face
			face.clear();
			in >> token;
			while(!in.fail()) {
				std::string str(token);
				size_t pos = str.find('/');
				if(pos > 0) str = str.substr(0, pos);
				ind = atoi(str.c_str());
				face.push_back(ind-1 - vOffset);
				in >> token;
			}
			node->addFace(face, reverseFace);
		}
		if((strcmp(label, "g") == 0 || stream->eof()) && node->nv() > 0) { //end of current model
			n = node->nv();
			vertex.set(0, 0, 0);
			for(i = 0; i < n; i++) vertex += node->_vertices[i];
			vertex *= 1.0f / n;
			for(i = 0; i < n; i++) {
				node->_vertices[i] -= vertex;
				node->_vertices[i].x *= scale.x;
				node->_vertices[i].y *= scale.y;
				node->_vertices[i].z *= scale.z;
			}
			node->setOneHull();
			node->writeData("res/common/");
			node->clearMesh();
			//vOffset += n;
		}
		if(strcmp(label, "g") == 0) { //start of new model
			in >> nodeName;
			node->setId(nodeName);
			node->_type = nodeName;
			node->_objType = "mesh";
			node->_mass = 10.0f;
			scale.set(1, 1, 1);
			cout << endl << "Model: " << nodeName << endl;
		}
	}
	stream->close();
}



