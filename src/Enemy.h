#ifndef ENEMY_H
#define ENEMY_H

#include <vector>
#include "Actor.h"

// Tuning for one enemy. Built from the enemy type character and the level number,
// so later levels get faster, sharper and harder to hit.
struct EnemyProfile{
    int health;
    int moveInterval;     // frames between steps (lower = faster)
    int fireCooldown;     // frames between shots
    int reactionFrames;   // frames between first spotting the player and being able to shoot
    int sightRange;       // how far it can see / shoot, in tiles
    int memoryFrames;     // how long it keeps hunting after losing sight of the player
    int dodgePercent;     // chance to sidestep an incoming shot
    bool omniscient;      // always knows where the player is

    // type: '!' grunt, 'H' hunter (fast, sharp-eyed, hard to shake), 'B' boss (tanky, relentless, rapid fire)
    static EnemyProfile For(char type, int levelNumber);
};

// States, roughly:
//   unaware  -> wanders with momentum until it sees the player or gets shot
//   hunting  -> paths (BFS) to the nearest tile that lines up a shot, or to the
//               last place it saw the player, and gives up after its memory runs out
//   attacking-> holds still, aims for a moment, then fires along the row/column
//               (never through walls or other enemies)
//   dodging  -> sidesteps shots that are heading straight at it
class Enemy : public Actor{
    EnemyProfile _profile;
    IEntity *_target;

    int _stepTimer;
    int _fireTimer;
    int _dodgeTimer;
    int _sightTimer;      // frames the player has been in view (reaction time counts from first sight)
    int _memory;
    bool _hasLastKnown;
    Vector2 _lastKnown;
    Vector2 _wanderDir;
    int _wanderSteps;
    IEntity *_lastThreat;

    public:
    Enemy(Level &level, char ch, int levelNumber = 1);

    void SetTarget(IEntity *target);

    virtual void TakeDamage(int damage);
    virtual void Collide(IEntity &entity);
    virtual void Update();

    private:
    bool HasLineOfSight(const Vector2 &from, const Vector2 &to);
    bool CanSeeTarget();
    bool HasClearShot(Vector2 &direction);
    bool CanStepOn(const Vector2 &pos);
    bool TryStep(const Vector2 &direction);
    bool TryDodge();
    bool Sidestep(const Vector2 &lineOfFire);
    void Wander();
    void Pursue(bool seesTarget);
    bool FindStep(const Vector2 &goal, bool lineUpShot, Vector2 &step);
    bool Search(const std::vector<char> &isGoal, const std::vector<char> &occupied, Vector2 &step);
};

#endif
