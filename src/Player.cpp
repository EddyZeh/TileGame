#include <iostream>
#include "Player.h"
#include "SingleProjectile.h"

Player::Player(Level &level, char ch)
    : Actor(level, ch){
        _direction = Vector2::East;
}

void Player::TakeDamage(int damage){
    // std::cout <<"Took damage!";
    Actor::TakeDamage(damage);
    if(GetHealth() <= 0){
        _level.Endlevel();
    }
}

void Player::Collide(IEntity &entity){
    
}
void Player::Update(){

}


void Player::FireProjectile(){
    if(_weapons.empty())
        return;

    if(!_weapons.top()->isActive(*this)){
        _weapons.pop();

        if(_weapons.empty())
            return;
    }

    _weapons.top()->Fire(*this, _direction);
}
