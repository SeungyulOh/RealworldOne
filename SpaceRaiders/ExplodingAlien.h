#pragma once

#include "GameObjects.h"
#include "PositionMap.h"

class EAExplosionCell : public GameObject
{
	int m_time = 1;
public:
	EAExplosionCell(Vector2D &pos);
	void Update(PlayField& world);
};


class ExplodingAlien : public Alien
{
protected:
	bool m_isDead = false;
	int m_currExplosionRing = 0;
	static const int m_maxExplosionRing = 14;
	static const int m_positionMapX = 2 * (m_maxExplosionRing + 1);
	static const int m_positionMapY = m_positionMapX;
	std::vector<Vector2D> m_collisionPoints;
	std::vector<Vector2D> m_newPoints;
	Vector2D m_explosionStartPoint;
	PositionMapStatic<m_positionMapX, m_positionMapY> m_positionMap;
	void OnObjectDestroyed(GameObject& attacker, PlayField& world, const Vector2D& collisionPoint);
	static inline int MapXPosition(float x) { return (int)(x + 0.5f + (float)m_maxExplosionRing); }
	static inline int MapYPosition(float y) { return MapXPosition(y); }
public:
	ExplodingAlien(Vector2D pos, float velocityY, bool enableFriendFire);
	virtual void Update(PlayField& world);
	void CreateCircle(int r);
};
