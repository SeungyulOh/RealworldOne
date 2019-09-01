
#include "stdafx.h"
#include "GameObjects.h"
#include "PowerUp.h"
#include "PlayField.h"


void PowerUp::OnObjectDestroyed(GameObject& attacker, PlayField& world, const Vector2D& collisionPoint)
{
	m_isCatched = true;
	world.ActivatePowerUp(*this);
}

void PowerUp::Update(PlayField& world)
{
	m_posPrev = m_pos;
	m_pos.y += 0.5f;
	if ((int)m_pos.y >= (int)world.GetBounds().y)
	{
		world.RemoveObject(this);
	}
}

void FasterShotsPowerUp::OnPowerUpCatched()
{
	m_world.MaxPlayerLasers = (int)((float)m_world.MaxPlayerLasers * 1.5f);
}
void FasterShotsPowerUp::OnPowerUpExpired()
{
	m_world.MaxPlayerLasers = (int)((float)m_world.MaxPlayerLasers / 1.5f);
}

void TripleShotsPowerUp::OnPowerUpCatched()
{
	// triple shots have more side effects than just modifying
	// m_world.MaxPlayerLasers so we have dedicated PlayField methods
	// for handling this
	m_world.SetTriplePlayerLaser();
}
void TripleShotsPowerUp::OnPowerUpExpired()
{
	m_world.UnsetTriplePlayerLaser();
}