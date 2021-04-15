#include "mat.h"

template<typename O>
class constraint {
public:
	constraint(O function) {
		c = function;
	}
	template<typename... Args>
	operator()(Args&&... args) {
		return O(std::forward<Args>(args)...);
	}
private:
	O c;
};

template<typename O>
constraint(O function)->constraint(O);