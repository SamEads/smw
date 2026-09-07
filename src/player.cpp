#include "player.h"
#include "room.h"
#include "keys.h"
#include "sound.h"
#include "particles/skidsmoke.h"
#include "mathhelper.h"

#define FIX (1.0f / 60.0f) *
#define ACCEL (1.0f / 3600.0f) *

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

constexpr float SPIN_JUMP_SPEED_INCREASE    = FIX 8.617875;

constexpr float SLOPE_GRADUAL_LIMIT         = 20.0f * 3.14159265f / 180.0f;
constexpr float SLOPE_NORMAL_LIMIT          = 30.0f * 3.14159265f / 180.0f;
constexpr float SLOPE_STEEP_LIMIT           = 55.0f * 3.14159265f / 180.0f;

Player::Player(Room *room) : PhysicsEntity(room)
{
    sprite.load("sprites/luigi_small.png", "sprites/luigi_small.json");
    sprite.setOrigin(16.0f, 32.0f);
    x = 48;
    y = 48;

    collider = sf::FloatRect({ -4.0f, -12.0f }, { 8.0f, 12.0f });
}

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
    handleSkidSmoke();
	
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
	
	move();

    // temp barrier
	if (x < 8)
    {
		x = 8;
		hspd = 0;
        isAtWall = true;
        walkingAgainstWall = true;
    }
	
	// hspd *= Slopes.horz_component(slope_type)
	
	walkingAgainstWall = (isAtWall || wasAtWall) && direction != 0;
}

void Player::draw(sf::RenderTarget &target)
{
    sprite.draw(target, std::floorf(x), std::floorf(y) + 1.0f);

    //sf::CircleShape feetMarker(2.0f);
    //feetMarker.setOrigin({ 2.0f, 2.0f });
    //feetMarker.setPosition({
    //    x + collider.position.x + collider.size.x * 0.5f,
    //    y + collider.position.y + collider.size.y
    //});
    //feetMarker.setFillColor(sf::Color::Blue);
    //target.draw(feetMarker);
}

void Player::onCeilingHit()
{
    Sound::play("sounds/bump.wav");
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
    if (!isOnFloor || !isOnSlopeSurface)
    {
        onSlopeType = SlopeType::NONE;
        return;
    }

    float angle = fabsf(slopeAngle);
    bool slopesDownRight = slopeAngle > 0.0f;
    if (angle < 8.0f * 3.14159265f / 180.0f)
        onSlopeType = SlopeType::NONE;
    else if (angle < SLOPE_GRADUAL_LIMIT)
        onSlopeType = slopesDownRight ? SlopeType::GRADUAL_RIGHT : SlopeType::GRADUAL_LEFT;
    else if (angle < SLOPE_NORMAL_LIMIT)
        onSlopeType = slopesDownRight ? SlopeType::NORMAL_RIGHT : SlopeType::NORMAL_LEFT;
    else if (angle < SLOPE_STEEP_LIMIT)
        onSlopeType = slopesDownRight ? SlopeType::STEEP_RIGHT : SlopeType::STEEP_LEFT;
    else
        onSlopeType = slopesDownRight ? SlopeType::VERY_STEEP_RIGHT : SlopeType::VERY_STEEP_LEFT;
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
    if (!spinJumping && !isOnFloor && airborneFrames > 0 &&
        !jumpingWithFullPMeter && !sliding && !ducking)
    {
        if (!Sound::isPlaying("sounds/scuttle.wav"))
        {
            Sound::play("sounds/scuttle.wav", MathHelper::randomFloat(0.9f, 1.0f), MathHelper::choose(0.9, 1.0, 1.1));
        }
    }

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
	
	if (Keys::pressed(sf::Keyboard::Scancode::C) && !ducking)
    {
		spinJumping = true;
        Sound::play("sounds/spin.wav");
		// AudioManager.play_sfx(AudioManager.SoundEffect.SPIN_JUMP);
    }
	else
    {
        Sound::play("sounds/jump.wav");
		// AudioManager.play_sfx(AudioManager.SoundEffect.JUMP);
    }
		
	vspd = getJumpSpeed();
}

