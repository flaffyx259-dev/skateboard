#pragma once
#include "enums.h"

#define MAX_WEAPONS	48

struct animtaglookup_t
{
	int nIndex;
	const char* szName;
};

#define REGISTER_ANIMTAG( _n ) { _n, #_n },
const animtaglookup_t g_AnimTagLookupTable[ ANIMTAG_COUNT ] =
{
	REGISTER_ANIMTAG( ANIMTAG_UNINITIALIZED )
	REGISTER_ANIMTAG( ANIMTAG_STARTCYCLE_N )
	REGISTER_ANIMTAG( ANIMTAG_STARTCYCLE_NE )
	REGISTER_ANIMTAG( ANIMTAG_STARTCYCLE_E )
	REGISTER_ANIMTAG( ANIMTAG_STARTCYCLE_SE )
	REGISTER_ANIMTAG( ANIMTAG_STARTCYCLE_S )
	REGISTER_ANIMTAG( ANIMTAG_STARTCYCLE_SW )
	REGISTER_ANIMTAG( ANIMTAG_STARTCYCLE_W )
	REGISTER_ANIMTAG( ANIMTAG_STARTCYCLE_NW )

	REGISTER_ANIMTAG( ANIMTAG_AIMLIMIT_YAWMIN_IDLE )
	REGISTER_ANIMTAG( ANIMTAG_AIMLIMIT_YAWMAX_IDLE )
	REGISTER_ANIMTAG( ANIMTAG_AIMLIMIT_YAWMIN_WALK )
	REGISTER_ANIMTAG( ANIMTAG_AIMLIMIT_YAWMAX_WALK )
	REGISTER_ANIMTAG( ANIMTAG_AIMLIMIT_YAWMIN_RUN )
	REGISTER_ANIMTAG( ANIMTAG_AIMLIMIT_YAWMAX_RUN )
	REGISTER_ANIMTAG( ANIMTAG_AIMLIMIT_YAWMIN_CROUCHIDLE )
	REGISTER_ANIMTAG( ANIMTAG_AIMLIMIT_YAWMAX_CROUCHIDLE )
	REGISTER_ANIMTAG( ANIMTAG_AIMLIMIT_YAWMIN_CROUCHWALK )
	REGISTER_ANIMTAG( ANIMTAG_AIMLIMIT_YAWMAX_CROUCHWALK )

	REGISTER_ANIMTAG( ANIMTAG_AIMLIMIT_PITCHMIN_IDLE )
	REGISTER_ANIMTAG( ANIMTAG_AIMLIMIT_PITCHMAX_IDLE )
	REGISTER_ANIMTAG( ANIMTAG_AIMLIMIT_PITCHMIN_WALKRUN )
	REGISTER_ANIMTAG( ANIMTAG_AIMLIMIT_PITCHMAX_WALKRUN )
	REGISTER_ANIMTAG( ANIMTAG_AIMLIMIT_PITCHMIN_CROUCH )
	REGISTER_ANIMTAG( ANIMTAG_AIMLIMIT_PITCHMAX_CROUCH )
	REGISTER_ANIMTAG( ANIMTAG_AIMLIMIT_PITCHMIN_CROUCHWALK )
	REGISTER_ANIMTAG( ANIMTAG_AIMLIMIT_PITCHMAX_CROUCHWALK )

	REGISTER_ANIMTAG( ANIMTAG_FLASHBANG_PASSABLE )

	REGISTER_ANIMTAG( ANIMTAG_WEAPON_POSTLAYER )
};

const char* const g_szWeaponPrefixLookupTable[ ] = {
	"knife",
	"pistol",
	"smg",
	"rifle",
	"shotgun",
	"sniper",
	"heavy",
	"c4",
	"grenade",
	"knife"
};

struct ClientHitVerify_t
{
	vec3_t m_pos;
	float m_time;
	float m_expire_time;
};

#define	MAX_EDICT_BITS				11
#define	MAX_EDICTS					( 1 << MAX_EDICT_BITS )

class CEntityInfo
{
public:
	CEntityInfo( ) {
		m_nOldEntity = -1;
		m_nNewEntity = -1;
		m_nHeaderBase = -1;
	}

	virtual	~CEntityInfo( ) { };

	void* m_pFrom;
	void* m_pTo;


	int				m_nOldEntity;	// current entity index in m_pFrom
	int				m_nNewEntity;	// current entity index in m_pTo

	int				m_nHeaderBase;
	int				m_nHeaderCount;

	int		        m_UpdateType;
	bool			m_bAsDelta;
};

class CPostDataUpdateCall
{
public:
	int					m_iEnt;
	int              	m_UpdateType;
};

class CEntityReadInfo : public CEntityInfo
{
public:
	int             m_DecodeEntity;

	void* m_pBuf;
	int				m_UpdateFlags;	// from the subheader
	bool			m_bIsEntity;

	int				m_nBaseline;	// what baseline index do we use (0/1)
	bool			m_bUpdateBaselines; // update baseline while parsing snaphsot

	int				m_nLocalPlayerBits; // profiling data
	int				m_nOtherPlayerBits; // profiling data

	CPostDataUpdateCall	m_PostDataUpdateCalls[ MAX_EDICTS ];
	int					m_nPostDataUpdateCalls;
};

struct RenderableInstance_t {
	uint8_t m_alpha;
	__forceinline RenderableInstance_t( ) : m_alpha{ 255ui8 } { }
};

class Entity {
public:
	// helper methods.
	template< typename t >
	__forceinline t& get( size_t offset ) {
		return *( t* )( ( uintptr_t )this + offset );
	}

	template< typename t >
	__forceinline void set( size_t offset, const t& val ) {
		*( t* )( ( uintptr_t )this + offset ) = val;
	}

	template< typename t >
	__forceinline t as( ) {
		return ( t )this;
	}

public:
	// netvars / etc.
	__forceinline datamap_t* GetPredDescMap( ) {
		return util::get_method< datamap_t * ( __thiscall* )( void* ) >( this, 17 )( this );
	}

	__forceinline float& m_flGravity( ) {
		static auto offset = g_netvars.FindInDataMap( GetPredDescMap( ), XOR( "m_flGravity" ) );
		return get< float >( offset );
	}

	__forceinline vec3_t& m_vecAbsVelocity( ) {
		static auto offset = g_netvars.FindInDataMap( GetPredDescMap( ), XOR( "m_vecAbsVelocity" ) );
		return get< vec3_t >( offset );
	}

	__forceinline vec3_t& m_vecAbsOrigin( ) {
		static auto offset = g_netvars.FindInDataMap( GetPredDescMap( ), XOR( "m_vecAbsOrigin" ) );
		return get< vec3_t >( offset );
	}

	__forceinline ang_t& m_angAbsRotation( ) {
		static auto offset = g_netvars.FindInDataMap( GetPredDescMap( ), XOR( "m_angAbsRotation" ) );
		return get< ang_t >( offset );
	}

	__forceinline ang_t& m_angRotation( ) {
		static auto offset = g_netvars.FindInDataMap( GetPredDescMap( ), XOR( "m_angRotation" ) );
		return get< ang_t >( offset );
	}

	// --- skateboard2018 addition ---
	__forceinline ang_t& m_angNetworkAngles( ) {
		return get< ang_t >( g_entoffsets.m_angNetworkAngles );
	}

	__forceinline uint32_t& m_fEffects( ) {
		static auto offset = g_netvars.FindInDataMap( GetPredDescMap( ), XOR( "m_fEffects" ) );
		return get< uint32_t >( offset );
	}

	__forceinline int& m_iEFlags( ) {
		static auto offset = g_netvars.FindInDataMap( GetPredDescMap( ), XOR( "m_iEFlags" ) );
		return get< int >( offset );
	}

	__forceinline int& m_MoveType( ) {
		static auto offset = g_netvars.FindInDataMap( GetPredDescMap( ), XOR( "m_MoveType" ) );
		return get< int >( offset );
	}

	__forceinline float& m_surfaceFriction( ) {
		static auto offset = g_netvars.FindInDataMap( GetPredDescMap( ), XOR( "m_surfaceFriction" ) );
		return get< float >( offset );
	}

	// netvars / etc.
	__forceinline vec3_t& m_vecOrigin( ) {
		return get< vec3_t >( g_entoffsets.m_vecOrigin );
	}

	__forceinline vec3_t& m_vecVelocity( ) {
		return get< vec3_t >( g_entoffsets.m_vecVelocity );
	}

	__forceinline vec3_t& m_vecMins( ) {
		return get< vec3_t >( g_entoffsets.m_vecMins );
	}

	__forceinline vec3_t& m_vecMaxs( ) {
		return get< vec3_t >( g_entoffsets.m_vecMaxs );
	}

	__forceinline int& m_iTeamNum( ) {
		return get< int >( g_entoffsets.m_iTeamNum );
	}

	__forceinline int& m_nSequence( ) {
		return get< int >( g_entoffsets.m_nSequence );
	}

	__forceinline float& m_flCycle( ) {
		return get< float >( g_entoffsets.m_flCycle );
	}

	__forceinline float& m_flAnimTime( )
	{
		return get< float >( g_entoffsets.m_flAnimTime );
	}

	__forceinline float& m_flC4Blow( ) {
		return get< float >( g_entoffsets.m_flC4Blow );
	}

	__forceinline bool& m_bBombTicking( ) {
		return get< bool >( g_entoffsets.m_bBombTicking );
	}

	__forceinline int& m_nModelIndex( ) {
		return get< int >( g_entoffsets.m_nModelIndex );
	}

	__forceinline bool& m_bReadyToDraw( ) {
		return get< bool >( g_entoffsets.m_bReadyToDraw );
	}

	// --- skateboard2018 additions ---
	__forceinline vec3_t& m_vecOldOrigin( ) {
		return get< vec3_t >( g_entoffsets.m_vecOldOrigin );
	}

	__forceinline float& m_flSpawnTime_Grenade( ) {
		return get< float >( g_entoffsets.m_flSpawnTime_Grenade );
	}

	__forceinline EHANDLE& m_hThrower( ) {
		return get< EHANDLE >( g_entoffsets.m_hThrower );
	}

public:
	// virtual indices
	enum indices : size_t {
		WORLDSPACECENTER = 78,
		GETMAXHEALTH = 122,
		ISPLAYER = 152,
		ISBASECOMBATWEAPON = 160,
	};

public:
	// virtuals.
	// renderable table.
	__forceinline void* renderable( ) {
		return ( void* )( ( uintptr_t )this + 0x4 );
	}

	__forceinline vec3_t& GetRenderOrigin( ) {
		return util::get_method< vec3_t & ( __thiscall* )( void* ) >( renderable( ), 1 )( renderable( ) );
	}

	// --- skateboard2018 addition ---
	__forceinline void GetRenderBounds( vec3_t& mins, vec3_t& maxs ) {
		return util::get_method< void( __thiscall* )( void*, vec3_t&, vec3_t& ) >( renderable( ), 17 )( renderable( ), mins, maxs );
	}

	__forceinline ang_t& GetRenderAngles( ) {
		return util::get_method< ang_t & ( __thiscall* )( void* ) >( renderable( ), 2 )( renderable( ) );
	}

	__forceinline const model_t* GetModel( ) {
		return util::get_method< const model_t * ( __thiscall* )( void* ) >( renderable( ), 8 )( renderable( ) );
	}

