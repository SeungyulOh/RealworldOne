#pragma once
#include "Randomization.h"

class Input
{
public:
	virtual bool Left() = 0;
	virtual bool Right() = 0;
	virtual bool Fire() = 0;
	virtual void Update() {}
};

class RndInput : public Input
{
public:
	virtual bool Fire() { return (getRandFloat(0.f, 1.f) < 0.5f); }
	virtual bool Left() { return (getRandFloat(0.f, 1.f) < 0.3f); }
	virtual bool Right() { return (getRandFloat(0.f, 1.f) < 0.4f); }
};

class KeyboardInput : public Input
{
protected:
	bool m_isLeftKeyDown = false;
	bool m_isRightKeyDown = false;
	bool m_isFirePressed = false;
public:
	KeyboardInput(){}
	virtual bool Left() { return m_isLeftKeyDown; }
	virtual bool Right() { return m_isRightKeyDown; }
	virtual bool Fire() { return m_isFirePressed; }
	virtual void Update();
};