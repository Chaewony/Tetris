#include "Position.h"

Position Position::up{ 0, 1 };
Position Position::down{ 0, -1 };
Position Position::right{ 1, 0 };
Position Position::left{ -1, 0 };
Position Position::zeros{ 0, 0 };
Position Position::ones{ 1, 1 };

//void Position::operator+=(Position& p1, const Position& p2) {
//	p1.x = p1.x + p2.x;
//	p1.y = p1.y + p2.y;
//}
/*Position operator-(const Position& p1, const Position& p2) {
	Position pos;
	pos.x = p1.x - p2.x;
	pos.y = p1.y - p2.y;
	return pos;
}*/