
#include "stdafx.h"
#include "GameObjects.h"
#include "PlayField.h"
#include "Renderer.h"

static GameObjectInfo gGameObjectsInfoArr[] =
{
	{ RS_Player,		"ot_PlayerShip"		},
	{ RS_Alien,			"ot_AlienShip"		},
	{ RS_PlayerLaser,	"ot_PlayerLaser"	},
	{ RS_AlienLaser,	"ot_AlienLaser"		},
	{ RS_Explosion,		"ot_Explosion"		},
	{ RS_WallBlock,		"ot_WallBlock"		}
};

GameObjectInfo* getObjectInfo(RaiderObjectTypeId objectTypeId)
{
	static GameObjectInfo uknownObject = { RS_Unknown, "ot_UnknownObject" };
	if ((int)objectTypeId >= 0 && objectTypeId < RI_End)
	{
		return &gGameObjectsInfoArr[(int)objectTypeId];
	}
	return &uknownObject;
}

GameObject::GameObject(RaiderObjectTypeId objectType, Vector2D pos, unsigned char sprite, int health, int strikeForce) :
	m_isActive(true),
	m_objType(objectType),
	m_pos(pos),
	m_posPrev(pos),
	m_strikeForce(strikeForce),
	m_health(health),
	m_isAutoDelete(true)
{
	GameObjectInfo *info = getObjectInfo(objectType);
	m_sprite = sprite == RS_TakeDefault ? (char)info->sprite : sprite;
	m_name = info->name;
}

// by default, object potential collision points (cells that are added to collision map)
// are its current position and its previous position (if it is not equal to its current position)
void GameObject::GetCollisionPoints(std::vector<Vector2D>& collisionVectorOut)
{
	collisionVectorOut.push_back(m_pos);
	if (!(m_pos.IntCmp(m_posPrev)))
	{
		collisionVectorOut.push_back(m_posPrev);
	}
}

UINT32 GameObject::SetupCollidingObjects(std::initializer_list<RaiderObjectTypeId> l)
{
	UINT32 retVal = 0;
	for (auto it : l)
	{
		retVal |= (1 << (int)it);
	}
	return retVal;
}

void GameObject::OnObjectStriked(GameObject& attacker, PlayField& world, const Vector2D& collisionPoint)
{
	m_health -= attacker.m_strikeForce;
	if (m_health <= 0)
	{
		world.RemoveObject(this);
		OnObjectDestroyed(attacker, world, collisionPoint);
	}
}

void GameObject::CheckCollision(GameObject& other, PlayField& world, const Vector2D& collisionPoint)
{
	if (IsCollidingWithObject(other))
	{
		OnObjectStriked(other, world, collisionPoint);
	}
}

Explosion::Explosion(Vector2D pos) : GameObject(RI_Explosion, pos, RS_Explosion, 0, 0) {}

void Explosion::Update(PlayField& world)
{
	m_timer--;
	if (!m_timer)
	{
		world.RemoveObject(this);
	}
}

Laser::Laser(RaiderObjectTypeId objectType, 
	Vector2D pos, 
	Vector2D direction, 
	unsigned char sprite, 
	IsObjectValidFunc isValidFunc) 
	:
	GameObject(objectType, pos, sprite, 1, 1),
	m_direction(direction),
	m_isValidFunc(isValidFunc)
{
	// declaring colTypes as static ensures that
	// SetupCollidingObjects will be invoked only once
	// which should be a performance improvement
	// (the same pattern applies to all other game object types)
	static UINT32 colTypes = SetupCollidingObjects({ RI_AlienLaser, RI_PlayerLaser,
		RI_Player, RI_Alien, RI_WallBlock, RI_ExplosionCell});
	m_collisionTypeBitmap = colTypes;
}

void Laser::OnObjectStriked(GameObject& attacker, PlayField& world, const Vector2D& collisionPoint)
{
	// let's assume that player lasers cannot hit player lasers
	// and alien lasers cannot hit alien lasers
	// (without this, if they move the same direction
	// and are very close to each other, they may mistakenly
	// by treated as collided when they occupy the same cell)
	if (attacker.GetType() != m_objType)
	{
		__super::OnObjectStriked(attacker, world, collisionPoint);
	}
}

void Laser::OnObjectDestroyed(GameObject& attacker, PlayField& world, const Vector2D& collisionPoint) 
{
	m_pos = collisionPoint;
	// there may be a situation in which there are 2 explosions on the same location
	// (both colliding parties generated explosion) but this shouldn't be a problem
	// since Exposion doesn't have any game behaviour side effects except exposing 
	// sprite on the screen
	world.AddObject(new Explosion(collisionPoint));
	world.DespawnLaser(this); 
}

