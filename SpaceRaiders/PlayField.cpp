
#include "stdafx.h"
#include <chrono>
#include <thread>
#include <algorithm>
#include "PlayField.h"
#include "ExplodingAlien.h"


PlayField::PlayField(Vector2D iBounds, GameConfig& config) : 
	m_bounds(iBounds), m_playerObject(nullptr), m_score(0),
	m_gameOver(false), m_infoString(Vector2D(4, iBounds.y - 1)),
	m_nextObjectsWaveTime(m_objectsSpawnWavesTimeDist),
	m_displayInfo(config.displayGameInfo),
	m_cotrollerInput(config.testRun ? (Input*)new RndInput() : new KeyboardInput()),
	m_maxIterations(config.testRun ? config.testIterations : -1),
	m_sleepTimeBetweenIterationsInMs(config.iterationSleepTimeInMs),
	m_isAliensFriendFireEnabled(config.aliensFriendFire),
	m_isSpecialFeatureEnabled(config.useSpecialFeature),
	m_wallBlocksPosProvider((int)iBounds.x, std::max((int)((float)iBounds.y - 6.f), 0)), // size.y - 5 upper rows, (-6 because actual bounds are iBounds.y - 1)
	m_aliensPosProvider((int)iBounds.x, std::min((int)((float)std::max(iBounds.y, 1.f) - 1.f), 4)), // 4 upper rows
	m_isHardMode(config.hardMode)
{
	m_bounds.y -= 1;
	if (config.displayGameInfo)
	{
		m_stringObjects.push_back(&m_infoString);
	}
	if (config.hardMode)
	{
		m_currMinAliensSpawnedPerWave *= 2.f;
		m_currMaxAliensSpawnedPerWave *= 2.f;
		m_startingAliensCount *= 2;
		m_aliensVelocityY *= 1.5f;
		MaxAlienLasers *= 2;
	}
}

int PlayField::GetCenteredStringXPosition(std::string& str)
{
	return ((int)m_bounds.x - (int)str.size()) / 2;
}

void PlayField::AddCenteredString(std::string str, StringObject &dest, int y)
{
	dest = StringObject(Vector2D((float)GetCenteredStringXPosition(str), (float)y), str.c_str());
	m_stringObjects.push_back(&dest);
}

void PlayField::NotifyGameOver()
{
	m_gameOver = true;
	AddCenteredString(" Game Over ", m_gameOverString, 13);
	AddCenteredString(" Score: " + std::to_string(m_score) + " ", m_scoreString, 15);
}

// displaying game info string at the bottom line of the game screen
// is one of the extra options available through cmd parameters
void PlayField::UpdateGameInfo()
{
	if (!m_displayInfo)
	{
		m_currIteration++;
		return;
	}
	m_infoString.GetStr().reserve(100);
	m_infoString.GetStr().resize(100);
	std::sprintf((char*)m_infoString.GetStr().c_str(),
		"Iteration: % 8d  Score: % 6d  Aliens: % 3d  Block walls: % 3d",
		m_currIteration++, m_score, m_aliensCount, m_wallBlocksCount);
	m_infoString.GetStr().resize(strlen(m_infoString.GetStr().c_str()));
	m_infoString.GetPos().x = (float)GetCenteredStringXPosition(m_infoString.GetStr());
}

bool PlayField::IsStillRunning()
{
    return !m_gameOver && (m_maxIterations == -1 || m_currIteration < m_maxIterations);
}

void PlayField::WaitBetweenIterations()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(
        m_sleepTimeBetweenIterationsInMs));
}

void PlayField::SetupGame()
{
    // Populate aliens
	SpawnAliens(m_startingAliensCount, m_isSpecialFeatureEnabled);
    // Add player
    AddPlayerObject(Vector2D(40, 27));
    // Add wall blocks
    SpawnWallBlocks(10);
}

