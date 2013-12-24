#ifndef TEMPLATEGAME_H_
#define TEMPLATEGAME_H_

#include "gameplay.h"
#include <cstring>
#include <cstdio>

using namespace gameplay;

using std::cout;
using std::endl;

/**
 * Main game class.
 */
class T4TApp: public Game, Control::Listener
{
public:

    /**
     * Constructor.
     */
    T4TApp();
    
    T4TApp* getInstance();
    
    bool printNode(Node *node);

    /**
     * @see Game::keyEvent
     */
     
    bool mouseEvent(Mouse::MouseEvent evt, int x, int y, int wheelDelta);

	void keyEvent(Keyboard::KeyEvent evt, int key);
    
    /**
     * @see Game::touchEvent
     */
    void touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex);
    
    void controlEvent(Control* control, EventType evt);
    
    void enableScriptCamera(bool enable);

protected:

    /**
     * @see Game::initialize
     */
    void initialize();

    /**
     * @see Game::finalize
     */
    void finalize();

    /**
     * @see Game::update
     */
    void update(float elapsedTime);

    /**
     * @see Game::render
     */
    void render(float elapsedTime);

private:

    /**
     * Draws the scene each frame.
     */
    bool drawScene(Node* node);
    
    Node* addCatalogItem(int catalogInd, float mass = 10.0f);
    void setSelected(Node* node);
    void placeSelected(float x, float y);
    void setMode(const char *mode);

    //see if the current touch coordinates intersect a given model in the scene
    bool checkTouchModel(Node* node);
    bool checkTouchEdge(Node* node);
    
    //misc functions
    const std::string printVector(Vector3& v);
    const std::string printVector2(Vector2& v);
    
    //model factory functions
    Node* createBoxNode(float width, float height, float depth);
    
    //UI factory functions
    Form* addMenu(Form *parent, const char *name);
    Form* addPanel(Form *parent, const char *name);
    template <class ButtonType> ButtonType* addButton(Form *menu, const char *name, const char *text = NULL);
    template <class ControlType> ControlType* addControl(Form *parent, const char *name, Theme::Style *style, const char *text = NULL);
    
    //standard projects
    void buildVehicle();
    Node* promptComponent();

	//scene setup
    Scene* _scene;
    Node* _lightNode;
    Light* _light;
    
    //T4T objects for modeling
    std::vector<Node*>* _catalog;
    std::vector<std::string>* _itemNames;
    std::vector<int>* _itemCount;
    
    //for placing objects
    Node *_selectedNode, *_lastNode, *_intersectModel;
    const BoundingBox* _selectedBox;
    Plane _groundPlane;
    Vector3 _intersectPoint;
    Vector2 _dragOffset;
    
    //for creating physical constraints between objects
    Node* _constraintNodes[2];
    unsigned short _constraintEdges[2];
    
    //current state
    std::string _mode;
    
    //user interface
    Form *_mainMenu, *_sideMenu, *_itemContainer, *_modeContainer, *_componentMenu;
    std::vector<Form*> *_submenus; //submenus
    std::map<std::string, Form*> _modeOptions;
    Button *_itemButton, *_modeButton; //submenu handles
    CheckBox *_gridCheckbox, *_debugCheckbox;
    Slider *_gridSlider, *_zoomSlider;
    std::vector<std::string> *_modeNames;
    std::vector<RadioButton*> *_modeButtons;
    Theme *_theme;
    Theme::Style *_formStyle, *_buttonStyle, *_titleStyle, *_hiddenStyle;
    Button *_clickOverlay;
    Form *_clickOverlayContainer;
    
    //use Control::Listener instances and enumerated lists of required components 
    //for progressing through standard project scripts
    class VehicleProject : public Button, Control::Listener {
    
public:
    	T4TApp *app;
    	std::vector<std::string> componentName;

    	VehicleProject(T4TApp *app_) : app(app_) {
    		_currentComponent = CHASSIS;
    		componentName.push_back("Chassis");
    		componentName.push_back("Front Wheels");
    		componentName.push_back("Back Wheels");
    	}
    	
    	static VehicleProject *create(T4TApp *app_, const char* id, Theme::Style* style) {
    		VehicleProject *v = new VehicleProject(app_);
    		v->_id = id;
    		v->_style = style;
    		return v;
    	}

		void controlEvent(Control *control, EventType evt);
		bool touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex);

		typedef enum {
			CHASSIS,
			FRONT_WHEELS,
			BACK_WHEELS,
			COMPLETE
		} VehicleComponent;

		VehicleComponent _currentComponent;
		Node *_chassisNode;
		
		void advanceComponent() {
			switch(_currentComponent) {
				case CHASSIS: _currentComponent = FRONT_WHEELS; break;
				case FRONT_WHEELS: _currentComponent = BACK_WHEELS; break;
				case BACK_WHEELS: _currentComponent = COMPLETE; break;
				default: break;
			}
		}
		void setActive(bool active);
	};
	VehicleProject *_vehicleProject;
	
/*	class OverlayPad : public Button
	{
public:
		T4TApp *app;
		OverlayPad::OverlayPad() {}
		void setApp(T4TApp *app_) { app = app_; }
		void touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex);
	};//*/
    
    class TouchPoint
    {
    public:
        unsigned int _id;
        Vector2 _coord;
        bool _isStale;
    };

    std::list<TouchPoint> _touchPoints;
    Vector2 _mousePoint, _touchPoint;
    std::string _mouseString;
    Font* _font;
    
    //debugging variables
    bool _physicsStopped;
    PhysicsRigidBody *_lastBody;
	
};

#endif
