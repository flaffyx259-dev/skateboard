#pragma once

class ShotRecord;

struct OverrideData
{
	float m_yaw;
	vec3_t m_start, m_end;
	Player* m_player;
};

// rax resolver.
class Resolver {
public:
	bool          m_override, m_override_update;
	ang_t         m_override_angle;

	OverrideData  m_override_data;
public:
	// record selection helpers (kept from the old skateboard2018 resolver so
	// the existing aimbot / chams / visuals / knifebot call sites keep working,
	// now driven by the rax resolve modes ).
	LagRecord* FindIdealRecord(AimPlayer* data);
	LagRecord* FindLastRecord(AimPlayer* data);
	void OnBodyUpdate(Player* player, float value);

	float AutoDirection(Player* player, std::vector<AdaptiveAngle> angles = {});
	float FindBestYaw(LagRecord* record, MoveData_t* move_data);

	void HandlePredUpdate(AimPlayer* data, LagRecord* current, LagRecord* previous);
	void MatchShot(AimPlayer* data, LagRecord* current, LagRecord* previous);
	void HandleModes(AimPlayer* data, LagRecord* current, LagRecord* previous);
	void ResolveAngles(AimPlayer* data, LagRecord* current, LagRecord* previous);
	void ResolveMove(AimPlayer* data, LagRecord* current, LagRecord* previous);
	void ResolveStand(AimPlayer* data, LagRecord* current, LagRecord* previous);
	void ResolveAir(AimPlayer* data, LagRecord* current, LagRecord* previous);

	// randomizes the fake lean / body yaw pose params while the enemy is airborne
	// so the rendered model does not leak our resolved angles.
	void ResolvePoses(Player* player, LagRecord* record);

	void Override();
};

extern Resolver g_resolver;
