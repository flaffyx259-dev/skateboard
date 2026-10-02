#include "includes.h"
#include "crash.h"

Resolver g_resolver{};

namespace {
	// translate a resolve mode to something readable in the crash log.
	const char* ModeName( int mode ) {
		switch ( mode ) {
		case RESOLVE_NONE: return "NONE";
		case RESOLVE_MOVE: return "MOVE";
		case RESOLVE_STAND: return "STAND";
		case RESOLVE_STAND_BALANCE: return "STAND_BALANCE";
		case RESOLVE_STAND_BALANCE2: return "STAND_BALANCE2";
		case RESOLVE_STAND_NO_DATA: return "STAND_NO_DATA";
		case RESOLVE_DISTORTION: return "DISTORTION";
		case RESOLVE_NOUPDATE: return "NOUPDATE";
		case RESOLVE_PREUPDATE: return "PREUPDATE";
		case RESOLVE_UPDATE: return "UPDATE";
		case RESOLVE_UPDATE_PRED: return "UPDATE_PRED";
		case RESOLVE_AIR: return "AIR";
		case RESOLVE_AIR_UPDATE: return "AIR_UPDATE";
		case RESOLVE_OVERRIDE: return "OVERRIDE";
		case RESOLVE_PREDICTED: return "PREDICTED";
		default: return "?";
		}
	}
}

LagRecord* Resolver::FindIdealRecord( AimPlayer* data ) {
	LagRecord* first_valid, * current;

	if ( data->m_records.empty( ) )
		return nullptr;

	first_valid = nullptr;

	// iterate records.
	for ( const auto& it : data->m_records ) {
		if ( it->dormant( ) || it->immune( ) || !it->valid( ) )
			continue;

		// get current record.
		current = it.get( );

		// first record that was valid, store it for later.
		if ( !first_valid )
			first_valid = current;

		// try to find a record with a shot, lby update, moving or no anti-aim.
		if ( it->m_shot ||
			 it->m_resolver_mode == RESOLVE_PREUPDATE ||
			 it->m_resolver_mode == RESOLVE_MOVE ||
			 it->m_resolver_mode == RESOLVE_NONE )
			return current;
	}

	// none found above, return the first valid record if possible.
	return ( first_valid ) ? first_valid : nullptr;
}

LagRecord* Resolver::FindLastRecord( AimPlayer* data ) {
	LagRecord* current;

	if ( data->m_records.empty( ) )
		return nullptr;

	// iterate records in reverse.
	for ( auto it = data->m_records.crbegin( ); it != data->m_records.crend( ); ++it ) {
		current = it->get( );

		// if this record is valid.
		// we are done since we iterated in reverse.
		if ( current->valid( ) && !current->immune( ) && !current->dormant( ) )
			return current;
	}

	return nullptr;
}

void Resolver::OnBodyUpdate( Player* player, float value ) {
	AimPlayer* data = &g_aimbot.m_players[ player->index( ) - 1 ];

	// set data.
	data->m_old_body = data->m_body;
	data->m_body = value;
}

