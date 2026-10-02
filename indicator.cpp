#include "includes.h"
#include "indicators.h"

hud_indicators g_indicators{ };

// Hardcoded too much
// indicators
void hud_indicators::Indicators()
{
	if (!g_csgo.m_engine->IsInGame())
		return;

	if (!g_menu.main.visuals.indicator.get())
		return;

	// get active weapon.
	Weapon* weapon = g_cl.m_local->GetActiveWeapon();
	if (!weapon)
		return;

	WeaponInfo* data = weapon->GetWpnData();
	if (!data)
		return;

	//do not do this on: bomb, knife and nades.
	int type = weapon->m_iItemDefinitionIndex();
	if (type == WEAPONTYPE_KNIFE || type == WEAPONTYPE_C4 || type == WEAPONTYPE_GRENADE)
		return;



	g_csgo.m_engine->GetScreenSize(m_width, m_height);
	Color color = g_gui.m_color;
	float spreadDist = ((weapon->GetInaccuracy() + weapon->GetSpread()) * 320.f) / std::tan(math::deg_to_rad(g_cl.m_local->GetFOV()) * 0.5f);
	float spreadRadius = (spreadDist * (m_height / 480.f)) * 50 / 250.f;

	auto local_player = g_cl.m_local;
	int screen_width, screen_height;
	g_csgo.m_engine->GetScreenSize(screen_width, screen_height);

	static float next_lby_update[65];
	//static float last_lby[65];

	const float curtime = g_csgo.m_globals->m_curtime;

	//if (local_player->GetVelocity().Length2D() > 0.1 && !global::is_fakewalking)
	//    return;

	CCSGOPlayerAnimState* state = g_cl.m_local->m_PlayerAnimState();
	if (!state)
		return;
	static float last_lby[65];
	if (last_lby[local_player->index()] != local_player->m_flLowerBodyYawTarget())
	{
		last_lby[local_player->index()] = local_player->m_flLowerBodyYawTarget();
		next_lby_update[local_player->index()] = curtime + 1.125f + g_csgo.m_globals->m_interval;
	}

	if (next_lby_update[local_player->index()] < curtime)
	{
		next_lby_update[local_player->index()] = curtime + 1.125f;
	}

	float time_remain_to_update = next_lby_update[local_player->index()] - local_player->m_flSimulationTime();
	float time_update = next_lby_update[local_player->index()];


	float fill = 0;
	fill = (((time_remain_to_update)));
	static float add = 0.000f;
	add = 1.125f - fill;

	// main window
	// alien calculations hehe
	int	x{ 8 };

	printf("nigga");

	//render::rect_filled(x + 10, m_height / 2 + 10 - 100, 190, 55, { 10, 10, 10, 230 });
	// overlay black.
	render::rect_filled(x + 10, m_height / 2 + 8 - 100, 190, 15 + 2, { 10, 10, 10, 255 });
	// line.
	render::rect_filled(x + 8, m_height / 2 + 8 - 100, 190, 2, { color.r(), color.g(), color.b(), 255 });
	// black outline.
	render::rect(x + 10 - 1, m_height / 2 + 9 - 100, 192, 55 + 1, { 10, 10, 10, 255});
	// text.
	render::indicators.string(x + 10 + 100, m_height / 2 + 13 - 100, { 255, 255, 255, 255 }, "indicators", render::ALIGN_CENTER);
	render::indicators.string(x + 10 + 6, m_height / 2 + 30 - 100, { 255, 255, 255, 255 }, "lby", render::ALIGN_LEFT);
	render::indicators.string(x + 10 + 6, m_height / 2 + 40 - 100, { 255, 255, 255, 255 }, "tick-base", render::ALIGN_LEFT);
	render::indicators.string(x + 10 + 6, m_height / 2 + 50 - 100, { 255, 255, 255, 255 }, "inaccuracy", render::ALIGN_LEFT);

	//inaccuracy
	render::rect_filled(x + 10 + 195 - 130, m_height / 2 + 52 - 100, 120, 7, { 10, 10, 10, 210 });
	render::gradient1337(x + 10 + 195 - 130, m_height / 2 + 52 - 100, spreadRadius / 2.2, 7, { 10, 10, 10, 255 }, { color.r(), color.g(), color.b(), 255 });

	//exploits
	if (g_aimbot.m_double_tap) {
		render::rect_filled(x + 10 + 195 - 130, m_height / 2 + 42 - 100, 120, 7, { 10, 10, 10, 210 });
		render::gradient1337(x + 10 + 195 - 130, m_height / 2 + 42 - 100, 120, 7, { 10, 10, 10, 255 }, { color.r(), color.g(), color.b(), 255 });
	}
	else {
		render::rect_filled(x + 10 + 195 - 130, m_height / 2 + 42 - 100, 120, 7, { 10, 10, 10, 210 });
	}

	//lby
	render::rect_filled(x + 10 + 195 - 130, m_height / 2 + 32 - 100, 120, 7, { 10, 10, 10, 210 });
	render::gradient1337(x + 10 + 195 - 130, m_height / 2 + 32 - 100, add * 107, 7, { 10, 10, 10, 255 }, { color.r(), color.g(), color.b(), 255 });



}