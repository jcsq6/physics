enum POLY_FLAGS {
	NONE = 0x00,
	SORT_PTS = 0x01,
	CHECK_HULL = 0x02,
	GET_CENTER = 0x04,
	GET_AREA = 0x08,
	GET_NORMALS = 0x10,
	COLLISION_REQS = SORT_PTS | CHECK_HULL | GET_CENTER | GET_NORMALS,
	DRAW_REQS = SORT_PTS,
	FULL = COLLISION_REQS | GET_AREA,
	//IS_GUARANTEED says that you are promising that the polygon can skip the checks, but still will be able to function perfectly in all other functions. USE WITH CAUTION 
	//If a poly is being created from another poly, this flag says to inherent all the flags without doing the checks. (AKA in the poly::convert method)
	IS_GUARANTEED = 0x20,
	NO_DRAW = 0x40,
	NO_COLLISION = 0x80
};