float Resolver::AutoDirection( Player* player, std::vector< AdaptiveAngle > angles ) {
	// constants.
	constexpr float STEP{ 4.f };
	constexpr float RANGE{ 32.f };

	vec3_t start = g_cl.m_local->GetEyePos( true );
	vec3_t enemy_pos = player->GetEyePos( true );

	ang_t to_enemy;
	math::VectorAngles( enemy_pos - start, to_enemy );

	// construct vector of angles to test.
	if ( angles.empty( ) ) {
		angles.emplace_back( to_enemy.y );
		angles.emplace_back( to_enemy.y + 90.f );
		angles.emplace_back( to_enemy.y - 90.f );
	}

	bool valid{ false };

	// iterate vector of angles.
	for ( auto it = angles.begin( ); it != angles.end( ); ++it ) {

		// compute the 'rough' estimation of where our head will be.
		vec3_t end{ enemy_pos.x + std::cos( math::deg_to_rad( it->m_yaw ) ) * RANGE,
			enemy_pos.y + std::sin( math::deg_to_rad( it->m_yaw ) ) * RANGE,
			enemy_pos.z };

		// compute the direction.
		vec3_t dir = end - start;
		float len = dir.normalize( );

		// should never happen.
		if ( len <= 0.f )
			continue;

		// step thru the total distance, 4 units per step.
		for ( float i{ 0.f }; i < len; i += STEP ) {
			// get the current step position.
			vec3_t point = start + ( dir * i );

			// get the contents at this point.
			int contents = g_csgo.m_engine_trace->GetPointContents( point, MASK_SHOT_HULL );

			// contains nothing that can stop a bullet.
			if ( !( contents & MASK_SHOT_HULL ) )
				continue;

			float mult = 1.f;

			// over 50% of the total length, prioritize this shit.
			if ( i > ( len * 0.5f ) )
				mult = 1.25f;

			// over 75% of the total length, prioritize this shit.
			if ( i > ( len * 0.75f ) )
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if ( i > ( len * 0.9f ) )
				mult = 2.f;

			// append 'penetrated distance'.
			it->m_dist += ( STEP * mult );

			// mark that we found anything.
			valid = true;
		}
	}

	if ( !valid )
		return math::NormalizedAngle( to_enemy.y );

	// put the most distance at the front of the container.
	std::sort( angles.begin( ), angles.end( ),
		[]( const AdaptiveAngle& a, const AdaptiveAngle& b ) {
			return a.m_dist > b.m_dist;
		} );

	// the best angle should be at the front now.
	AdaptiveAngle* best = &angles.front( );
	return math::NormalizedAngle( best->m_yaw );
}

float Resolver::FindBestYaw( LagRecord* record, MoveData_t* move_data ) {
	float assumed_yaw = math::NormalizedAngle( record->m_body - move_data->m_body_delta );
	float assumed_yaw_inverted = math::NormalizedAngle( record->m_body + move_data->m_body_delta );

	ang_t away;
	math::VectorAngles( record->m_abs_origin - g_cl.m_local->m_vecAbsOrigin( ), away );

	float delta = fabs( math::NormalizedAngle( away.y - assumed_yaw ) );
	float delta_inverted = fabs( math::NormalizedAngle( away.y - assumed_yaw_inverted ) );

	if ( delta <= delta_inverted )
		return assumed_yaw;

	return assumed_yaw_inverted;
}

void Resolver::MatchShot( AimPlayer* data, LagRecord* current, LagRecord* previous ) {
	Weapon* weapon = data->m_player->GetActiveWeapon( );

	if ( !weapon )
		return;

	const int& shot_tick = game::TIME_TO_TICKS( weapon->m_fLastShotTime( ) );
	const int& sim_tick  = game::TIME_TO_TICKS( current->m_sim_time );
	const int& anim_tick = game::TIME_TO_TICKS( current->m_anim_time );

	if ( shot_tick > sim_tick || shot_tick < anim_tick )
		return;

	if ( shot_tick == anim_tick )
		return;

	current->m_eye_angles.x = previous->m_eye_angles.x;
}

void Resolver::HandleModes( AimPlayer* data, LagRecord* current, LagRecord* previous ) {
	if ( previous ? ( previous->m_anim_flags & FL_ONGROUND ) : ( current->m_anim_flags & FL_ONGROUND ) ) {
		if ( data->m_anim_velocity.length_2d( ) > 0.1f ) {
			data->m_move_data.Store( current );
			data->m_update_data.OnMove( current->m_anim_time );

			data->m_misses[ MISS_STAND_NO_DATA ] = 0;
			data->m_misses[ MISS_UPDATE_PRED ] = 0;
			data->m_misses[ MISS_NOUPDATE ] = 0;

			current->m_resolver_mode = RESOLVE_MOVE;
			return;
		}

		if ( previous ) {
			if ( current->m_body != previous->m_body ) {
				data->m_update_data.m_should_have_updated = true;
				data->m_update_data.m_next_body_update = current->m_anim_time + 1.1f;

				if ( data->m_move_data.m_updates == 0 )
					data->m_move_data.m_body_delta = math::NormalizedAngle( current->m_body - previous->m_body );

				++data->m_move_data.m_updates;
				current->m_resolver_mode = RESOLVE_UPDATE;
				return;
			}

			if ( current->m_anim_time > data->m_update_data.m_next_body_update ) {
				data->m_update_data.m_should_have_updated = true;
				data->m_update_data.m_next_body_update = current->m_anim_time + 1.1f;
				HandlePredUpdate( data, current, previous );

				if ( data->m_update_data.m_foot_side != 0 ) {
					current->m_resolver_mode = RESOLVE_UPDATE_PRED;
					return;
				}
			}
		}

		current->m_resolver_mode = RESOLVE_STAND;
		return;
	}

	data->m_update_data.m_distortion = false;
	current->m_resolver_mode = RESOLVE_AIR;
}

