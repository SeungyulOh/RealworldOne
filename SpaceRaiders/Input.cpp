
#include "stdafx.h"
#include "Input.h"
#include <Windows.h>

void KeyboardInput::Update()
{
	static const HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
	DWORD inputEventsCount = 0, inputRecordsCount = 0;
	// ReadConsoleInput(...) is blocking so we will firstly check
	// how many events is available right now to determine
	// how many records we can read by ReadConsoleInput()
	// without blocking
	if (!GetNumberOfConsoleInputEvents(hIn, &inputEventsCount))
	{
		return;
	}
	
	for (DWORD i = 0; i < inputEventsCount; i++)
	{
		INPUT_RECORD in;
		if (!ReadConsoleInput(hIn, &in, 1, &inputRecordsCount) 
			|| inputRecordsCount == 0 || in.EventType != KEY_EVENT)
		{
			continue;
		}
		bool isPressed = in.Event.KeyEvent.bKeyDown == TRUE;
		switch (in.Event.KeyEvent.wVirtualKeyCode)
		{
		case VK_LEFT:
			m_isLeftKeyDown = isPressed;
			break;
		case VK_RIGHT:
			m_isRightKeyDown = isPressed;
			break;
		case 'F':
			m_isFirePressed = isPressed;
			break;
		}
	}
}