#include "player.h"
#include "room.h"
#include "keys.h"
#include "sound.h"
#include "skidsmoke.h"
#include "mathhelper.h"

#include <string>

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

constexpr float SLOPE_GRADUAL_LIMIT         = 20.0f * M_PI / 180.0f;
constexpr float SLOPE_NORMAL_LIMIT          = 30.0f * M_PI / 180.0f;
constexpr float SLOPE_STEEP_LIMIT           = 55.0f * M_PI / 180.0f;

Player::Player(Room *room, const Game& game) : PhysicsEntity(room)
{
    setCharacter(game.playerCharacter);
    x = 48;
    y = 48;

    collider = sf::FloatRect({ -4.0f, -14.0f }, { 8.0f, 14.0f });
}
#include <iostream>
void Player::step()
{
    if (Keys::pressed(sf::Keyboard::Scancode::LShift))
    {
        setCharacter(character == PlayerCharacter::MARIO ?
            PlayerCharacter::LUIGI : PlayerCharacter::MARIO);
    }

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
	
    bool wasAtWall = isAtWall();

	move();

    // left barrier
	if (x < 8)
    {
		x = 8;
		hspd = 0;
        setAtWall();
        walkingAgainstWall = true;
    }
	
    walkingAgainstWall = (isAtWall() || wasAtWall) && direction != 0;
}

void Player::setCharacter(PlayerCharacter newCharacter)
{
    character = newCharacter;
    if (room->game)
        room->game->playerCharacter = newCharacter;

    const char* characterName = character == PlayerCharacter::MARIO ? "mario" : "luigi";
    sprite.load(
        std::string("sprites/") + characterName + "_small.png",
        std::string("sprites/") + characterName + "_small.json");
    sprite.setOrigin(16.0f, 32.0f);
}

void Player::draw(sf::RenderTarget &target, float interp)
{
    float xx = MathHelper::lerp(xPrevious, x, interp);
    float yy = MathHelper::lerp(yPrevious, y, interp);
    if (xPrevious != x) std::cout << xPrevious << "," << x << "\n";
    sprite.draw(target, std::floorf(xx), std::floorf(yy) + 1.0f);
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
    if (!isOnFloor() || !isOnSlopeSurface)
    {
        onSlopeType = SlopeType::NONE;
        return;
    }

    float angle = fabsf(slopeAngle);
    bool slopesDownRight = slopeAngle > 0.0f;
    if (angle < 8.0f * M_PI / 180.0f)
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
        if (isOnFloor() && runButtonHeld())
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

    if (!isOnFloor() && direction == 0)
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

    if ((fabsf(hspd) > fabsf(maxSpd)) && (isOnFloor() || isHoldingForwards()))
    {
        decelerate(forceDecel);
        return;
    }
}

void Player::handleStopping()
{
    if (isAtWall())
    {
        // todo...
        hspd = 0.0f;
    }
}

void Player::handleDucking()
{
    if (!isOnFloor())
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
    if (!isOnFloor())
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
    if (character == PlayerCharacter::LUIGI)
    {
        if (!spinJumping && !isOnFloor() &&
            !jumpingWithFullPMeter && !sliding && !ducking)
        {
            if (!Sound::isPlaying("sounds/scuttle.wav"))
            {
                Sound::play("sounds/scuttle.wav", MathHelper::randomFloat(0.9f, 1.0f), MathHelper::choose(0.9, 1.0, 1.1));
            }
        }
    }

    if (isOnFloor())
    {
		consecutiveBounces = 0;
		inAirFromDiagonalPipe = false;
    }

    if (!isOnFloor() || !canJump)
    {
		return;
    }

	if (!shouldKeepJumpState)
    {
        if (spinJumping)
        {
            facingDirection = MathHelper::choose(-1, 1);
        }
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

    if (!spinJumping && std::fabsf(hspd) >= P_METER_START_SPEED)
    {
        altScuttleTimer = std::floorf(std::fabsf(hspd) * 4.0f);
        leaveGroundSpeed = hspd;
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
    if (isOnFloor() || altScuttleTimer > 0)
		handleFloorAnimations();
	else
		handleAirAnimations();
}

void Player::handleSkidSmoke()
{
    if (smokeTimer++ == 4)
    {
        smokeTimer = 0;
        if (isSlipperyLevel() || !isOnFloor()) return;
        if (!isHoldingBackwards() && !((ducking || sliding) && fabsf(hspd) > 0.2f)) return;
        auto particle = room->create<SkidSmoke>(room);
        particle->x = x + (-direction * 6);
        particle->y = y;
    }
}

void Player::handleFloorAnimations()
{
    if (hspd == 0 && direction == 0 && isOnFloor())
    {
		if (lookingUp)
			sprite.play("idle_up");
        else
        {
			sprite.play("idle");
        }
		return;
    }

	if (isHoldingBackwards() && isOnFloor())
    {
		sprite.play("skid");
		return;
    }
	
    float legibleAngle = fabsf(slopeAngle) / M_PI * 180.0f;
	if (!isPMeterFull() || (isWalkingUpSlope() && legibleAngle >= 40.0f))
    {
		sprite.play("walk");
    }
	else
    {
		sprite.play("run");
    }

    float mult = (isPMeterFull()) ? 0.2f : 0.15f;
    if (altScuttleTimer > 0)
    {
        sprite.frame += MathHelper::max(0.125f, MathHelper::max(std::fabsf(hspd), std::fabsf(leaveGroundSpeed)) * mult);
    }
    else if (walkingAgainstWall)
    {
        sprite.frame += 0.125f;
    }
    else
    {
        sprite.frame += MathHelper::max(0.125f, fabsf(hspd) * mult);
    }

    if (altScuttleTimer > 0.0f) altScuttleTimer--;
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

bool Player::isWalkingUpSlope()
{
    int slopeDir = getSlopeDirection();
    if (slopeDir == 0 || direction == 0) return false;
    return getSlopeDirection() != direction;
}

bool Player::duckingOnFloor()
{
    return ducking && isOnFloor();
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
