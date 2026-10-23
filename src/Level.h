#ifndef LEVEL_H
#define LEVEL_H

#include "ITile.h"
#include "IEntity.h"
#include "Vector2.h"
#include <string>
#include <vector>

class IActor;
class IProjectile;

class Level{
    private:
        bool _hasEnded;
        int _deferDepth;
        int _width, _height;
        std::vector<ITile*> _tiles;
        std::vector<IEntity*> _entities;
        std::vector<IEntity*> _entitiesToRemove;

        // While one of these is alive, RemoveEntity only queues. The queue is flushed when the last one goes away.
        class DeferRemovals{
            Level &_level;
            public:
                DeferRemovals(Level &level);
                ~DeferRemovals();
        };

    public:
        Level(int width, int height);
        ~Level();
        Level(const Level&) = delete;
        Level& operator=(const Level&) = delete;

        int GetWidth() const;
        int GetHeight() const;

        void Endlevel();
        bool HasLevelEnded() const;

        void SetTile(ITile &tile, Vector2 pos);
        void SetTiles(ITile &tile, Vector2 from, Vector2 to);
        void SetEntity(IEntity *entity, Vector2 pos);
        bool MoveEntity(IEntity &entity, Vector2 pos);
        void RemoveEntity(IEntity *entity);

        // Takes ownership. Anything standing on the spawn tile is hit immediately,
        // a wall on the spawn tile destroys the projectile.
        bool SpawnProjectile(IProjectile *projectile, const Vector2 &pos);

        // Queries (used by enemy AI)
        bool InBounds(const Vector2 &pos) const;
        bool IsBlocked(const Vector2 &pos, IEntity &who) const;
        bool IsPickUp(const Vector2 &pos) const;
        IEntity* GetEntityAt(const Vector2 &position) const;
        IActor* GetActorAt(const Vector2 &position) const;
        const std::vector<IEntity*>& GetEntities() const;

        template<typename T>
        int CountEntitiesOfType() const{
            int count = 0;
            for(auto entity : _entities){
                if(dynamic_cast<T*>(entity) != nullptr)
                    count++;
            }
            return count;
        }

        void Update();

        void RenderAll();
        void DrawText(int x, int y, const std::string &text);

    private:
        int GetIndexForXY(int x, int y) const;
        void SetCharacter(char ch, int x, int y);
        void RedrawCell(int x, int y);
        void FlushRemovals();
};

#endif
