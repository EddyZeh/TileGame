#ifndef IPROJECTILE_H
#define IPROJECTILE_H

#include "IEntity.h"
#include "Vector2.h"

class IProjectile : public IEntity{
    public:
        virtual int GetDamage() = 0;
        virtual Vector2 GetVelocity() const = 0;
};
 
#endif
