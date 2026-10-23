#include <iostream>
#include <windows.h>
#include <algorithm>
#include "Level.h"
using namespace std;


Level::Level(int width, int height): _width(width), _height(height), _tiles(width * height), _hasEnded(false){

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

void Level::RemoveEntity(IEntity *entity){
    if(_isUpdating){
        for(auto i = _entitiesToRemove.begin(); i != _entitiesToRemove.end(); i++){
            if((*i) == entity)
                return;
        }
        _entitiesToRemove.push_back(entity);
        // if(std::find(_entitiesToRemove.begin(), _entitiesToRemove.end(), entity) == _entitiesToRemove.end()){
        //     _entitiesToRemove.push_back(entity);
        // }
        return;
    }

    
    Vector2 position = entity->GetPosition();
    auto tile = _tiles[GetIndexForXY(position.GetX(), position.GetY())];

    for(auto i = _entities.begin(); i != _entities.end(); i++){
        if((*i) != entity)
            continue;

        delete (*i);
        _entities.erase(i);
        break;
    }

    SetCharacter(tile->GetCharacter(), position.GetX(), position.GetY());
}

bool Level ::MoveEntity(IEntity &entity, Vector2 pos){
    if(pos.GetX() < 0 || pos.GetX() >= _width || pos.GetY() < 0 || pos.GetY() >= _height)
        return false;
    
    auto tileToEnter = _tiles[GetIndexForXY(pos.GetX(), pos.GetY())];
    if(!tileToEnter->CanEnter(entity))
        return false;

    auto entityToEnter = GetEntityAt(pos);
    if(entityToEnter != nullptr){
            entityToEnter->Collide(entity);
            entity.Collide(*entityToEnter);            
        }

    tileToEnter->Enter(entity);

    auto oldPos = entity.GetPosition();
    auto oldTile = _tiles[GetIndexForXY(oldPos.GetX(), oldPos.GetY())];

    SetCharacter(oldTile->GetCharacter(), oldPos.GetX(), oldPos.GetY());
    SetCharacter(entity.GetCharacter(), pos.GetX(), pos.GetY());

    entity.SetPosition(pos);
    return true;
}

void Level ::RenderAll(){
    for(auto x=0; x < _width; x++){
        for(auto y = 0; y < _height; y++){
            auto tile = _tiles[GetIndexForXY(x,y)];
            cout << tile->GetCharacter();
            SetCharacter(tile->GetCharacter(), x, y);
        }
    }

    for(auto i = _entities.begin(); i!= _entities.end(); i++){
        auto entity = *i;
        const Vector2& pos = entity->GetPosition();
        SetCharacter(entity->GetCharacter(), pos.GetX(), pos.GetY());
    }
}

IEntity* Level::GetEntityAt(const Vector2 &position) const{
    for(auto i = _entities.begin(); i != _entities.end(); i++){
        if((*i)->GetPosition() == position)
            return (*i);
    }

    return nullptr;
}

int Level::GetIndexForXY(int x, int y) const{
    return _width * y + x;
}

void Level::SetCharacter(char ch, int x, int y){
    COORD pos = {x,y};
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(output, pos);
    cout <<ch; 
}

void Level::Update(){
    _isUpdating = true;

    for(auto i = 0; i < _entities.size(); i++){
        _entities[i]->Update();
    }

    _isUpdating = false;

    for(auto i = _entitiesToRemove.begin(); i != _entitiesToRemove.end(); i++){
        RemoveEntity(*i);
    }
    _entitiesToRemove.clear();

}