void Player::handleSliding()
{
    bool onVerySteepSlope = onSlopeType == SlopeType::VERY_STEEP_LEFT ||
        onSlopeType == SlopeType::VERY_STEEP_RIGHT;

    if (direction != 0 || jumping || (sliding && hspd == 0.0f) ||
        (!onVerySteepSlope && !doSlideAnimation))
    {
        sliding = false;
    }

    if (onVerySteepSlope)
    {
        if (!sliding)
            doSlideAnimation = false;
        if (Keys::held(sf::Keyboard::Scancode::Down))
            doSlideAnimation = true;
        sliding = true;
    }
    else if (isOnSlope() && direction == 0 &&
        (Keys::held(sf::Keyboard::Scancode::Down) || sliding))
    {
        sliding = true;
        doSlideAnimation = true;
    }

    if (sliding)
        ducking = false;
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
    if (sliding && doSlideAnimation)
    {
		sprite.play("slide");
		return;
    }
	if (isOnFloor)
		handleFloorAnimations();
	else
		handleAirAnimations();
}

void Player::handleSkidSmoke()
{
    if (smokeTimer++ == 4)
    {
        smokeTimer = 0;
        if (isSlipperyLevel() || !isOnFloor) return;
        if (!isHoldingBackwards() && !((ducking || sliding) && fabsf(hspd) > 0.2f)) return;
        auto particle = std::make_unique<SkidSmoke>(room);
        particle->x = x + (-direction * 6);
        particle->y = y;
        room->addObject(std::move(particle));
    }
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
    {
		sprite.play("run");
    }

    if (walkingAgainstWall)
    {
        sprite.frame += 0.125f;
    }
    else
    {
        sprite.frame += MathHelper::max(0.125f, fabsf(hspd) * 0.15f);
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
            sprite.frame += 0.375f;
        }
        else
        {
            sprite.play("fall");
            sprite.frame += 0.375f;
        }
        return;
    }
    sprite.play("fall");
    sprite.frame += 0.375f;
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
    switch (onSlopeType)
    {
    case SlopeType::GRADUAL_LEFT:
    case SlopeType::NORMAL_LEFT:
    case SlopeType::STEEP_LEFT:
    case SlopeType::VERY_STEEP_LEFT:
        return -1;
    case SlopeType::GRADUAL_RIGHT:
    case SlopeType::NORMAL_RIGHT:
    case SlopeType::STEEP_RIGHT:
    case SlopeType::VERY_STEEP_RIGHT:
        return 1;
    default:
        return 0;
    }
}

float Player::getSlopeSlideSpeed()
{
    switch (onSlopeType)
    {
    case SlopeType::GRADUAL_LEFT:
    case SlopeType::GRADUAL_RIGHT: return FIX 150.0f * getSlopeDirection();
    case SlopeType::NORMAL_LEFT:
    case SlopeType::NORMAL_RIGHT: return FIX 165.0f * getSlopeDirection();
    case SlopeType::STEEP_LEFT:
    case SlopeType::STEEP_RIGHT: return FIX 180.0f * getSlopeDirection();
    case SlopeType::VERY_STEEP_LEFT:
    case SlopeType::VERY_STEEP_RIGHT: return FIX 120.0f * getSlopeDirection();
    default: return 0.0f;
    }
}

float Player::getSlopeAutoWalkSpeed()
{
    switch (onSlopeType)
    {
    case SlopeType::STEEP_LEFT: return FIX -60.0f;
    case SlopeType::STEEP_RIGHT: return FIX 60.0f;
    case SlopeType::VERY_STEEP_LEFT: return FIX -120.0f;
    case SlopeType::VERY_STEEP_RIGHT: return FIX 120.0f;
    default: return 0.0f;
    }
}

