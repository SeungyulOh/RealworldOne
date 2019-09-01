#pragma once

#include <basetsd.h> // for UINT32
#include "Vector2D.h"
#include <functional>
#include <vector>

enum RaiderObjectTypeId
{
	RI_Player = 0,
	RI_Alien,
	RI_PlayerLaser,
	RI_AlienLaser,
	RI_Explosion,
	RI_WallBlock,
	RI_PowerUp,
	RI_ExplosionCell,
	RI_End
};

enum RaiderSprites
{
	RS_TakeDefault = 0xff,
	RS_BackgroundTile = ' ',// I didn't like this 0xB0, so replacing this with space
	RS_Player = 'P',
	RS_Alien = 'A',
	RS_BetterAlien = 'B',
	RS_PlayerLaser = '^',
	RS_AlienLaser = '|',
	RS_StrongAlienLaser = 'w',
	RS_Explosion = '*',
	RS_WallBlock = 'X',
	RS_PlayerLaserLR = 'o',
	RS_PowerUp = '7', // lucky 7
	RS_ExplosionCell = '+',
	RS_ExplodingAlien = 'E',
	RS_Unknown = '?'
};

typedef struct
{
	RaiderSprites sprite;
	const char *name;
} GameObjectInfo;

GameObjectInfo* getObjectInfo(RaiderObjectTypeId objectTypeId);

class PlayField;
class GameObject
{
protected:
	// we can afford simple 32-bit bitmap since we have less then 32 types of game objects
	UINT32 m_collisionTypeBitmap;
	bool m_isActive;
	bool m_isAutoDelete;
	RaiderObjectTypeId m_objType;
	Vector2D m_pos;
	Vector2D m_posPrev;
	int m_strikeForce;
	int m_health;
	unsigned char m_sprite;
	const char *m_name;
	inline bool IsCollidingWithObject(GameObject& other) 
	{
		return (m_collisionTypeBitmap & (1 << (other.m_objType))) > 0;
	}
	static UINT32 SetupCollidingObjects(std::initializer_list<RaiderObjectTypeId> l);
	void UnsetCollidingObject(RaiderObjectTypeId objType) { m_collisionTypeBitmap &= ~(1U << objType); }
	// CheckCollision(...), OnObjectStriked(...) and OnObjectDestroyed(...) are
	// methods that have an intention to make collision handling implemenation
	// more flexible.
	// In general case, they are invoked in the following order:
	//	CheckCollision(...) [ invoked by PlayField if it detected that 2 objects has touched each other ]
	//		OnObjectStriked(...) [if CheckCollision detected that attacking object is able to collid with "this" object based on attacker type]
	//			OnObjectDestroyed(...) [if OnObjectStriked detected that objects health dropped to 0 or below]
	virtual void OnObjectDestroyed(GameObject& attacker, PlayField& world, const Vector2D& collisionPoint) {}
	virtual void OnObjectStriked(GameObject& attacker, PlayField& world, const Vector2D& collisionPoint);
public:
	GameObject(RaiderObjectTypeId objectType, Vector2D pos, unsigned char sprite, int health, int strikeForce);
	virtual ~GameObject(){}
	virtual void GetCollisionPoints(std::vector<Vector2D>& collisionVectorOut);
	virtual void Update(PlayField& world) {}
	virtual void CheckCollision(GameObject& other, PlayField& world, const Vector2D& collisionPoint);
	void SetToInactive() { m_isActive = false; }
	bool IsActive() { return m_isActive; }
	void SetAutoDelete(bool isEnabled) { m_isAutoDelete = isEnabled; }
	bool IsAutoDelete() { return m_isAutoDelete; }
	RaiderObjectTypeId GetType() { return m_objType; }
	const Vector2D& GetPos() { return m_pos; }
	const Vector2D& GetPosPrev() { return m_posPrev; }
	int GetStrikeForce() { return m_strikeForce; }
	unsigned char GetSprite() { return m_sprite; }
};

