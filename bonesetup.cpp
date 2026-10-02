#include "includes.h"
#include "crash.h"

Bones g_bones{};

// rax resolver / lagcomp bone build.
// builds the bone chain by hand at the requested curtime so we can generate
// bones for a predicted ( future ) record without disturbing the game's cache.
bool Bones::Setup( Player* target, const int& mask, const float& curtime, BoneArray* out ) {
	alignas( 16 ) vec3_t       pos[ 128 ];
	alignas( 16 ) quaternion_t q[ 128 ];

	CrashLog::Trace( "bones: RAX setup ent=%d mask=%08X curtime=%.4f out=%p",
		target ? target->index( ) : -1, mask, curtime, out );

	if ( !out ) {
		CrashLog::Trace( "bones: !! null out pointer" );
		return false;
	}

	CStudioHdr* hdr = target->GetModelPtr( );

	if ( !hdr ) {
		CrashLog::Trace( "bones: !! null studio hdr" );
		return false;
	}

	if ( !hdr->m_studio_hdr ) {
		CrashLog::Trace( "bones: !! studiohdr->m_studio_hdr is null" );
		return false;
	}


	const float backup_curtime    = g_csgo.m_globals->m_curtime;
	const float backup_frametime  = g_csgo.m_globals->m_frametime;
	const float backup_weight     = target->m_AnimOverlay( )[ 12 ].m_weight;
	const int   backup_eflags     = target->m_iEFlags( );

	g_csgo.m_globals->m_curtime    = curtime;
	g_csgo.m_globals->m_frametime  = g_csgo.m_globals->m_interval;

	// zero the alive loop so it doesnt fight our layers.
	target->m_AnimOverlay( )[ 12 ].m_weight = 0.f;

	target->InvalidateBoneCache( );

	target->m_iEFlags( ) |= EFL_SETTING_UP_BONES;

	// first we setup needed shit for bones
	target->StandardBlendingRules( hdr, pos, q, curtime, mask );

	// build chain.
	static int32_t chain[ 128 ] = {};
	const auto chain_length = hdr->m_studio_hdr->m_num_bones;

	CrashLog::Trace( "bones: chain_length=%d parents=%d", chain_length, hdr->m_bone_parent.Count( ) );

	// a model can never have more bones than we have slots for.
	if ( chain_length <= 0 || chain_length > 128 ) {
		CrashLog::Trace( "bones: !! bad chain_length %d", chain_length );
		return false;
	}

	for ( auto i = 0; i < chain_length; i++ )
		chain[ chain_length - i - 1 ] = i;

	// build transformations.
	// (this actually set the bones up)
	static matrix3x4_t rotation;
	math::AngleMatrix( target->m_angAbsRotation( ), target->m_vecAbsOrigin( ), rotation );

	for ( auto j = chain_length - 1; j >= 0; j-- ) {
		const auto i = chain[ j ];
		const auto parent = hdr->m_bone_parent.Count( ) > i ? &hdr->m_bone_parent[ i ] : nullptr;

		if ( !parent )
			continue;

		static matrix3x4_t qua;
		qua = math::QuaternionMatrix( q[ i ], pos[ i ] );

		if ( *parent == -1 )
			math::ConcatTransforms( rotation, qua, out[ i ] );
		else
			math::ConcatTransforms( out[ *parent ], qua, out[ i ] );
	}

	// start interpolation again.
	g_csgo.m_globals->m_curtime    = backup_curtime;
	g_csgo.m_globals->m_frametime  = backup_frametime;
	target->m_AnimOverlay( )[ 12 ].m_weight = backup_weight;
	target->m_iEFlags( ) = backup_eflags;

	return true;
}

bool Bones::setup(Player* player, BoneArray* out, LagRecord* record) {
	// if the record isnt setup yet.
	if (!record->m_setup) {
		// run setupbones rebuilt.
		if (!BuildBones(player, 0x7FF00, record->m_bones, record))
			return false;

		// we have setup this record bones.
		record->m_setup = true;
	}

	// record is setup.
	if (out && record->m_setup)
		std::memcpy(out, record->m_bones, sizeof(BoneArray) * 128);

	return true;
}

bool Bones::BuildBones(Player* target, int mask, BoneArray* out, LagRecord* record) {
	vec3_t		     pos[128];
	quaternion_t     q[128];
	vec3_t           backup_origin;
	ang_t            backup_angles;
	float            backup_poses[24];
	C_AnimationLayer backup_layers[16];

	// get hdr.
	CStudioHdr* hdr = target->GetModelPtr();
	if (!hdr)
		return false;

	// get ptr to bone accessor.
	CBoneAccessor* accessor = &target->m_BoneAccessor();
	if (!accessor)
		return false;

	// store origial output matrix.
	// likely cachedbonedata.
	BoneArray* backup_matrix = accessor->m_pBones;
	if (!backup_matrix)
		return false;

	// prevent the game from calling ShouldSkipAnimationFrame.
	auto bSkipAnimationFrame = *reinterpret_cast<int*>(uintptr_t(target) + 0x260);
	*reinterpret_cast<int*>(uintptr_t(target) + 0x260) = NULL;

	// backup original.
	backup_origin = target->GetAbsOrigin();
	backup_angles = target->GetAbsAngles();
	target->GetPoseParameters(backup_poses);
	target->GetAnimLayers(backup_layers);

	// compute transform from raw data.
	matrix3x4_t transform;
	math::AngleMatrix(record->m_abs_ang, record->m_pred_origin, transform);

	// set non interpolated data.
	target->AddEffect(EF_NOINTERP);
	target->SetAbsOrigin(record->m_pred_origin);
	target->SetAbsAngles(record->m_abs_ang);
	target->SetPoseParameters(record->m_poses);
	target->SetAnimLayers(record->m_layers);

	// force game to call AccumulateLayers - pvs fix.
	m_running = true;

	// set bone array for write.
	accessor->m_pBones = out;

	// compute and build bones.
	target->StandardBlendingRules(hdr, pos, q, record->m_pred_time, mask);

	uint8_t computed[0x100];
	std::memset(computed, 0, 0x100);
	target->BuildTransformations(hdr, pos, q, transform, mask, computed);

	// restore old matrix.
	accessor->m_pBones = backup_matrix;

	// restore original interpolated entity data.
	target->m_fEffects() &= ~EF_NOINTERP;
	target->SetAbsOrigin(backup_origin);
	target->SetAbsAngles(backup_angles);
	target->SetPoseParameters(backup_poses);
	target->SetAnimLayers(backup_layers);

	// revert to old game behavior.
	m_running = false;

	// allow the game to call ShouldSkipAnimationFrame.
	*reinterpret_cast<int*>(uintptr_t(target) + 0x260) = bSkipAnimationFrame;

	return true;
}