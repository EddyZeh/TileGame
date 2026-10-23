#include <cstdlib>
#include <algorithm>
#include <queue>
#include "Enemy.h"
#include "IProjectile.h"

namespace {
    const Vector2 Directions[4] = { Vector2(0, -1), Vector2(0, 1), Vector2(1, 0), Vector2(-1, 0) };
    const int DodgeRange = 10;

    int Sign(int value){
        return value > 0 ? 1 : (value < 0 ? -1 : 0);
    }
}

EnemyProfile EnemyProfile::For(char type, int levelNumber){
    int n = levelNumber < 1 ? 0 : (levelNumber > 10 ? 9 : levelNumber - 1);   // 0..9

    EnemyProfile p;
    p.health         = 60 + n * 8;
    p.moveInterval   = 14 - (n * 8) / 9;     // 14 -> 6
    p.fireCooldown   = 60 - n * 4;           // 60 -> 24
    p.reactionFrames = 18 - (n * 13) / 9;    // 18 -> 5
    p.sightRange     = 12 + n * 2;           // 12 -> 30
    p.memoryFrames   = 60 + n * 15;          // 2s -> 6.5s
    p.dodgePercent   = 10 + n * 7;           // 10% -> 73%
    p.omniscient     = false;

    if(type == 'H'){
        p.moveInterval = std::max(4, p.moveInterval - 3);
        p.sightRange  += 6;
        p.memoryFrames *= 3;
        p.dodgePercent += 15;
    }
    else if(type == 'B'){
        p.health         *= 2;
        p.moveInterval   += 3;
        p.fireCooldown    = std::max(20, p.fireCooldown * 2 / 3);
        p.reactionFrames /= 2;
        p.omniscient      = true;
    }

    p.dodgePercent = std::min(p.dodgePercent, 85);
    return p;
}

Enemy::Enemy(Level &level, char ch, int levelNumber)
    : Actor(level, ch), _profile(EnemyProfile::For(ch, levelNumber)), _target(nullptr),
      _stepTimer(0), _fireTimer(0), _dodgeTimer(0), _sightTimer(0), _memory(0),
      _hasLastKnown(false), _wanderDir(Vector2::East), _wanderSteps(0), _lastThreat(nullptr){

    _direction = Vector2::East;
    SetHealth(_profile.health);

    // stagger enemies so a group doesn't move and fire in lockstep
    _stepTimer = rand() % _profile.moveInterval;
    _fireTimer = rand() % _profile.fireCooldown;
}

void Enemy::SetTarget(IEntity *target){
    _target = target;
}

void Enemy::Collide(IEntity &entity){

}

// Being shot tells the enemy where the player is, even through walls
void Enemy::TakeDamage(int damage){
    Actor::TakeDamage(damage);

    if(damage > 0 && _target != nullptr){
        _lastKnown = _target->GetPosition();
        _hasLastKnown = true;
        _memory = _profile.memoryFrames;
        _sightTimer = _profile.reactionFrames;   // no need to "notice" anything, it just got shot
    }
}

void Enemy::Update(){
    if(_target == nullptr || GetWeapon() == nullptr)
        return;

    if(_fireTimer > 0)  _fireTimer--;
    if(_dodgeTimer > 0) _dodgeTimer--;
    _stepTimer++;

    bool sees = _profile.omniscient || CanSeeTarget();
    if(sees){
        _lastKnown = _target->GetPosition();
        _hasLastKnown = true;
        _memory = _profile.memoryFrames;
    }
    else if(_memory > 0){
        _memory--;
    }

    if(sees)
        _sightTimer++;
    else if(_memory == 0)
        _sightTimer = 0;

    // 1. get out of the way of incoming fire
    if(TryDodge())
        return;

    // 2. shoot if there is a clear line: stand still while it reacts, then fire
    Vector2 shotDirection;
    if(sees && HasClearShot(shotDirection)){
        _direction = shotDirection;

        if(_sightTimer >= _profile.reactionFrames && _fireTimer == 0){
            GetWeapon()->Fire(*this, shotDirection);
            _fireTimer = _profile.fireCooldown + rand() % (_profile.fireCooldown / 3 + 1);

            // shoot and scoot: skilled enemies often step out of the line of fire afterwards
            if(rand() % 100 < _profile.dodgePercent / 2 && Sidestep(shotDirection))
                _dodgeTimer = 8;
        }
        return;
    }

    // 3. otherwise move
    if(_stepTimer < _profile.moveInterval)
        return;
    _stepTimer = 0;

    if(sees || (_hasLastKnown && _memory > 0))
        Pursue(sees);
    else
        Wander();
}

