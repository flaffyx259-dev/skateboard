#pragma once

// pre-declare.
class LagRecord;

class BackupRecord {
public:
	BoneArray* m_bones;
	int        m_bone_count;
	vec3_t     m_origin, m_abs_origin;
	vec3_t     m_mins;
	vec3_t     m_maxs;
	ang_t      m_abs_ang;

public:
	__forceinline void store( Player* player ) {
		// get bone cache ptr.
		CBoneCache* cache = &player->m_BoneCache( );

		// store bone data.
		m_bones      = cache->m_pCachedBones;
		m_bone_count = cache->m_CachedBoneCount;
		m_origin     = player->m_vecOrigin( );
		m_mins       = player->m_vecMins( );
		m_maxs       = player->m_vecMaxs( );
		m_abs_origin = player->GetAbsOrigin( );
		m_abs_ang    = player->GetAbsAngles( );
	}

	__forceinline void restore( Player* player ) {
		// get bone cache ptr.
		CBoneCache* cache = &player->m_BoneCache( );

		cache->m_pCachedBones    = m_bones;
		cache->m_CachedBoneCount = m_bone_count;

		player->m_vecOrigin( ) = m_origin;
		player->m_vecMins( )   = m_mins;
		player->m_vecMaxs( )   = m_maxs;
		player->SetAbsAngles( m_abs_ang );
		player->SetAbsOrigin( m_origin );
	}
};

enum {
	LC_LOST_TRACK,
	LC_TIME_DECREMENT
};

// rax resolver modes.
enum {
	RESOLVE_NONE,
	RESOLVE_MOVE,
	RESOLVE_STAND,
	RESOLVE_STAND_BALANCE,
	RESOLVE_STAND_BALANCE2,
	RESOLVE_STAND_NO_DATA,
	RESOLVE_DISTORTION,
	RESOLVE_NOUPDATE,
	RESOLVE_PREUPDATE,
	RESOLVE_UPDATE,
	RESOLVE_UPDATE_PRED,
	RESOLVE_AIR,
	RESOLVE_AIR_UPDATE,
	RESOLVE_OVERRIDE,
	// we use this to mark as low resolve chance.
	RESOLVE_PREDICTED
};

enum {
	RESOLVE_CONFIDENCE_LOW,
	RESOLVE_CONFIDENCE_MEDIUM,
	RESOLVE_CONFIDENCE_HIGH,
	RESOLVE_CONFIDENCE_VERY_HIGH,
	RESOLVE_CONFIDENCE_OVERRIDE,
};

struct SafeData_t {
	BoneArray m_bones[ 128 ];
	float     m_yaw;

	SafeData_t( ) {
		m_yaw = 0.f;
	}
};

class LagRecord {
public:
	// data.
	Player*  m_player;
	EHANDLE  m_weapon_handle;
	float    m_immune;
	int      m_tick;
	int      m_lag;
	bool     m_dormant;

	// netvars.
	float  m_sim_time;
	float  m_old_sim_time;
	float  m_duck, m_duck_speed;
	float  m_body;

	int    m_flags, m_anim_flags;
	int    m_move_state, m_move_type;

	ang_t  m_eye_angles;
	ang_t  m_abs_ang;

	vec3_t m_origin, m_abs_origin, m_old_origin;
	vec3_t m_velocity, m_abs_velocity;

	vec3_t m_mins, m_maxs;

	C_AnimationLayer m_layers[ 13 ];
	float            m_poses[ PoseParam::POSE_COUNT ];

	// anim velocity used by the resolver.
	vec3_t m_anim_velocity;

	// bone stuff.
	bool       m_setup;
	BoneArray* m_bones;

	// lagfix stuff.
	bool   m_broke_lc;
	vec3_t m_pred_origin;
	vec3_t m_pred_velocity;
	float  m_pred_time;
	int    m_pred_flags;

	// resolver stuff.
	int   m_server_tick;
	int   m_choke;
	bool  m_lagcomp[ 2 ];
	int   m_resolver_mode;
	bool  m_missed;
	bool  m_predicted;
	bool  m_fake_walk;
	bool  m_shot;
	float m_away;
	float m_anim_time;

	// other stuff.
	float  m_interp_time;
public:

	// default ctor.
	__forceinline LagRecord( ) :
		m_setup{ false },
		m_broke_lc{ false },
		m_fake_walk{ false },
		m_shot{ false },
		m_missed{ false },
		m_predicted{ false },
		m_dormant{ false },
		m_lag{},
		m_server_tick{ -1 },
		m_choke{ 1 },
		m_resolver_mode{ RESOLVE_NONE },
		m_bones{} {

		m_player        = nullptr;
		m_weapon_handle = NULL;

		for ( bool& lc : m_lagcomp )
			lc = false;
	}

