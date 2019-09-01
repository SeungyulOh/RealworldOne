#pragma once

#include <Windows.h>
#include <vector>

class RenderItemBase
{
public:
	RenderItemBase(const Vector2D& iPos) : m_pos(iPos) {}
	Vector2D m_pos;
	virtual const char* GetSpriteCharsArray() = 0;
};

class RenderItemSprite : public RenderItemBase
{
protected:
	char m_sprite[2];
public:
	RenderItemSprite(const Vector2D& iPos, char iSprite) : RenderItemBase(iPos)
	{
		m_sprite[0] = iSprite;
		m_sprite[1] = 0;
	}
	virtual const char* GetSpriteCharsArray() { return m_sprite; }
};

class RenderItemString : public RenderItemBase
{
protected:
	const char *m_str;
public:
	RenderItemString(const Vector2D& iPos, const char *str) : RenderItemBase(iPos), m_str(str) {}
	virtual const char* GetSpriteCharsArray() 
	{ 
		return m_str; 
	}
};

typedef union _RenderItemUnionBuffer
{
	char reserved1[sizeof(RenderItemSprite)];
	char reserved2[sizeof(RenderItemString)];
	_RenderItemUnionBuffer(){}
} RenderItemUnion;

typedef std::vector<RenderItemUnion> RenderItemList;
class PlayField;
class Renderer
{
private:
	// it's better to place render list vector as class member
	// because this can avoid constant memory reallocations on adding
	// new items to vector
	RenderItemList m_renderList;
    HANDLE m_hout;
public:
	Renderer(const Vector2D& bounds);
	~Renderer();

	// Draws all game objects after clearing filling the Canvas with _ symbol
	void Update(PlayField& world);
    bool AdjustConsoleSize();
    void SetcursorVisibility(bool isVisible);

private:
	Vector2D m_renderBounds;
	// there is no point in using double buffering here since
	// console screen is not autonomously updated when we are writing to
	// our private buffer like it is done i.e. in GPU frame buffers
	unsigned char* m_canvas = nullptr;
	int m_canvasSize = 0;
	unsigned char* CurCanvas(int x, int y) { return &m_canvas[x + (int)m_renderBounds.x * y];  }

	// Fills whole m_canvas array with m_sprite
	void FillCanvas(unsigned char m_sprite);
	// Prints m_canvas char array on console
	void DrawCanvas();
};