// ---- perception -----------------------------------------------------------

// Bresenham walk between two tiles; only walls block sight
bool Enemy::HasLineOfSight(const Vector2 &from, const Vector2 &to){
    int x0 = from.GetX(), y0 = from.GetY();
    int x1 = to.GetX(),   y1 = to.GetY();
    int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while(true){
        int e2 = 2 * err;
        if(e2 >= dy){ err += dy; x0 += sx; }
        if(e2 <= dx){ err += dx; y0 += sy; }

        if(x0 == x1 && y0 == y1)
            return true;
        if(_level.IsBlocked(Vector2(x0, y0), *this))
            return false;
    }
}

bool Enemy::CanSeeTarget(){
    const Vector2 &from = GetPosition();
    const Vector2 &to = _target->GetPosition();

    int dx = to.GetX() - from.GetX();
    int dy = to.GetY() - from.GetY();
    if(dx * dx + dy * dy > _profile.sightRange * _profile.sightRange)
        return false;

    return HasLineOfSight(from, to);
}

// Shots only travel along rows and columns. Walls and other actors in between block it.
bool Enemy::HasClearShot(Vector2 &direction){
    const Vector2 &from = GetPosition();
    const Vector2 &to = _target->GetPosition();

    int dx = to.GetX() - from.GetX();
    int dy = to.GetY() - from.GetY();
    if((dx != 0 && dy != 0) || (dx == 0 && dy == 0))
        return false;

    int distance = std::abs(dx) + std::abs(dy);
    if(distance > _profile.sightRange)
        return false;

    direction = Vector2(Sign(dx), Sign(dy));

    Vector2 pos = from + direction;
    for(int i = 1; i < distance; i++, pos += direction){
        if(_level.IsBlocked(pos, *this) || _level.GetActorAt(pos) != nullptr)
            return false;
    }
    return true;
}

// ---- movement ---------------------------------------------------------------

bool Enemy::CanStepOn(const Vector2 &pos){
    return !_level.IsBlocked(pos, *this) && !_level.IsPickUp(pos) && _level.GetEntityAt(pos) == nullptr;
}

bool Enemy::TryStep(const Vector2 &direction){
    if(!CanStepOn(GetPosition() + direction))
        return false;
    Move(direction);
    return true;
}

// Looks for a projectile travelling straight at us and, depending on skill, sidesteps it
bool Enemy::TryDodge(){
    if(_dodgeTimer > 0)
        return false;

    const Vector2 me = GetPosition();
    IEntity *threat = nullptr;
    Vector2 threatVelocity;
    int nearest = DodgeRange + 1;

    for(auto entity : _level.GetEntities()){
        auto projectile = dynamic_cast<IProjectile*>(entity);
        if(projectile == nullptr)
            continue;

        Vector2 v = projectile->GetVelocity();
        Vector2 d = me - projectile->GetPosition();
        int distance = std::abs(d.GetX()) + std::abs(d.GetY());
        if(distance == 0 || distance >= nearest)
            continue;

        bool incoming = (v.GetX() != 0 && d.GetY() == 0 && Sign(d.GetX()) == Sign(v.GetX())) ||
                        (v.GetY() != 0 && d.GetX() == 0 && Sign(d.GetY()) == Sign(v.GetY()));
        if(!incoming)
            continue;

        nearest = distance;
        threat = entity;
        threatVelocity = v;
    }

    if(threat == nullptr){
        _lastThreat = nullptr;
        return false;
    }

    // decide once per projectile, otherwise the odds would compound every frame
    if(threat == _lastThreat)
        return false;
    _lastThreat = threat;

    if(rand() % 100 >= _profile.dodgePercent)
        return false;

    if(Sidestep(threatVelocity)){
        _dodgeTimer = 8;
        return true;
    }
    return false;
}

// Steps one tile perpendicular to a line of fire (either side, random order)
bool Enemy::Sidestep(const Vector2 &lineOfFire){
    Vector2 side(std::abs(lineOfFire.GetY()), std::abs(lineOfFire.GetX()));
    Vector2 options[2] = { side, Vector2(-side.GetX(), -side.GetY()) };
    if(rand() % 2 == 0)
        std::swap(options[0], options[1]);

    for(auto &option : options){
        if(TryStep(option))
            return true;
    }
    return false;
}