float Player::getSlopeMaxSpeedModifier()
{
    int movingDirection = (hspd > 0.0f ? 1 : hspd < 0.0f ? -1 : 0) * getSlopeDirection();
    switch (onSlopeType)
    {
    case SlopeType::NORMAL_LEFT:
    case SlopeType::NORMAL_RIGHT:
        if (movingDirection == -1) return FIX -15.0f;
        return isPMeterFull() || runButtonHeld() ? 0.0f : FIX 7.5f;
    case SlopeType::STEEP_LEFT:
    case SlopeType::STEEP_RIGHT:
        if (movingDirection == -1) return isPMeterFull() ? FIX -30.0f :
            runButtonHeld() ? FIX -30.0f : FIX -15.0f;
        return isPMeterFull() || runButtonHeld() ? 0.0f : FIX 60.0f;
    case SlopeType::VERY_STEEP_LEFT:
    case SlopeType::VERY_STEEP_RIGHT:
        if (movingDirection == -1) return isPMeterFull() ? FIX -195.0f :
            runButtonHeld() ? FIX -165.0f : FIX -135.0f;
        return isPMeterFull() || runButtonHeld() ? 0.0f : FIX 60.0f;
    default:
        return 0.0f;
    }
}

float Player::getSlopeSlideAccel()
{
    switch (onSlopeType)
    {
    case SlopeType::GRADUAL_LEFT:
    case SlopeType::GRADUAL_RIGHT: return ACCEL 112.5f;
    case SlopeType::NORMAL_LEFT:
    case SlopeType::NORMAL_RIGHT: return ACCEL 225.0f;
    case SlopeType::STEEP_LEFT:
    case SlopeType::STEEP_RIGHT: return ACCEL 337.5f;
    case SlopeType::VERY_STEEP_LEFT:
    case SlopeType::VERY_STEEP_RIGHT: return ACCEL 562.5f;
    default: return 0.0f;
    }
}

float Player::getSlopeSlipperyAccel()
{
    int movingDirection = (hspd > 0.0f ? 1 : hspd < 0.0f ? -1 : 0) * getSlopeDirection();
    switch (onSlopeType)
    {
    case SlopeType::GRADUAL_LEFT:
    case SlopeType::NORMAL_LEFT: return movingDirection == -1 ? ACCEL -56.25f : ACCEL 28.125f;
    case SlopeType::GRADUAL_RIGHT:
    case SlopeType::NORMAL_RIGHT: return movingDirection == -1 ? ACCEL 56.25f : ACCEL -28.125f;
    case SlopeType::STEEP_LEFT: return ACCEL 112.5f;
    case SlopeType::STEEP_RIGHT: return movingDirection == -1 ? ACCEL 112.5f : ACCEL -28.125f;
    case SlopeType::VERY_STEEP_LEFT: return ACCEL -450.0f;
    case SlopeType::VERY_STEEP_RIGHT: return ACCEL 450.0f;
    default: return 0.0f;
    }
}

float Player::getSlopeAutoWalkAccelModifier()
{
    float autoWalkSpeed = getSlopeAutoWalkSpeed();
    bool aboveAutoWalkSpeed = hspd == 0.0f ||
        ((hspd > 0.0f) == (autoWalkSpeed > 0.0f) && fabsf(hspd) >= fabsf(autoWalkSpeed));
    switch (onSlopeType)
    {
    case SlopeType::NORMAL_LEFT:
    case SlopeType::NORMAL_RIGHT: return aboveAutoWalkSpeed ? ACCEL -56.25f : ACCEL 112.5f;
    case SlopeType::STEEP_LEFT:
    case SlopeType::STEEP_RIGHT: return aboveAutoWalkSpeed ? ACCEL -168.75f : ACCEL 225.0f;
    case SlopeType::VERY_STEEP_LEFT:
    case SlopeType::VERY_STEEP_RIGHT: return aboveAutoWalkSpeed ? ACCEL -450.0f : ACCEL 675.0f;
    default: return 0.0f;
    }
}