	__forceinline void DrawModel( int flags = STUDIO_RENDER, const RenderableInstance_t& instance = {} ) {
		return util::get_method< void( __thiscall* )( void*, int, const RenderableInstance_t& )>( renderable( ), 9 )( renderable( ), flags, instance );
	}

	__forceinline bool SetupBones( matrix3x4_t* out, int max, int mask, float time ) {
		return util::get_method< bool( __thiscall* )( void*, matrix3x4_t*, int, int, float )>( renderable( ), 13 )( renderable( ), out, max, mask, time );
	}

	// networkable table.
	__forceinline void* networkable( ) {
		return ( void* )( ( uintptr_t )this + 0x8 );
	}

	__forceinline void Release( ) {
		return util::get_method< void( __thiscall* )( void* ) >( networkable( ), 1 )( networkable( ) );
	}

	__forceinline ClientClass* GetClientClass( ) {
		return util::get_method< ClientClass * ( __thiscall* )( void* ) >( networkable( ), 2 )( networkable( ) );
	}

	__forceinline void OnDataChanged( DataUpdateType_t type ) {
		return util::get_method< void( __thiscall* )( void*, DataUpdateType_t ) >( networkable( ), 5 )( networkable( ), type );
	}

	__forceinline void PreDataUpdate( DataUpdateType_t type ) {
		return util::get_method< void( __thiscall* )( void*, DataUpdateType_t ) >( networkable( ), 6 )( networkable( ), type );
	}

	__forceinline void PostDataUpdate( DataUpdateType_t type ) {
		return util::get_method< void( __thiscall* )( void*, DataUpdateType_t ) >( networkable( ), 7 )( networkable( ), type );
	}

	__forceinline bool dormant( ) {
		return util::get_method< bool( __thiscall* )( void* ) >( networkable( ), 9 )( networkable( ) );
	}

	__forceinline int index( ) {
		return util::get_method< int( __thiscall* )( void* ) >( networkable( ), 10 )( networkable( ) );
	}

	__forceinline void SetDestroyedOnRecreateEntities( ) {
		return util::get_method< void( __thiscall* )( void* ) >( networkable( ), 13 )( networkable( ) );
	}

	// normal table.
	__forceinline const vec3_t& GetAbsOrigin( ) {
		return util::get_method< const vec3_t & ( __thiscall* )( void* ) >( this, 10 )( this );
	}

	__forceinline const ang_t& GetAbsAngles( ) {
		return util::get_method< const ang_t & ( __thiscall* )( void* ) >( this, 11 )( this );
	}

	__forceinline bool IsPlayer( ) {
		return util::get_method< bool( __thiscall* )( void* ) >( this, ISPLAYER )( this );
	}

	// --- skateboard2018 addition ---
	__forceinline vec3_t GetBonePosition( int iBone ) {
		matrix3x4_t boneMatrixes[ 128 ];

		if ( this->SetupBones( boneMatrixes, 128, 0x100, 0 ) ) {
			matrix3x4_t boneMatrix = boneMatrixes[ iBone ];
			return vec3_t( boneMatrix.m_flMatVal[ 0 ][ 3 ], boneMatrix.m_flMatVal[ 1 ][ 3 ], boneMatrix.m_flMatVal[ 2 ][ 3 ] );
		}

		return vec3_t( 0.f, 0.f, 0.f );
	}

	__forceinline bool IsBaseCombatWeapon( ) {
		return util::get_method< bool( __thiscall* )( void* ) >( this, ISBASECOMBATWEAPON )( this );
	}

	__forceinline std::string GetBombsiteName( ) {
		std::string out;

		// note - dex; bomb_target + 0x150 has a char array for site name... not sure how much memory gets allocated for it.
		out.resize( 32u );

		std::memcpy( &out[ 0 ], ( const void* )( ( uintptr_t )this + 0x150 ), 32u );

		return out;
	}

	__forceinline void InvalidatePhysicsRecursive( InvalidatePhysicsBits_t bits ) {
		using InvalidatePhysicsRecursive_t = void( __thiscall* )( decltype( this ), InvalidatePhysicsBits_t );
		g_csgo.InvalidatePhysicsRecursive.as< InvalidatePhysicsRecursive_t >( )( this, bits );
	}

	__forceinline void SetAbsAngles( const ang_t& angles ) {
		using SetAbsAngles_t = void( __thiscall* )( decltype( this ), const ang_t& );
		g_csgo.SetAbsAngles.as< SetAbsAngles_t >( )( this, angles );
	}

	__forceinline void SetAbsOrigin( const vec3_t& origin ) {
		using SetAbsOrigin_t = void( __thiscall* )( decltype( this ), const vec3_t& );
		g_csgo.SetAbsOrigin.as< SetAbsOrigin_t >( )( this, origin );
	}

	__forceinline void SetAbsVelocity( const vec3_t& velocity ) {
		using SetAbsVelocity_t = void( __thiscall* )( decltype( this ), const vec3_t& );
		g_csgo.SetAbsVelocity.as< SetAbsVelocity_t >( )( this, velocity );
	}

	__forceinline void AddEffect( int effects ) {
		m_fEffects( ) |= effects;
	}

	__forceinline int get_class_id( ) {
		ClientClass* cc{ GetClientClass( ) };

		return ( cc ) ? cc->m_ClassID : -1;
	}

	__forceinline bool is( hash32_t hash ) {
		return g_netvars.GetClientID( hash ) == get_class_id( );
	}
};

// define these here since its related to entities.
inline bool CGameTrace::DidHitWorld( ) const {
	return m_entity && m_entity->index( ) == 0;
}

inline bool CGameTrace::DidHitNonWorldEntity( ) const {
	return m_entity && m_entity->index( ) > 0 && m_entity->index( ) <= 65;
}

inline bool CGameTrace::DidHit( ) const
{
	return m_fraction < 1 || m_allsolid || m_startsolid;
}

// --- skateboard2018 additions ---

#define OFFSET( t, n, o )\
t &n( )\
{\
    return *( t * )( ( uintptr_t )this + o );\
}

struct c_cs_inferno : public Entity
{
	float& get_entity_spawn_time( ) {
		return *( float* )( ( uintptr_t )this + 0x20 );
	}

	OFFSET( float, get_time, 0x20 )
		int& m_DmgRadius( );
	int& m_flDamage( );

	static constexpr float expire_time = 7.f;
};

struct mstudioevent_t
{
	float				cycle;
	int					event;
	int					type;
	inline const char* pszOptions(void) const { return options; }
	char				options[64];

	int					szeventindex;
	inline char* const pszEventName(void) const { return ((char*)this) + szeventindex; }
};

struct mstudioiklock_t
{
	int			chain;
	float		flPosWeight;
	float		flLocalQWeight;
	int			flags;

	int			unused[4];
};

struct mstudioautolayer_t
{
	//private:
	short				iSequence;
	short				iPose;
	//public:
	int					flags;
	float				start;	// beginning of influence
	float				peak;	// start of full influence
	float				tail;	// end of full influence
	float				end;	// end of all influence
};

struct mstudioactivitymodifier_t
{
	int					sznameindex;
	inline char* pszName() { return (sznameindex) ? (char*)(((byte*)this) + sznameindex) : NULL; }
};

struct mstudioanimtag_t
{
	int					tag;
	float				cycle;

	int					sztagindex;
	inline char* const pszTagName(void) const { return ((char*)this) + sztagindex; }
};

struct mstudioseqdesc_t
{
	int					baseptr;
	inline studiohdr_t* pStudiohdr(void) const { return (studiohdr_t*)(((byte*)this) + baseptr); }

	int					szlabelindex;
	inline char* const pszLabel(void) const { return ((char*)this) + szlabelindex; }

	int					szactivitynameindex;
	inline char* const pszActivityName(void) const { return ((char*)this) + szactivitynameindex; }

	int					flags;		// looping/non-looping flags

	int					activity;	// initialized at loadtime to game DLL values
	int					actweight;

	int					numevents;
	int					eventindex;
	inline mstudioevent_t* pEvent(int i) const { return (mstudioevent_t*)(((byte*)this) + eventindex) + i; };

	vec3_t				bbmin;		// per sequence bounding box
	vec3_t				bbmax;

	int					numblends;

	// Index into array of shorts which is groupsize[0] x groupsize[1] in length
	int					animindexindex;

	inline int			anim(int x, int y) const
	{
		if (x >= groupsize[0])
		{
			x = groupsize[0] - 1;
		}

		if (y >= groupsize[1])
		{
			y = groupsize[1] - 1;
		}

		int offset = y * groupsize[0] + x;
		short* blends = (short*)(((byte*)this) + animindexindex);
		int value = (int)blends[offset];
		return value;
	}

	int					movementindex;	// [blend] float array for blended movement
	int					groupsize[2];
	int					paramindex[2];	// X, Y, Z, XR, YR, ZR
	float				paramstart[2];	// local (0..1) starting value
	float				paramend[2];	// local (0..1) ending value
	int					paramparent;

	float				fadeintime;		// ideal cross fate in time (0.2 default)
	float				fadeouttime;	// ideal cross fade out time (0.2 default)

	int					localentrynode;		// transition node at entry
	int					localexitnode;		// transition node at exit
	int					nodeflags;		// transition rules

	float				entryphase;		// used to match entry gait
	float				exitphase;		// used to match exit gait

	float				lastframe;		// frame that should generation EndOfSequence

	int					nextseq;		// auto advancing sequences
	int					pose;			// index of delta animation between end and nextseq

	int					numikrules;

	int					numautolayers;	//
	int					autolayerindex;
	inline mstudioautolayer_t* pAutolayer(int i) const { return (mstudioautolayer_t*)(((byte*)this) + autolayerindex) + i; };

	int					weightlistindex;
	inline float* pBoneweight(int i) const { return ((float*)(((byte*)this) + weightlistindex) + i); };
	inline float		weight(int i) const { return *(pBoneweight(i)); };

	// FIXME: make this 2D instead of 2x1D arrays
	int					posekeyindex;
	float* pPoseKey(int iParam, int iAnim) const { return (float*)(((byte*)this) + posekeyindex) + iParam * groupsize[0] + iAnim; }
	float				poseKey(int iParam, int iAnim) const { return *(pPoseKey(iParam, iAnim)); }

	int					numiklocks;
	int					iklockindex;
	inline mstudioiklock_t* pIKLock(int i) const { return (mstudioiklock_t*)(((byte*)this) + iklockindex) + i; };

	// Key values
	int					keyvalueindex;
	int					keyvaluesize;
	inline const char* KeyValueText(void) const { return keyvaluesize != 0 ? ((char*)this) + keyvalueindex : NULL; }

	int					cycleposeindex;		// index of pose parameter to use as cycle index

	int					activitymodifierindex;
	int					numactivitymodifiers;
	inline mstudioactivitymodifier_t* pActivityModifier(int i) const { return activitymodifierindex != 0 ? (mstudioactivitymodifier_t*)(((byte*)this) + activitymodifierindex) + i : NULL; };

	int					animtagindex;
	int					numanimtags;
	inline mstudioanimtag_t* pAnimTag(int i) const { return (mstudioanimtag_t*)(((byte*)this) + animtagindex) + i; };

	int					rootDriverIndex;

	int					unused[2];		// remove/add as appropriate (grow back to 8 ints on version change!)
};

class CStudioHdr {
public:
	studiohdr_t* m_studio_hdr;
	void* m_virtual_model;
	void* m_soft_body;

