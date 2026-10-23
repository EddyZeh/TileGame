#include "Vector2.h"

const Vector2 Vector2::North = Vector2(0,-1);
const Vector2 Vector2::South = Vector2(0,1);
const Vector2 Vector2::East = Vector2(1,0);
const Vector2 Vector2::West = Vector2(-1,0);

Vector2 :: Vector2(int x, int y): _x(x), _y(y){ }
Vector2::Vector2() : _x(0), _y(0){ }

int Vector2::GetX() const{ return _x; }
int Vector2::GetY() const{return _y; }

Vector2 Vector2::operator+(const Vector2 &rhs) const{
    Vector2 temp(*this);
    temp += rhs;
    return  temp;
}

Vector2& Vector2::operator+=(const Vector2 &rhs){
    _x += rhs._x;
    _y += rhs._y;
    return *this;
}

Vector2 Vector2::operator-(const Vector2 &rhs) const{
    Vector2 temp(*this);
    temp -= rhs;
    return  temp;
}

Vector2& Vector2::operator-=(const Vector2 &rhs){
    _x -= rhs._x;
    _y -= rhs._y;
    return *this;
}

bool Vector2::operator== (const Vector2 &rhs) const{
    return _x == rhs._x && _y == rhs._y;
}

bool Vector2::operator!= (const Vector2 &rhs) const{
    return !(*this == rhs);
}

// Vector2& Vector2::operator=(const Vector2 &pos){
//     Vector2 &pos2 = const_cast<Vector2&>(pos);
//     return pos2;
// }