	// ctor.
	__forceinline LagRecord( Player* player ) :
		m_setup{ false },
		m_broke_lc{ false },
		m_fake_walk{ false },
		m_shot{ false },
		m_missed{ false },
		m_predicted{ false },
		m_dormant{ false },
		m_lag{},
		m_server_tick{ -1 },
		m_choke{ 1 },
		m_resolver_mode{ RESOLVE_NONE },
		m_bones{} {

		store( player );
	}

	// dtor.
	__forceinline ~LagRecord( ) {
		// free heap allocated game mem.
		g_csgo.m_mem_alloc->Free( m_bones );
	}

	__forceinline void invalidate( ) {
		// free heap allocated game mem.
		g_csgo.m_mem_alloc->Free( m_bones );

		// mark as not setup.
		m_setup = false;

		// allocate new memory.
		m_bones = ( BoneArray* )g_csgo.m_mem_alloc->Alloc( sizeof( BoneArray ) * 128 );
	}

	// function: allocates memory for SetupBones and stores relevant data.
	void store( Player* player ) {
		// allocate game heap.
		m_bones = ( BoneArray* )g_csgo.m_mem_alloc->Alloc( sizeof( BoneArray ) * 128 );

		// player data.
		m_player        = player;
		m_weapon_handle = player->m_hActiveWeapon( );
		m_immune        = player->m_fImmuneToGunGameDamageTime( );
		m_tick          = g_csgo.m_cl->m_server_tick;

		// netvars.
		m_pred_time     = m_sim_time = player->m_flSimulationTime( );
		m_old_sim_time  = player->m_flOldSimulationTime( );
		m_anim_time     = m_old_sim_time + g_csgo.m_globals->m_interval;
		m_pred_flags    = m_flags = m_anim_flags = player->m_fFlags( );
		m_pred_origin   = m_origin = m_abs_origin = player->m_vecOrigin( );
		m_old_origin    = player->m_vecOldOrigin( );
		m_abs_velocity  = m_velocity = player->m_vecVelocity( );
		m_eye_angles    = player->m_angEyeAngles( );
		m_abs_ang       = player->GetAbsAngles( );
		m_body          = player->m_flLowerBodyYawTarget( );
		m_mins          = player->m_vecMins( );
		m_maxs          = player->m_vecMaxs( );
		m_duck          = player->m_flDuckAmount( );
		m_duck_speed    = player->m_flDuckSpeed( );
		m_move_state    = player->m_iMoveState( );
		m_move_type     = player->m_MoveType( );

		// save networked animlayers.
		player->GetAnimLayers( m_layers );

		// normalize eye angles.
		m_eye_angles.normalize( );
		math::clamp( m_eye_angles.x, -90.f, 90.f );

		// get lag.
		m_lag = game::TIME_TO_TICKS( m_sim_time - m_old_sim_time );

		// choke / server tick.
		m_server_tick  = g_csgo.m_cl->m_server_tick;
		m_choke        = 1;
		m_resolver_mode = RESOLVE_NONE;
		m_predicted     = false;
		m_missed        = false;

		for ( bool& lc : m_lagcomp )
			lc = false;
	}

	// function: restores 'predicted' variables to their original.
	__forceinline void predict( ) {
		m_broke_lc      = false;
		m_pred_origin   = m_origin;
		m_pred_velocity = m_velocity;
		m_pred_time     = m_sim_time;
		m_pred_flags    = m_flags;
	}

	// function: writes current record to bone cache.
	__forceinline void cache( ) {
		// get bone cache ptr.
		CBoneCache* cache = &m_player->m_BoneCache( );

		cache->m_pCachedBones    = m_bones;
		cache->m_CachedBoneCount = 128;

		m_player->m_vecOrigin( ) = m_pred_origin;
		m_player->m_vecMins( )   = m_mins;
		m_player->m_vecMaxs( )   = m_maxs;

		m_player->SetAbsAngles( m_abs_ang );
		m_player->SetAbsOrigin( m_pred_origin );
	}

	__forceinline bool dormant( ) {
		return m_dormant;
	}

	__forceinline bool immune( ) {
		return m_immune > 0.f;
	}

	// function: checks if LagRecord obj is hittable if we were to fire at it now.
	bool valid( ) {
		// use prediction curtime for this.
		float curtime = game::TICKS_TO_TIME( g_cl.m_local->m_nTickBase( ) );

		// correct is the amount of time we have to correct game time,
		float correct = g_cl.m_lerp + g_cl.m_latency[ INetChannel::FLOW_OUTGOING ];

		// stupid fake latency goes into the incoming latency.
		correct += g_cl.m_latency[ INetChannel::FLOW_INCOMING ];

		// check bounds [ 0, sv_maxunlag ]
		math::clamp( correct, 0.f, g_csgo.sv_maxunlag->GetFloat( ) );

		// calculate difference between tick sent by player and our latency based tick.
		// ensure this record isn't too old.
		return std::abs( correct - ( curtime - m_sim_time ) ) < 0.19f;
	}