void Resolver::ResolveAngles( AimPlayer* data, LagRecord* current, LagRecord* previous ) {
	if ( !g_cl.m_local || !g_cl.m_local->alive( ) )
		return;

	// the anim state is what every branch below reasons about ( abs_yaw,
	// move_yaw, duck amount, walk/run transition ). without it we can only
	// fall back to the raw body yaw.
	if ( !data->m_state )
		data->m_state = data->m_player->m_PlayerAnimState( );

	CrashLog::Trace( "resolve: ent=%d rec=%p prev=%p state=%p maxspd=%.1f animvel=%.2f",
		data->m_player ? data->m_player->index( ) : -1,
		current, previous, data->m_state, data->m_max_speed,
		data->m_anim_velocity.length( ) );

	HandleModes( data, current, previous );

	if ( !data->m_state ) {
		current->m_resolver_mode = RESOLVE_NONE;
		return;
	}

	if ( previous && current->m_choke <= 1 && previous->m_choke <= 1 ) {
		current->m_resolver_mode = RESOLVE_NONE;
		return;
	}

	MatchShot( data, current, previous );

	switch ( current->m_resolver_mode ) {
	case RESOLVE_MOVE:
		ResolveMove( data, current, previous );
		data->m_update_data.m_should_have_updated = false;
		break;
	case RESOLVE_STAND:
		ResolveStand( data, current, previous );
		data->m_update_data.m_should_have_updated = false;
		break;
	case RESOLVE_UPDATE:
	case RESOLVE_UPDATE_PRED:
		current->m_eye_angles.y = current->m_body;
		break;
	case RESOLVE_AIR:
		ResolveAir( data, current, previous );
		data->m_update_data.m_should_have_updated = false;
		break;
	}

	current->m_eye_angles.normalize( );
	data->m_player->m_angEyeAngles( ) = current->m_eye_angles;

	CrashLog::Trace( "resolve: -> mode=%s yaw=%.2f body=%.2f chance=%d",
		ModeName( current->m_resolver_mode ), current->m_eye_angles.y,
		current->m_body, current->ResolveChance( ) );
}