	mutable CUtlVector<const studiohdr_t*> m_studio_hdr_cache;
	mutable int        m_frame_unlock_counter;
	int* m_frame_unlock_counter_mutex;
	std::byte        pad0[0x8];
	CUtlVector<int>    m_bone_flags;
	CUtlVector<int>    m_bone_parent;
	void* m_activity_to_sequence;
public:
	mstudioseqdesc_t* pSeqDesc(int seq)
	{
		if (!seq)
			return nullptr;

		// fix this shit
		static auto target = pattern::find(g_csgo.m_client_dll, "55 8B EC 56 8B 75 08 57 8B F9 85 F6 78 18").as<mstudioseqdesc_t * (__thiscall*)(CStudioHdr*, int)>();
		return target(this, seq);
	}
};

struct mstudioposeparamdesc_t {
	int					m_name_index;
	inline char* const  GetName(void) const { return ((char*)this) + m_name_index; }
	int					m_flags;	// ????
	float				m_start;	// starting value
	float				m_end;	// ending value
	float				m_loop;	// looping range, 0 for no looping, 360 for rotations, etc.
};

struct animstate_pose_param_cache_t
{
	bool		m_init;
	int			m_index;
	const char* m_name;

	animstate_pose_param_cache_t()
	{
		m_init = false;
		m_index = -1;
		m_name = "";
	}

	mstudioposeparamdesc_t* pPoseParameter(CStudioHdr* hdr, int index) {
		return g_csgo.PoseParameter(hdr, index);
	}

	bool Init(Player* pPlayer, const char* szPoseParamName);
	void SetValue(Player* player, float flValue);
};

struct aimmatrix_transition_t
{
	float	m_duration_state_has_been_valid;
	float	m_duration_state_has_been_invalid;
	float	m_how_long_to_wait_until_transition_can_blend_in;
	float	m_how_long_to_wait_until_transition_can_blend_out;
	float	m_blend_value;

	void UpdateTransitionState(bool bStateShouldBeValid, float flTimeInterval, float flSpeed)
	{
		if (bStateShouldBeValid)
		{
			m_duration_state_has_been_invalid = 0;
			m_duration_state_has_been_valid += flTimeInterval;
			if (m_duration_state_has_been_valid >= m_how_long_to_wait_until_transition_can_blend_in)
			{
				m_blend_value = math::Approach(1, m_blend_value, flSpeed);
			}
		}
		else
		{
			m_duration_state_has_been_valid = 0;
			m_duration_state_has_been_invalid += flTimeInterval;
			if (m_duration_state_has_been_invalid >= m_how_long_to_wait_until_transition_can_blend_out)
			{
				m_blend_value = math::Approach(0, m_blend_value, flSpeed);
			}
		}
	}

	void Init(void)
	{
		m_duration_state_has_been_valid = 0;
		m_duration_state_has_been_invalid = 0;
		m_how_long_to_wait_until_transition_can_blend_in = 0.3f;
		m_how_long_to_wait_until_transition_can_blend_out = 0.3f;
		m_blend_value = 0;
	}

	aimmatrix_transition_t()
	{
		Init();
	}
};

class CCSGOPlayerAnimState {
public:
	int* m_layer_order_preset = nullptr;
	bool					m_first_run_since_init = false;

	bool					m_first_foot_plant_since_init = false;
	int						m_last_update_tick = 0;
	float					m_eye_position_smooth_lerp = 0.0f;

	float					m_strafe_change_weight_smooth_fall_off = 0.0f;

	float					m_stand_walk_duration_state_has_been_valid = 0.0f;
	float					m_stand_walk_duration_state_has_been_invalid = 0.0f;
	float					m_stand_walk_how_long_to_wait_until_transition_can_blend_in = 0.0f;
	float					m_stand_walk_how_long_to_wait_until_transition_can_blend_out = 0.0f;
	float					m_stand_walk_blend_value = 0.0f;

	float					m_stand_run_duration_state_has_been_valid = 0.0f;
	float					m_stand_run_duration_state_has_been_invalid = 0.0f;
	float					m_stand_run_how_long_to_wait_until_transition_can_blend_in = 0.0f;
	float					m_stand_run_how_long_to_wait_until_transition_can_blend_out = 0.0f;
	float					m_stand_run_blend_value = 0.0f;

	float					m_crouch_walk_duration_state_has_been_valid = 0.0f;
	float					m_crouch_walk_duration_state_has_been_invalid = 0.0f;
	float					m_crouch_walk_how_long_to_wait_until_transition_can_blend_in = 0.0f;
	float					m_crouch_walk_how_long_to_wait_until_transition_can_blend_out = 0.0f;
	float					m_crouch_walk_blend_value = 0.0f;

	int						m_cached_model_index = 0;

	float					m_step_height_left = 0.0f;
	float					m_step_height_right = 0.0f;

	Weapon* m_weapon_last_bone_setup = nullptr;

	Player* m_player = nullptr;//0x0060 
	Weapon* m_weapon = nullptr;//0x0064
	Weapon* m_weapon_last = nullptr;//0x0068

	float					m_last_update_time = 0.0f;//0x006C	
	int						m_last_update_frame = 0;//0x0070 
	float					m_last_update_increment = 0.0f;//0x0074 

	float					m_eye_yaw = 0.0f; //0x0078 
	float					m_eye_pitch = 0.0f; //0x007C 
	float					m_abs_yaw = 0.0f; //0x0080 
	float					m_abs_yaw_last = 0.0f; //0x0084 
	float					m_move_yaw = 0.0f; //0x0088 
	float					m_move_yaw_ideal = 0.0f; //0x008C 
	float					m_move_yaw_current_to_ideal = 0.0f; //0x0090 	
	float					m_time_to_align_lower_body;

	float					m_primary_cycle = 0.0f; //0x0098
	float					m_move_weight = 0.0f; //0x009C 

	float					m_move_weight_smoothed = 0.0f;
	float					m_anim_duck_amount = 0.0f; //0x00A4
	float					m_duck_additional = 0.0f; //0x00A8
	float					m_recrouch_weight = 0.0f;

	vec3_t					m_position_current = vec3_t( 0, 0, 0 ); //0x00B0
	vec3_t					m_position_last = vec3_t( 0, 0, 0 ); //0x00BC 

	vec3_t					m_velocity = vec3_t( 0, 0, 0 ); //0x00C8
	vec3_t					m_velocity_normalized = vec3_t( 0, 0, 0 ); // 
	vec3_t					m_velocity_normalized_non_zero = vec3_t( 0, 0, 0 ); //0x00E0
	float					m_velocity_length_xy = 0.0f; //0x00EC
	float					m_velocity_length_z = 0.0f; //0x00F0

	float					m_speed_as_portion_of_run_top_speed = 0.0f; //0x00F4
	float					m_speed_as_portion_of_walk_top_speed = 0.0f; //0x00F8 
	float					m_speed_as_portion_of_crouch_top_speed = 0.0f; //0x00FC

	float					m_duration_moving = 0.0f; //0x0100
	float					m_duration_still = 0.0f; //0x0104

	bool					m_on_ground = false; //0x0108 

	bool					m_landing = false; //0x0109
	float					m_jump_to_fall = 0.0f;
	float					m_duration_in_air = 0.0f; //0x0110
	float					m_left_ground_height = 0.0f; //0x0114 
	float					m_land_anim_multiplier = 0.0f; //0x0118 

	float					m_walk_run_transition = 0.0f; //0x011C

	bool					m_landed_on_ground_this_frame = false;
	bool					m_left_the_ground_this_frame = false;
	float					m_in_air_smooth_value = 0.0f;

	bool					m_on_ladder = false; //0x0124
	float					m_ladder_weight = 0.0f; //0x0128
	float					m_ladder_speed = 0.0f;

	bool					m_walk_to_run_transition_state = false;

	bool					m_defuse_started = false;
	bool					m_plant_anim_started = false;
	bool					m_twitch_anim_started = false;
	bool					m_adjust_started = false;

	char					m_activity_modifiers_server[ 20 ] = { };
	//CUtlVector<CUtlSymbol>	m_activity_modifiers;

	float					m_next_twitch_time = 0.0f;

	float					m_time_of_last_known_injury = 0.0f;

	float					m_last_velocity_test_time = 0.0f;
	vec3_t					m_velocity_last = vec3_t( 0, 0, 0 );
	vec3_t					m_target_acceleration = vec3_t( 0, 0, 0 );
	vec3_t					m_acceleration = vec3_t( 0, 0, 0 );
	float					m_acceleration_weight = 0.0f;

	float					m_aim_matrix_transition = 0.0f;
	float					m_aim_matrix_transition_delay = 0.0f;

	bool					m_flashed = false;

	float					m_strafe_change_weight = 0.0f;
	float					m_strafe_change_target_weight = 0.0f;
	float					m_strafe_change_cycle = 0.0f;
	int						m_strafe_sequence = 0;
	bool					m_strafe_changing = false;
	float					m_duration_strafing = 0.0f;

	float					m_foot_lerp = 0.0f;

	bool					m_feet_crossed = false;

	bool					m_player_is_accelerating = false;

	animstate_pose_param_cache_t m_pose_param_mappings[ 20 ] = { };

	float					m_duration_move_weight_is_too_high = 0.0f;
	float					m_static_approach_speed = 0.0f;

	int						m_previous_move_state = 0;
	float					m_stutter_step = 0.0f;

	float					m_action_weight_bias_remainder = 0.0f;

	vec3_t m_foot_left_pos_anim = vec3_t( 0, 0, 0 );
	vec3_t m_foot_left_pos_anim_last = vec3_t( 0, 0, 0 );
	vec3_t m_foot_left_pos_plant = vec3_t( 0, 0, 0 );
	vec3_t m_foot_left_plant_vel = vec3_t( 0, 0, 0 );
	float m_foot_left_lock_amount = 0.0f;
	float m_foot_left_last_plant_time = 0.0f;

	vec3_t m_foot_right_pos_anim = vec3_t( 0, 0, 0 );
	vec3_t m_foot_right_pos_anim_last = vec3_t( 0, 0, 0 );
	vec3_t m_foot_right_pos_plant = vec3_t( 0, 0, 0 );
	vec3_t m_foot_right_plant_vel = vec3_t( 0, 0, 0 );
	float m_foot_right_lock_amount = 0.0f;
	float m_foot_right_last_plant_time = 0.0f;

	float					m_camera_smooth_height = 0.0f;
	bool					m_smooth_height_valid = false;
	float					m_last_time_velocity_over_ten = 0.0f;

	float					m_aim_yaw_min = 0.0f;//0x0330
	float					m_aim_yaw_max = 0.0f;//0x0334
	float					m_aim_pitch_min = 0.0f;
	float					m_aim_pitch_max = 0.0f;

	int						m_animstate_model_version = 0;
}; // size: 0x344

class C_AnimationLayer {
public:
	float	m_anim_time;
	float	m_fade_out_time;

	// dispatch flags
	CStudioHdr* m_studio_hdr;
	int		m_dispatched_src;
	int		m_dispatched_dst;

	int		    m_order;
	int		    m_sequence;
	float		m_prev_cycle;
	float		m_weight;
	float		m_weight_delta_rate;

	// used for automatic crossfades between sequence changes
	float		m_playback_rate;
	float		m_cycle;

