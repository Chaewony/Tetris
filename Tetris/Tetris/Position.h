#pragma once
struct Position
{
	int x;
	int y;

	Position(int x = 0, int y = 0) : x(x), y(y) {}
	Position(const Position& other) : Position(other.x, other.y) {} // copy constructor


	static Position up;
	static Position right;
	static Position down;
	static Position left;
	static Position zeros;
	static Position ones;

	//static void operator+=(Position& p1, const Position& p2);
	friend static Position operator-(const Position& p1, const Position& p2);
	friend static Position operator+(const Position& p1, const Position& p2);
	friend static bool operator==(const Position& p1, const Position& p2);
};

Position operator-(const Position& p1, const Position& p2) {
	Position pos;
	pos.x = p1.x - p2.x;
	pos.y = p1.y - p2.y;
	return pos;
}
Position operator+(const Position& p1, const Position& p2) {
	Position pos;
	pos.x = p1.x + p2.x;
	pos.y = p1.y + p2.y;
	return pos;
}

bool operator==(const Position& p1, const Position& p2) {
	if (p1.x == p2.x && p1.y == p2.y)
		return true;
	else return false;
}

typedef Position Dimension;

