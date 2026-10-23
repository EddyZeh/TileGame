#ifndef ENEMY_H
#define ENEMY_H

#include  "Actor.h"

class Enemy : public Actor{

    int _framecount;
    public:
    Enemy(Level &level, char ch);
    

    virtual void Collide(IEntity &entity);
    virtual void Update();
};

#endif