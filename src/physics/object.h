#ifndef PARTICLE_H
#define PARTICLE_H

#include "bound.h"

PHYSICS_BEG

class particle
{
public:
    glm::vec2 pos;
    glm::vec2 v;
    glm::vec2 a;
    float angle;
    float w;
    float alpha;
    float m;
    float I;

    static constexpr float infinity = std::numeric_limits<float>::infinity();

    void update(float dt)
    {
        v += a * dt;
        pos += v * dt;

        w += alpha * dt;
        angle += w * dt;
    }
};

struct object
{
	particle pt;
	glm::vec2 scale;
	const abstract_shape *shape;
};

PHYSICS_END

#endif