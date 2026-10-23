#ifndef IENTITY_H
#define IENTITY_H

#include "Vector2.h"

class IEntity{
    public:
        virtual ~IEntity() {}
        virtual char GetCharacter() const = 0;
        virtual const Vector2 &GetPosition() const = 0;
        virtual void SetPosition(const Vector2 &position) = 0;
        virtual void Collide(IEntity &entity)  = 0;
        virtual void Update() = 0;
};

#endif