	void*               m_owner;
	int					m_invalidate_physics_bits;
}; // size: 0x0038

class CBoneAccessor {
public:
	void* m_pAnimating;
	BoneArray* m_pBones;
	int        m_ReadableBones;
	int        m_WritableBones;
};

class CBoneCache {
public:
	BoneArray* m_pCachedBones;
	PAD( 0x8 );
	int        m_CachedBoneCount;
};

class Ragdoll : public Entity {
public:
	__forceinline Player* GetPlayer( ) {
		return g_csgo.m_entlist->GetClientEntityFromHandle< Player* >( m_hPlayer( ) );
	}

	__forceinline EHANDLE& m_hPlayer( ) {
		return get< EHANDLE >( g_entoffsets.m_hPlayer );
	}

	__forceinline float& m_flDeathYaw( ) {
		return get< float >( g_entoffsets.m_flDeathYaw );
	}

	// --- skateboard2018 addition ( ragdoll abs yaw proxy ) ---
	__forceinline float& m_flAbsYaw( ) {
		return get< float >( g_entoffsets.m_flAbsYaw );
	}
};

class Player : public Entity {
public:
	__forceinline int& m_lifeState( ) {
		return get< int >( g_entoffsets.m_lifeState );
	}

	__forceinline int& m_vphysicsCollisionState( ) {
		return get< int >( g_entoffsets.m_vphysicsCollisionState);
	}

	__forceinline int& m_fFlags( ) {
		return get< int >( g_entoffsets.m_fFlags );
	}

	__forceinline int& m_iHealth( ) {
		return get< int >( g_entoffsets.m_iHealth );
	}

	__forceinline int& m_iAccount( ) {
		return get< int >( g_entoffsets.m_iAccount );
	}

	__forceinline bool& m_bHasDefuser( ) {
		return get< bool >( g_entoffsets.m_bHasDefuser );
	}

	__forceinline int& m_nHitboxSet( ) {
		return get< int >( g_entoffsets.m_nHitboxSet );
	}

	__forceinline CBaseHandle& m_hViewModel( )
	{
		return get< CBaseHandle >( g_entoffsets.m_hViewModel );
	}

	__forceinline bool m_bIsLocalPlayer( ) {
		// .text:101E0078 674     84 C0				   test    al, al          ; Logical Compare
		// .text:101E007A 674     74 17				   jz      short loc_101E0093; Jump if Zero( ZF = 1 )
		// .text:101E007C 674     8A 83 F8 35 00 00	   mov     al, [ ebx + 35F8h ]
		return get< bool >( g_csgo.IsLocalPlayer );
	}

	__forceinline CCSGOPlayerAnimState* m_PlayerAnimState( ) {
		// .text:1037A5B8 00C     E8 E3 40 E6 FF         call    C_BasePlayer__Spawn ; Call Procedure
		// .text:1037A5BD 00C     80 BE E1 39 00 00 00   cmp     byte ptr[ esi + 39E1h ], 0; Compare Two Operands
		// .text:1037A5C4 00C     74 48                  jz      short loc_1037A60E; Jump if Zero( ZF = 1 )
		// .text:1037A5C6 00C     8B 8E 74 38 00 00      mov     ecx, [ esi + 3874h ]; this
		// .text:1037A5CC 00C     85 C9                  test    ecx, ecx; Logical Compare
		// .text:1037A5CE 00C     74 3E                  jz      short loc_1037A60E; Jump if Zero( ZF = 1 )
		return get< CCSGOPlayerAnimState* >( g_csgo.PlayerAnimState );
	}

	__forceinline CStudioHdr* m_studioHdr( ) {
		// .text:1017E902 08C    8B 86 3C 29 00 00    mov     eax, [ esi + 293Ch ]
		// .text:1017E908 08C    89 44 24 10          mov[ esp + 88h + var_78 ], eax
		return get< CStudioHdr* >( g_csgo.studioHdr );
	}

	__forceinline ulong_t& m_iMostRecentModelBoneCounter( ) {
		// .text:101AC9A9 000    89 81 80 26 00 00    mov[ ecx + 2680h ], eax
		return get< ulong_t >( g_csgo.MostRecentModelBoneCounter );
	}

	__forceinline ulong_t& m_iMostRecentBoneSetupRequest( ) {
		// .text:101AC9A9 000    89 81 80 26 00 00    mov[ ecx + 2680h ], eax
		return get< ulong_t >( g_csgo.MostRecentBoneSetupRequest );
	}

	__forceinline matrix3x4_t& m_rgflCoordinateFrame( ) {
		// .text:101AC9A9 000    89 81 80 26 00 00    mov[ ecx + 2680h ], eax
		return get< matrix3x4_t >( g_entoffsets.m_rgflCoordinateFrame );
	}

	__forceinline float& m_flLastBoneSetupTime( ) {
		// .text:101AC99F 000    C7 81 14 29 00 00 FF FF+    mov     dword ptr [ecx+2914h], 0FF7FFFFFh;
		return get< float >( g_csgo.LastBoneSetupTime );
	}

	__forceinline int& m_nTickBase( ) {
		return get< int >( g_entoffsets.m_nTickBase );
	}

	__forceinline float& m_flNextAttack( ) {
		return get< float >( g_entoffsets.m_flNextAttack );
	}

	__forceinline float& m_flDuckAmount( ) {
		return get< float >( g_entoffsets.m_flDuckAmount );
	}

	__forceinline float& m_flDuckSpeed( ) {
		return get< float >( g_entoffsets.m_flDuckSpeed );
	}

	__forceinline float& m_flSimulationTime( ) {
		return get< float >( g_entoffsets.m_flSimulationTime );
	}

	__forceinline float& m_flOldSimulationTime( ) {
		return get< float >( g_entoffsets.m_flOldSimulationTime );
	}

	__forceinline float& m_flLowerBodyYawTarget( ) {
		return get< float >( g_entoffsets.m_flLowerBodyYawTarget );
	}

	__forceinline float& m_fImmuneToGunGameDamageTime( ) {
		return get< float >( g_entoffsets.m_fImmuneToGunGameDamageTime );
	}

	__forceinline bool& m_bHasHelmet( ) {
		return get< bool >( g_entoffsets.m_bHasHelmet );
	}

	__forceinline bool& m_bClientSideAnimation( ) {
		return get< bool >( g_entoffsets.m_bClientSideAnimation );
	}

	__forceinline bool& m_bHasHeavyArmor( ) {
		return get< bool >( g_entoffsets.m_bHasHeavyArmor );
	}

	__forceinline bool& m_bIsScoped( ) {
		return get< bool >( g_entoffsets.m_bIsScoped );
	}

	__forceinline bool& m_bDucking( ) {
		return get< bool >( g_entoffsets.m_bDucking );
	}

	__forceinline bool& m_bStrafing( ) {
		return get< bool >( g_entoffsets.m_bStrafing );
	}

	__forceinline float& m_flThirdpersonRecoil( ) {
		return get< float >( g_entoffsets.m_flThirdpersonRecoil);
	}

	__forceinline bool& m_bSpotted( ) {
		return get< bool >( g_entoffsets.m_bSpotted );
	}

	__forceinline bool& m_bIsWalking( ) {
		return get< bool >( g_entoffsets.m_bIsWalking );
	}

	__forceinline bool& m_bUseNewAnimstate( ) {
		return get< bool >( g_entoffsets.m_bUseNewAnimstate);
	}

	__forceinline int& m_iObserverMode( ) {
		return get< int >( g_entoffsets.m_iObserverMode );
	}

	__forceinline int& m_ArmorValue( ) {
		return get< int >( g_entoffsets.m_ArmorValue );
	}

	__forceinline int& m_iMoveState( ) {
		return get< int >( g_entoffsets.m_iMoveState );
	}

	__forceinline float& m_flMaxspeed( ) {
		return get< float >( g_entoffsets.m_flMaxspeed );
	}

	__forceinline float& m_flFlashBangTime( ) {
		return get< float >( g_entoffsets.m_flFlashBangTime );
	}

	__forceinline ang_t& m_angEyeAngles( ) {
		return get< ang_t >( g_entoffsets.m_angEyeAngles );
	}

	__forceinline ang_t& m_aimPunchAngle( ) {
		return get< ang_t >( g_entoffsets.m_aimPunchAngle );
	}

	__forceinline ang_t& m_viewPunchAngle( ) {
		return get< ang_t >( g_entoffsets.m_viewPunchAngle );
	}

	__forceinline ang_t& m_aimPunchAngleVel( ) {
		return get< ang_t >( g_entoffsets.m_aimPunchAngleVel );
	}

	__forceinline vec3_t& m_vecViewOffset( ) {
		return get< vec3_t >( g_entoffsets.m_vecViewOffset );
	}

	__forceinline CUserCmd& m_PlayerCommand( ) {
		return get< CUserCmd >( g_entoffsets.m_PlayerCommand );
	}

	__forceinline CUserCmd*& m_pCurrentCommand( ) {
		return get< CUserCmd* >( g_entoffsets.m_pCurrentCommand );
	}

	__forceinline float* m_flPoseParameter( ) {
		return ( float* )( ( uintptr_t )this + g_entoffsets.m_flPoseParameter );
	}

	__forceinline CBaseHandle* m_hMyWearables( ) {
		return ( CBaseHandle* )( ( uintptr_t )this + g_entoffsets.m_hMyWearables );
	}

	__forceinline CBoneCache& m_BoneCache( ) {
		// TODO; sig
		return get< CBoneCache >( g_entoffsets.m_BoneCache );
	}

	__forceinline EHANDLE& m_hObserverTarget( ) {
		return get< EHANDLE >( g_entoffsets.m_hObserverTarget );
	}

	__forceinline EHANDLE& m_hActiveWeapon( ) {
		return get< EHANDLE >( g_entoffsets.m_hActiveWeapon );
	}

	__forceinline EHANDLE& m_hGroundEntity( ) {
		return get< EHANDLE >( g_entoffsets.m_hGroundEntity );
	}

	__forceinline CBaseHandle* m_hMyWeapons( ) {
		return ( CBaseHandle* )( ( uintptr_t )this + g_entoffsets.m_hMyWeapons );
	}

	__forceinline C_AnimationLayer* m_AnimOverlay( ) {
		// .text:1017EAB1 08C    8B 47 1C                mov     eax, [edi+1Ch]
		// .text:1017EAB4 08C    8D 0C D5 00 00 00 00    lea     ecx, ds:0[ edx * 8 ]; Load Effective Address
		// .text:1017EABB 08C    2B CA                   sub     ecx, edx; Integer Subtraction
		// .text:1017EABD 08C    8B 80 70 29 00 00       mov     eax, [ eax + 2970h ]
		// .text:1017EAC3 08C    8D 34 C8                lea     esi, [ eax + ecx * 8 ]; Load Effective Address
		// .text:1017EAC6
		return get< C_AnimationLayer* >( g_csgo.AnimOverlay );
	}

	__forceinline float& m_flSpawnTime( ) {
		// .text:10381AB3 00C    F3 0F 10 49 10             movss   xmm1, dword ptr [ecx+10h] ; Move Scalar Single-FP
		// .text:10381AB8 00C    F3 0F 5C 88 90 A2 00 00    subss   xmm1, dword ptr[ eax + 0A290h ]; Scalar Single - FP Subtract
		return get< float >( g_csgo.SpawnTime );
	}

