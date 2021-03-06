
-- Move speed (m/s)
MOVE_SPEED_NORMAL = 1
MOVE_SPEED_FAST = 5

-- Move flags
MOVE_FORWARD = 1
MOVE_BACKWARD = 2
MOVE_RIGHT = 3
MOVE_LEFT = 4

_useScriptCamera = false
_forwardSpeed = 0
_sideSpeed = 0
_touch = Vector2.new()
_delta = Vector2.new()
_move = Vector2.new()
_moveFlags = { false, false, false, false }
_yaw = 3.1415926535/2.0
_pitch = 0.1
_radius = 30.0
_moveFast = false

_eyeMatrix = Matrix.new()
_eye = Vector3.new()
_up = Vector3.new()
_target = Vector3.new()
_translation = Vector3.new()
_rotation = Quaternion.new()
_scale = Vector3.new()

_node = nil
_nodeRotation = Matrix.new()
_nodeTranslation = Vector3.new()

function camera_setActive(flag)
	--print("Lua setting camera active")
    _useScriptCamera = flag

    if _useScriptCamera then
        Game.getInstance():setMultiTouch(true)
        
        _scene = Scene.getScene("scene")
        --_vehicle = Scene.getScene("vehicle")
        _cameraNodes = {_scene:getActiveCamera():getNode()}
        --, _vehicle:getActiveCamera():getNode()}

        -- Set initial camera angles
        --local eulers = camera_quatToEuler(_cameraNode:getRotation())
        --_yaw = eulers:y()
        --_pitch = eulers:x()
    else
		-- Release scene and camera
		_scene = nil
		_vehicle = nil
		_cameraNodes = nil
	end
end

function camera_setSpeed(normal, fast)
    MOVE_SPEED_NORMAL = normal
    MOVE_SPEED_FAST = fast
end

function camera_update(elapsedTime)
	
    if not _useScriptCamera then
        return
    end

    if USE_PHYSICS_CHARACTER then

        local char = _scene:findNode("camera"):getCollisionObject():asCharacter()
        local speed = MOVE_SPEED_NORMAL
        if _moveFast then
            speed = MOVE_SPEED_FAST
        end

        -- Forward motion
        if _moveFlags[MOVE_FORWARD] then
            char:setForwardVelocity(speed)
        elseif _moveFlags[MOVE_BACKWARD] then
            char:setForwardVelocity(-speed)
        else
            char:setForwardVelocity(0)
        end

        -- Strafing
        if _moveFlags[MOVE_LEFT] then
            char:setRightVelocity(-speed)
        elseif _moveFlags[MOVE_RIGHT] then
            char:setRightVelocity(speed)
        else
            char:setRightVelocity(0)
        end

    else

        -- Manual camera movement
        local secs = elapsedTime / 1000.0

        _move:set(0,0)

        -- Forward motion
        if _moveFlags[MOVE_FORWARD] then
            _move:y(1)
        elseif _moveFlags[MOVE_BACKWARD] then
            _move:y(-1)
        end

        -- Strafing
        if _moveFlags[MOVE_LEFT] then
            _move:x(-1)
        elseif _moveFlags[MOVE_RIGHT] then
            _move:x(1)
        end

        if not _move:isZero() then
            local speed = MOVE_SPEED_NORMAL
            if _moveFast then
                speed = MOVE_SPEED_FAST
            end

            _move:normalize():scale(secs * speed)

            camera_moveForward(_move:y());
            camera_moveRight(_move:x());
        end

    end
end

function camera_keyEvent(evt, key)

    if not _useScriptCamera then
        return
    end

    if evt == Keyboard.KEY_PRESS then
        if key == Keyboard.KEY_W or key == Keyboard.KEY_CAPITAL_W then
            _moveFlags[MOVE_FORWARD] = true
        elseif key == Keyboard.KEY_S or key == Keyboard.KEY_CAPITAL_S then
            _moveFlags[MOVE_BACKWARD] = true
        elseif key == Keyboard.KEY_A or key == Keyboard.KEY_CAPITAL_A then
            _moveFlags[MOVE_LEFT] = true
        elseif key == Keyboard.KEY_D or key == Keyboard.KEY_CAPITAL_D then
            _moveFlags[MOVE_RIGHT] = true
        elseif key == Keyboard.KEY_SHIFT then
            _moveFast = true
        end
    elseif evt == Keyboard.KEY_RELEASE then
        if key == Keyboard.KEY_W or key == Keyboard.KEY_CAPITAL_W then
            _moveFlags[MOVE_FORWARD] = false
        elseif key == Keyboard.KEY_S or key == Keyboard.KEY_CAPITAL_S then
            _moveFlags[MOVE_BACKWARD] = false
        elseif key == Keyboard.KEY_A or key == Keyboard.KEY_CAPITAL_A then
            _moveFlags[MOVE_LEFT] = false
        elseif key == Keyboard.KEY_D or key == Keyboard.KEY_CAPITAL_D then
            _moveFlags[MOVE_RIGHT] = false
        elseif key == Keyboard.KEY_SHIFT then
            _moveFast = false
        end
    end

