#ifndef MULTIFIREBALL_H
#define MULTIFIREBALL_H

#include <map>
#include "IWeapon.h"
#include "Level.h"
#include "IEntity.h"

class MultiFireball : public IWeapon{
    char _character;
    Level &_level;
    int _damage;
    std::map<IEntity *, int> _ammo;
    
    public:
        MultiFireball(char character, Level &level, int damage);
        virtual void Attach(IEntity &entity);
        virtual void Fire(IEntity &entity, const Vector2 &direction);
        virtual bool isActive(IEntity &entity);

    private:
        void CreateProjectile(const Vector2 &position, const Vector2 &direction);
};
#endif