void Resolver::ResolveMove( AimPlayer* data, LagRecord* current, LagRecord* previous ) {
	current->m_eye_angles.y = current->m_body;

	if ( !previous )
		return;

	if ( previous->m_layers[ 6 ].m_weight > 0.0f ) {
		// check if our velocity is completly accurate.
		if ( current->m_layers[ 11 ].m_weight <= 0.55f || current->m_layers[ 11 ].m_weight >= 0.9f )
			return;

		// make sure our layer isnt lerped.
		if ( current->m_layers[ 6 ].m_weight <= previous->m_layers[ 6 ].m_weight )
			return;

		const float m_flSpeedAsPortionOfWalkTopSpeed = data->m_anim_velocity.length_2d( ) / ( data->m_max_speed * 0.52f );
		const float m_flSpeedAsPortionOfCrouchTopSpeed = data->m_anim_velocity.length_2d( ) / ( data->m_max_speed * 0.34f );
		const float m_flAnimDuckAmount = std::clamp( math::Approach( std::clamp( data->m_player->m_flDuckAmount( ) + data->m_state->m_duck_additional, 0.f, 1.f ), data->m_state->m_anim_duck_amount, game::TICKS_TO_TIME( current->m_choke ) * 6.0f ), 0.f, 1.f );
		const float m_flTargetMoveWeight = math::Lerp( m_flAnimDuckAmount, std::clamp( m_flSpeedAsPortionOfWalkTopSpeed, 0.f, 1.f ), std::clamp( m_flSpeedAsPortionOfCrouchTopSpeed, 0.f, 1.f ) );

		if ( m_flTargetMoveWeight <= 0.0f )
			return;

		std::vector< std::pair< float, float > > footYaws;

		footYaws.emplace_back( data->m_state->m_abs_yaw, 0.f );
		footYaws.emplace_back( current->m_body + 58.f, 0.f );
		footYaws.emplace_back( current->m_body - 58.f, 0.f );

		// narrow the available aim matrix width as speed increases
		float flAimMatrixWidthRange = math::Lerp( std::clamp( m_flSpeedAsPortionOfWalkTopSpeed, 0.f, 1.f ), 1.0f, math::Lerp( data->m_state->m_walk_to_run_transition_state, 0.8f, 0.5f ) );

		if ( m_flAnimDuckAmount > 0 ) {
			flAimMatrixWidthRange = math::Lerp( m_flAnimDuckAmount * std::clamp( m_flSpeedAsPortionOfCrouchTopSpeed, 0.f, 1.f ), flAimMatrixWidthRange, 0.5f );
		}

		const float flTempYawMax = 58.f * flAimMatrixWidthRange;
		const float flTempYawMin = 58.f * flAimMatrixWidthRange;

		for ( std::pair< float, float >& footData : footYaws ) {
			float m_flFootYaw = std::clamp( footData.first, -360.f, 360.f );
			const float flEyeFootDelta = math::AngleDiff( current->m_body, m_flFootYaw );

			if ( flEyeFootDelta > flTempYawMax ) {
				m_flFootYaw = current->m_body - abs( flTempYawMax );
			}
			else if ( flEyeFootDelta < flTempYawMin ) {
				m_flFootYaw = current->m_body + abs( flTempYawMin );
			}

			m_flFootYaw = math::NormalizedAngle( m_flFootYaw );

			float m_flMoveYaw = data->m_state->m_move_yaw;

			// convert horizontal velocity vec to angular yaw
			float flRawYawIdeal = ( atan2( -data->m_anim_velocity[ 1 ], -data->m_anim_velocity[ 0 ] ) * 180.f / math::pi );

			if ( flRawYawIdeal < 0 )
				flRawYawIdeal += 360;

			const float m_flMoveYawIdeal = math::NormalizedAngle( math::AngleDiff( flRawYawIdeal, m_flFootYaw ) );
			const float m_flMoveYawCurrentToIdeal = math::NormalizedAngle( math::AngleDiff( m_flMoveYawIdeal, m_flMoveYaw ) );

			if ( previous->m_layers[ ANIMATION_LAYER_MOVEMENT_STRAFECHANGE ].m_weight >= 1 ) {
				m_flMoveYaw = m_flMoveYawIdeal;
			}
			else {
				float flRatio = math::Bias( m_flTargetMoveWeight, 0.18f ) + 0.1f;

				m_flMoveYaw = math::NormalizedAngle( m_flMoveYaw + ( m_flMoveYawCurrentToIdeal * flRatio ) );
			}

			vec3_t vecMoveYawDir;
			math::AngleVectors( ang_t( 0, math::NormalizedAngle( m_flFootYaw + m_flMoveYaw + 180 ), 0 ), &vecMoveYawDir );
			const float flYawDeltaAbsDot = abs( data->m_anim_velocity.normalized( ).dot( vecMoveYawDir ) );

			float m_flMoveWeightWithAirSmooth = m_flTargetMoveWeight * math::Bias( flYawDeltaAbsDot, 0.2f ) * data->m_state->m_in_air_smooth_value;

			// dampen move weight for landings
			m_flMoveWeightWithAirSmooth *= std::max( ( 1.0f - previous->m_layers[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ].m_weight ), 0.55f );

			footData.second = m_flMoveWeightWithAirSmooth;
		}

		std::sort( footYaws.begin( ), footYaws.end( ),
			[&]( const std::pair< float, float >& a, const std::pair< float, float >& b ) {
				return fabs( current->m_layers[ 6 ].m_weight - a.second ) < fabs( current->m_layers[ 6 ].m_weight - b.second );
			} );

		data->m_state->m_abs_yaw = footYaws[ 0 ].first;
		return;
	}

	// if so, move yaw = move yaw ideal, and we can calculate foot yaw by an accuracy of ~45 degrees
	constexpr float angles[] = { 180.f, 135.f, 90.f, 45.f, 0.f, -45.f, -90.f, -135.f };

	const float& local_cycle_increment = current->m_layers[ 6 ].m_playback_rate;
	const int& move_seq = previous->m_layers[ 6 ].m_sequence;

	float move_yaw = FLT_MAX;
	float raw_yaw_ideal = atan2( -data->m_anim_velocity.y, -data->m_anim_velocity.x ) * 180.f / math::pi;

	if ( raw_yaw_ideal < 0.f )
		raw_yaw_ideal += 360.f;

	for ( int i = ANIMTAG_STARTCYCLE_N; i <= ANIMTAG_STARTCYCLE_NW; i++ ) {
		const float pred_cycle = math::ClampCycle(
			data->m_player->GetFirstSequenceAnimTag( move_seq, i, 0.f, 1.f ) + local_cycle_increment );

		if ( static_cast< int >( pred_cycle * 1000.f ) == static_cast< int >( current->m_layers[ ANIMATION_LAYER_MOVEMENT_MOVE ].m_cycle * 1000.f ) ) {
			move_yaw = angles[ i - ANIMTAG_STARTCYCLE_N ];
			break;
		}
	}

	if ( move_yaw != FLT_MAX )
		data->m_state->m_abs_yaw = math::AngleDiff( raw_yaw_ideal, move_yaw );
}

