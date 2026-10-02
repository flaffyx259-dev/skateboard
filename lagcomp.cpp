#include "includes.h"
#include "crash.h"

LagCompensation g_lagcomp{};

LagRecord* LagCompensation::StartPrediction( AimPlayer* data ) {
	if ( data->m_records.size( ) <= 1 )
		return nullptr;

	LagRecord* current  = data->m_records[ 0 ].get( );
	LagRecord* previous = data->m_records[ 1 ].get( );

	CrashLog::Trace( "lagcomp: ent=%d recs=%d arrival=%d charged=%d outlat=%.4f inlat=%.4f",
		data->m_player ? data->m_player->index( ) : -1,
		( int )data->m_records.size( ), g_cl.m_arrival_tick, g_cl.m_charged_ticks,
		g_cl.m_latency[ INetChannel::FLOW_OUTGOING ], g_cl.m_latency[ INetChannel::FLOW_INCOMING ] );

	CrashLog::Trace( "lagcomp: cur tick=%d prev tick=%d sim=%.4f anim=%.4f bones=%p setup=%d",
		current->m_server_tick, previous->m_server_tick,
		current->m_sim_time, current->m_anim_time,
		current->m_bones, ( int )current->m_setup );

	// they wont update before our shot registers.
	if ( current->m_server_tick + ( current->m_server_tick - previous->m_server_tick ) > g_cl.m_arrival_tick ) {
		// ouf we cant hit any recent record.
		if ( current->m_lagcomp[ LC_LOST_TRACK ] &&
			current->m_lagcomp[ LC_TIME_DECREMENT ] &&
			!current->ValidTime( ) )
			return nullptr;

		return current;
	}

	LagRecord* predicted      = &data->m_predicted.first;
	LagRecord* prev_predicted = &data->m_predicted.second;

	// each predicted record needs its own bone buffer, otherwise the memcpy
	// below makes every record alias the source record's bones and SetupBones
	// would clobber the real ( networked ) record.
	auto alloc_bones = []( LagRecord* record ) {
		g_csgo.m_mem_alloc->Free( record->m_bones );
		record->m_bones = ( BoneArray* )g_csgo.m_mem_alloc->Alloc( sizeof( BoneArray ) * 128 );
		record->m_setup = false;
	};

	// we recieved a new update.
	if ( predicted->m_server_tick != current->m_server_tick ) {
		const int choked_ticks = current->m_server_tick - previous->m_server_tick;

		// note: rax upstream wrote 'g_cl.m_latency[FLOW_OUTGOING]' ( seconds )
		//       straight into an int tick counter here, which makes the
		//       prediction loop a no-op. we convert properly.
		const int latency_ticks = game::TIME_TO_TICKS( g_cl.m_latency[ INetChannel::FLOW_OUTGOING ] ) + g_cl.m_charged_ticks;

		if ( choked_ticks <= 0 )
			return current;

		int ticks_to_predict = 0;

		while ( ticks_to_predict < latency_ticks )
			ticks_to_predict += choked_ticks;

	if ( ticks_to_predict ) {
		// copy over newest data.
		std::memcpy( predicted, current, sizeof( LagRecord ) );

		// both scratch records own their bone buffer, so the copies below can
		// never alias each other or the networked record.
		alloc_bones( predicted );
		alloc_bones( prev_predicted );

		// predict each movement tick.
		int predicted_ticks = 0;

		data->m_player->GetAnimLayers( m_layers );
		data->m_player->GetPoseParameters( m_poses );
		data->m_player->GetAnimState( &m_state );

		while ( predicted_ticks < ticks_to_predict ) {
			// keep our own bone buffer across the copy.
			BoneArray* prev_bones = prev_predicted->m_bones;

			std::memcpy( prev_predicted, predicted, sizeof( LagRecord ) );
			prev_predicted->m_bones = prev_bones;

			PlayerMove( predicted );

			if ( predicted_ticks % choked_ticks == 0 )
				UpdatePredictedAnimations( data, predicted, prev_predicted, choked_ticks );

			++predicted_ticks;
		}


			data->m_player->m_vecOrigin( ) = m_origin;
			data->m_player->m_vecVelocity( ) = m_velocity;
			data->m_player->m_flDuckAmount( ) = m_duck_amount;
			data->m_player->m_flDuckSpeed( ) = m_duck_speed;
			data->m_player->m_fFlags( ) = m_flags;

			data->m_player->m_vecAbsOrigin( ) = m_abs_origin;
			data->m_player->m_vecAbsVelocity( ) = m_abs_velocity;

			data->m_player->SetAnimLayers( m_layers );
			data->m_player->SetPoseParameters( m_poses );
			data->m_player->SetAnimState( &m_state );

			g_csgo.m_globals->m_curtime = m_curtime;
			g_csgo.m_globals->m_frametime = m_frametime;

			predicted->m_lagcomp[ LC_LOST_TRACK ] = ( predicted->m_origin - current->m_origin ).length_sqr( ) > 4096.f;
		}
		// freak ass choking.
		else
			return nullptr;
	}

	// something went wrong.
	if ( !predicted->m_predicted )
		return nullptr;

	// they are standing.
	if ( current->m_abs_velocity.length_2d( ) <= 0.1f )
		return nullptr;

	// welp i guess no predicting.
	if ( predicted->m_lagcomp[ LC_LOST_TRACK ] &&
		predicted->m_lagcomp[ LC_TIME_DECREMENT ] &&
		!predicted->ValidTime( ) )
		return nullptr;

	CrashLog::Trace( "lagcomp: -> predicted rec=%p sim=%.4f tick=%d choke=%d orig=%p bones=%p valid=%d",
		predicted, predicted->m_sim_time, predicted->m_server_tick,
		predicted->m_choke, predicted->m_origin, predicted->m_bones,
		( int )predicted->ValidTime( ) );

	// boi got predicted.
	return predicted;
}