void PlayField::Update()
{
	if (m_gameOver)
	{
		return;
	}
	m_cotrollerInput->Update();
	// clear collision map first
	for (int i = 0; i < COLLISION_MAP_SIZE; i++)
	{
		m_collisionMap[i].clear();
	}
	for (auto it : m_gameObjects)
	{
		if (!it->IsActive())
		{
			continue;
		}
		it->Update(*this);
		// Check collisions with already updated objects
		HandleCollisions(it);
	}
	HandleSpawningNewObjects();
	HandlePowerUpes();
	ApplyObjectsCollectionChanges();
	UpdateGameInfo();
}

bool PlayField::CanNewLasersBeSpawned(RaiderObjectTypeId laserType, int count)
{
	if (laserType == RI_AlienLaser)
		return AlienLasers + count <= MaxAlienLasers;
	if (laserType == RI_PlayerLaser)
		return PlayerLasers + count <= MaxPlayerLasers;
	return false;
}

bool PlayField::AreStrongAlienLasersAllowed()
{
	return m_isHardMode;
}

void PlayField::SpawnLaser(GameObject* newObj)
{
	if (newObj->GetType() == RI_AlienLaser)
		AlienLasers++;
	else if (newObj->GetType() == RI_PlayerLaser)
		PlayerLasers++;
	AddObject(newObj);
}

void PlayField::DespawnLaser(GameObject* newObj)
{
if (newObj->GetType() == RI_AlienLaser)
	AlienLasers--;
else if (newObj->GetType() == RI_PlayerLaser)
	PlayerLasers--;
}

void PlayField::SetTriplePlayerLaser()
{
	MaxPlayerLasers *= 3;
	m_playerObject->SetTripleShots(true);
}

void PlayField::UnsetTriplePlayerLaser()
{
	MaxPlayerLasers /= 3;
	m_playerObject->SetTripleShots(false);
}

void PlayField::AddPlayerObject(Vector2D pos)
{
	m_playerObject = new PlayerShip(pos);
	AddObject(m_playerObject);
}

void PlayField::AddObject(GameObject* newObj)
{
	m_gameObjectsToAdd.push_back(newObj);
}

void PlayField::RemoveObject(GameObject* obj)
{
	obj->SetToInactive();
}

// gets collision vector based on hash function applied on objects collision point
std::vector<std::pair<Vector2D, GameObjPtr>>* PlayField::GetCollisionVector(Vector2D& pos)
{
	if (pos.x < 0 || pos.y < 0)
	{
		return nullptr;
	}
	int index = ((int)pos.y % COLLISION_MAP_Y_CELLS) * COLLISION_MAP_X_CELLS + ((int)pos.x % COLLISION_MAP_X_CELLS);
	return &m_collisionMap[index];
}

void PlayField::HandleCollisions(GameObject* obj)
{
	if (!obj->IsActive())
	{
		return;
	}
	m_tmpCollisionPoints.clear();
	// each object can provide more than 1 collision point
	// to handle situation when 2 objects crosses they paths in point
	// that is not occupied by any of them after update
	// so the assumption here is that they will provide
	// all points they touched since last iteration (inclusively)
	// and we will check collisions for all of these points
	obj->GetCollisionPoints(m_tmpCollisionPoints);
	for (auto vIt : m_tmpCollisionPoints)
	{
		// get collision vector based on current collision point
		auto collisionVector = GetCollisionVector(vIt);
		if (collisionVector == nullptr)
		{
			continue;
		}
		for (auto it : *collisionVector)
		{
			if (it.second->IsActive() && CheckObjectsCollision(*obj, *it.second))
			{
				// this means that obj is inactive after collision (CheckObjectsCollision(...) returned true)
				break;
			}
		}
		if (!obj->IsActive())
		{
			break;
		}
		// add obj collision point to collison map
		collisionVector->push_back({ vIt, obj });
	}
}

void PlayField::NotifyWallBlockDestroyed()
{
	m_wallBlocksCount--;
}

void PlayField::NotifyAlienDestroyed()
{
	m_aliensCount--;
}

