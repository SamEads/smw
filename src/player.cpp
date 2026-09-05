#include "player.h"
#include "room.h"
#include "keys.h"
#include "mathhelper.h"

#define FIX (1.0f / 60.0f) *

constexpr float WALK_SPEED                  = 1.25f;
constexpr float RUN_SPEED                   = 2.25f;
constexpr float P_METER_RUN_SPEED           = 3.0f;

constexpr float WALK_ACCEL                  = 0.09375f;
constexpr float RUN_ACCEL                   = 0.09375f;
constexpr float STOP_DECEL                  = 0.0625f;
constexpr float WALK_DECEL                  = 0.15625f;
constexpr float RUN_DECEL                   = 0.3125f;
constexpr float WALK_ACCEL_SLIPPERY         = 0.03125f;
constexpr float RUN_ACCEL_SLIPPERY          = 0.09375f;
constexpr float STOP_DECEL_SLIPPERY         = 0.005f;
constexpr float WALK_DECEL_SLIPPERY         = 0.015625f;
constexpr float RUN_DECEL_SLIPPERY          = 0.15625f;


constexpr float P_METER_MAX                 = 112;
constexpr float P_METER_START_SPEED         = 2.1875f;

constexpr float FALL_SPEED_CAP              = 4.0f;
constexpr float JUMP_SPEED                  = 5.0f;
constexpr float SPIN_JUMP_SPEED             = 4.625f;
constexpr float JUMP_SPEED_INCREASE         = 0.15625f;
constexpr float GRAVITY                     = 0.375f;
constexpr float GRAVITY_JUMP                = 0.1875f;

constexpr float SPIN_JUMP_SPEED_INCREASE    = FIX 8.617875; // ??>??

void Player::step()
{
    handleDirection();
	handleSlopes();
	handlePMeter();
	handleWalking();
	handleStopping();
	handleDucking();
	handleLookingUp();
	handleJumping();
	handleSliding();
	handleFalling();
	// handleFireballs();
	
	//  # elif _entering_pipe():
	//  	# _handle_entering_pipe()
	//  # elif _exiting_pipe():
	//  	# _handle_exiting_pipe()
	//  # elif gameplay_state == GameplayState.DYING:
	//  	# _handle_dying(delta)
	
	handleAnimation();
	
	bool wasAtWall = isAtWall;
		
	// cliffnote's of wye's note:
	// before calling move_and_slide adjust the x speed if on a slope then readjust it afterwards
	// in super mario world all speed calcs are done assuming mario's Xspd is how fast he moves
	// along the x axis (purely horizontal) even on a slope
	// however in godot, it's how fast he moves along the surfaec of the slope
	// to make the speeds work here like in SMW we need to factor in how "horizontal" the slope is
	
	// do this right before and right after move and slide so all other calcs can be done normally
	
	// good thas an option - floor_constant_speed - that somewhat covers this issue but that would
	// apparently change the default speeds on slopes n interfere with implementing smw values
	
	// so after move and slide rescale the value instead of restoring from a backup
	// since move&slide can change this
	// hspd /= Slopes.horz_component(slope_type)
	
	float tmp = hspd;
	// hspd = floor(hspd / 3.75f) * 3.75f;
	
	moveAndSlide();

    // temp barrier
	if (x < 8)
    {
		x = 8;
		hspd = 0;
        isAtWall = true;
        walkingAgainstWall = true;
    }
	
	hspd = tmp;
	// hspd *= Slopes.horz_component(slope_type)
	
	walkingAgainstWall = (isAtWall || wasAtWall) && direction != 0;
}

bool Player::isPMeterFull()
{
    return p_meter >= P_METER_MAX;
}

void Player::handleDirection()
{
    if (Keys::held(sf::Keyboard::Scancode::Left) && Keys::held(sf::Keyboard::Scancode::Right))
        direction = 1;
    else
    {
        direction = 0;
        if (Keys::held(sf::Keyboard::Scancode::Left))
            direction = -1;
        if (Keys::held(sf::Keyboard::Scancode::Right))
            direction = 1;
    }
    if (direction != 0 && !duckingOnFloor())
    {
        facingDirection = direction;
    }
    sprite.flipX = facingDirection < 0;
}

void Player::handleSlopes()
{
    // TODO: Slopes
}

void Player::handlePMeter()
{
    if (fabsf(hspd) >= P_METER_START_SPEED)
    {
        if (isOnFloor && runButtonHeld())
        {
            p_meter += 2;
        }
        else if (!jumpingWithFullPMeter)
        {
            p_meter -= 1;
        }
    }
    else
    {
        p_meter -= 1;
    }
    p_meter = MathHelper::clamp(p_meter, 0, P_METER_MAX);
}

