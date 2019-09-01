#include "stdafx.h"
#include <vector>
#include <iostream>
#include "Vector2D.h"
#include "PlayField.h"
#include "Renderer.h"
#include "GameObjects.h"
#include <new>

const HANDLE setCursorPosition(int x, int y)
{
	static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coord = { (SHORT)x, (SHORT)y };
	SetConsoleCursorPosition(hOut, coord);
	return hOut;
}

Renderer::Renderer(const Vector2D& bounds) :
    m_renderBounds(bounds),
    m_hout(INVALID_HANDLE_VALUE)
{
	m_canvasSize = (int)(bounds.x * bounds.y);
	m_canvas = new unsigned char[m_canvasSize];
}


Renderer::~Renderer()
{
	delete[] m_canvas;
}

bool Renderer::AdjustConsoleSize()
{
	m_hout = GetStdHandle(STD_OUTPUT_HANDLE);
	// we are setting window height to m_renderBounds.y + 1 to avoid game screen jumping up and down because of 
	// inserting last character from m_canvas to console window (then cursor moves to new line that is beyond 
	// rendering area and when we invoke setCursorPosition(0, 0) all game screen jumps one character up)
	
	CONSOLE_SCREEN_BUFFER_INFO  info;
	GetConsoleScreenBufferInfo(m_hout, &info);

	//our desired buffsize and windowRect size.
	COORD bufSize = { (SHORT)m_renderBounds.x, (SHORT)m_renderBounds.y + 1 };
	SMALL_RECT consoleWindowRect = { 0, 0, (SHORT)m_renderBounds.x - 1, (SHORT)m_renderBounds.y };

	// If the Current Buffer is Larger than what we want, Resize the 
	// Console Window First, then the Buffer 
	if ((DWORD)info.dwSize.X * info.dwSize.Y > (DWORD)bufSize.X * bufSize.Y)
	{
		if (!SetConsoleWindowInfo(m_hout, TRUE, &consoleWindowRect))
		{
			std::printf("Error unable to set console window size to %hux%hu\n",
				consoleWindowRect.Right, consoleWindowRect.Bottom);
			return false;
		}

		if (!SetConsoleScreenBufferSize(m_hout, bufSize))
		{
			std::printf("Error unable to set console screen buffer size to %hux%hu\n", bufSize.X, bufSize.Y);
			return false;
		}
	}
	// If the Current Buffer is Smaller than what we want, Resize the 
	// Buffer First, then the Console Window 
	else if ((DWORD)info.dwSize.X * info.dwSize.Y < (DWORD)bufSize.X * bufSize.Y)
	{
		if (!SetConsoleScreenBufferSize(m_hout, bufSize))
		{
			std::printf("Error unable to set console screen buffer size to %hux%hu\n", bufSize.X, bufSize.Y);
			return false;
		}
		
		if (!SetConsoleWindowInfo(m_hout, TRUE, &consoleWindowRect))
		{
			std::printf("Error unable to set console window size to %hux%hu\n",
				consoleWindowRect.Right, consoleWindowRect.Bottom);
			return false;
		}
	}
	else
	{
		// If the Current Buffer *is* the Size we want, Don't do anything! 
	}

    return true;
}

void Renderer::SetcursorVisibility(bool isVisible)
{
    CONSOLE_CURSOR_INFO info = { isVisible ? (DWORD)10 : (DWORD)100, isVisible ? TRUE : FALSE };
    SetConsoleCursorInfo(m_hout, &info);
}


void Renderer::Update(PlayField& world)
{
	FillCanvas(RS_BackgroundTile);

	// reserve memory for render items 
	// (each item have size of max(sizeof(RenderItemSprite), sizeof(RenderItemString)))
	m_renderList.resize(world.GameObjects().size() + world.StringObjects().size());
	int i = 0;
	for (auto it : world.GameObjects())
	{
		new(&m_renderList[i++])RenderItemSprite(it->GetPos(), it->GetSprite());
	}
	for (auto it : world.StringObjects())
	{
		new(&m_renderList[i++])RenderItemString(it->GetPos(), it->GetStr().c_str());
	}
	for (auto ri : m_renderList)
	{
		RenderItemBase *item = (RenderItemBase*)&ri;
		if (item->m_pos.x < 0 || item->m_pos.y < 0)
		{
			continue;
		}
		int x = int(item->m_pos.x);
		int y = int(item->m_pos.y);
		if (y >= m_renderBounds.y)
		{
			continue;
		}
		for(auto renderChars = item->GetSpriteCharsArray(); 
			*renderChars != 0 && x < m_renderBounds.x; renderChars++, x++)
		{
			*CurCanvas(x, y) = *renderChars;
		}
	}

	DrawCanvas();
}

void Renderer::FillCanvas(unsigned char m_sprite)
{
	memset(CurCanvas(0, 0), m_sprite, m_canvasSize);
}

void Renderer::DrawCanvas()
{
	// printing characters one by one on the screen can produce ugly effects,
	// it's much better to print the entire buffer at once
	auto handle = setCursorPosition(0, 0);
	WriteConsoleA(handle, (const char*)CurCanvas(0, 0), m_canvasSize, NULL, NULL);
}