	__forceinline CBoneAccessor& m_BoneAccessor( ) {
		// .text:101A9253 1C4    C7 81 A0 26 00 00 00 FF 0F 00    mov     dword ptr[ ecx + 26A0h ], 0FFF00h
		// .text:101A925D 1C4    C7 81 9C 26 00 00 00 FF 0F 00    mov     dword ptr[ ecx + 269Ch ], 0FFF00h
		// .text:101A9267 1C4    8B 10                            mov     edx, [ eax ]
		// .text:101A9269 1C4    8D 81 94 26 00 00                lea     eax, [ ecx + 2694h ]; Load Effective Address
		// .text:101A926F 1C4    50                               push    eax
		return get< CBoneAccessor >( g_csgo.BoneAccessor );
	}

	__forceinline int& nextThinkTick( )
	{
		return *reinterpret_cast< int* >( ( uintptr_t )this + 0xF8 );
	}

	__forceinline int& m_afButtonForced( )
	{
		return *reinterpret_cast< int* >( ( uintptr_t )this + 0x3310 );
	}

	__forceinline int& m_nButtons( )
	{
		return *reinterpret_cast< int* >( ( uintptr_t )this + 0x31E8 );
	}

	__forceinline int& m_nImpulse( )
	{
		return *reinterpret_cast< int* >( ( uintptr_t )this + 0x31EC );
	}

	__forceinline int& m_afButtonLast( )
	{
		return *reinterpret_cast< int* >( ( uintptr_t )this + 0x31DC );
	}

	__forceinline int& m_afButtonPressed( )
	{
		return *reinterpret_cast< int* >( ( uintptr_t )this + 0x31E0 );
	}

	__forceinline int& m_afButtonReleased( )
	{
		return *reinterpret_cast< int* >( ( uintptr_t )this + 0x31E4 );
	}

	__forceinline float& m_flVelocityModifier( )
	{
		return get< float >( g_entoffsets.m_flVelocityModifier );
	}

	__forceinline float& m_flFallVelocity( )
	{
		return get< float >( g_entoffsets.m_flFallVelocity );
	}

	__forceinline vec3_t& m_vecBaseVelocity( )
	{
		return get< vec3_t >( g_entoffsets.m_vecBaseVelocity );
	}

public:
	enum indices : size_t {
		GETREFEHANDLE = 2,
		TESTHITBOXES = 52,
		BUILDTRANSFORMATIONS = 184,
		DOEXTRABONEPROCESSING = 192,
		STANDARDBLENDINGRULES = 200,
		UPDATECLIENTSIDEANIMATION = 218, // 55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74 36
		GETACTIVEWEAPON = 262,
		GETEYEPOS = 163,
		GETFOV = 321,
		UPDATECOLLISIONBOUNDS = 329 // 56 57 8B F9 8B 0D ? ? ? ? F6 87 ? ? ? ? ?
	};

public:
	int LookupBone(const char* szName) {
		return g_csgo.LookupBone(this, szName);
	}

	__forceinline int LookupPoseParameter(CStudioHdr* pStudioHdr, const char* szName)
	{
		// 55 8B EC 57 8B 7D 08 85 FF 75 08
		return g_csgo.LookupPoseParameter(pStudioHdr, szName);
	}

	__forceinline bool PhysicsRunThink( int thinkMethod )
	{
		typedef bool( __thiscall* PhysicsRunThinkFN )( void*, int );
		static auto PhysicsRunThink = pattern::find( g_csgo.m_client_dll, "55 8B EC 83 EC 10 53 56 57 8B F9 8B 87" ).as<PhysicsRunThinkFN>( );

		return PhysicsRunThink( this, thinkMethod );
	}

	__forceinline void Think( )
	{
		typedef void( __thiscall* ThinkFN )( void* );
		return ( *reinterpret_cast< ThinkFN** >( this ) )[ 137 ]( this );
	}

	__forceinline void PreThink( )
	{
		typedef void( __thiscall* PreThinkFN )( void* );
		return ( *reinterpret_cast< PreThinkFN** >( this ) )[ 307 ]( this );
	}

	__forceinline void UpdateButtonState( int nUserCmdButtonMask )
	{
		int m_afButtonLast = nUserCmdButtonMask;
		int buttonsChanged = m_afButtonLast ^ this->m_nButtons( );
		this->m_afButtonLast( ) = this->m_nButtons( );
		this->m_nButtons( ) = m_afButtonLast;
		this->m_afButtonPressed( ) = m_afButtonLast & buttonsChanged;
		this->m_afButtonReleased( ) = buttonsChanged & ~m_afButtonLast;
	}

	__forceinline int IndexFromAnimTagName( const char* szName )
	{
		for ( int i = 1; i < ANIMTAG_COUNT; i++ )
		{
			const animtaglookup_t* pAnimTag = &g_AnimTagLookupTable[ i ];
			if ( !std::strcmp( szName, pAnimTag->szName ) )
			{
				return pAnimTag->nIndex;
			}
		}
		return ANIMTAG_INVALID;
	}

	__forceinline float GetFirstSequenceAnimTag( int sequence, int nDesiredTag, float flStart, float flEnd )
	{
		CStudioHdr* pstudiohdr = GetModelPtr( );

		if ( !pstudiohdr/* || sequence >= pstudiohdr->GetNumSeq()*/ )
			return flStart;

		mstudioseqdesc_t* seqdesc = pstudiohdr->pSeqDesc( sequence );
		if ( seqdesc->numanimtags == 0 )
			return flStart;

		mstudioanimtag_t* panimtag = NULL;

		for ( int index = 0; index < ( int )seqdesc->numanimtags; index++ )
		{
			panimtag = seqdesc->pAnimTag( index );

			if ( panimtag->tag == ANIMTAG_INVALID )
				continue;

			if ( panimtag->tag == ANIMTAG_UNINITIALIZED )
			{
				panimtag->tag = IndexFromAnimTagName( panimtag->pszTagName( ) );
			}

			if ( panimtag->tag == nDesiredTag && panimtag->cycle >= flStart && panimtag->cycle < flEnd )
			{
				return panimtag->cycle;
			}
		}

		return flStart;
	}

	__forceinline float GetLayerIdealWeightFromSeqCycle( C_AnimationLayer* pLayer )
	{
		auto model = GetModelPtr( );
		if ( !model )
			return 0.f;

		auto seqdesc = model->pSeqDesc( pLayer->m_sequence );
		if ( !seqdesc )
			return 0.f;

		float flCycle = pLayer->m_cycle;
		if ( flCycle >= 0.999f )
			flCycle = 1;

		float flEaseIn = seqdesc->fadeintime; // seqdesc.fadeintime;
		float flEaseOut = seqdesc->fadeouttime; // seqdesc.fadeouttime;
		float flIdealWeight = 1;

		if ( flEaseIn > 0 && flCycle < flEaseIn )
		{
			flIdealWeight = math::smoothstep_bounds( 0, flEaseIn, flCycle );
		}
		else if ( flEaseOut < 1 && flCycle > flEaseOut )
		{
			flIdealWeight = math::smoothstep_bounds( 1.0f, flEaseOut, flCycle );
		}

		if ( flIdealWeight < 0.0015f )
			return 0.f;

		return std::clamp( flIdealWeight, 0.f, 1.f );
	}

	__forceinline int LookupSequence( const char* label )
	{
		if ( !label )
			return -1;

		typedef int( __thiscall* fnLookupSequence )( void*, const char* );
		static auto lookup_sequnece_adr = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 56 8B F1 83 BE 3C 29 00 00 00 75 14 8B 46 04 8D 4E 04 FF 50 ?? 85 C0 74 07 8B CE E8 ?? ?? ?? ?? 8B B6 3C 29 00 00 85 F6 74 48" ) ).as<fnLookupSequence>( );

		return lookup_sequnece_adr( this, label );
	}

	__forceinline int GetLayerActivity( C_AnimationLayer* pLayer )
	{
		if ( !pLayer )
			return ACT_INVALID;

		return GetSequenceActivity( pLayer->m_sequence );
	}

	__forceinline void IncrementLayerCycle( float increment, C_AnimationLayer* pLayer, bool loop )
	{
		if ( !pLayer )
			return;

		if ( abs( pLayer->m_playback_rate ) <= 0 )
			return;

		float flCurrentCycle = pLayer->m_cycle;
		flCurrentCycle += increment * pLayer->m_playback_rate;

		if ( !loop && flCurrentCycle >= 1 )
		{
			flCurrentCycle = 0.999f;
		}

		pLayer->m_cycle = math::ClampCycle( flCurrentCycle );
	}

	__forceinline void IncrementLayerWeight( float increment, C_AnimationLayer* pLayer )
	{
		if ( !pLayer )
			return;

		if ( abs( pLayer->m_weight_delta_rate ) <= 0.f )
			return;

		float flCurrentWeight = pLayer->m_weight;
		flCurrentWeight += increment * pLayer->m_weight_delta_rate;
		flCurrentWeight = std::clamp( flCurrentWeight, 0.f, 1.f );

		pLayer->m_weight = flCurrentWeight;
	}

	__forceinline void UpdateAnimLayer( C_AnimationLayer* pLayer, int nSequence, float flPlaybackRate, float flWeight, float flCycle )
	{
		if ( nSequence > 1 )
		{
			pLayer->m_sequence = nSequence;
			pLayer->m_playback_rate = flPlaybackRate;
			pLayer->m_cycle = std::clamp( flCycle, 0.f, 1.f );

			pLayer->m_weight = std::clamp( flWeight, 0.f, 1.f );
		}
	}

	__forceinline bool IsLayerSequenceCompleted( float increment, C_AnimationLayer* pLayer )
	{
		if ( !pLayer )
			return false;

		return ( pLayer->m_cycle + ( increment * pLayer->m_playback_rate ) ) >= 1;
	}

	__forceinline void IncrementLayerCycleWeightRateGeneric( float increment, C_AnimationLayer* pLayer )
	{
		if ( !pLayer )
			return;

		float flWeightPrevious = pLayer->m_weight;
		IncrementLayerCycle( increment, pLayer, false );
		pLayer->m_weight = GetLayerIdealWeightFromSeqCycle( pLayer );
		pLayer->m_weight_delta_rate = ( pLayer->m_weight - flWeightPrevious ) / increment;
	}

	__forceinline void SetLayerSequence( C_AnimationLayer* layer, int sequence )
	{
		if ( !layer )
			return;

		layer->m_cycle = 0.0f;
		layer->m_weight = 0.0f;
		layer->m_sequence = sequence;
		layer->m_playback_rate = GetLayerSequenceCycleRate( layer, sequence );
	}

	__forceinline float GetSequenceCycleRate( std::ptrdiff_t seq )
	{
		return util::get_method<float( __thiscall* )( decltype( this ), CStudioHdr*, std::ptrdiff_t )>( this, 216 )( this, GetModelPtr( ), seq );
	}

	__forceinline float GetLayerSequenceCycleRate( C_AnimationLayer* layer, std::ptrdiff_t seq )
	{
		return util::get_method<float( __thiscall* )( decltype( this ), C_AnimationLayer*, std::ptrdiff_t )>( this, 217 )( this, layer, seq );
	}