void Laser::Update(PlayField& world)
{
	m_posPrev = m_pos;
	m_pos += m_direction;
	// check if laser is not beyound acceptable area
	if (!m_isValidFunc(*this, world))
	{
		world.RemoveObject(this);
		world.DespawnLaser(this);
	}
}

AlienLaser::AlienLaser(GameObject *parent) :
	Laser(RI_AlienLaser, parent->GetPos() + Vector2D(0, 1), Vector2D(0, 1), RS_TakeDefault,
		[](GameObject& obj, PlayField& world)->bool { return obj.GetPos().y <= world.GetBounds().y; })
{}

StrongAlienLaser::StrongAlienLaser(GameObject *parent) :
	AlienLaser(parent)
{
	// PlayerShip lasers will not be able to destroy strong alien lasers
	// (unlike usual alien lasers)
	UnsetCollidingObject(RI_PlayerLaser);
	m_strikeForce = 5;
	m_sprite = RS_StrongAlienLaser;
}

PlayerLaser::PlayerLaser(GameObject *parent) :
	Laser(RI_PlayerLaser, parent->GetPos() + Vector2D(0, -1), Vector2D(0, -1), RS_TakeDefault,
		[](GameObject& obj, PlayField& world)->bool { return obj.GetPos().y >= 0; })
{}

PlayerLaserLR::PlayerLaserLR(GameObject *parent, bool isLeft) : 
	Laser(RI_PlayerLaser, parent->GetPos().Floor() + Vector2D(0, -1), Vector2D(0, 0), RS_PlayerLaserLR,
		[](GameObject& obj, PlayField& world)->bool 
			{ return obj.GetPos().y >= 0 && obj.GetPos().x >= 0 && obj.GetPos().x <= world.GetBounds().x - 1; })
{
	// if left/right lasers would have speed (-1/1,-1), then
	// their relative speed would be higher then
	// straight laser speed, lets avoid that by dividing
	// such vector by sqrt(2.f)
	float speed = 1.f/sqrt(2.f);
	m_direction = Vector2D(isLeft ? -speed : speed, -speed);
	m_pos.x += isLeft ? -1 : 1;

}

void SpaceShip::OnObjectDestroyed(GameObject& attacker, PlayField& world, const Vector2D& collisionPoint)
{ 
	m_pos = collisionPoint;
	if (attacker.GetPosPrev().x == attacker.GetPos().x)
	{
		// to avoid confusing impression that laser striked space ship
		// at wrong x position (Explosion doesn't cross laser path)
		// we are setting space ship x point to laser x point
		m_pos.x = attacker.GetPos().x;
	}
	world.AddObject(new Explosion(collisionPoint));
}

Alien::Alien(Vector2D pos, float velocityY, bool enableFriendFire) : 
	SpaceShip(RI_Alien, pos, RS_TakeDefault, 1, 10),
	m_velocityY(velocityY)
{
	static UINT32 colTypes = SetupCollidingObjects(
		{ RI_AlienLaser, RI_PlayerLaser, RI_Player, RI_ExplosionCell });
	m_collisionTypeBitmap = colTypes;
	if (!enableFriendFire)
	{
		UnsetCollidingObject(RI_AlienLaser);
	}
	// we will randomize transform energy so that aliens
	// will not transform to better aliens in waves
	floatRand updateRate(0.f, 4.f);
	m_transformEnergy = updateRate(rGen) + 1.f;
	// as well as direction to avoid all aliens going the same direction at spawn time
	m_direction = updateRate(rGen) < 2.f ? 1.f : -1.f;
}

void Alien::OnObjectDestroyed(GameObject& attacker, PlayField& world, const Vector2D& collisionPoint)
{
	m_pos = collisionPoint;
	if (attacker.GetType() == RI_PlayerLaser)
	{
		// 10 points for normal aliens and 20 for better aliens
		world.AddScore(m_state == as_Normal ? 10 : 20);
	}
	__super::OnObjectDestroyed(attacker, world, collisionPoint);
	world.NotifyAlienDestroyed();
	// add power-up with 10% prob.
	if (getRandInt(0, 9) == 0)
	{
		world.AddPowerUp(m_pos);
	}
}

void Alien::Transform()
{
	m_state = as_Better;
	m_sprite = RS_BetterAlien;
	m_velocityX *= 2.f;
	// I decided to preserve the direction vector of normal alien
	// so I have to also increase y direction velocity
	m_velocityY *= 2.f;
	m_health *= 2;
	m_fireRateBorder *= 1.5f;
}

