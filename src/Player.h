#ifndef PLAYER_H
#define PLAYER_H

#include <stack>
#include "IEntity.h"
#include "Level.h"
#include "IWeapon.h"
#include "Actor.h"


class Player : public Actor{
    public:
    Player(Level &level, char ch);
    
    void FireProjectile();

    virtual void TakeDamage(int damage);
    virtual void Collide(IEntity &entity);
    virtual void Update();

};

#endif