	__forceinline float GetSequenceMoveDist( int iSequence, float* poses )
	{
		CStudioHdr* pStudioHdr = GetModelPtr( );
		if ( !pStudioHdr )
			return 0.f;

		vec3_t vecReturn;

		typedef void( __fastcall* GetSequenceLinearMotionFN )( CStudioHdr*, int, float*, vec3_t* );
		static auto GetSequenceLinearMotion = pattern::find( g_csgo.m_client_dll, "55 8B EC 83 EC 0C 56 8B F1 57 8B FA 85 F6 75 ? 68" ).as<GetSequenceLinearMotionFN>( );

		__asm {
			mov eax, dword ptr[ GetSequenceLinearMotion ]
				lea ecx, dword ptr[ vecReturn ] // fourth argument
					push ecx
						mov edx, dword ptr[ poses ] // third argument
						push edx
							mov edx, dword ptr[ iSequence ] // second argument
							mov ecx, dword ptr[ pStudioHdr ] // first argument

								call eax
									add esp, 0x8
		}

		return vecReturn.length( );
	}

	// virtuals.
	__forceinline ulong_t GetRefEHandle( ) {
		using GetRefEHandle_t = ulong_t( __thiscall* )( decltype( this ) );
		return util::get_method< GetRefEHandle_t >( this, GETREFEHANDLE )( this );
	}

	__forceinline void BuildTransformations( CStudioHdr* hdr, vec3_t* pos, quaternion_t* q, const matrix3x4_t& transform, int mask, uint8_t* computed ) {
		using BuildTransformations_t = void( __thiscall* )( decltype( this ), CStudioHdr*, vec3_t*, quaternion_t*, matrix3x4_t const&, int, uint8_t* );
		return util::get_method< BuildTransformations_t >( this, BUILDTRANSFORMATIONS )( this, hdr, pos, q, transform, mask, computed );
	}

	__forceinline void StandardBlendingRules( CStudioHdr* hdr, vec3_t* pos, quaternion_t* q, float time, int mask ) {
		using StandardBlendingRules_t = void( __thiscall* )( decltype( this ), CStudioHdr*, vec3_t*, quaternion_t*, float, int );
		return util::get_method< StandardBlendingRules_t >( this, STANDARDBLENDINGRULES )( this, hdr, pos, q, time, mask );
	}

	__forceinline float GetFOV( ) {
		return util::get_method< float( __thiscall* )( decltype( this ) ) >( this, GETFOV )( this );
	}

	__forceinline const vec3_t& WorldSpaceCenter( ) {
		return util::get_method< const vec3_t & ( __thiscall* )( void* ) >( this, WORLDSPACECENTER )( this );
	}

	__forceinline vec3_t GetEyePos( bool abs_origin = false ) {
		if ( abs_origin )
			return m_vecAbsOrigin( ) + m_vecViewOffset( );

		return m_vecOrigin( ) + m_vecViewOffset( );
	}

	// --- skateboard2018 additions ---

	// the game's own C_BaseEntity::GetEyePos ( more accurate than netvar math ).
	__forceinline void GetEyePos( vec3_t* pos ) {
		util::get_method< void( __thiscall* )( decltype( this ), vec3_t* ) >( this, GETEYEPOS )( this, pos );
	}

	__forceinline void ModifyEyePosition( CCSGOPlayerAnimState* state, vec3_t* pos ) {
		if ( !state || !pos )
			return;

		if ( state->m_player &&
			 ( state->m_landing || state->m_player->m_flDuckAmount( ) != 0.f || !state->m_player->GetGroundEntity( ) ) ) {

			constexpr int head_bone = 8;

			BoneArray* bones = state->m_player->m_BoneCache( ).m_pCachedBones;

			if ( bones ) {
				vec3_t head_pos(
					bones[ head_bone ][ 0 ][ 3 ],
					bones[ head_bone ][ 1 ][ 3 ],
					bones[ head_bone ][ 2 ][ 3 ] );

				const float v7 = head_pos.z + 1.7f;

				if ( pos->z > v7 ) {
					const float v3 = pos->z - v7;

					float lerp = ( v3 - 4.f ) * 0.16666667f;

					if ( lerp >= 0.f )
						lerp = std::fminf( lerp, 1.f );

					pos->z = ( ( v7 - pos->z ) * ( ( lerp * lerp ) * 3.0 - ( ( lerp * lerp ) * 2.0 ) * lerp ) ) + pos->z;
				}
			}
		}
	}

	__forceinline vec3_t GetShootPosition( ) {
		vec3_t pos;

		GetEyePos( &pos );

		if ( *reinterpret_cast< int32_t* >( uintptr_t( this ) + 0x39E1 ) ) {
			auto state = m_PlayerAnimState( );

			if ( state )
				ModifyEyePosition( state, &pos );
		}

		return pos;
	}

	__forceinline void UpdateClientSideAnimation( ) {
		return util::get_method< void( __thiscall* )( decltype( this ) ) >( this, UPDATECLIENTSIDEANIMATION )( this );
	}

	__forceinline void UpdateCollisionBounds( ) {
		return util::get_method< void( __thiscall* )( decltype( this ) ) >( this, UPDATECOLLISIONBOUNDS )( this );
	}

	// misc funcs.
	__forceinline CStudioHdr* GetModelPtr( ) {
		using LockStudioHdr_t = void( __thiscall* )( decltype( this ) );

		if ( !m_studioHdr( ) )
			g_csgo.LockStudioHdr.as< LockStudioHdr_t >( )( this );

		return m_studioHdr( );
	}

	__forceinline Weapon* GetActiveWeapon( ) {
		return g_csgo.m_entlist->GetClientEntityFromHandle< Weapon* >( m_hActiveWeapon( ) );
	}

	__forceinline Entity* GetObserverTarget( ) {
		return g_csgo.m_entlist->GetClientEntityFromHandle( m_hObserverTarget( ) );
	}

	__forceinline Entity* GetGroundEntity( ) {
		return g_csgo.m_entlist->GetClientEntityFromHandle( m_hGroundEntity( ) );
	}

	__forceinline void SetBones( const BoneArray* bones ) {
		CBoneCache& cache = m_BoneCache( );
		if ( !cache.m_pCachedBones )
			return;

		std::memcpy( cache.m_pCachedBones, bones, sizeof( BoneArray ) * cache.m_CachedBoneCount );
	}

	__forceinline void GetBones( BoneArray* bones ) {
		CBoneCache& cache = m_BoneCache( );
		if ( !cache.m_pCachedBones )
			return;

		std::memcpy( bones, cache.m_pCachedBones, sizeof( BoneArray ) * cache.m_CachedBoneCount );
	}

	__forceinline void SetAnimState( const CCSGOPlayerAnimState* state ) {
		std::memcpy( m_PlayerAnimState( ), state, sizeof( CCSGOPlayerAnimState ) );
	}

	__forceinline void GetAnimState( CCSGOPlayerAnimState* state ) {
		std::memcpy( state, m_PlayerAnimState( ), sizeof( CCSGOPlayerAnimState ) );
	}

	__forceinline void SetAnimLayers( const C_AnimationLayer* layers ) {
		std::memcpy( m_AnimOverlay( ), layers, sizeof( C_AnimationLayer ) * 13 );
	}

	__forceinline void GetAnimLayers( C_AnimationLayer* layers ) {
		std::memcpy( layers, m_AnimOverlay( ), sizeof( C_AnimationLayer ) * 13 );
	}

	__forceinline void SetPoseParameters( const float* poses ) {
		std::memcpy( m_flPoseParameter( ), poses, sizeof( float ) * PoseParam::POSE_COUNT);
	}

	__forceinline void GetPoseParameters( float* poses ) {
		std::memcpy( poses, m_flPoseParameter( ), sizeof( float ) * PoseParam::POSE_COUNT);
	}

	__forceinline bool ComputeHitboxSurroundingBox( vec3_t* mins, vec3_t* maxs ) {
		using ComputeHitboxSurroundingBox_t = bool( __thiscall* )( void*, vec3_t*, vec3_t* );

		return g_csgo.ComputeHitboxSurroundingBox.as< ComputeHitboxSurroundingBox_t >( )( this, mins, maxs );
	}

	__forceinline int GetSequenceActivity( int sequence ) {
		using GetSequenceActivity_t = int( __fastcall* )( CStudioHdr*, int );

		return g_csgo.GetSequenceActivity.as< GetSequenceActivity_t >( )( GetModelPtr( ), sequence );
	}

	__forceinline bool HasC4( ) {
		using HasC4_t = bool( __thiscall* )( decltype( this ) );
		return g_csgo.HasC4.as< HasC4_t >( )( this );
	}

	__forceinline void InvalidateBoneCache( ) {
		m_iMostRecentModelBoneCounter( ) = 0xFFFFFFFF;
		m_iMostRecentBoneSetupRequest( ) = -1;
		m_flLastBoneSetupTime( ) = -FLT_MAX;
	}

	__forceinline bool alive( ) {
		return m_lifeState( ) == LIFE_ALIVE;
	}

	__forceinline bool enemy( Player* from ) {
		if ( m_iTeamNum( ) != from->m_iTeamNum( ) )
			return true;

		else if ( g_csgo.mp_teammates_are_enemies->GetInt( ) )
			return true;

		return false;
	}
};

