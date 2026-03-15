#include <stdafx.hpp>

namespace features::indicators {

	void display::on_render( )
	{
		const auto pawn = systems::g_local.pawn( );
		if ( !pawn || !systems::g_local.alive( ) )
			return;

		const auto [sw, sh] = zdraw::get_display_size( );
		this->draw_keys( static_cast< float >( sw ), static_cast< float >( sh ) );
	}

	void display::draw_keys( float sw, float sh )
	{
		auto& cfg = settings::g_misc.m_indicators;
		if ( !cfg.keys_enabled )
			return;

		const auto accent = zui::get_accent_color( );

		const auto cell_sz = static_cast< float >( cfg.keys_size );
		constexpr auto cell_gap = 2.0f;

		const auto grid_w = cell_sz * 3.0f + cell_gap * 2.0f;
		const auto grid_h = cell_sz * 2.0f + cell_gap + 6.0f;

		float speed_h = 0.0f;
		std::string speed_str{};
		float speed_tw = 0.0f;

		if ( cfg.show_speed )
		{
			const auto pawn = systems::g_local.pawn( );
			if ( pawn )
			{
				const auto vel = g::memory.read<math::vector3>( pawn + SCHEMA( "C_BaseEntity", "m_vecAbsVelocity"_hash ) );
				speed_str = std::to_string( static_cast< int >( vel.length_2d( ) ) ) + " u/s";
				const auto [tw, th] = zdraw::measure_text( speed_str );
				speed_tw = tw;
				speed_h = th + 4.0f;
			}
		}

		const auto total_w = grid_w;
		const auto total_h = grid_h + speed_h;

		if ( cfg.keys_x == 0.0f && cfg.keys_y == 0.0f )
		{
			cfg.keys_x = sw * 0.5f - total_w * 0.5f;
			cfg.keys_y = sh - 140.0f;
		}

		cfg.keys_x = std::clamp( cfg.keys_x, 0.0f, sw - total_w );
		cfg.keys_y = std::clamp( cfg.keys_y, 0.0f, sh - total_h );

		if ( g::menu.is_open( ) )
		{
			POINT cursor{};
			::GetCursorPos( &cursor );
			const auto mx = static_cast< float >( cursor.x );
			const auto my = static_cast< float >( cursor.y );

			const auto hovering = mx >= cfg.keys_x && mx <= cfg.keys_x + total_w && my >= cfg.keys_y && my <= cfg.keys_y + total_h;

			if ( ( ::GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) != 0 )
			{
				if ( !this->m_dragging && hovering )
				{
					this->m_dragging = true;
					this->m_drag_offset_x = mx - cfg.keys_x;
					this->m_drag_offset_y = my - cfg.keys_y;
				}

				if ( this->m_dragging )
				{
					cfg.keys_x = mx - this->m_drag_offset_x;
					cfg.keys_y = my - this->m_drag_offset_y;
				}
			}
			else
			{
				this->m_dragging = false;
			}
		}
		else
		{
			this->m_dragging = false;
		}

		const auto gx = cfg.keys_x;
		const auto gy = cfg.keys_y;

		auto draw_key_cell = [&]( const char* label, int vk, float cx, float cy, float cw, float ch )
		{
			const auto pressed = ( GetAsyncKeyState( vk ) & 0x8000 ) != 0;
			const auto bg = pressed ? zdraw::rgba{ accent.r, accent.g, accent.b, 40 } : zdraw::rgba{ 8, 8, 12, 180 };
			const auto border = pressed ? zdraw::rgba{ accent.r, accent.g, accent.b, 120 } : zdraw::rgba{ 38, 38, 38, 150 };
			const auto text_col = pressed ? accent : cfg.key_color;

			zdraw::rect_filled( cx, cy, cw, ch, bg );
			zdraw::rect( cx, cy, cw, ch, border );

			const auto [tw, th] = zdraw::measure_text( label );
			zdraw::text( cx + ( cw - tw ) * 0.5f, cy + ( ch - th ) * 0.5f, label, text_col );
		};

		draw_key_cell( "W", 'W', gx + cell_sz + cell_gap, gy, cell_sz, cell_sz );

		const auto row1_y = gy + cell_sz + cell_gap;
		draw_key_cell( "A", 'A', gx, row1_y, cell_sz, cell_sz );
		draw_key_cell( "S", 'S', gx + cell_sz + cell_gap, row1_y, cell_sz, cell_sz );
		draw_key_cell( "D", 'D', gx + ( cell_sz + cell_gap ) * 2.0f, row1_y, cell_sz, cell_sz );

		const auto space_y = row1_y + cell_sz + cell_gap;
		draw_key_cell( " ", VK_SPACE, gx, space_y, grid_w, 4.0f );

		if ( cfg.show_speed && !speed_str.empty( ) )
		{
			const auto speed_y = space_y + 4.0f + 4.0f;
			zdraw::text( gx + ( grid_w - speed_tw ) * 0.5f, speed_y, speed_str, cfg.key_color );
		}
	}

}