void Alien::Update(PlayField& world)
{
	m_posPrev = m_pos;
	m_pos.x += m_direction * m_velocityX;
	m_pos.y += m_velocityY;
	// Border check
	if (m_pos.x < 0 || m_pos.x >= world.GetBounds().x - 1)
	{	
		m_direction = -m_direction;
		m_pos.x = m_direction > 0 ? 0 : world.GetBounds().x - 1;
	}

	// Border check vertical:
	if (m_pos.y >= world.GetBounds().y - 1)
	{
		world.NotifyGameOver();
		return;
	}

	// Transform into better Alien
	if (m_isTransformationEnabled && m_state != as_Better)
	{
		m_energy += getRandFloat(0, 2 * m_maxUpdateRate);
		if (m_energy >= m_transformEnergy)
			Transform();
	}

	if (getRandFloat(0.f, 1.f) < m_fireRateBorder && world.CanNewLasersBeSpawned(RI_AlienLaser, 1))
	{
		// Spawn laser
		// if strong alien lasers are supported (hard mode) and it is better alien, spawn strong laser with 50% probability
		if (m_state == as_Better && world.AreStrongAlienLasersAllowed() && getRandInt(0, 1) == 0)
			world.SpawnLaser(new StrongAlienLaser(this));
		else
			world.SpawnLaser(new AlienLaser(this));
	}
}

PlayerShip::PlayerShip(Vector2D pos) : SpaceShip(RI_Player, pos, RS_TakeDefault, 1, 10)
{
	static UINT32 colTypes = SetupCollidingObjects({ RI_AlienLaser, RI_Alien, RI_PowerUp, RI_ExplosionCell });
	m_collisionTypeBitmap = colTypes;
}

void PlayerShip::OnObjectDestroyed(GameObject& attacker, PlayField& world, const Vector2D& collisionPoint)
{
	__super::OnObjectDestroyed(attacker, world, collisionPoint);
	world.NotifyGameOver();
}

void PlayerShip::Update(PlayField& world)
{
	m_posPrev = m_pos;
	int xLast = (int)m_pos.x;
	if (world.GetControllerInput().Left())
		m_pos.x -= m_movementSpeed;
	else if (world.GetControllerInput().Right())
		m_pos.x += m_movementSpeed;

	if (m_pos.x <= 0.f)
		m_pos.x = 0.f;
	else if (m_pos.x >= world.GetBounds().x - 1)
		m_pos.x = world.GetBounds().x - 1;
	m_collisionPoints.clear();

	// we will use m_collisionPoints only if we moved 
	// for more then one game cell from last iteration
	int xCurr = (int)m_pos.x;
	if (xCurr - xLast > 1)
	{
		int iterDiff = xLast < xCurr ? -1 : 1;
		for (int i = xCurr; i != xLast; i += iterDiff)
		{
			m_collisionPoints.push_back(Vector2D((float)i, m_pos.y));
		}
		m_collisionPoints.push_back(Vector2D((float)xLast, m_pos.y));
	}
	int shotsCount = m_useTripleShots ? 3 : 1;
	if (world.GetControllerInput().Fire() && world.CanNewLasersBeSpawned(RI_PlayerLaser, shotsCount))
	{
		//Spawn laser
		world.SpawnLaser(new PlayerLaser(this));
		if (m_useTripleShots)
		{
			world.SpawnLaser(new PlayerLaserLR(this, true));
			world.SpawnLaser(new PlayerLaserLR(this, false));
		}
	}
}

void PlayerShip::GetCollisionPoints(std::vector<Vector2D>& collisionVectorOut)
{
	if (m_collisionPoints.size() != 0)
	{
		collisionVectorOut.insert(collisionVectorOut.end(), m_collisionPoints.begin(), m_collisionPoints.end());
		return;
	}
	__super::GetCollisionPoints(collisionVectorOut);
}

WallBlock::WallBlock(Vector2D pos) :
	GameObject(RI_WallBlock, pos, RS_TakeDefault, 5, 10)
{
	static UINT32 colTypes = SetupCollidingObjects({ RI_PlayerLaser, RI_ExplosionCell, RI_AlienLaser });
	m_collisionTypeBitmap = colTypes;
}

void WallBlock::OnObjectStriked(GameObject& attacker, PlayField& world, const Vector2D& collisionPoint)
{
	// In case of alien laser, only strong one can hurt wall block
	if (attacker.GetType() != RI_AlienLaser || attacker.GetStrikeForce() >= 5)
	{
		__super::OnObjectStriked(attacker, world, collisionPoint);
	}
}

void WallBlock::OnObjectDestroyed(GameObject& attacker, PlayField& world, const Vector2D& collisionPoint)
{
	world.NotifyWallBlockDestroyed();
	world.AddObject(new Explosion(m_pos));
}