end

function camera_touchEvent(evt, x, y, contactIndex)

    if not _useScriptCamera then
        return
    end
    --print("Lua touch event at ", x, ',', y)

    if evt == Touch.TOUCH_PRESS then
        if true then --contactIndex == 0 then
            _touch:set(x, y)
            print('touch down at', x, y)
        elseif contactIndex == 1 then
            _moveFlags[MOVE_FORWARD] = true
        elseif contactIndex == 2 then
            _moveFast = true
        end
    elseif evt == Touch.TOUCH_RELEASE then
        if contactIndex == 1 then
            _moveFlags[MOVE_FORWARD] = false
        elseif contactIndex == 2 then
            _moveFast = false
        end
    elseif evt == Touch.TOUCH_MOVE then
    	if true then --contactIndex == 0 then
	        _delta:set(-x + _touch:x(), -y + _touch:y())
	        _touch:set(x, y)
	        _pitch = _pitch - math.rad(_delta:y() * 0.5)
	        _yaw = _yaw - math.rad(_delta:x() * 0.5)
		    
		    camera_setPosition()
	    end
    end
end

function camera_setNode(nodeID)
	if nodeID == nil then
		_node = nil
	else
		_node = _scene:findNode(nodeID)
	end
end

function camera_rotateTo(yaw, pitch)
	_pitch = pitch
	_yaw = yaw
	camera_setPosition()
end

function camera_setVectors(_eyeVec, _targetVec, _upVec)
	_eye:set(_eyeVec)
	_target:set(_targetVec)
	_up:set(_upVec)
	print('camera now at', _eye:x(), _eye:y(), _eye:z())
	print('camera looking at', _target:x(), _target:y(), _target:z())
	print('camera up is', _up:x(), _up:y(), _up:z())
	camera_set()
end

--call when _pitch, _yaw, or _radius has been changed, to set the camera position accordingly
function camera_setPosition()
	-- Create lookAt matrix
	_eye:set(_radius*math.cos(_yaw)*math.cos(_pitch), _radius*math.sin(_pitch), _radius*math.sin(_yaw)*math.cos(_pitch))
	_target:set(0.0, 0.0, 0.0)
	_up:set(0.0, 1.0, 0.0)
	if(_node ~= nil) then
		_node:getRotation(_nodeRotation)
		_nodeTranslation:set(_node:getTranslationWorld())
		_nodeRotation:transformVector(_eye)
		_nodeRotation:transformVector(_up)
		_eye:add(_nodeTranslation)
		_target:add(_nodeTranslation)
	end
	camera_set()
end

function camera_set()
	Matrix.createLookAt(_eye, _target, _up, _eyeMatrix)
	_eyeMatrix:invert();

	-- Pull SRT components out of matrix
	_eyeMatrix:decompose(_scale, _rotation, _translation)
	-- Set SRT on node
	for ind,_cameraNode in ipairs(_cameraNodes) do
		_cameraNode:setScale(_scale)
		_cameraNode:setTranslation(_translation)
		_cameraNode:setRotation(_rotation)
	end
end

function camera_setRadius(radius)
	_radius = radius
	camera_setPosition()
end

function camera_moveForward(by)
	for ind,_cameraNode in ipairs(_cameraNodes) do
		local v = _cameraNode:getForwardVector()
		v:normalize():scale(by)
		_cameraNode:translate(v)
	end
end

function camera_moveRight(by)
	for ind,_cameraNode in ipairs(_cameraNodes) do
		local v = _cameraNode:getRightVector()
		v:normalize():scale(by)
		_cameraNode:translate(v)
	end
end

function camera_quatToEuler(quat)
    local qx = quat:x()
    local qy = quat:y()
    local qz = quat:z()
    local qw = quat:w()
    local qx2 = qx * qx
    local qy2 = qy * qy
    local qz2 = qz * qz
    local qw2 = qw * qw

    local rotx = 0
    local roty = 0
    local rotz = 0

    if (qx*qy + qz*qw) == 0.5 then
        rotx = 0
        roty = 2 * math.atan2(qx, qw)
    elseif (qx*qy + qz*qw) == -0.5 then
        rotx = 0
        roty = -2 * math.atan2(qx, qw)
    else
        rotx = math.atan2(2*qx*qw-2*qy*qz , 1 - 2*qx2 - 2*qz2)
        roty = math.atan2(2*qy*qw-2*qx*qz , 1 - 2*qy2 - 2*qz2)
    end

    rotz = math.asin(2*qx*qy + 2*qz*qw)

    return Vector3.new(rotx, roty, rotz)
end
