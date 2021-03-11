enum POLY_FLAGS {
	NONE = 0x00,
	CHECK_HULL = 0x01,
	SORT_PTS = 0x02,
	GET_CENTER = 0x04,
	COLLISION_REQS = SORT_PTS | CHECK_HULL,
	DRAW_REQS = SORT_PTS,
	FULL = CHECK_HULL | SORT_PTS | GET_CENTER,
	//IS_GUARANTEED says that you are promising that the polygon can skip the checks, but still will be able to function perfectly in all other functions. USE WITH CAUTION 
	//If a poly is being created from another poly, this flag says to inherent all the flags without doing the checks.
	IS_GUARANTEED = 0x08,
	NO_DRAW = 0x10,
	NO_COLLISION = 0x20
};