typedef GameObject* GameObjPtr;

class StringObject
{
protected:
	Vector2D m_pos;
	std::string m_str;
public:
	StringObject(Vector2D pos) : m_pos(pos) {}
	StringObject() : StringObject(Vector2D(0, 0)) {}
	StringObject(Vector2D pos, const char *str) : m_pos(pos), m_str(str) {}
	Vector2D& GetPos() { return m_pos; }
	std::string& GetStr() { return m_str; }
};

class Explosion : public GameObject
{
protected:
	int m_timer = 5;
public:
	// Explosion lasts 5 ticks before it dissappears
	Explosion(Vector2D pos);
	void Update(PlayField& world);
};

typedef const std::function <bool(GameObject&, PlayField&)> IsObjectValidFunc;

class Laser : public GameObject
{
protected:
	IsObjectValidFunc m_isValidFunc;
	Vector2D m_direction;
	virtual void OnObjectStriked(GameObject& attacker, PlayField& world, const Vector2D& collisionPoint);
	virtual void OnObjectDestroyed(GameObject& attacker, PlayField& world, const Vector2D& collisionPoint);
	Laser(RaiderObjectTypeId objectType, Vector2D pos, Vector2D direction, unsigned char sprite, IsObjectValidFunc isValidFunc);
public:
	virtual void Update(PlayField& world);
};

class AlienLaser : public Laser
{
public:
	AlienLaser(GameObject *parent);
};

class StrongAlienLaser : public AlienLaser
{
public:
	StrongAlienLaser(GameObject *parent);
};

class PlayerLaser : public Laser
{
public:
	PlayerLaser(GameObject *parent);
};

// player laser left-right
class PlayerLaserLR : public Laser
{
public:
	PlayerLaserLR(GameObject *parent, bool isLeft);
};


class SpaceShip : public GameObject
{
protected:
	virtual void OnObjectDestroyed(GameObject& attacker, PlayField& world, const Vector2D& collisionPoint);
	SpaceShip(RaiderObjectTypeId objectType, Vector2D pos, unsigned char sprite, int health, int strikeForce)
		: GameObject(objectType, pos, sprite, health, strikeForce) {}
};

class Alien : public SpaceShip
{
protected:
	bool m_isTransformationEnabled = true;
	virtual void OnObjectDestroyed(GameObject& attacker, PlayField& world, const Vector2D& collisionPoint);
public:
	Alien(Vector2D pos, float velocityY, bool enableFriendFire);
	virtual void Update(PlayField& world);
private:
	const float m_maxUpdateRate = 0.01f;
	float m_transformEnergy;
	enum AlienState
	{
		as_Normal,
		as_Better
	};
	float m_energy = 0.f;
	float m_direction;
	float m_velocityX = 0.5f;
	float m_velocityY = 0.02f;
	float m_fireRateBorder = 0.5f;
	AlienState m_state = as_Normal;

	void Transform();
};

class PlayerShip : public SpaceShip
{
protected:
	float m_movementSpeed = 1.f;
	bool m_useTripleShots = false;
	virtual void OnObjectDestroyed(GameObject& attacker, PlayField& world, const Vector2D& collisionPoint);
	std::vector<Vector2D> m_collisionPoints;
public:
	PlayerShip(Vector2D pos);
	void SetMovementSpeed(float speed) { m_movementSpeed = speed; }
	void SetTripleShots(bool areEnabled) { m_useTripleShots = areEnabled; }
	void Update(PlayField& world);
	virtual void GetCollisionPoints(std::vector<Vector2D>& collisionVectorOut);
};

class WallBlock : public GameObject
{
protected:
	virtual void OnObjectStriked(GameObject& attacker, PlayField& world, const Vector2D& collisionPoint);
	virtual void OnObjectDestroyed(GameObject& attacker, PlayField& world, const Vector2D& collisionPoint);
public:
	WallBlock(Vector2D pos);
};