void Player::handleWalking()
{
    float maxSpd = getMaxSpeed();
    float accel = getBaseAccel();
    float decel = getBaseDecel();
    float forceDecel = (isSlipperyLevel()) ? STOP_DECEL_SLIPPERY : STOP_DECEL;

    if (!isOnFloor && direction == 0)
        return;

    if (isOnSlope() && (sliding || !isHoldingBackwards()))
    {
        if (sliding)
        {
            maxSpd = fabsf(maxSpd) * getSlopeDirection();
        }
        if (fabsf(hspd) >= fabsf(maxSpd))
        {
            decelerate(forceDecel);
        }
        else
        {
            accelerate(accel, maxSpd);
        }
        return;
    }

    if (duckingOnFloor() || direction == 0)
    {
        decelerate(forceDecel);
        return;
    }

    if ((fabsf(hspd) < fabsf(maxSpd)) && isHoldingForwards())
    {
        accelerate(accel, maxSpd);
        return;
    }

    if (isHoldingBackwards())
    {
        decelerate(decel);
        return;
    }

    if ((fabsf(hspd) >= fabsf(maxSpd)) && (isOnFloor || isHoldingForwards()))
    {
        decelerate(forceDecel);
        return;
    }
}

void Player::handleStopping()
{
    if (isAtWall)
    {
        // todo...
        hspd = 0.0f;
    }
}

void Player::handleDucking()
{
	if (!isOnFloor)
		return;
	if (spinJumping || sliding || isOnSlope())
    {
		ducking = false;
		return;
    }
		
	// TODO: steal swim

	ducking = Keys::held(sf::Keyboard::Scancode::Down);
}

void Player::handleLookingUp()
{
    if (!isOnFloor)
    {
        lookingUp = false;
        return;
    }
    bool touchHoriz = Keys::held(sf::Keyboard::Scancode::Left) || Keys::held(sf::Keyboard::Scancode::Right);
    bool lookUpButton = Keys::held(sf::Keyboard::Scancode::Up);
    lookingUp = !touchHoriz && lookUpButton;
}

void Player::handleJumping()
{
	if (isOnFloor)
    {
		consecutiveBounces = 0;
		inAirFromDiagonalPipe = false;
    }

	if (!isOnFloor || !canJump)
		return;

	if (!shouldKeepJumpState)
    {
		jumping = false;
		spinJumping = false;
		jumpingWithFullPMeter = false;
    }
	shouldKeepJumpState = false;
	
	if (!Keys::pressed(sf::Keyboard::Scancode::X) && !Keys::pressed(sf::Keyboard::Scancode::C))
		return;
		
	jumping = true;
	if (isPMeterFull())
    {
		jumpingWithFullPMeter = true;
		// running_jump_animation_timer.start()
    }
	
	if (Keys::pressed(sf::Keyboard::Scancode::C))
    {
		spinJumping = true;
		// AudioManager.play_sfx(AudioManager.SoundEffect.SPIN_JUMP);
    }
	else
    {
		// AudioManager.play_sfx(AudioManager.SoundEffect.JUMP);
    }
		
	vspd = getJumpSpeed();
}

void Player::handleSliding()
{

}

void Player::handleFalling()
{
	float gravity = GRAVITY;
	if (Keys::held(sf::Keyboard::Scancode::X) || Keys::held(sf::Keyboard::Scancode::C))
		gravity = GRAVITY_JUMP;
	vspd = MathHelper::min(vspd, FALL_SPEED_CAP);
	vspd += gravity;
}

void Player::handleAnimation()
{
	if (ducking)
    {
		sprite.play("crouch");
		return;
    }
	if (sliding)
    {
		sprite.play("slide");
		return;
    }
	if (isOnFloor)
		handleFloorAnimations();
	else
		handleAirAnimations();
}

void Player::handleFloorAnimations()
{
    if (hspd == 0 && direction == 0)
    {
		if (lookingUp)
			sprite.play("idle_up");
        else
        {
			sprite.play("idle");
        }
		return;
    }

	if (isHoldingBackwards())
    {
		sprite.play("skid");
		return;
    }
	
	if (!isPMeterFull())
    {
		sprite.play("walk");
    }
	else
		sprite.play("run");
    if (walkingAgainstWall)
    {
        sprite.frame += 0.125f;
    }
    else
    {
        sprite.frame += MathHelper::max(0.125f, fabsf(hspd) * 0.125f);
    }
}

void Player::handleAirAnimations()
{
    if (spinJumping)
    {
        sprite.play("spin");
        sprite.frame += 0.5f;
        return;
    }
    if (jumping)
    {
        if (jumpingWithFullPMeter)
        {
            sprite.play("jump_run");
        }
        else if (vspd < 0.0f)
        {
            sprite.play("jump");
            sprite.frame += 0.3f;
        }
        else
        {
            sprite.play("fall");
            sprite.frame += 0.3f;
        }
        return;
    }
    sprite.play("fall");
    sprite.frame += 0.3f;
}

