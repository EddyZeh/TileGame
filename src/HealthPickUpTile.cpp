#include "HealthPickUpTile.h"
#include "IActor.h"

HealthPickUpTile::HealthPickUpTile(char ch, int healthToGive)
    : _character(ch), _healthToGive(healthToGive){ }
    
char HealthPickUpTile::GetCharacter() const{
    return _character;
}
bool HealthPickUpTile::CanEnter(IEntity &entity){
    return true;
}
void HealthPickUpTile::Enter(IEntity &entity){
    auto actor = dynamic_cast<IActor*>(&entity);

    if(actor == nullptr)
        return;

    actor->TakeDamage(-_healthToGive);
}