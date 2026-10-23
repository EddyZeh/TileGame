#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <conio.h>
#include "Vector2.h"
#include "Level.h"
#include "SingleProjectile.h"
#include "Player.h"
#include "Fireball.h"
#include "WeaponPickUpTile.h"
#include "MultiFireball.h"
#include "Enemy.h"
#include "HealthPickUpTile.h"

using namespace std;

class BasicTile : public ITile{
    private:
        char _character;
        bool _canEnter;
    public:
        BasicTile(char ch, bool canEnter): _character(ch), _canEnter(canEnter){
        }
    
        virtual char GetCharacter() const { return _character; }
        virtual bool CanEnter(IEntity &entity){ return _canEnter; }
        virtual void Enter(IEntity &entity){ }
};

class BasicEntity : public IEntity{
    private:
        char _character;
        Vector2 _position;
    public:
        BasicEntity(char ch) : _character(ch){

        }
        virtual char GetCharacter() const{ return _character; }
        virtual const Vector2 &GetPosition() const { return _position; }
        virtual void SetPosition(const Vector2 &position){ _position = position;}
        virtual void Collide(IEntity &entity) { }
        virtual void Update() { }
};

int main(){
    ifstream levelStream("assets/level.txt");
    if(!levelStream){
        cout <<"Can't find level.txt!\n";
        cin.get();
        return 1;
    }

    vector<string> levelMap;

    int levelWidth = 0;

    while(!levelStream.eof()){
        string line;
        getline(levelStream, line);

        levelWidth = levelWidth < line.size() ? line.size() : levelWidth;

        levelMap.push_back(line);
    }

    levelStream.close();
    
    Level level(levelWidth,levelMap.size());

    BasicTile empty(' ', true);
    BasicTile wall('#', false);
    HealthPickUpTile health('+', 100);
    WeaponPickUpTile weapon1('%', new MultiFireball('^', level, 30));

    Player *player = nullptr;

    for(auto y = 0; y < levelMap.size(); y++){
        auto line = levelMap[y];
        for(auto x = 0; x < levelWidth; x++){
            char currentCell = x < line.length()? line[x] : ' ';
            if(currentCell == ' ')
                level.SetTile(empty,Vector2(x,y));
            else if(currentCell == '#')
                level.SetTile(wall ,Vector2(x,y));
            else if(currentCell == '+')
                level.SetTile(health, Vector2(x,y));
            else if(currentCell == '%')
                level.SetTile(weapon1, Vector2(x,y));
            else{
                level.SetTile(empty, Vector2(x,y));
                if(currentCell == '$'){
                    level.SetEntity(player = new Player(level, '$'), Vector2(x,y));
                    player->SetWeapon(new Fireball('*',level, 20));
                }
                else if(currentCell == '!'){
                    Enemy *enemy = nullptr;
                    level.SetEntity(enemy = new Enemy(level, '!'), Vector2(x,y));
                    enemy->SetWeapon(new Fireball('*',level, 20));
                }
            }
        }
    }



    char input = '\0';
    level.RenderAll();
   
    auto lastTime = timeGetTime();
    
    while(input != 27){
        if(level.HasLevelEnded()){
            cout <<"you died!";
            cin.get();
            break;
        }

        auto timeElapsed = timeGetTime() - lastTime;
            if(timeElapsed > 33.3){
                lastTime = timeGetTime();
                level.Update();
            }

        if(_kbhit()){
            
            input = _getch();
            auto pos = player->GetPosition();
            if(input == 'w'){
                player->Move(Vector2::North);
            }
            else if(input == 'a'){
                player->Move(Vector2::West);
            }
            else if(input == 's'){
                player->Move(Vector2::South);
            }
            else if(input == 'd'){
                player->Move(Vector2::East);
            }
            else if(input == ' '){
                player->FireProjectile();
            }
        }
    }

    return 0;
};