void Resolver::ResolveStand( AimPlayer* data, LagRecord* current, LagRecord* previous ) {
	if ( m_override_data.m_player == data->m_player ) {
		current->m_eye_angles.y = m_override_angle.y + m_override_data.m_yaw;
		current->m_resolver_mode = RESOLVE_OVERRIDE;
		return;
	}

	// we have no move data.
	if ( data->m_move_data.m_anim_time < 0.0f ) {
		current->m_resolver_mode = RESOLVE_STAND_NO_DATA;

		switch ( data->m_misses[ MISS_STAND_NO_DATA ] % 3 ) {
		case 0:
			current->m_eye_angles.y = AutoDirection( data->m_player );
			break;
		case 1:
			current->m_eye_angles.y = current->m_body;
			break;
		case 2:
			current->m_eye_angles.y = current->m_body + 180.f;
			break;
		}

		return;
	}

	// we have no previous records. :(
	if ( !previous ) {
		current->m_eye_angles.y = current->m_body;
		return;
	}

	// they just stopped moving.
	if ( ( current->m_anim_time - data->m_move_data.m_anim_time ) <= 0.22f ) {
		current->m_eye_angles.y = current->m_body;
		current->m_resolver_mode = RESOLVE_PREUPDATE;
		data->m_update_data.m_distortion = false;
		return;
	}

	int adjust_activity = data->m_player->GetSequenceActivity( current->m_layers[ 3 ].m_sequence );

	// they never triggered balance adjust.
	if ( adjust_activity == ACT_CSGO_IDLE_ADJUST_STOPPEDMOVING ) {
		// they did not change lby.
		// maybe they are not breaking at all.
		if ( data->m_move_data.m_updates == 0 && data->m_misses[ MISS_NOUPDATE ] <= 0 ) {
			current->m_eye_angles.y = current->m_body;
			current->m_resolver_mode = RESOLVE_NOUPDATE;
			data->m_update_data.m_distortion = false;
			return;
		}
	}

	bool prev_distortion = data->m_update_data.m_distortion;
	bool balance_adjust = false;
	int  previous_adjust_activity = data->m_player->GetSequenceActivity( previous->m_layers[ 3 ].m_sequence );

	if ( adjust_activity == ACT_CSGO_IDLE_TURN_BALANCEADJUST ) {
		// they just updated the sequence.
		if ( ( ( current->m_layers[ 3 ].m_weight == 0.0f && current->m_layers[ 3 ].m_cycle == 0.0f ) &&
			  ( previous->m_layers[ 3 ].m_weight > 0.0f && previous->m_layers[ 3 ].m_cycle > 0.0f ) ) ||
			previous_adjust_activity != ACT_CSGO_IDLE_TURN_BALANCEADJUST ) {
			balance_adjust = true;
		}

		if ( data->m_update_data.m_should_have_updated && previous_adjust_activity == ACT_CSGO_IDLE_TURN_BALANCEADJUST ) {
			if ( balance_adjust ) {
				data->m_state->m_abs_yaw = current->m_body + std::max( std::min( fabs( math::NormalizedAngle( data->m_state->m_abs_yaw - current->m_body ) ), 58.f ), 35.f );
				data->m_update_data.m_foot_side = 1;
				data->m_update_data.m_distortion = false;
			}

			if ( current->m_layers[ 3 ].m_weight > 0.0f && current->m_layers[ 3 ].m_cycle > 0.0f &&
				previous->m_layers[ 3 ].m_weight > 0.0f && previous->m_layers[ 3 ].m_cycle > 0.0f ) {
				data->m_update_data.m_distortion = true;
				data->m_update_data.m_foot_side = 0;
			}
		}

		// they are balance adjusting.
		if ( current->m_layers[ 3 ].m_weight > 0.0f )
			balance_adjust = true;
	}

	float final_body_delta;
	bool fake_last_move = fabs( math::NormalizedAngle( current->m_body - data->m_move_data.m_body ) ) <= 35.f;

	// we can use layer data to resolve.
	if ( balance_adjust ) {
		int miss_mode = MISS_STAND_BALANCE;
		current->m_resolver_mode = RESOLVE_STAND_BALANCE;

		if ( fake_last_move ) {
			if ( data->m_update_data.m_distortion ) {
				miss_mode = MISS_STAND_DISTORTION;
				current->m_resolver_mode = RESOLVE_DISTORTION;
			}
			else if ( data->m_update_data.m_foot_side == 0 ) {
				miss_mode = MISS_STAND_BALANCE2;
				current->m_resolver_mode = RESOLVE_STAND_BALANCE2;
			}
		}

		switch ( data->m_misses[ miss_mode ] % 5 ) {
		case 1:
			data->m_move_data.m_body_delta = 180.f;
			break;
		case 2:
			data->m_move_data.m_body_delta = 120.f;
			break;
		case 3:
			data->m_move_data.m_body_delta = -120.f;
			break;
		case 4:
			data->m_move_data.m_body_delta = 0.f;
			break;
		}

		if ( data->m_update_data.m_foot_side != 0 )
			data->m_move_data.m_body_delta = fabs( data->m_move_data.m_body_delta ) * -data->m_update_data.m_foot_side;

		switch ( current->m_resolver_mode ) {
		case RESOLVE_DISTORTION:
		case RESOLVE_STAND_BALANCE2:
			current->m_eye_angles.y = FindBestYaw( current, &data->m_move_data );
			return;
		}

		final_body_delta = data->m_move_data.m_body_delta;
	}
	else {
		final_body_delta = data->m_move_data.m_body_delta;

		switch ( data->m_misses[ MISS_STAND ] % 5 ) {
		// invert the angle
		case 1:
			final_body_delta *= -1.f;
			break;
		// maybe they are not breaking lby.
		case 2:
			final_body_delta = 0.f;
			break;
		// lets try tilted yaw angles.
		case 3:
			final_body_delta = 35.f;
			break;
		case 4:
			final_body_delta = -35.f;
			break;
		}
	}

	current->m_eye_angles.y = current->m_body - final_body_delta;
}

