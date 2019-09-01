#pragma once

#include <random>
#include <vector>
#include "PositionMap.h"
#include "Vector2D.h"

extern std::default_random_engine rGen;
typedef std::uniform_int_distribution<int> intRand;
typedef std::uniform_real_distribution<float> floatRand;

inline int getRandInt(int min, int max)
{
	intRand tmp(min, max);
	return tmp(rGen);
}

inline float getRandFloat(float min, float max)
{
	floatRand tmp(min, max);
	return tmp(rGen);
}

class RandomPositionProvider : public PositionMapDynamic
{
protected:
	std::vector<int> m_freeIndexes;
	inline bool GetPositionFromIndex(int index, int &xOut, int &yOut)
	{
		if (index < 0 || index >= m_size)
		{
			return false;
		}
		xOut = index % m_sizeX;
		yOut = index / m_sizeX;
		return true;
	}
public:
	RandomPositionProvider(int sizeX, int sizeY) : PositionMapDynamic(sizeX, sizeY){}
	void InitRandomFreePositions()
	{
		m_freeIndexes.clear();
		for (int i = 0; i < m_size; i++)
		{
			if (!m_positions[i])
			{
				m_freeIndexes.push_back(i);
			}
		}
	}

	bool GetNextRandomPosition(Vector2D& vecOut)
	{
		if (m_freeIndexes.size() == 0)
		{
			return false;
		}
		int index = getRandInt(0, (int)m_freeIndexes.size() - 1);
		int x = 0, y = 0;
		GetPositionFromIndex(index, x, y);
		m_freeIndexes[index] = m_freeIndexes[m_freeIndexes.size() - 1];
		m_freeIndexes.resize(m_freeIndexes.size() - 1);
		vecOut = Vector2D((float)x, (float)y);
		return true;
	}
};