	// rax resolver / lagcomp: can the server still accept a usercmd stamped at this time?
	__forceinline bool ValidTime( ) const {
		// lagcompensation is turned off.
		if ( g_csgo.cl_lagcompensation->GetInt( ) == 0 )
			return false;

		int dead_time = game::TICKS_TO_TIME( g_cl.m_arrival_tick ) - g_csgo.sv_maxunlag->GetFloat( );

		// time is out of bounds.
		if ( m_sim_time < dead_time )
			return false;

		// the server wont accept our command.
		if ( m_sim_time > game::TICKS_TO_TIME( g_cl.m_arrival_tick + g_csgo.sv_max_usercmd_future_ticks->GetInt( ) ) )
			return false;

		float curtime = g_cl.m_local && g_cl.m_local->alive( ) ? g_cl.m_curtime : g_csgo.m_globals->m_curtime;

		// correct is the amount of time we have to correct game time
		float correct = g_cl.m_lerp + g_cl.m_latency[ INetChannel::FLOW_OUTGOING ] + g_cl.m_latency[ INetChannel::FLOW_INCOMING ];

		// check bounds [0,sv_maxunlag]
		correct = std::clamp( correct, 0.0f, g_csgo.sv_maxunlag->GetFloat( ) );

		// calculate difference between tick sent by player and our latency based tick
		const float deltaTime = correct - ( curtime - m_sim_time );

		return fabs( deltaTime ) <= 0.2f;
	}

	__forceinline void SetData( ) {
		m_player->SetAbsOrigin( m_origin );
		m_player->SetAbsAngles( m_abs_ang );

		m_player->m_vecMins( ) = m_mins;
		m_player->m_vecMaxs( ) = m_maxs;

		m_player->SetBones( m_bones );
	}

	__forceinline void UpdateBounds( ) {
		m_mins = m_player->m_vecMins( );
		m_maxs = m_player->m_vecMaxs( );
	}

	__forceinline bool IsBodyUpdate( ) const {
		switch ( m_resolver_mode ) {
		case RESOLVE_UPDATE:
		case RESOLVE_UPDATE_PRED:
		case RESOLVE_AIR_UPDATE:
			return true;
		}

		return false;
	}

	__forceinline int ResolveChance( ) const {
		switch ( m_resolver_mode ) {
			// they aint choking shit.
		case RESOLVE_NONE:
			return RESOLVE_CONFIDENCE_VERY_HIGH;
			// they are moving :D
		case RESOLVE_MOVE:
			return RESOLVE_CONFIDENCE_HIGH;
			// we might have them resolved :D
		case RESOLVE_STAND:
		case RESOLVE_STAND_BALANCE:
			return RESOLVE_CONFIDENCE_MEDIUM;
			// they might be doing some goofy antiaim.
		case RESOLVE_STAND_BALANCE2:
			return RESOLVE_CONFIDENCE_LOW;
			// oh fuck no...
		case RESOLVE_STAND_NO_DATA:
		case RESOLVE_DISTORTION:
			return RESOLVE_CONFIDENCE_LOW;
			// they are not breaking lby :D
		case RESOLVE_NOUPDATE:
			return RESOLVE_CONFIDENCE_HIGH;
			// they might be doing funny stuff or they can be right on lastmove :D
		case RESOLVE_PREUPDATE:
			return RESOLVE_CONFIDENCE_MEDIUM;
			// they updated lby :O
		case RESOLVE_UPDATE:
			return RESOLVE_CONFIDENCE_VERY_HIGH;
			// predict that flick :sunglasses:
		case RESOLVE_UPDATE_PRED:
			return RESOLVE_CONFIDENCE_HIGH;
			// rabbit man very hard to hit :(
		case RESOLVE_AIR:
			return RESOLVE_CONFIDENCE_LOW;
		case RESOLVE_AIR_UPDATE:
			return RESOLVE_CONFIDENCE_HIGH;
			// welp its just luck now.
		case RESOLVE_PREDICTED:
			return RESOLVE_CONFIDENCE_LOW;
		}

		// go on master. resolve them.
		return RESOLVE_CONFIDENCE_OVERRIDE;
	}

	__forceinline bool IsResolved( ) const {
		switch ( m_resolver_mode ) {
		case RESOLVE_NONE:
		case RESOLVE_MOVE:
		case RESOLVE_UPDATE:
		case RESOLVE_UPDATE_PRED:
		case RESOLVE_AIR_UPDATE:
			return true;
		}

		return false;
	}
};
