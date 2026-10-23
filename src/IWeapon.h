#ifndef IWEAPON_H
#define IWEAPON_H

#include "IEntity.h"
#include "Vector2.h"

class IWeapon{
    public:
        virtual void Attach(IEntity &entity) = 0;
        virtual void Fire(IEntity &entity, const Vector2 &pos) = 0;
        virtual bool isActive(IEntity &entity) = 0;
};

#endif