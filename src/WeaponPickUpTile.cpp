#include "WeaponPickUpTile.h"
#include "IActor.h"

WeaponPickUpTile::WeaponPickUpTile(char ch, IWeapon *weapon)
    : _character(ch), _weapon(weapon){ }

char WeaponPickUpTile::GetCharacter() const{
    return _character;
}

bool WeaponPickUpTile::CanEnter(IEntity &entity){
    return true;
}

void WeaponPickUpTile::Enter(IEntity &entity){
    auto actor = dynamic_cast<IActor*>(&entity);

    if(actor == nullptr)
        return;

    actor->SetWeapon(_weapon);
}