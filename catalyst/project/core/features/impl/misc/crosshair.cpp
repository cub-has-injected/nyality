#include <stdafx.hpp>

namespace features::misc {

	void crosshair::on_render( )
	{
		const auto& cfg = settings::g_misc.m_crosshair;

		const auto [w, h] = zdraw::get_display_size( );
		const auto cx = w * 0.5f;
		const auto cy = h * 0.5f;

		if ( cfg.enabled )
		{
			auto draw_crosshair = true;

			if ( cfg.sniper_only )
			{
				const auto& ctx = features::combat::g_shared.ctx( );
				if ( !ctx.valid || ctx.weapon_type != cstypes::sniper )
					draw_crosshair = false;
			}

			if ( draw_crosshair )
			{
				const auto size = static_cast< float >( cfg.size );
				const auto gap = static_cast< float >( cfg.gap );
				const auto thick = static_cast< float >( cfg.thickness );

				zdraw::line( cx - gap - size, cy, cx - gap, cy, cfg.color, thick );
				zdraw::line( cx + gap, cy, cx + gap + size, cy, cfg.color, thick );
				zdraw::line( cx, cy - gap - size, cx, cy - gap, cfg.color, thick );
				zdraw::line( cx, cy + gap, cx, cy + gap + size, cfg.color, thick );
			}
		}

		if ( cfg.hitmarker )
		{
			const auto global_vars = g::memory.read<std::uintptr_t>( g::offsets.global_vars );
			const auto current_time = global_vars ? g::memory.read<float>( global_vars + 0x30 ) : 0.0f;

			const auto players = systems::g_collector.players( );
			for ( const auto& player : players )
			{
				const auto it = this->m_last_health.find( player.controller );
				if ( it != this->m_last_health.end( ) )
				{
					if ( player.health < it->second )
					{
						this->m_hitmarker_time = current_time;
					}
				}
				this->m_last_health[ player.controller ] = player.health;
			}

			if ( this->m_hitmarker_time > 0.0f )
			{
				const auto elapsed = current_time - this->m_hitmarker_time;

				if ( elapsed < cfg.hitmarker_duration )
				{
					const auto alpha_frac = 1.0f - ( elapsed / cfg.hitmarker_duration );
					const auto a = static_cast< std::uint8_t >( alpha_frac * cfg.hitmarker_color.a );
					const auto col = zdraw::rgba{ cfg.hitmarker_color.r, cfg.hitmarker_color.g, cfg.hitmarker_color.b, a };
					const auto sz = static_cast< float >( cfg.hitmarker_size );
					const auto gap = static_cast< float >( cfg.gap ) + 2.0f;
					const auto hm_thick = static_cast< float >( cfg.hitmarker_thickness );

					zdraw::line( cx - gap - sz, cy - gap - sz, cx - gap, cy - gap, col, hm_thick );
					zdraw::line( cx + gap, cy - gap, cx + gap + sz, cy - gap - sz, col, hm_thick );
					zdraw::line( cx - gap - sz, cy + gap + sz, cx - gap, cy + gap, col, hm_thick );
					zdraw::line( cx + gap, cy + gap, cx + gap + sz, cy + gap + sz, col, hm_thick );
				}
			}
		}
	}

}