void Player::moveAndSlide()
{
    isOnFloor = false;
    isAtWall = false;

    x += hspd;
    for (auto& c : room->collisions)
    {
        sf::FloatRect wall(
            { c.x, c.y },
            { c.width, c.height }
        );

        sf::FloatRect playerRect(
            { x - 5, y - 10 },
            { 10, 10 }
        );

        auto hit = wall.findIntersection(playerRect);

        if (hit.has_value())
        {
            if (hspd > 0.0f)
            {
                // moving right
                x = c.x - 5;
            }
            else if (hspd < 0.0f)
            {
                // moving left
                x = c.x + c.width + 5;
            }

            hspd = 0.0f;
            isAtWall = true;
        }
    }

    y += vspd;

    for (auto& c : room->collisions)
    {
        bool oneWay = c.height <= 2.0f;
        float colHeight = oneWay ? 5.0f : c.height;
        sf::FloatRect wall(
            { c.x, c.y },
            { c.width, colHeight }
        );

        sf::FloatRect playerRect(
            { x - 5, y - 10 },
            { 10, 10 }
        );

        auto hit = wall.findIntersection(playerRect);

        if (hit.has_value())
        {
            if (oneWay)
            {
                if (vspd >= 0.0f)
                {
                    y = c.y;
                    vspd = 0.0f;
                    isOnFloor = true;
                }
                continue;
            }
            if (vspd > 0.0f)
            {
                // falling
                y = c.y; // because bottom of player is player.y
                isOnFloor = true;
            }
            else if (vspd < 0.0f)
            {
                // moving upward
                y = c.y + c.height + 10;
            }

            vspd = 0.0f;
        }
    }
}

bool Player::isSlipperyLevel()
{
    return false;
}

bool Player::isOnSlope()
{
    return onSlopeType != SlopeType::NONE;
}

bool Player::duckingOnFloor()
{
    return ducking && isOnFloor;
}

bool Player::runButtonHeld()
{
    return Keys::held(sf::Keyboard::Scancode::Z);
}

float Player::getMaxSpeed()
{
	if (sliding)
		return getSlopeSlideSpeed();
	if (direction == 0)
		return getSlopeAutoWalkSpeed();

	float baseMaxSpeed = 0.0f;
	if (isPMeterFull())
    {
		baseMaxSpeed = P_METER_RUN_SPEED;
    }
	else if (runButtonHeld())
    {
		baseMaxSpeed = RUN_SPEED;
    }
	else
    {
		baseMaxSpeed = WALK_SPEED;
    }
		
	return float(direction) * fabsf(baseMaxSpeed + getSlopeMaxSpeedModifier());
}

float Player::getBaseAccel()
{
    if (isSlipperyLevel())
        return (runButtonHeld()) ? RUN_ACCEL_SLIPPERY : WALK_ACCEL_SLIPPERY;
    return (runButtonHeld()) ? RUN_ACCEL : WALK_ACCEL;
}

float Player::getBaseDecel()
{
    if (isSlipperyLevel())
        return (runButtonHeld()) ? RUN_DECEL_SLIPPERY : WALK_DECEL_SLIPPERY;
    return (runButtonHeld()) ? RUN_DECEL : WALK_DECEL;
}

float Player::getJumpSpeed()
{
	float base_speed = JUMP_SPEED;
	float speed_incr = JUMP_SPEED_INCREASE;
	if (spinJumping)
    {
		base_speed = SPIN_JUMP_SPEED;
		speed_incr = SPIN_JUMP_SPEED_INCREASE;
    }
	return -(base_speed + speed_incr * int(fabsf(hspd) * 2.0f));
}

int Player::getSlopeDirection()
{
    if (!isOnSlope()) return 0;
    // TODO
    return 0;
}

float Player::getSlopeSlideSpeed()
{
    return 0.0f;
}

float Player::getSlopeAutoWalkSpeed()
{
    return 0.0f;
}

float Player::getSlopeMaxSpeedModifier()
{
    return 0.0f;
}

float Player::getSlopeSlideAccel()
{
    return 0.0f;
}

float Player::getSlopeSlipperyAccel()
{
    return 0.0f;
}

float Player::getSlopeAutoWalkAccelModifier()
{
    return 0.0f;
}

float Player::getSlopeAccelModifier()
{
    return 0.0f;
}

float Player::getSlopeDecelModifier()
{
    return 0.0f;
}

bool Player::isHoldingBackwards()
{
    if (direction == 0) return false;
    return (float(direction) * hspd) < 0.0f;
}

bool Player::isHoldingForwards()
{
    if (direction == 0) return false;
    return direction * hspd >= 0.0f;
}

void Player::accelerate(float baseAccel, float maxSpeed)
{
    float finalAccel = 0.0f;
    if (sliding && isOnSlope())
    {
        finalAccel = getSlopeSlideAccel();
    }
    else if (isSlipperyLevel() && isOnSlope() && direction == 0)
    {
        finalAccel = getSlopeSlipperyAccel();
    }
    else if (direction == 0)
    {
        finalAccel = baseAccel + getSlopeAutoWalkAccelModifier();
    }
    else
    {
        finalAccel = baseAccel + getSlopeAccelModifier();
    }
    hspd = MathHelper::moveToward(hspd, maxSpeed, fabsf(finalAccel));
}

void Player::decelerate(float baseDecel)
{
    float finalDecel = baseDecel + getSlopeDecelModifier();
	hspd = MathHelper::moveToward(hspd, 0, fabsf(finalDecel));
}
