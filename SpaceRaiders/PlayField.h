#pragma once

#include <vector>
#include <map>
#include <string>
#include "GameObjects.h"
#include "Input.h"
#include "PowerUp.h"
#include "PositionMap.h"

typedef struct
{
    bool testRun;
    int testIterations;
    bool displayGameInfo;
    bool hardMode;
    bool useSpecialFeature;
    bool aliensFriendFire;
    int seed;
    int iterationSleepTimeInMs;
} GameConfig;

class PlayField
{
private:
	std::vector<GameObjPtr> m_gameObjects;
	std::vector<GameObjPtr> m_gameObjectsToAdd;
	StringObject			m_scoreString;
	StringObject			m_gameOverString;
	static const int COLLISION_MAP_X_CELLS = 20;
	static const int COLLISION_MAP_Y_CELLS = 20;
	static const int COLLISION_MAP_SIZE = COLLISION_MAP_X_CELLS * COLLISION_MAP_Y_CELLS;
	int						m_objectsSpawnWavesTimeDist = 50;
	int						m_wallBlocksCount = 0;
    int                     m_maxIterations = -1;
    int                     m_sleepTimeBetweenIterationsInMs = 50;
	int						m_startingAliensCount = 20;
	std::vector<std::pair<Vector2D, GameObjPtr>> m_collisionMap[COLLISION_MAP_SIZE];
	std::vector<StringObject*> m_stringObjects;
	std::vector<GameObjPtr> m_invisibleObjects;
	std::vector<Vector2D>   m_tmpCollisionPoints; // putting it here just to avoid frequent dynamic mem allocation in case of decaring on stack
	std::map<PowerUpType, PowerUp*>		m_catchedPowerUpes;
	std::vector<PowerUp*>				m_powerUpsToDelete;
	RandomPositionProvider		m_aliensPosProvider;
	RandomPositionProvider		m_wallBlocksPosProvider;
	bool					m_isSpecialFeatureEnabled;
	int						m_score;
	int						m_nextObjectsWaveTime;
	float					m_currMinAliensSpawnedPerWave = 2.f;
	float					m_currMaxAliensSpawnedPerWave = 4.f;
	const float				MaxAliensSpawnedPerWave = 20.f;
	bool					m_isAliensFriendFireEnabled;
	float					m_aliensVelocityY = 0.02f;

	std::vector<std::pair<Vector2D, GameObjPtr>>* GetCollisionVector(Vector2D& pos);
	PlayerShip *m_playerObject;
	bool m_displayInfo;
	int m_currIteration = 0;
	int m_aliensCount = 0;
	bool m_gameOver;
	bool m_isHardMode;
	StringObject m_infoString;
	Input * m_cotrollerInput = nullptr;
	Vector2D m_bounds;
	int GetCenteredStringXPosition(std::string& str);
	void AddCenteredString(std::string str, StringObject &dest, int y);
	void HandlePowerUpes();
	bool CheckObjectsCollision(GameObject& o1, GameObject& o2);
	void HandleSpawningNewObjects();
	void FillObjectsPositionMaps();
	void HandleCollisions(GameObject* obj);
	void ApplyObjectsCollectionChanges();
	void UpdateGameInfo();
	const int MaxBlockWalls = 40;
	const int MaxAliens = 200;
public:
	int MaxPlayerLasers = 4;
	int MaxAlienLasers = 10;
	int AlienLasers = 0;
	int PlayerLasers = 0;

	PlayField(Vector2D iBounds, GameConfig& config);
	const std::vector<GameObjPtr>& GameObjects() { return m_gameObjects; }
	const std::vector<StringObject*>& StringObjects() { return m_stringObjects; }
	void AddScore(int value) { m_score += value; }
	const Vector2D& GetBounds() { return m_bounds; }
    void SetupGame();
	void Update();
    void WaitBetweenIterations();
    bool IsStillRunning();
	Input& GetControllerInput() { return *m_cotrollerInput; }
	void NotifyGameOver();
	void SpawnLaser(GameObject* newObj);
	bool CanNewLasersBeSpawned(RaiderObjectTypeId laserType, int count);
	bool AreStrongAlienLasersAllowed();
	void DespawnLaser(GameObject* newObj);
	void SetTriplePlayerLaser();
	void UnsetTriplePlayerLaser();
	void AddPlayerObject(Vector2D pos);
	void AddObject(GameObject* newObj);
	void RemoveObject(GameObject* obj);
	void NotifyWallBlockDestroyed();
	void NotifyAlienDestroyed();
	void SpawnWallBlocks(int count);
	void SpawnAliens(int count, bool allowForExplodingAlien);
	void AddPowerUp(Vector2D& pos);
	void ActivatePowerUp(PowerUp& powerUp);
};