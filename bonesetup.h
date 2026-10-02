#pragma once

class Bones {

public:
	bool m_running;

public:
	// rax resolver / lagcomp: self-contained manual bone build at an arbitrary
	// curtime. does not touch the game's cached bone array, so it is safe to
	// call while the game is animating.
	bool Setup( Player* target, const int& mask, const float& curtime, BoneArray* out );

	// skateboard2018 legacy path ( record-based, uses BuildTransformations ).
	bool setup( Player* player, BoneArray* out, LagRecord* record );
	bool BuildBones( Player* target, int mask, BoneArray* out, LagRecord* record );
};

extern Bones g_bones;
