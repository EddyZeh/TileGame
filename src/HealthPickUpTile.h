#ifndef HEALTHPICKUPTILE_H
#define HEALTHPICKUPTILE_H

#include "ITile.h"

class HealthPickUpTile : public ITile{
    private:
        char _character;
        int _healthToGive;

    public:
        HealthPickUpTile(char ch, int healthToGive);
    
        virtual char GetCharacter() const;
        virtual bool CanEnter(IEntity &entity);
        virtual void Enter(IEntity &entity);
        virtual bool IsPickUp() const { return true; }
};

#endif