class WeaponInfo {
private:
	PAD( 0x4 );											// 0x0000

public:
	const char* m_weapon_name;						// 0x0004 -- actual weapon name, even for usp-s and revolver. ex: "weapon_revolver"
	PAD( 0xC );												// 0x0008
	int               m_max_clip1;							// 0x0014
	int				  m_max_clip2;							// 0x0018
	int				  m_default_clip1;						// 0x001C
	int		          m_default_clip2;						// 0x0020
	int               m_max_reserve;						// 0x0024
	PAD( 0x4 );												// 0x0028
	const char* m_world_model;						// 0x002C
	const char* m_view_model;							// 0x0030
	const char* m_world_dropped_model;				// 0x0034
	PAD( 0x48 );											// 0x0038
	const char* m_ammo_type;							// 0x0080
	uint8_t           pad_0084[ 4 ];						// 0x0084
	const char* m_sfui_name;							// 0x0088
	const char* m_deprecated_weapon_name;				// 0x008C -- shitty weapon name, shows "weapon_deagle" for revolver / etc.
	uint8_t           pad_0090[ 56 ];						// 0x0090
	CSWeaponType      m_weapon_type;						// 0x00C8
	int			      m_in_game_price;						// 0x00CC
	int               m_kill_award;							// 0x00D0
	const char* m_animation_prefix;					// 0x00D4
	float			  m_cycletime;							// 0x00D8
	float			  m_cycletime_alt;						// 0x00DC
	float			  m_time_to_idle;						// 0x00E0
	float			  m_idle_interval;						// 0x00E4
	bool			  m_is_full_auto;						// 0x00E5
	PAD( 0x3 );												// 0x00E8
	int               m_damage;								// 0x00EC
	float             m_armor_ratio;						// 0x00F0
	int               m_bullets;							// 0x00F4
	float             m_penetration;						// 0x00F8
	float             m_flinch_velocity_modifier_large;		// 0x00FC
	float             m_flinch_velocity_modifier_small;		// 0x0100
	float             m_range;								// 0x0104
	float             m_range_modifier;						// 0x0108
	float			  m_throw_velocity;						// 0x010C
	PAD( 0xC );												// 0x0118
	bool			  m_has_silencer;						// 0x0119
	PAD( 0x3 );												// 0x011C
	const char* m_silencer_model;						// 0x0120
	int				  m_crosshair_min_distance;				// 0x0124
	int				  m_crosshair_delta_distance;			// 0x0128
	float             m_max_player_speed;					// 0x012C
	float             m_max_player_speed_alt;				// 0x0130
	float			  m_spread;								// 0x0134
	float			  m_spread_alt;							// 0x0138
	float             m_inaccuracy_crouch;					// 0x013C
	float             m_inaccuracy_crouch_alt;				// 0x0140
	float             m_inaccuracy_stand;					// 0x0144
	float             m_inaccuracy_stand_alt;				// 0x0148
	float             m_inaccuracy_jump_initial;			// 0x014C
	float             m_inaccuracy_jump;					// 0x0150
	float             m_inaccuracy_jump_alt;				// 0x0154
	float             m_inaccuracy_land;					// 0x0158
	float             m_inaccuracy_land_alt;				// 0x015C
	float             m_inaccuracy_ladder;					// 0x0160
	float             m_inaccuracy_ladder_alt;				// 0x0164
	float             m_inaccuracy_fire;					// 0x0168
	float             m_inaccuracy_fire_alt;				// 0x016C
	float             m_inaccuracy_move;					// 0x0170
	float             m_inaccuracy_move_alt;				// 0x0174
	float             m_inaccuracy_reload;					// 0x0178
	int               m_recoil_seed;						// 0x017C
	float			  m_recoil_angle;						// 0x0180
	float             m_recoil_angle_alt;					// 0x0184
	float             m_recoil_angle_variance;				// 0x0188
	float             m_recoil_angle_variance_alt;			// 0x018C
	float             m_recoil_magnitude;					// 0x0190
	float             m_recoil_magnitude_alt;				// 0x0194
	float             m_recoil_magnitude_variance;			// 0x0198
	float             m_recoil_magnitude_variance_alt;		// 0x019C
	float             m_recovery_time_crouch;				// 0x01A0
	float             m_recovery_time_stand;				// 0x01A4
	float             m_recovery_time_crouch_final;			// 0x01A8
	float             m_recovery_time_stand_final;			// 0x01AC
	float             m_recovery_transition_start_bullet;	// 0x01B0
	float             m_recovery_transition_end_bullet;		// 0x01B4
	bool			  m_unzoom_after_shot;					// 0x01B5
	PAD( 0x3 );												// 0x01B8
	bool		      m_hide_view_model_zoomed;				// 0x01B9
	bool			  m_zoom_levels;						// 0x01BA
	PAD( 0x2 );												// 0x01BC
	int				  m_zoom_fov[ 2 ];						// 0x01C4
	float			  m_zoom_time[ 3 ];						// 0x01D0
	PAD( 0x8 );												// 0x01D8
	float             m_addon_scale;						// 0x01DC
	PAD( 0x8 );												// 0x01E4
	int				  m_tracer_frequency;					// 0x01E8
	int				  m_tracer_frequency_alt;				// 0x01EC
	PAD( 0x18 );											// 0x0200
	int				  m_health_per_shot;					// 0x0204
	PAD( 0x8 );												// 0x020C
	float			  m_inaccuracy_pitch_shift;				// 0x0210
	float			  m_inaccuracy_alt_sound_threshold;		// 0x0214
	float			  m_bot_audible_range;					// 0x0218
	PAD( 0x8 );												// 0x0220
	const char* m_wrong_team_msg;						// 0x0224
	bool			  m_has_burst_mode;						// 0x0225
	PAD( 0x3 );												// 0x0228
	bool			  m_is_revolver;						// 0x0229
	bool			  m_can_shoot_underwater;				// 0x022A
	PAD( 0x2 );												// 0x022C	
public:
	__forceinline bool IsSniper( ) const {
		return m_weapon_type == WEAPONTYPE_SNIPER_RIFLE;
	}
};

class IRefCounted {
private:
	volatile long refCount;

public:
	virtual void destructor( char bDelete ) = 0;
	virtual bool OnFinalRelease( ) = 0;

	void unreference( ) {
		if ( InterlockedDecrement( &refCount ) == 0 && OnFinalRelease( ) ) {
			destructor( 1 );
		}
	}
};

struct SpreadSeeds_t {
	float a;
	float b;
	float c;

	SpreadSeeds_t( ) {
		a = 0.f;
		b = 0.f;
		c = 0.f;
	}
};

class Weapon : public Entity {
public:
	using ref_vec_t = CUtlVector< IRefCounted* >;

	__forceinline bool& m_bInReload( ) {
		static auto offset = g_netvars.FindInDataMap( GetPredDescMap( ), XOR( "m_bInReload" ) );
		return get< bool >( offset );
	}

	// netvars / etc.
	__forceinline ref_vec_t& m_CustomMaterials( ) {
		return get< ref_vec_t >( g_entoffsets.m_CustomMaterials );
	}

	__forceinline ref_vec_t& m_CustomMaterials2( ) {
		return get< ref_vec_t >( g_entoffsets.m_CustomMaterials2 );
	}

	__forceinline ref_vec_t& m_VisualsDataProcessors( ) {
		return get< ref_vec_t >( g_entoffsets.m_VisualsDataProcessors );
	}

	__forceinline bool& m_bCustomMaterialInitialized( ) {
		return get< bool >( g_entoffsets.m_bCustomMaterialInitialized );
	}

	__forceinline int& m_iItemDefinitionIndex( ) {
		return get< int >( g_entoffsets.m_iItemDefinitionIndex );
	}

	__forceinline int& m_iClip1( ) {
		return get< int >( g_entoffsets.m_iClip1 );
	}

	__forceinline int& m_iPrimaryReserveAmmoCount( ) {
		return get< int >( g_entoffsets.m_iPrimaryReserveAmmoCount );
	}

	__forceinline int& m_Activity( ) {
		return get< int >( g_entoffsets.m_Activity );
	}

	__forceinline float& m_fFireDuration( ) {
		return get< float >( g_entoffsets.m_fFireDuration );
	}

	__forceinline float& m_fAccuracyPenalty( ) {
		return get< float >( g_entoffsets.m_fAccuracyPenalty );
	}

	__forceinline int& m_iBurstShotsRemaining( ) {
		return get< int >( g_entoffsets.m_iBurstShotsRemaining );
	}

	__forceinline float& m_flNextPrimaryAttack( ) {
		return get< float >( g_entoffsets.m_flNextPrimaryAttack );
	}

	__forceinline float& m_flNextSecondaryAttack( ) {
		return get< float >( g_entoffsets.m_flNextSecondaryAttack );
	}

	__forceinline float& m_flThrowStrength( ) {
		return get< float >( g_entoffsets.m_flThrowStrength );
	}

	__forceinline float& m_fNextBurstShot( ) {
		return get< float >( g_entoffsets.m_fNextBurstShot );
	}

	__forceinline int& m_zoomLevel( ) {
		return get< int >( g_entoffsets.m_zoomLevel );
	}

	__forceinline float& m_flRecoilIndex( ) {
		return get< float >( g_entoffsets.m_flRecoilIndex );
	}

	__forceinline int& m_weaponMode( ) {
		return get< int >( g_entoffsets.m_weaponMode );
	}

	__forceinline int& m_nFallbackPaintKit( ) {
		return get< int >( g_entoffsets.m_nFallbackPaintKit );
	}

	__forceinline int& m_nFallbackStatTrak( ) {
		return get< int >( g_entoffsets.m_nFallbackStatTrak );
	}

	__forceinline int& m_nFallbackSeed( ) {
		return get< int >( g_entoffsets.m_nFallbackSeed );
	}

	__forceinline float& m_flFallbackWear( ) {
		return get< float >( g_entoffsets.m_flFallbackWear );
	}

	__forceinline int& m_iViewModelIndex( ) {
		return get< int >( g_entoffsets.m_iViewModelIndex );
	}

	__forceinline int& m_iWorldModelIndex( ) {
		return get< int >( g_entoffsets.m_iWorldModelIndex );
	}

	__forceinline int& m_iAccountID( ) {
		return get< int >( g_entoffsets.m_iAccountID );
	}

	__forceinline int& m_iItemIDHigh( ) {
		return get< int >( g_entoffsets.m_iItemIDHigh );
	}

	__forceinline int& m_iEntityQuality( ) {
		return get< int >( g_entoffsets.m_iEntityQuality );
	}

	__forceinline int& m_OriginalOwnerXuidLow( ) {
		return get< int >( g_entoffsets.m_OriginalOwnerXuidLow );
	}

	__forceinline int& m_OriginalOwnerXuidHigh( ) {
		return get< int >( g_entoffsets.m_OriginalOwnerXuidHigh );
	}

	__forceinline bool& m_bPinPulled( ) {
		return get< bool >( g_entoffsets.m_bPinPulled );
	}

	__forceinline float& m_fThrowTime( ) {
		return get< float >( g_entoffsets.m_fThrowTime );
	}

	__forceinline EHANDLE& m_hWeapon( ) {
		return get< EHANDLE >( g_entoffsets.m_hWeapon );
	}

	__forceinline EHANDLE& m_hWeaponWorldModel( ) {
		return get< EHANDLE >( g_entoffsets.m_hWeaponWorldModel );
	}

	__forceinline EHANDLE& m_hOwnerEntity( ) {
		return get< EHANDLE >( g_entoffsets.m_hOwnerEntity );
	}

	__forceinline float& m_flConstraintRadius( ) {
		return get< float >( g_entoffsets.m_flConstraintRadius );
	}

	__forceinline float& m_fLastShotTime( ) {
		return get< float >( g_entoffsets.m_fLastShotTime );
	}

public:
	enum indices : size_t {
		SETMODELINDEX = 75,
		GETMAXCLIP1 = 367,
		GETSPREAD = 439,
		GETWPNDATA = 446, // C_WeaponCSBaseGun::GetCSWpnData
		GETINACCURACY = 469,
		UPDATEACCURACYPENALTY = 471,
	};
public:
	__forceinline CSWeaponType GetWeaponType( ) {
		if ( !this )
			return CSWeaponType::WEAPONTYPE_UNKNOWN;

		WeaponInfo* weaponInfo = GetWpnData( );
		if ( !weaponInfo )
			return CSWeaponType::WEAPONTYPE_UNKNOWN;

		return weaponInfo->m_weapon_type;
	}

	const char* GetWeaponPrefix( )
	{
		if ( !this )
			return "";

		int nWeaponType = ( int )GetWeaponType( ); // knife
		int nWeaponID = m_iItemDefinitionIndex( );

		if ( nWeaponID == MAG7 )
		{
			nWeaponType = WEAPONTYPE_RIFLE;
		}
		else if ( nWeaponID == ZEUS )
		{
			nWeaponType = WEAPONTYPE_PISTOL;
		}

		if ( nWeaponType == WEAPONTYPE_STACKABLEITEM )
		{
			nWeaponType = WEAPONTYPE_GRENADE; // redirect healthshot, adrenaline, etc to the grenade archetype
		}

		return g_szWeaponPrefixLookupTable[ std::clamp( nWeaponType, 0, 9 ) ];
	}

	// virtuals.
	__forceinline int GetMaxClip1( ) {
		return util::get_method< int( __thiscall* )( void* ) >( this, GETMAXCLIP1 )( this );
	}

	__forceinline void SetGloveModelIndex( int index ) {
		return util::get_method< void( __thiscall* )( void*, int ) >( this, SETMODELINDEX )( this, index );
	}

