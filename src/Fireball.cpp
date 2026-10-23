#include "Fireball.h"
#include "SingleProjectile.h"

Fireball::Fireball(char character, Level &level, int damage)
    : _character(character), _level(level), _damage(damage){ }

void Fireball::Attach(IEntity &entity){

}
void Fireball::Fire(IEntity &entity, const Vector2 &direction){
    _level.SpawnProjectile(new SingleProjectile(_level, _damage, _character, direction), entity.GetPosition() + direction);
}

bool Fireball::isActive(IEntity &entity){
    return true;
}