void Resolver::HandlePredUpdate( AimPlayer* data, LagRecord* current, LagRecord* previous ) {
	if ( !previous )
		return;

	if ( data->m_player->GetSequenceActivity( current->m_layers[ 3 ].m_sequence ) != ACT_CSGO_IDLE_TURN_BALANCEADJUST ) {
		data->m_update_data.m_foot_side = 0;
		data->m_update_data.m_distortion = false;
		return;
	}

	float foot_delta = std::max( std::min( fabs( math::NormalizedAngle( data->m_state->m_abs_yaw - current->m_body ) ), 58.f ), 35.f );

	if ( current->m_layers[ 3 ].m_weight > 0.0f || current->m_layers[ 3 ].m_cycle > 0.0f ) {
		if ( !data->m_update_data.m_distortion && data->m_update_data.m_foot_side > 0 )
			data->m_state->m_abs_yaw = current->m_body + foot_delta;

		return;
	}

	if ( data->m_player->GetSequenceActivity( previous->m_layers[ 3 ].m_sequence ) != ACT_CSGO_IDLE_TURN_BALANCEADJUST ) {
		data->m_state->m_abs_yaw = current->m_body - foot_delta;
		data->m_update_data.m_foot_side = -1;
		data->m_update_data.m_distortion = false;
		return;
	}

	if ( previous->m_layers[ 3 ].m_weight <= 0.0f && previous->m_layers[ 3 ].m_cycle <= 0.0f )
		return;

	data->m_state->m_abs_yaw = current->m_body - foot_delta;
	data->m_update_data.m_foot_side = -1;
	data->m_update_data.m_distortion = false;
}