	__forceinline WeaponInfo* GetWpnData( ) {
		return util::get_method< WeaponInfo * ( __thiscall* )( void* ) >( this, GETWPNDATA )( this );
	}

	__forceinline float GetInaccuracy( ) {
		return util::get_method< float( __thiscall* )( void* ) >( this, GETINACCURACY )( this );
	}

	__forceinline float GetSpread( ) {
		return util::get_method< float( __thiscall* )( void* ) >( this, GETSPREAD )( this );
	}

	__forceinline void UpdateAccuracyPenalty( ) {
		return util::get_method< void( __thiscall* )( void* ) >( this, UPDATEACCURACYPENALTY )( this );
	}

	// misc funcs.
	__forceinline Weapon* GetWeapon( ) {
		return g_csgo.m_entlist->GetClientEntityFromHandle< Weapon* >( m_hWeapon( ) );
	}

	__forceinline Weapon* GetWeaponWorldModel( ) {
		return g_csgo.m_entlist->GetClientEntityFromHandle< Weapon* >( m_hWeaponWorldModel( ) );
	}

	__forceinline bool IsKnife( ) {
		return ( GetWpnData( )->m_weapon_type == WEAPONTYPE_KNIFE && m_iItemDefinitionIndex( ) != ZEUS );
	}

	// --- skateboard2018 additions ---

	__forceinline int& m_nSmokeEffectTickBegin( ) {
		return get< int >( g_entoffsets.m_nSmokeEffectTickBegin );
	}

	__forceinline bool IsGun( ) {
		if ( !this )
			return false;

		int id = m_iItemDefinitionIndex( );

		if ( !id )
			return false;

		if ( IsKnife( ) || IsGrenade( ) )
			return false;

		return true;
	}

	__forceinline bool isShotgun( ) {
		int id = m_iItemDefinitionIndex( );
		return id == XM1014 || id == NOVA || id == SAWEDOFF || id == MAG7;
	}

	__forceinline bool DTable( ) {
		int id = m_iItemDefinitionIndex( );
		return IsGun( ) && !isShotgun( ) && id != SSG08 && id != AWP && id != REVOLVER && id != ZEUS;
	}

	__forceinline bool IsGrenade( ) {
		return m_iItemDefinitionIndex( ) >= Weapons_t::FLASHBANG && m_iItemDefinitionIndex( ) <= Weapons_t::FIREBOMB;
	}

	__forceinline bool IsZoomable( bool extra_check = true ) {
		return m_iItemDefinitionIndex( ) == Weapons_t::SSG08
			|| m_iItemDefinitionIndex( ) == Weapons_t::SCAR20
			|| m_iItemDefinitionIndex( ) == Weapons_t::AWP
			|| m_iItemDefinitionIndex( ) == Weapons_t::G3SG1
			|| ( extra_check && ( m_iItemDefinitionIndex( ) == Weapons_t::SG553 || m_iItemDefinitionIndex( ) == Weapons_t::AUG ) );
	}

	// seed-based spread ( the rax resolver/lagcomp does not use this one,
	// but the existing skateboard2018 aimbot hitchance does ).
	__forceinline vec3_t CalculateSpread( int seed, float inaccuracy, float spread, bool revolver2 = false ) {
		WeaponInfo* wep_info;
		int         item_def_index;
		float       recoil_index, r1, r2, r3, r4, s1, c1, s2, c2;

		// if we have no bullets, we have no spread.
		wep_info = GetWpnData( );

		if ( !wep_info || !wep_info->m_bullets )
			return { };

		// get some data for later.
		item_def_index = m_iItemDefinitionIndex( );
		recoil_index   = m_flRecoilIndex( );

		// seed randomseed.
		g_csgo.RandomSeed( ( seed & 0xff ) + 1 );

		// generate needed floats.
		r1 = g_csgo.RandomFloat( 0.f, 1.f );
		r2 = g_csgo.RandomFloat( 0.f, math::pi_2 );

		if ( g_csgo.weapon_accuracy_shotgun_spread_patterns->GetInt( ) > 0 )
			g_csgo.GetShotgunSpread( item_def_index, 0, 0 + wep_info->m_bullets * recoil_index, &r4, &r3 );
		else {
			r3 = g_csgo.RandomFloat( 0.f, 1.f );
			r4 = g_csgo.RandomFloat( 0.f, math::pi_2 );
		}

		// revolver secondary spread.
		if ( item_def_index == REVOLVER && revolver2 ) {
			r1 = 1.f - ( r1 * r1 );
			r3 = 1.f - ( r3 * r3 );
		}

		// negev spread.
		else if ( item_def_index == NEGEV && recoil_index < 3.f ) {
			for ( int i{ 3 }; i > recoil_index; --i ) {
				r1 *= r1;
				r3 *= r3;
			}

			r1 = 1.f - r1;
			r3 = 1.f - r3;
		}

		// get needed sine / cosine values.
		c1 = std::cos( r2 );
		c2 = std::cos( r4 );
		s1 = std::sin( r2 );
		s2 = std::sin( r4 );

		// calculate spread vector.
		return {
			( c1 * ( r1 * inaccuracy ) ) + ( c2 * ( r3 * spread ) ),
			( s1 * ( r1 * inaccuracy ) ) + ( s2 * ( r3 * spread ) ),
			0.f
		};
	}

	__forceinline vec3_t CalculateSpread( int seed, bool revolver2 = false ) {
		return CalculateSpread( seed, GetInaccuracy( ), GetSpread( ), revolver2 );
	}

	__forceinline vec3_t CalculateSpread( const SpreadSeeds_t* seed, float inaccuracy, float spread, bool revolver2 = false ) {
		float rand_inaccuracy = seed->b;
		float rand_spread = seed->c;

		float disp_x = cos( seed->a );
		float disp_y = sin( seed->a );

		if ( m_iItemDefinitionIndex( ) == NEGEV && m_flRecoilIndex( ) < 3.f ) { /*NEGEV WILD BEAST*/
			for ( int j = 3; j > ( int )m_flRecoilIndex( ); --j ) {
				rand_inaccuracy *= rand_inaccuracy;
			}

			rand_inaccuracy = 1.f - rand_inaccuracy;
		}

		const float seed_inaccuracy = inaccuracy * rand_inaccuracy;
		const float seed_spread = spread * rand_spread;

		const float inaccuracy_disp_x = disp_x * seed_inaccuracy;
		const float inaccuracy_disp_y = disp_y * seed_inaccuracy;

		const float spread_disp_x = disp_x * seed_spread;
		const float spread_disp_y = disp_y * seed_spread;

		float totalDisplacementX = inaccuracy_disp_x + spread_disp_x;
		float totalDisplacementY = inaccuracy_disp_y + spread_disp_y;

		return { totalDisplacementX, totalDisplacementY, 0 };
	}

	__forceinline std::string GetLocalizedName( ) {
		C_EconItemView* item_view;
		CEconItemDefinition* item_def;

		item_view = g_csgo.GetEconItemView( this );
		if ( !item_view )
			return XOR( "error" );

		item_def = g_csgo.GetStaticData( item_view );
		if ( !item_def )
			return XOR( "error" );

		return util::WideToMultiByte( g_csgo.m_localize->Find( item_def->GetItemBaseName( ) ) );
	}
};

class CTraceFilterSimple_game {
public:
	void* m_vmt;
	const Entity* m_pass_ent1;
	int             m_collision_group;
	ShouldHitFunc_t m_shouldhit_check_fn;

public:
	__forceinline CTraceFilterSimple_game( ) :
		m_vmt{ g_csgo.CTraceFilterSimple_vmt.as< void* >( ) },
		m_pass_ent1{},
		m_collision_group{},
		m_shouldhit_check_fn{} {
	}

	__forceinline CTraceFilterSimple_game( const Entity* pass_ent1, int collision_group = COLLISION_GROUP_NONE, ShouldHitFunc_t shouldhit_check_fn = nullptr ) :
		m_vmt{ g_csgo.CTraceFilterSimple_vmt.as< void* >( ) },
		m_pass_ent1{ pass_ent1 },
		m_collision_group{ collision_group },
		m_shouldhit_check_fn{ shouldhit_check_fn } {
	}

	__forceinline bool ShouldHitEntity( Entity* entity, int contents_mask ) {
		// note - dex; game is dumb, this gets the real vmt.
		void* real_vmt = *( void** )m_vmt;

		return util::get_method< bool( __thiscall* )( void*, Entity*, int ) >( real_vmt, 0 )( real_vmt, entity, contents_mask );
	}

	// note - dex; don't really care about calling the virtuals for these two functions, they only set members in the class for us.
	__forceinline void SetPassEntity( Entity* pass_ent1 ) {
		m_pass_ent1 = pass_ent1;

		// util::get_method< void (__thiscall *)( void *, Entity* ) >( m_vmt, 2 )( m_vmt, pass_ent1 );
	}

	__forceinline void SetCollisionGroup( int collision_group ) {
		m_collision_group = collision_group;

		// util::get_method< void (__thiscall *)( void *, int ) >( m_vmt, 3 )( m_vmt, collision_group );
	}
};

class CTraceFilterSkipTwoEntities_game {
public:
	void* m_vmt;
	const Entity* m_pass_ent1;
	int             m_collision_group;
	ShouldHitFunc_t m_shouldhit_check_fn;
	const Entity* m_pass_ent2;

public:
	__forceinline CTraceFilterSkipTwoEntities_game( ) :
		m_vmt{ g_csgo.CTraceFilterSkipTwoEntities_vmt.as< void* >( ) },
		m_pass_ent1{},
		m_collision_group{},
		m_shouldhit_check_fn{},
		m_pass_ent2{} {
	}

	__forceinline CTraceFilterSkipTwoEntities_game( const Entity* pass_ent1, const Entity* pass_ent2, int collision_group = COLLISION_GROUP_NONE, ShouldHitFunc_t shouldhit_check_fn = nullptr ) :
		m_vmt{ g_csgo.CTraceFilterSimple_vmt.as< void* >( ) },
		m_pass_ent1{ pass_ent1 },
		m_collision_group{ collision_group },
		m_shouldhit_check_fn{ shouldhit_check_fn },
		m_pass_ent2{ pass_ent2 } {
	}

	__forceinline bool ShouldHitEntity( Entity* entity, int contents_mask ) {
		// note - dex; game is dumb, this gets the real vmt.
		void* real_vmt = *( void** )m_vmt;

		return util::get_method< bool( __thiscall* )( void*, Entity*, int ) >( m_vmt, 0 )( m_vmt, entity, contents_mask );
	}

	// note - dex; don't really care about calling the virtuals for these two functions, they only set members in the class for us.
	__forceinline void SetPassEntity( Entity* pass_ent1 ) {
		m_pass_ent1 = pass_ent1;

		// util::get_method< void (__thiscall *)( void *, Entity* ) >( m_vmt, 2 )( m_vmt, pass_ent1 );
	}

	__forceinline void SetCollisionGroup( int collision_group ) {
		m_collision_group = collision_group;

		// util::get_method< void (__thiscall *)( void *, int ) >( m_vmt, 3 )( m_vmt, collision_group );
	}

	__forceinline void SetPassEntity2( Entity* pass_ent2 ) {
		m_pass_ent2 = pass_ent2;
		// util::get_method< void (__thiscall *)( void *, Entity* ) >( m_vmt, 4 )( m_vmt, pass_ent2 );
	}
};