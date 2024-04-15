#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include <functional>
#include <glm/vec2.hpp>

struct state
{
	glm::vec2 v;
	float w;
};

class constraint
{
public:

private:
	std::function<float(state)> d1;
};

#endif