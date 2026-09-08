#pragma once

#include "physicsentity.h"
#include "sprite.h"
#include "slopes.h"
#include "game.h"
#include "playercharacter.h"

class Player : public PhysicsEntity
{
public:
    Player(Room* room, const Game& game);
    void setCharacter(PlayerCharacter newCharacter);

private:
    void handleDirection();
	void handleSlopes();
	void handlePMeter();
	void handleWalking();
	void handleStopping();
	void handleDucking();
	void handleLookingUp();
	void handleJumping();
	void handleSliding();
	void handleFalling();
    void handleAnimation();
    void handleSkidSmoke();
    void handleFloorAnimations();
    void handleAirAnimations();

private:
    bool isSlipperyLevel();
    bool duckingOnFloor();
    bool isHoldingBackwards();
    bool isHoldingForwards();
    bool runButtonHeld();
    float getMaxSpeed();
    float getBaseAccel();
    float getBaseDecel();
    float getJumpSpeed();

private:
    bool isOnSlope();
    int getSlopeDirection();
    float getSlopeSlideSpeed();
    float getSlopeAutoWalkSpeed();
    float getSlopeMaxSpeedModifier();
    float getSlopeSlideAccel();
    float getSlopeSlipperyAccel();
    float getSlopeAutoWalkAccelModifier();
    float getSlopeAccelModifier();
    float getSlopeDecelModifier();

private:
    void accelerate(float baseAccel, float maxSpeed);
    void decelerate(float baseDecel);

public:
    PlayerCharacter character = PlayerCharacter::MARIO;

    // current moving direction
    int direction = 0;

    // current facing direction (only updates when a direction is prsesed)
    int facingDirection = 1;

    // p meter (used to track the state of max running speed)
    float p_meter = 0.0f;

    // whether player is currently jumping
    // different from just being in the air. animation purposes
    bool jumping = false;

    // whether mario is currently spin jumping
    // in addition to jumping. so is only true when jumping is true
    bool spinJumping = false;

    // whether the jump was started with a full p meter
    // this is to make sure it always shows the flying animation for the whole duration of the jump
    // even when the p meter counts down during the jump
    // same as sipn jumping, this is true when jumping is true
    bool jumpingWithFullPMeter = false;

    // whether player is currently ducking
    bool ducking = false;

    // whether player is currently looking up
    bool lookingUp = false;

    // the type of slope the player is currently on
    SlopeType onSlopeType = SlopeType::NONE;

    // whether the player is currently sliding from pressing DOWN on a slope
    bool sliding = false;

    // whether the sliding animation should play when the player is sliding
    // this is needed because very steep slopes force the player to b e sliding
    // but this does not play the sliding animation
    bool doSlideAnimation = false;

    // when the player walks against the wall & vel = 0 it still makes him walk about like a dipshit
    bool walkingAgainstWall = false;

    // whether the player is in the air after shitting out of a diagonal pipe
    // this stays true even after gameplay_state is no longer SHOOTING_FROM_DIAGONAL_PIPE
    // until the player hits the ground
    bool inAirFromDiagonalPipe = false;

    // whether the player has jumped on an enemy in the current frame
    // used to prevent jumps falsely counting as damage and also so the player doesnt
    // hit multiple enemies
    bool jumpedOnEnemyThisFrame = false;

    // number of enemies the player has jumped on without touching the ground
    int consecutiveBounces = 0;

    // whether the player should keep their jump state (including whether theyre spin jumping)
    // for one more frame even after hitting the ground
    // this is needed at least in the most straightforward way, to make block interaction work
    // eg not losing the spin jump state when he spin breaks a turn block
    bool shouldKeepJumpState = false;

    // is mario allowed to jump
    bool canJump = true;

    // used for timing the flashing animation when the player goes owieee
    float flashCounter = 0.0f;

    int smokeTimer = 0;
    int scuttleTimer = 0;

	Sprite sprite;

public:
    void step() override;
    void draw(sf::RenderTarget& target, float interp) override;

public:
    bool isPMeterFull();

protected:
    void onCeilingHit() override;
};