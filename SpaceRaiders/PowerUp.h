#pragma once

#include "GameObjects.h"

typedef enum
{
	BT_MovementSpeed = 0,
	BT_FasterShots,
	BT_TripleShots
} PowerUpType;

class PlayField;
class PowerUp : public GameObject
{
protected:
	int m_timeLeft = -1;
	PowerUpType m_powerUpType;
	bool m_isCatched;
	virtual void OnObjectDestroyed(GameObject& attacker, PlayField& world, const Vector2D& collisionPoint);
public:
	PowerUp(Vector2D pos, int timeLeft, PowerUpType powerUpType) : 
		GameObject(RI_PowerUp, pos, RS_PowerUp, 1, 0),
		m_timeLeft(timeLeft), m_powerUpType(powerUpType),
		m_isCatched(false)
	{
		static UINT32 colTypes = SetupCollidingObjects({ RI_Player});
		m_collisionTypeBitmap = colTypes;
	}
	
	bool Tick() 
	{ 
		// infinite power-ups are marekd with m_timeLeft == -1
		if (m_isCatched && m_timeLeft != -1 && --m_timeLeft == 0)
		{
			OnPowerUpExpired();
			return false;
		}
		return true;
	}
	virtual void Update(PlayField& world);
	// if player will catch power-up of the same type 
	// that he catched previosly and this power-up
	// is still active, then Merge(...) function will be invoked
	// on previously catched power-up with newly catched power-up
	// passed as argument (without this, powe-up benefits would expire
	// at certain time without taking into account that we catched
	// the same power-up in the meantime what should refresh power-up expiration timer)
	virtual void Merge(PowerUp& powerUp)
	{
		if (powerUp.GetType() == m_powerUpType && m_timeLeft != -1)
		{
			m_timeLeft += powerUp.m_timeLeft;
		}
	}
	virtual void OnPowerUpCatched() = 0;
	virtual void OnPowerUpExpired() {};
	PowerUpType GetType() { return m_powerUpType; }
};

class MovementSpeedPowerUp : public PowerUp
{
protected:
	PlayerShip& m_playerShip;
public:
	MovementSpeedPowerUp(PlayerShip& playerShip, Vector2D pos) :
		PowerUp(pos, 300, BT_MovementSpeed),
		m_playerShip(playerShip)
	{}
	virtual void OnPowerUpCatched()
	{
		m_playerShip.SetMovementSpeed(1.5f);
	}
	virtual void OnPowerUpExpired()
	{
		m_playerShip.SetMovementSpeed(1.f);
	}
};

class FasterShotsPowerUp : public PowerUp
{
protected:
	PlayField& m_world;
public:
	FasterShotsPowerUp(PlayField& world, Vector2D pos) :
		PowerUp(pos, -1, BT_FasterShots),
		m_world(world)
	{}
	virtual void OnPowerUpCatched();
	virtual void OnPowerUpExpired();
};

class TripleShotsPowerUp : public PowerUp
{
protected:
	PlayField & m_world;
public:
	TripleShotsPowerUp(PlayField& world, Vector2D pos) :
		PowerUp(pos, -1, BT_TripleShots),
		m_world(world)
	{}
	virtual void OnPowerUpCatched();
	virtual void OnPowerUpExpired();
};