float Player::getSlopeAccelModifier()
{
    int movingDirection = (hspd > 0.0f ? 1 : hspd < 0.0f ? -1 : 0) * getSlopeDirection();
    if (isSlipperyLevel())
    {
        switch (onSlopeType)
        {
        case SlopeType::NORMAL_LEFT:
        case SlopeType::NORMAL_RIGHT: return movingDirection == -1 ?
            (runButtonHeld() ? ACCEL 56.25f : 0.0f) : (runButtonHeld() ? 0.0f : ACCEL 225.0f);
        case SlopeType::STEEP_LEFT:
        case SlopeType::STEEP_RIGHT: return movingDirection == -1 ?
            (runButtonHeld() ? ACCEL -112.5f : 0.0f) : (runButtonHeld() ? 0.0f : ACCEL 225.0f);
        case SlopeType::VERY_STEEP_LEFT:
        case SlopeType::VERY_STEEP_RIGHT: return movingDirection == -1 ?
            (runButtonHeld() ? ACCEL -1012.5f : ACCEL -787.5f) :
            (runButtonHeld() ? ACCEL 562.5f : ACCEL 787.5f);
        default: break;
        }
    }
    switch (onSlopeType)
    {
    case SlopeType::NORMAL_LEFT:
    case SlopeType::NORMAL_RIGHT: return movingDirection == -1 ? ACCEL -56.25f : 0.0f;
    case SlopeType::STEEP_LEFT:
    case SlopeType::STEEP_RIGHT: return movingDirection == -1 ? ACCEL -112.5f : 0.0f;
    case SlopeType::VERY_STEEP_LEFT:
    case SlopeType::VERY_STEEP_RIGHT: return movingDirection == -1 ? ACCEL -1012.5f : ACCEL 562.5f;
    default: return 0.0f;
    }
}

float Player::getSlopeDecelModifier()
{
    int movingDirection = (hspd > 0.0f ? 1 : hspd < 0.0f ? -1 : 0) * getSlopeDirection();
    if (isSlipperyLevel())
    {
        switch (onSlopeType)
        {
        case SlopeType::NORMAL_LEFT:
        case SlopeType::NORMAL_RIGHT: return movingDirection == -1 ? ACCEL 56.25f :
            (runButtonHeld() ? ACCEL -56.25f : 0.0f);
        case SlopeType::STEEP_LEFT:
        case SlopeType::STEEP_RIGHT: return movingDirection == -1 ?
            (runButtonHeld() ? ACCEL 112.5f : ACCEL 618.75f) :
            (runButtonHeld() ? ACCEL -112.5f : 0.0f);
        case SlopeType::VERY_STEEP_LEFT:
        case SlopeType::VERY_STEEP_RIGHT: return movingDirection == -1 ?
            (runButtonHeld() ? ACCEL 112.5f : ACCEL 618.75f) :
            (runButtonHeld() ? ACCEL -1237.5f : ACCEL -731.25f);
        default: break;
        }
    }
    switch (onSlopeType)
    {
    case SlopeType::NORMAL_LEFT:
    case SlopeType::NORMAL_RIGHT: return movingDirection == -1 ?
        (runButtonHeld() ? ACCEL 112.5f : ACCEL 56.25f) :
        (runButtonHeld() ? ACCEL -112.5f : ACCEL -56.25f);
    case SlopeType::STEEP_LEFT:
    case SlopeType::STEEP_RIGHT: return movingDirection == -1 ?
        (runButtonHeld() ? ACCEL 225.0f : ACCEL 112.5f) :
        (runButtonHeld() ? ACCEL -225.0f : ACCEL -112.5f);
    case SlopeType::VERY_STEEP_LEFT:
    case SlopeType::VERY_STEEP_RIGHT: return movingDirection == -1 ?
        (runButtonHeld() ? ACCEL 225.0f : ACCEL 112.5f) :
        (runButtonHeld() ? ACCEL -2475.0f : ACCEL -1237.5f);
    default: return 0.0f;
    }
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