void LagCompensation::PlayerMove( LagRecord* record ) {
	vec3_t                start, end, normal;
	CGameTrace            trace;
	CTraceFilterWorldOnly filter;

	// predict gravity.
	if ( !( record->m_flags & FL_ONGROUND ) )
		record->m_velocity.z -= g_csgo.sv_gravity->GetFloat( ) * g_csgo.m_globals->m_interval;

	// define trace start.
	start = record->m_origin;

	// move trace end one tick into the future using predicted velocity.
	end = start + ( record->m_velocity * g_csgo.m_globals->m_interval );

	// trace.
	g_csgo.m_engine_trace->TraceRay( Ray( start, end, record->m_mins, record->m_maxs ), CONTENTS_SOLID, &filter, &trace );

	// we hit shit
	// we need to fix hit.
	if ( trace.m_fraction != 1.f ) {

		// fix sliding on planes.
		for ( int i{}; i < 2; ++i ) {
			record->m_velocity -= trace.m_plane.m_normal * record->m_velocity.dot( trace.m_plane.m_normal );

			float adjust = record->m_velocity.dot( trace.m_plane.m_normal );

			if ( adjust < 0.f )
				record->m_velocity -= ( trace.m_plane.m_normal * adjust );

			start = trace.m_endpos;
			end = start + ( record->m_velocity * ( g_csgo.m_globals->m_interval * ( 1.f - trace.m_fraction ) ) );

			g_csgo.m_engine_trace->TraceRay( Ray( start, end, record->m_mins, record->m_maxs ), CONTENTS_SOLID, &filter, &trace );

			if ( trace.m_fraction == 1.f )
				break;
		}
	}

	// set new final origin.
	start = end = record->m_origin = trace.m_endpos;

	// move endpos 2 units down.
	// this way we can check if we are in/on the ground.
	end.z -= 2.f;

	// trace.
	g_csgo.m_engine_trace->TraceRay( Ray( start, end, record->m_mins, record->m_maxs ), CONTENTS_SOLID, &filter, &trace );

	// strip onground flag.
	record->m_flags &= ~FL_ONGROUND;

	// add back onground flag if we are onground.
	if ( trace.m_fraction != 1.f && trace.m_plane.m_normal.z > 0.7f )
		record->m_flags |= FL_ONGROUND;

	// the rax prediction mutates m_origin / m_velocity / m_flags in place,
	// while the rest of skateboard2018 ( hitbox selection, bonesetup, knifebot,
	// penetration ) reads the m_pred_* shadow copy. keep them in sync so a
	// predicted record is usable everywhere a normal record is.
	record->m_pred_origin = record->m_origin;
	record->m_pred_velocity = record->m_velocity;
	record->m_pred_flags = record->m_flags;
}

