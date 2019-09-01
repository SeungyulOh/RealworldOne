#pragma once

struct Vector2D
{
public:
	Vector2D() : x(0), y(0) {}

	Vector2D(const Vector2D& vector) : x(vector.x), y(vector.y){}

	Vector2D(float x, float y) : x(x), y(y) {}
	~Vector2D() {}

	bool IntCmp(const Vector2D& vec) const { return int(x) == int(vec.x) && int(y) == int(vec.y); }
	// Operator overloading
	Vector2D operator + (const Vector2D& other) const
	{
		return Vector2D(other.x + this->x, other.y + this->y);
	}

	Vector2D& operator += (const Vector2D& other)
	{
		this->x += other.x;
		this->y += other.y;
		return *this;
	}
#define FLOOR(x) ((float)(int)(x))

	Vector2D Floor() const
	{
		return Vector2D(FLOOR(x), FLOOR(y));
	}

	bool operator == (const Vector2D& other)
	{
		return this->x == other.x && this->y == other.y;
	}

	float x;
	float y;
};

