#ifndef ACTOR_H
#define ACTOR_H

#include <stack>
#include "IEntity.h"
#include "Level.h"
#include "IWeapon.h"
#include "IActor.h"

class Actor : public IEntity, public IActor{
    char _character;
    Vector2 _position;
    int _health;

    protected:
        Vector2 _direction;
        std::stack<IWeapon *> _weapons;
        Level &_level;

    public:
    Actor(Level &level, char ch);
    
    virtual void Move(const Vector2 &direction);

    virtual IWeapon* GetWeapon(); 
    virtual void SetWeapon(IWeapon *weapon);
    virtual int GetHealth() const;
    virtual void TakeDamage(int damage);
    
    virtual char GetCharacter() const;
    virtual const Vector2 &GetPosition() const;
    virtual void SetPosition(const Vector2 &position);

};

#endif