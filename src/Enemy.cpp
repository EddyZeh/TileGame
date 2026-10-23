#include <iostream>
#include "Enemy.h"

Enemy::Enemy(Level &level, char ch)
    : Actor(level, ch), _framecount(0){
        _direction = Vector2(1,0);
}
void Enemy::Collide(IEntity &entity){

}
void Enemy::Update(){
    if(rand() % 4 == 0)
        _weapons.top()->Fire(*this, _direction);

    if(_framecount++ < 15)
        return;
    
    _framecount = 0;
    
    auto dir = rand() % 4;
    if(dir == 0)
        Move(Vector2::North);
    else if(dir == 1)
        Move(Vector2::South);
    else if(dir == 2)
        Move(Vector2::East);
    else if(dir == 3)
        Move(Vector2::West);
}