#define NOMINMAX
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <cstdlib>
#include <ctime>
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

const int FinalLevel = 10;

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

enum class Outcome { Won, Died, Quit, Error };

// Reads assets/level<N>.txt. Returns false if the file doesn't exist.
bool ReadLevelFile(int levelNumber, vector<string> &levelMap, int &levelWidth){
    ifstream levelStream("assets/level" + to_string(levelNumber) + ".txt");
    if(!levelStream)
        return false;

    levelMap.clear();
    levelWidth = 0;

    string line;
    while(getline(levelStream, line)){
        if(!line.empty() && line.back() == '\r')
            line.pop_back();

        levelWidth = levelWidth < (int)line.size() ? (int)line.size() : levelWidth;
        levelMap.push_back(line);
    }

    // ignore blank lines at the end of the file
    while(!levelMap.empty() && levelMap.back().empty())
        levelMap.pop_back();

    return !levelMap.empty();
}

void ClearScreen(){
    system("cls");
}

void FlushKeyboard(){
    while(_kbhit())
        _getch();
}

string Pad(string text, size_t width){
    if(text.size() < width)
        text.append(width - text.size(), ' ');
    return text;
}

// Plays one level. playerHealth is the health the player starts with, and on a win the health they finished with.
Outcome PlayLevel(int levelNumber, int &playerHealth){
    vector<string> levelMap;
    int levelWidth = 0;

    if(!ReadLevelFile(levelNumber, levelMap, levelWidth)){
        cout <<"Can't find assets/level" << levelNumber << ".txt!\n";
        cin.get();
        return Outcome::Error;
    }

    // weapons are shared with pickup tiles, so the game owns them and they outlive the level
    vector<unique_ptr<IWeapon>> weapons;
    auto own = [&weapons](IWeapon *weapon){
        weapons.emplace_back(weapon);
        return weapon;
    };

    Level level(levelWidth,(int)levelMap.size());

    BasicTile empty(' ', true);
    BasicTile wall('#', false);
    HealthPickUpTile health('+', 100);
    WeaponPickUpTile weapon1('%', own(new MultiFireball('^', level, 30)));

    Player *player = nullptr;
    vector<Enemy*> enemies;

    for(size_t y = 0; y < levelMap.size(); y++){
        auto line = levelMap[y];
        for(auto x = 0; x < levelWidth; x++){
            char currentCell = x < (int)line.length()? line[x] : ' ';
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
                    player->SetWeapon(own(new Fireball('*',level, 20)));
                }
                // '!' grunt, 'H' hunter, 'B' boss
                else if(currentCell == '!' || currentCell == 'H' || currentCell == 'B'){
                    Enemy *enemy = new Enemy(level, currentCell, levelNumber);
                    level.SetEntity(enemy, Vector2(x,y));
                    enemy->SetWeapon(own(new Fireball('o',level, 20)));
                    enemies.push_back(enemy);
                }
            }
        }
    }

    if(player == nullptr){
        cout <<"assets/level" << levelNumber << ".txt has no player ($)!\n";
        cin.get();
        return Outcome::Error;
    }

    player->SetHealth(playerHealth);
    for(auto enemy : enemies)
        enemy->SetTarget(player);

    ClearScreen();
    level.RenderAll();

    const int hudY = level.GetHeight() + 1;
    
    auto lastTime = timeGetTime();
    char input = '\0';
    Outcome outcome = Outcome::Quit;

    while(true){
        auto timeElapsed = timeGetTime() - lastTime;
        if(timeElapsed >= 33){
            lastTime = timeGetTime();
            level.Update();

            if(level.HasLevelEnded()){
                outcome = Outcome::Died;
                break;
            }

            int enemiesLeft = level.CountEntitiesOfType<Enemy>();
            playerHealth = player->GetHealth();

            level.DrawText(0, hudY, Pad("Level " + to_string(levelNumber) + "/" + to_string(FinalLevel)
                + "   HP: " + to_string(playerHealth)
                + "   Enemies: " + to_string(enemiesLeft)
                + "   [WASD] move  [SPACE] fire  [ESC] quit", 90));

            if(enemiesLeft == 0){
                outcome = Outcome::Won;
                break;
            }
        }

        if(_kbhit()){
            input = _getch();
            if(input >= 'A' && input <= 'Z')
                input = input - 'A' + 'a';

            if(input == 27){
                outcome = Outcome::Quit;
                break;
            }
            else if(input == 'w'){
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

            // the player can die from their own move (walking into a shot), and is gone from the level by now
            if(level.HasLevelEnded()){
                outcome = Outcome::Died;
                break;
            }
        }

        Sleep(1);
    }

    return outcome;
}

int main(int argc, char *argv[]){
    srand((unsigned)time(nullptr));

    // optional: TileGame.exe <level> starts at that level (1-10)
    int levelNumber = 1;
    if(argc > 1){
        levelNumber = atoi(argv[1]);
        if(levelNumber < 1 || levelNumber > FinalLevel)
            levelNumber = 1;
    }
    int playerHealth = 100;

    while(levelNumber <= FinalLevel){
        int startHealth = playerHealth;
        Outcome outcome = PlayLevel(levelNumber, playerHealth);

        if(outcome == Outcome::Error)
            return 1;
        if(outcome == Outcome::Quit)
            return 0;

        ClearScreen();

        if(outcome == Outcome::Died){
            cout <<"You died on level " << levelNumber << "!\n\nPress R to retry the level, any other key to quit.\n";
            FlushKeyboard();
            char choice = _getch();
            if(choice != 'r' && choice != 'R')
                return 0;

            playerHealth = startHealth;
            continue;
        }

        // won the level: top the player up a bit and move on
        playerHealth = playerHealth + 30;
        if(playerHealth > 100) playerHealth = 100;
        if(playerHealth < 60) playerHealth = 60;

        if(levelNumber < FinalLevel){
            cout <<"Level " << levelNumber << " complete!\n";
            Sleep(1500);
            FlushKeyboard();
        }
        levelNumber++;
    }

    cout <<"You cleared all " << FinalLevel << " levels. You win!\n";
    cin.get();
    return 0;
};
