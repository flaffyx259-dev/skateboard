#pragma once

class AimPlayer;

// rax lag compensation / record prediction.
class LagCompensation {
public:
	enum LagType : size_t {
		INVALID = 0,
		CONSTANT,
		ADAPTIVE,
		RANDOM,
	};

private:
	// animfix backup stuff.
	float  m_curtime;
	float  m_frametime;
	vec3_t m_origin;
	vec3_t m_abs_origin;
	vec3_t m_velocity;
	vec3_t m_abs_velocity;
	float  m_duck_amount;
	float  m_duck_speed;
	int    m_flags;
	C_AnimationLayer m_layers[ 13 ];
	float            m_poses[ PoseParam::POSE_COUNT ];
	CCSGOPlayerAnimState m_state;
public:
	void PlayerMove( LagRecord* record );
	LagRecord* StartPrediction( AimPlayer* data );
	void UpdatePredictedAnimations( AimPlayer* data, LagRecord* predicted, LagRecord* previous, int update_ticks );
};

extern LagCompensation g_lagcomp;
