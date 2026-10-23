#define NOMINMAX
#include <iostream>
#include <windows.h>
#include <algorithm>
#include "Level.h"
#include "IActor.h"
#include "IProjectile.h"
using namespace std;


Level::DeferRemovals::DeferRemovals(Level &level) : _level(level){
    _level._deferDepth++;
}
Level::DeferRemovals::~DeferRemovals(){
    if(--_level._deferDepth == 0)
        _level.FlushRemovals();
}

Level::Level(int width, int height)
    : _hasEnded(false), _deferDepth(0), _width(width), _height(height), _tiles(width * height){

}

Level::~Level(){
    for(auto entity : _entities)
        delete entity;
}

int Level ::GetWidth() const{ return _width; }
int Level ::GetHeight() const{ return _height; }

void Level ::Endlevel(){
    _hasEnded = true;
}
bool Level ::HasLevelEnded() const{
    return _hasEnded;
}

void Level ::SetTile(ITile &tile, Vector2 pos){
    
    _tiles[GetIndexForXY(pos.GetX(),pos.GetY())] = &tile;
}

void Level ::SetTiles(ITile &tile, Vector2 from, Vector2 to){
    for(auto x = from.GetX(); x <= to.GetX(); x++){
        for(auto y = from.GetY(); y <= to.GetY(); y++){
            _tiles[GetIndexForXY(x,y)] = &tile;
        }
    }
}

void Level ::SetEntity(IEntity *entity, Vector2 pos){

    auto tile = _tiles[GetIndexForXY(pos.GetX(), pos.GetY())];

    if(!tile->CanEnter(*entity))
        return;

    _entities.push_back(entity);
    entity->SetPosition(pos);

    SetCharacter(entity->GetCharacter(), pos.GetX(), pos.GetY());
}

bool Level::SpawnProjectile(IProjectile *projectile, const Vector2 &pos){
    if(!InBounds(pos) || !_tiles[GetIndexForXY(pos.GetX(), pos.GetY())]->CanEnter(*projectile)){
        delete projectile;
        return false;
    }

    DeferRemovals guard(*this);

    auto target = GetActorAt(pos);
    if(target != nullptr){
        target->TakeDamage(projectile->GetDamage());
        delete projectile;
        return true;
    }

    SetEntity(projectile, pos);
    return true;
}

void Level::RemoveEntity(IEntity *entity){
    if(std::find(_entitiesToRemove.begin(), _entitiesToRemove.end(), entity) == _entitiesToRemove.end())
        _entitiesToRemove.push_back(entity);

    if(_deferDepth == 0)
        FlushRemovals();
}

void Level::FlushRemovals(){
    for(auto entity : _entitiesToRemove){
        auto found = std::find(_entities.begin(), _entities.end(), entity);
        if(found == _entities.end())
            continue;

        Vector2 position = entity->GetPosition();
        delete entity;
        _entities.erase(found);

        RedrawCell(position.GetX(), position.GetY());
    }
    _entitiesToRemove.clear();
}

bool Level ::MoveEntity(IEntity &entity, Vector2 pos){
    if(!InBounds(pos))
        return false;
    
    auto tileToEnter = _tiles[GetIndexForXY(pos.GetX(), pos.GetY())];
    if(!tileToEnter->CanEnter(entity))
        return false;

    // actors can't walk through each other
    if(dynamic_cast<IActor*>(&entity) != nullptr && GetActorAt(pos) != nullptr)
        return false;

    DeferRemovals guard(*this);

    for(size_t i = 0; i < _entities.size(); i++){
        auto other = _entities[i];
        if(other == &entity || other->GetPosition() != pos)
            continue;

        other->Collide(entity);
        entity.Collide(*other);
    }

    tileToEnter->Enter(entity);

    auto oldPos = entity.GetPosition();
    entity.SetPosition(pos);

    RedrawCell(oldPos.GetX(), oldPos.GetY());
    SetCharacter(entity.GetCharacter(), pos.GetX(), pos.GetY());

    return true;
}

bool Level::InBounds(const Vector2 &pos) const{
    return pos.GetX() >= 0 && pos.GetX() < _width && pos.GetY() >= 0 && pos.GetY() < _height;
}

bool Level::IsBlocked(const Vector2 &pos, IEntity &who) const{
    if(!InBounds(pos))
        return true;
    return !_tiles[GetIndexForXY(pos.GetX(), pos.GetY())]->CanEnter(who);
}

bool Level::IsPickUp(const Vector2 &pos) const{
    if(!InBounds(pos))
        return false;
    return _tiles[GetIndexForXY(pos.GetX(), pos.GetY())]->IsPickUp();
}

const std::vector<IEntity*>& Level::GetEntities() const{
    return _entities;
}

void Level ::RenderAll(){
    for(auto x=0; x < _width; x++){
        for(auto y = 0; y < _height; y++){
            auto tile = _tiles[GetIndexForXY(x,y)];
            SetCharacter(tile->GetCharacter(), x, y);
        }
    }

    for(auto i = _entities.begin(); i!= _entities.end(); i++){
        auto entity = *i;
        const Vector2& pos = entity->GetPosition();
        SetCharacter(entity->GetCharacter(), pos.GetX(), pos.GetY());
    }
}

void Level::DrawText(int x, int y, const std::string &text){
    COORD pos = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
    cout << text;
}

IEntity* Level::GetEntityAt(const Vector2 &position) const{
    for(auto i = _entities.begin(); i != _entities.end(); i++){
        if((*i)->GetPosition() == position)
            return (*i);
    }

    return nullptr;
}

IActor* Level::GetActorAt(const Vector2 &position) const{
    for(auto i = _entities.begin(); i != _entities.end(); i++){
        if((*i)->GetPosition() != position)
            continue;
        auto actor = dynamic_cast<IActor*>(*i);
        if(actor != nullptr)
            return actor;
    }

    return nullptr;
}

int Level::GetIndexForXY(int x, int y) const{
    return _width * y + x;
}

void Level::SetCharacter(char ch, int x, int y){
    COORD pos = { (SHORT)x, (SHORT)y };
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(output, pos);
    cout <<ch; 
}

// draws whatever should be visible in a cell: an entity if there is one, otherwise the tile
void Level::RedrawCell(int x, int y){
    auto entity = GetEntityAt(Vector2(x, y));
    if(entity != nullptr)
        SetCharacter(entity->GetCharacter(), x, y);
    else
        SetCharacter(_tiles[GetIndexForXY(x, y)]->GetCharacter(), x, y);
}

void Level::Update(){
    DeferRemovals guard(*this);

    for(size_t i = 0; i < _entities.size(); i++){
        _entities[i]->Update();
    }
}
