#ifndef WEAPONPICKUPTILE_H
#define WEAPONPICKUPTILE_H

#include "ITile.h"
#include "IWeapon.h"

class WeaponPickUpTile : public ITile{
    private:
        char _character;
        IWeapon *_weapon;
    public:
        WeaponPickUpTile(char ch, IWeapon *weapon);
    
        virtual char GetCharacter() const;
        virtual bool CanEnter(IEntity &entity);
        virtual void Enter(IEntity &entity);
        virtual bool IsPickUp() const { return true; }
};

#endif