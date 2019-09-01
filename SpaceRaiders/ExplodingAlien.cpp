
#include "stdafx.h"
#include "ExplodingAlien.h"
#include "PlayField.h"

// EAExplosionCell stands for ExplodinAlien ExplosionCell

// Exploding alien explosion cells will be very strong (they shold kill any other game object with one hit),
// they will also be resistant to any other objects
EAExplosionCell::EAExplosionCell(Vector2D& pos) :
	GameObject(RI_ExplosionCell, pos, RS_ExplosionCell, 10000, 10000)
{
	m_collisionTypeBitmap = 0;
}

void EAExplosionCell::Update(PlayField& world)
{
	if (m_time-- == 0)
	{
		world.RemoveObject(this);
	}
}

void ExplodingAlien::OnObjectDestroyed(GameObject& attacker, PlayField& world, const Vector2D& collisionPoint)
{
	if (m_isDead)
	{
		return;
	}
	__super::OnObjectDestroyed(attacker, world, collisionPoint);
	m_isDead = true;
	m_isActive = true; // we will fake PlayField that we are still active so that we will still be able to respond to Update(..)
	m_explosionStartPoint = m_pos.Floor() + Vector2D(0.5f, 0.5f); // instead of using round() later, we will just add now [0.5,0.5] vector to starting point
	m_pos.x = m_pos.y = -1.f; // let's move this object beyond visible area
	m_posPrev = m_pos;
	m_objType = RI_ExplosionCell;
}
ExplodingAlien::ExplodingAlien(Vector2D pos, float velocityY, bool enableFriendFire) :
	Alien(pos, velocityY, enableFriendFire)
{
	m_sprite = RS_ExplodingAlien;
	// exploding aliens will not be able to transform to better aliens 
	// (we don't want to change their attributes at runtime)
	m_isTransformationEnabled = false;
}

void ExplodingAlien::Update(PlayField& world)
{
	if (!m_isDead)
	{
		__super::Update(world);
		return;
	}
	if (m_currExplosionRing == m_maxExplosionRing)
	{
		world.RemoveObject(this);
		return;
	}
	CreateCircle(m_currExplosionRing++);
	for (auto it : m_newPoints)
	{
		world.AddObject(new EAExplosionCell(it));
	}
}

void ExplodingAlien::CreateCircle(int r)
{
	// x^2 + y^2 = r^2
	// x = sqrt(r^2 - y^2)
	// y = sqrt(r^2 - x^2)
	m_newPoints.clear();
	if (r == 0)
	{
		m_newPoints.push_back(m_explosionStartPoint);
		m_positionMap.SetPosition(MapXPosition(0), MapYPosition(0), true);
		return;
	}
	// we will do this in 2 steps
	// 1. step through x points from x = -r/sqrt(2) to x = r/sqrt(2) (inclusively)
	int x0Maped = MapXPosition(0);
	int border = MapXPosition((float)r / sqrt(2.f)) - x0Maped;
	float r2 = (float)(r*r);
	m_newPoints.push_back(Vector2D(0, (float)r));
	for (float x = 1.f; (int)x <= border; x += 1.f)
	{
		float y = sqrt(r2 - x * x);
		m_newPoints.push_back(Vector2D(x, y));
		m_newPoints.push_back(Vector2D(-x, y));
	}
	int size = (int)m_newPoints.size();
	for (int i = 0; i < size; i++)
	{
		m_newPoints.push_back(Vector2D(m_newPoints[i].x, -m_newPoints[i].y));
	}

	// 2. add points with reveresd x/y
	for (int i = 0; i < size; i++)
	{
		m_newPoints.push_back(Vector2D(m_newPoints[i].y, m_newPoints[i].x));
		m_newPoints.push_back(Vector2D(-m_newPoints[i].y, m_newPoints[i].x));
	}

	// 3. add points in places that are not adjecent (in the same row) to any point from last ring points
	// i.e. for r=2, we are doing the following transformation (L stands for point from last ring (r=1))
	//  +++       +++
	// + L +     ++L++
	// +LLL+  -> +LLL+
	// + L +     ++L++
	//  +++       +++
	for (auto& it : m_newPoints)
	{
		m_positionMap.SetPosition(MapXPosition(it.x), MapYPosition(it.y), true);
	}
	size = (int)m_newPoints.size();
	for (int i = 0; i < size; i++)
	{
		auto& pos = m_newPoints[i];
		if (pos.x == 0)
		{
			continue;
		}
		int direction = pos.x < 0 ? 1 : -1;
		int xStart = MapXPosition(pos.x);
		void *line = m_positionMap.GetLine(MapYPosition(pos.y));
		for (int xMapped = xStart + direction; xMapped != x0Maped; xMapped += direction)
		{
			bool valTmp = m_positionMap.GetPositionOnLine(line, xMapped);
			if (valTmp)
			{
				break;
			}
			m_newPoints.push_back(Vector2D(pos.x + (float)(xMapped - xStart), pos.y));
			m_positionMap.SetPositionOnLine(line, xMapped, true);
		}
	}
	// 4. add ring start point to all explosion points from this iteration
	for(int i = 0; i < m_newPoints.size(); i++)
	{
		m_newPoints[i] += m_explosionStartPoint;
	}
}