// returns true if o1 has beed destroyed, false otherwise
bool PlayField::CheckObjectsCollision(GameObject& o1, GameObject& o2)
{
	// This algorighm can be little inaccurate in general case but for our game
	// where motion vectors are short this should work

	// 1. Check if they current objects positions are colliding
	if (o1.GetPos().IntCmp(o2.GetPos()))
	{
		o1.CheckCollision(o2, *this, o1.GetPos());
		o2.CheckCollision(o1, *this, o1.GetPos());
		return !o1.IsActive();
	}

	// 2. Check if their mid positions are colliding
	//    i.e we have 2 opposite lasers that in game iteration x had positions a=(20, 9) and b=(20, 10)
	//	  and in game iteration x + 1 they have a=(20, 10) and b=(20, 9), so that they
	//    are not colliding with each other in their current positions but
	//    they are colliding with each other in the middle of their paths between
	//    game iteration x and x + 1
	auto o1PosPrev = o1.GetPosPrev();
	auto o2PosPrev = o2.GetPosPrev();
	Vector2D xy1((FLOOR(o1PosPrev.x) + FLOOR(o1.GetPos().x)) / 2,
		(FLOOR(o1PosPrev.y) + FLOOR(o1.GetPos().y)) / 2);
	float x2 = (FLOOR(o2PosPrev.x) + FLOOR(o2.GetPos().x)) / 2;
	float y2 = (FLOOR(o2PosPrev.y) + FLOOR(o2.GetPos().y)) / 2;
	float xDiff = x2 - xy1.x;
	float yDiff = y2 - xy1.y;
	// assume that if distance between movement mid points of comparing
	// objects is at least 0.8, they have just collided
	if (xDiff * xDiff + yDiff * yDiff <= 0.64f) // 0.64 == 0.8 * 0.8
	{
		o1.CheckCollision(o2, *this, xy1);
		o2.CheckCollision(o1, *this, xy1);
	}
	return !o1.IsActive();
}

void PlayField::SpawnWallBlocks(int count)
{
	m_wallBlocksPosProvider.InitRandomFreePositions();
	Vector2D pos;
	for (int i = 0; i < count && m_wallBlocksPosProvider.GetNextRandomPosition(pos) 
		&& m_wallBlocksCount < MaxBlockWalls; i++)
	{
		AddObject(new WallBlock(pos));
		m_wallBlocksCount++;
	}
}

void PlayField::SpawnAliens(int count, bool allowForExplodingAlien)
{
	count = std::min(count, MaxAliens - m_aliensCount);
	if (count <= 0)
	{
		return;
	}
	m_aliensPosProvider.InitRandomFreePositions();
	Vector2D pos;
	for (int i = 0; i < count && m_aliensPosProvider.GetNextRandomPosition(pos); i++)
	{
		AddObject(new Alien(pos, m_aliensVelocityY, m_isAliensFriendFireEnabled));
		m_aliensCount++;
	}

	// if allowForExplodingAlien==true, spawn exploding alien with 50% prob.
	if (allowForExplodingAlien && getRandInt(0, 1) == 0
		&& m_aliensPosProvider.GetNextRandomPosition(pos) && m_aliensCount < MaxAliens)
	{
		// Exploding aliens will go down faster to provide more fun
		AddObject(new ExplodingAlien(pos, m_aliensVelocityY * 3.f, m_isAliensFriendFireEnabled));
		m_aliensCount++;
	}
}

void PlayField::HandlePowerUpes()
{
	m_powerUpsToDelete.clear();
	for (auto it : m_catchedPowerUpes)
	{
		if (!it.second->Tick())
		{
			m_powerUpsToDelete.push_back(it.second);
		}
	}
	for (int i = 0; i < m_powerUpsToDelete.size(); i++)
	{
		m_catchedPowerUpes.erase(m_powerUpsToDelete[i]->GetType());
		delete m_powerUpsToDelete[i];
	}
}