// Random walk with momentum: keeps going the same way for a few steps, rarely turns back, sometimes pauses
void Enemy::Wander(){
    if(rand() % 6 == 0)
        return;

    if(_wanderSteps > 0 && TryStep(_wanderDir)){
        _wanderSteps--;
        return;
    }

    Vector2 reverse(-_wanderDir.GetX(), -_wanderDir.GetY());
    std::vector<Vector2> options;
    for(auto &d : Directions){
        if(d != reverse && CanStepOn(GetPosition() + d))
            options.push_back(d);
    }
    if(options.empty() && CanStepOn(GetPosition() + reverse))
        options.push_back(reverse);
    if(options.empty())
        return;

    _wanderDir = options[rand() % options.size()];
    _wanderSteps = 2 + rand() % 6;
    TryStep(_wanderDir);
}

void Enemy::Pursue(bool seesTarget){
    Vector2 step;

    if(seesTarget){
        // head for a tile that lines up a shot rather than walking into melee range
        const Vector2 goal = _target->GetPosition();
        if(FindStep(goal, true, step) || FindStep(goal, false, step)){
            TryStep(step);
            return;
        }
        Wander();
        return;
    }

    // lost sight: go to where the player was last seen, then give up and search around
    if(GetPosition() == _lastKnown){
        _hasLastKnown = false;
        _memory = 0;
        Wander();
        return;
    }
    if(FindStep(_lastKnown, false, step))
        TryStep(step);
    else
        Wander();
}

// ---- path finding -----------------------------------------------------------

// lineUpShot = true : goal tiles are the open tiles on the player's row/column (3+ tiles away if possible)
// lineUpShot = false: the goal is the tile 'goal' itself
bool Enemy::FindStep(const Vector2 &goal, bool lineUpShot, Vector2 &step){
    const int w = _level.GetWidth();
    const int h = _level.GetHeight();

    std::vector<char> occupied(w * h, 0);
    for(auto entity : _level.GetEntities()){
        if(entity == this || dynamic_cast<IProjectile*>(entity) != nullptr)
            continue;
        const Vector2 &p = entity->GetPosition();
        if(_level.InBounds(p))
            occupied[p.GetY() * w + p.GetX()] = 1;
    }

    const int minDistances[2] = { 3, 1 };
    for(int minDistance : minDistances){
        std::vector<char> isGoal(w * h, 0);
        bool any = false;

        if(lineUpShot){
            for(auto &d : Directions){
                Vector2 p = goal + d;
                for(int k = 1; k <= _profile.sightRange; k++, p += d){
                    if(_level.IsBlocked(p, *this) || occupied[p.GetY() * w + p.GetX()])
                        break;
                    if(k >= minDistance && !_level.IsPickUp(p)){
                        isGoal[p.GetY() * w + p.GetX()] = 1;
                        any = true;
                    }
                }
            }
        }
        else if(_level.InBounds(goal)){
            isGoal[goal.GetY() * w + goal.GetX()] = 1;
            any = true;
        }

        if(any && Search(isGoal, occupied, step))
            return true;
        if(!lineUpShot)
            break;
    }
    return false;
}

// Breadth-first search from our tile to the nearest goal tile, returns the first step of that path
bool Enemy::Search(const std::vector<char> &isGoal, const std::vector<char> &occupied, Vector2 &step){
    const int w = _level.GetWidth();
    const int h = _level.GetHeight();

    std::vector<char> seen(w * h, 0);
    std::vector<signed char> firstStep(w * h, -1);
    std::queue<Vector2> frontier;

    const Vector2 start = GetPosition();
    seen[start.GetY() * w + start.GetX()] = 1;
    frontier.push(start);

    while(!frontier.empty()){
        Vector2 current = frontier.front();
        frontier.pop();

        // random neighbour order so enemies don't all pick identical routes
        int order[4] = { 0, 1, 2, 3 };
        for(int i = 3; i > 0; i--)
            std::swap(order[i], order[rand() % (i + 1)]);

        for(int i = 0; i < 4; i++){
            Vector2 next = current + Directions[order[i]];
            if(!_level.InBounds(next))
                continue;

            int index = next.GetY() * w + next.GetX();
            if(seen[index])
                continue;

            bool goal = isGoal[index] != 0;
            if(_level.IsBlocked(next, *this))
                continue;
            if(!goal && (occupied[index] || _level.IsPickUp(next)))
                continue;

            seen[index] = 1;
            firstStep[index] = (current == start) ? order[i] : firstStep[current.GetY() * w + current.GetX()];

            if(goal){
                step = Directions[firstStep[index]];
                return true;
            }
            frontier.push(next);
        }
    }
    return false;
}