void Resolver::ResolveAir( AimPlayer* data, LagRecord* current, LagRecord* previous ) {
	if ( previous && current->m_body != previous->m_body ) {
		current->m_eye_angles.y = current->m_body;
		current->m_resolver_mode = RESOLVE_AIR_UPDATE;
		return;
	}

	ang_t to_enemy;
	math::VectorAngles( current->m_abs_origin - g_cl.m_local->m_vecAbsOrigin( ), to_enemy );

	std::vector< AdaptiveAngle > angles;
	angles.emplace_back( current->m_body );

	if ( current->m_velocity.length_2d( ) > 0.1f ) {
		ang_t angle;
		math::VectorAngles( current->m_velocity, angle );
		angles.emplace_back( angle.y + 180.f );
	}

	angles.emplace_back( to_enemy.y );
	current->m_eye_angles.y = AutoDirection( data->m_player, angles );
}

void Resolver::ResolvePoses( Player* player, LagRecord* record ) {
	AimPlayer* data = &g_aimbot.m_players[ player->index( ) - 1 ];

	// only do this bs when in air.
	if ( record->m_resolver_mode == RESOLVE_AIR ) {
		// ang = pose min + pose val x ( pose range )

		// lean_yaw
		player->m_flPoseParameter( )[ 2 ] = g_csgo.RandomInt( 0, 4 ) * 0.98f;

		// body_yaw
		player->m_flPoseParameter( )[ 11 ] = g_csgo.RandomInt( 1, 4 ) * 0.85f;
	}
}

void Resolver::Override() {
	OverrideData* data = &m_override_data;

	if ( !data )
		return;

	if ( !g_cl.m_local || !g_cl.m_local->alive( ) ) {
		m_override_update = false;
		m_override = false;
		data->m_player = nullptr;
		return;
	}

	// update override.
	bool old_override = m_override_update;
	m_override_update = g_input.GetKeyState( g_menu.main.aimbot.correction_override.get( ) );

	if ( m_override_update != old_override && m_override_update )
		m_override = !m_override;

	if ( !m_override ) {
		m_override_angle = g_cl.m_view_angles;
		data->m_player = nullptr;
		return;
	}

	// dont override when dead.
	if ( !g_cl.m_local || !g_cl.m_local->alive( ) )
		return;

	// our override should have gotten updated.
	if ( !m_override_update )
		return;

	vec3_t from = g_cl.m_local->GetEyePos( true );

	float best_fov = FLT_MAX;
	vec3_t best_point = { 0, 0, 0 };

	for ( int i = 1; i <= g_csgo.m_globals->m_max_clients; i++ ) {
		Player* player = ( Player* )g_csgo.m_entlist->GetClientEntity( i );

		if ( !player || !player->alive( ) || player->dormant( ) || !player->enemy( g_cl.m_local ) )
			continue;

		vec3_t enemy_eye_pos = player->GetEyePos( true );

		const float fov = math::GetFOV( m_override_angle, from, enemy_eye_pos );

		if ( fov < best_fov ) {
			best_point = enemy_eye_pos;
			best_fov = fov;
			data->m_player = player;
		}
	}

	if ( best_fov == FLT_MAX )
		return;

	float dist = ( best_point - from ).length( );

	static vec3_t dir[ 2 ];
	math::AngleVectors( m_override_angle, &dir[ 0 ] );
	dir[ 0 ] *= dist;

	math::AngleVectors( g_cl.m_view_angles, &dir[ 1 ] );
	dir[ 1 ] *= dist;

	data->m_start = from + dir[ 0 ];
	data->m_end = from + dir[ 1 ];

	static vec2_t screen_pos[ 2 ];

	render::WorldToScreen( data->m_start, screen_pos[ 0 ] );
	render::WorldToScreen( data->m_end, screen_pos[ 1 ] );

	vec2_t delta = screen_pos[ 0 ] - screen_pos[ 1 ];

	data->m_yaw = atan2( delta.x, delta.y ) * ( 180.f / math::pi );
}
