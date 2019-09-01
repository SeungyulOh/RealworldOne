#pragma once

// PositionMap is bitmap that is used for tracking objects presence at certain points
class PositionMap
{
protected:
	bool *m_positions = nullptr;
	int m_sizeX;
	int m_sizeY;
	int m_size;
	std::vector<int> m_freeIndexes;
public:
	PositionMap(int sizeX, int sizeY, bool *buffer) : 
		m_sizeX(sizeX), m_sizeY(sizeY), m_size(sizeX * sizeY), m_positions(buffer)
	{ 
		Clear(); 
	}
	int getSizeX() { return m_sizeX; }
	int getSizeY() { return m_sizeY; }
	virtual ~PositionMap(){}
	inline void *GetLine(int line) { return &m_positions[line * m_sizeX]; }
	inline void SetPositionOnLine(void *line, int x, bool value) { ((bool*)line)[x] = value; }
	inline bool GetPositionOnLine(void *line, int x) { return ((bool*)line)[x]; }
	inline void SetPosition(int x, int y, bool value) { m_positions[x + m_sizeX * y] = value; }
	inline bool GetPosition(int x, int y) { return m_positions[x + m_sizeX * y]; }
	void Clear() { memset(m_positions, 0, m_sizeX * m_sizeY); }
};

template
<int sizeX, int sizeY>
class PositionMapStatic : public PositionMap
{
protected:
	bool m_positionsBuffer[sizeX * sizeY];
public:
	PositionMapStatic<sizeX, sizeY>() : PositionMap(sizeX, sizeY, m_positionsBuffer) {}
};

class PositionMapDynamic : public PositionMap
{
public:
	PositionMapDynamic(int sizeX, int sizeY) : PositionMap(sizeX, sizeY, new bool [sizeX * sizeY]) {}
	virtual ~PositionMapDynamic() { delete[] m_positions; }
};
