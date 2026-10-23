#ifndef ITILE_H
#define ITILE_H

#include "IEntity.h"

class ITile{
    public:
        virtual ~ITile() {}
        virtual char GetCharacter() const= 0;
        virtual bool CanEnter(IEntity &entity) = 0;
        virtual void Enter(IEntity &entity) = 0;

        // Health / weapon tiles return true. Enemy AI never walks onto these on purpose.
        virtual bool IsPickUp() const { return false; }
};

#endif
