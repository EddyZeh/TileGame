#ifndef IACTOR_H
#define IACTOR_H

#include "IWeapon.h"

class IActor{
    public:
    virtual ~IActor() {}
    virtual IWeapon* GetWeapon() = 0;
    virtual void SetWeapon(IWeapon *weapon) = 0;
    virtual int GetHealth() const = 0;
    virtual void TakeDamage(int damage) = 0;
};

#endif