void PlayField::AddPowerUp(Vector2D& pos)
{
	switch (getRandInt(0, 2))
	{
	case 0: AddObject(new MovementSpeedPowerUp(*m_playerObject, pos)); break;
	case 1: AddObject(new FasterShotsPowerUp(*this, pos)); break;
	case 2: AddObject(new TripleShotsPowerUp(*this, pos)); break;
	}
}

void PlayField::ActivatePowerUp(PowerUp& powerUp)
{
	auto it = m_catchedPowerUpes.find(powerUp.GetType());
	if (it != m_catchedPowerUpes.end())
	{
		it->second->Merge(powerUp);
		return;
	}
	powerUp.OnPowerUpCatched();
	m_catchedPowerUpes.insert({ powerUp.GetType(), &powerUp });
	// object will be removed from m_gameObjects collection but not deleted from memory
	powerUp.SetAutoDelete(false);
}

void PlayField::ApplyObjectsCollectionChanges()
{
	// we will swap all inactive objects with locally last object and then shrink vector size,
	// this should be faster then searching through whole vector every time we want to delete an object
	size_t sizeUpdated = m_gameObjects.size();
	for (size_t i = 0; i < sizeUpdated; i++)
	{
		if (!m_gameObjects[i]->IsActive())
		{
			iter_swap(m_gameObjects.begin() + i, m_gameObjects.begin() + sizeUpdated - 1);
			sizeUpdated--;
			// we have to also check the object just swapped from the end
			// so we will handle the same index in next iteration
			i--;
		}
	}
	if (sizeUpdated != m_gameObjects.size())
	{
		// remove inactive objects from m_gameObjects collection
		// and release them (calling delete ...) if necessary
		for (size_t i = sizeUpdated; i < m_gameObjects.size(); i++)
		{
			if (m_gameObjects[i]->IsAutoDelete())
			{
				delete m_gameObjects[i];
			}
		}
		m_gameObjects.resize(sizeUpdated);
	}
	m_gameObjects.insert(m_gameObjects.end(), m_gameObjectsToAdd.begin(), m_gameObjectsToAdd.end());
	m_gameObjectsToAdd.clear();
}

// used in process of spawning new objects (aliens and wall blocks) per wave
void PlayField::FillObjectsPositionMaps()
{
	m_wallBlocksPosProvider.Clear();
	m_aliensPosProvider.Clear();
	for (auto it : m_gameObjects)
	{
		if (it->GetPos().x < 0 || it->GetPos().y < 0)
		{
			continue;
		}
		if (it->GetType() == RI_Alien && (int)it->GetPos().y < m_aliensPosProvider.getSizeY())
			m_aliensPosProvider.SetPosition((int)it->GetPos().x, (int)it->GetPos().y, true);
		else if(it->GetType() == RI_WallBlock && (int)it->GetPos().y < m_wallBlocksPosProvider.getSizeY())
			m_wallBlocksPosProvider.SetPosition((int)it->GetPos().x, (int)it->GetPos().y, true);
	}
}

void PlayField::HandleSpawningNewObjects()
{
	if (--m_nextObjectsWaveTime > 0)
	{
		return;
	}
	m_nextObjectsWaveTime = m_objectsSpawnWavesTimeDist;
	FillObjectsPositionMaps();
	SpawnWallBlocks(3);
	SpawnAliens(getRandInt((int)m_currMinAliensSpawnedPerWave, (int)m_currMaxAliensSpawnedPerWave),
		m_isSpecialFeatureEnabled);

	// 50% prob. of min max aliens count increase for next wave
	float mul = (float)getRandInt(0, 1);
	m_currMinAliensSpawnedPerWave = std::min(m_currMinAliensSpawnedPerWave * (1 + (mul * 0.10f)), 
		MaxAliensSpawnedPerWave/2.f);
	m_currMaxAliensSpawnedPerWave = std::min(m_currMaxAliensSpawnedPerWave * (1 + (mul * 0.20f)), 
		MaxAliensSpawnedPerWave);
}