#ifndef IEFFECT_H
#define IEFFECT_H

class IEffect
{
public:
    virtual ~IEffect() = default;

    virtual void update(float deltaTime) = 0;

    virtual void trigger() { };

    virtual bool isFinished() const = 0;
};

#endif // IEFFECT_H