void LagCompensation::UpdatePredictedAnimations( AimPlayer* data, LagRecord* predicted, LagRecord* previous, int update_ticks ) {
	CCSGOPlayerAnimState* state = data->m_player->m_PlayerAnimState( );

	if ( !state )
		return;

	CrashLog::Trace( "lagcomp: anim pred=%p prev=%p ticks=%d state=%p",
		predicted, previous, update_ticks, state );

	predicted->m_resolver_mode = RESOLVE_PREDICTED;

	// store backup data.
	m_curtime = g_csgo.m_globals->m_curtime;
	m_frametime = g_csgo.m_globals->m_frametime;

	m_origin = data->m_player->m_vecOrigin( );
	m_abs_origin = data->m_player->m_vecAbsOrigin( );
	m_velocity = data->m_player->m_vecVelocity( );
	m_abs_velocity = data->m_player->m_vecAbsVelocity( );
	m_duck_amount = data->m_player->m_flDuckAmount( );
	m_duck_speed = data->m_player->m_flDuckSpeed( );
	m_flags = data->m_player->m_fFlags( );

	// set our animations
	data->m_player->m_fFlags( ) = previous->m_flags;

	data->m_player->m_flDuckAmount( ) = predicted->m_duck;
	data->m_player->m_flDuckSpeed( ) = predicted->m_duck_speed;

	// predict simulation time.
	predicted->m_sim_time += game::TICKS_TO_TIME( update_ticks );

	// predict the tick we will be recieved on.
	// note: rax upstream added TICKS_TO_TIME( update_ticks ) to this, which is
	//       seconds, to a field that is measured in ticks.
	predicted->m_server_tick += update_ticks;

	predicted->m_choke = update_ticks;

	if ( predicted->m_choke >= 2 ) {
		const float fraction = 1.f / predicted->m_choke;

		predicted->m_abs_origin = math::Lerp( fraction, previous->m_origin, predicted->m_origin );
		predicted->m_abs_velocity = math::Lerp( fraction, previous->m_velocity, predicted->m_velocity );
		predicted->m_anim_time = previous->m_sim_time + g_csgo.m_globals->m_interval;
	}

	data->m_player->SetAbsOrigin( predicted->m_abs_origin );
	data->m_player->SetAbsVelocity( predicted->m_abs_velocity );
	data->m_player->SetAnimLayers( previous->m_layers );

	g_csgo.m_globals->m_curtime = predicted->m_anim_time;
	g_csgo.m_globals->m_frametime = g_csgo.m_globals->m_interval;

	game::UpdateAnimationState( state, predicted->m_eye_angles );

	data->m_player->SetAbsOrigin( predicted->m_origin );

	g_bones.Setup( data->m_player, BONE_USED_BY_ANYTHING, predicted->m_sim_time, predicted->m_bones );
	predicted->UpdateBounds( );

	// bones are already built for this ( future ) state, so the legacy
	// bonesetup path must not rebuild them over the top.
	predicted->m_setup = true;

	// keep the m_pred_* shadow copy in sync ( see PlayerMove ).
	predicted->m_pred_time = predicted->m_sim_time;
	predicted->m_pred_origin = predicted->m_origin;
	predicted->m_pred_velocity = predicted->m_velocity;
	predicted->m_pred_flags = predicted->m_flags;

	predicted->m_predicted = true;
}
