#include <stdafx.hpp>

namespace features::misc {

	void watermark::on_render( )
	{
		auto& cfg = settings::g_misc.m_watermark;
		if ( !cfg.enabled )
			return;

		const auto global_vars = g::memory.read<std::uintptr_t>( g::offsets.global_vars );
		if ( global_vars )
		{
			const auto frametime = g::memory.read<float>( global_vars + 0x08 );
			if ( frametime > 0.0f )
			{
				this->m_fps_accumulator += 1.0f / frametime;
				this->m_fps_samples++;
			}
		}

		const auto dt = zdraw::get_delta_time( );
		this->m_fps_timer += dt;

		if ( this->m_fps_timer >= 1.0f && this->m_fps_samples > 0 )
		{
			this->m_fps = this->m_fps_accumulator / static_cast< float >( this->m_fps_samples );
			this->m_fps_accumulator = 0.0f;
			this->m_fps_samples = 0;
			this->m_fps_timer = 0.0f;
		}

		const auto accent = zui::get_accent_color( );
		const auto text_col = zdraw::rgba{ 215, 215, 215, 255 };

		std::string label( cfg.custom_text );

		if ( cfg.show_fps )
		{
			label += std::format( " | {} fps", static_cast< int >( this->m_fps ) );
		}

		const auto [tw, th] = zdraw::measure_text( label );
		const auto [sw, sh] = zdraw::get_display_size( );

		constexpr auto pad_x{ 10.0f };
		constexpr auto pad_y{ 6.0f };
		constexpr auto margin{ 10.0f };

		const auto box_w = tw + pad_x * 2.0f;
		const auto box_h = th + pad_y * 2.0f;

		if ( this->m_last_screen_w != 0 && this->m_last_screen_h != 0 )
		{
			if ( sw != this->m_last_screen_w || sh != this->m_last_screen_h )
			{
				cfg.x = margin;
				cfg.y = margin;
			}
		}

		this->m_last_screen_w = sw;
		this->m_last_screen_h = sh;

		cfg.x = std::clamp( cfg.x, 0.0f, static_cast< float >( sw ) - box_w );
		cfg.y = std::clamp( cfg.y, 0.0f, static_cast< float >( sh ) - box_h );

		if ( g::menu.is_open( ) )
		{
			POINT cursor{};
			::GetCursorPos( &cursor );
			const auto mx = static_cast< float >( cursor.x );
			const auto my = static_cast< float >( cursor.y );

			const auto hovering = mx >= cfg.x && mx <= cfg.x + box_w && my >= cfg.y && my <= cfg.y + box_h;

			if ( ( ::GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) != 0 )
			{
				if ( !this->m_dragging && hovering )
				{
					this->m_dragging = true;
					this->m_drag_offset_x = mx - cfg.x;
					this->m_drag_offset_y = my - cfg.y;
				}

				if ( this->m_dragging )
				{
					cfg.x = mx - this->m_drag_offset_x;
					cfg.y = my - this->m_drag_offset_y;
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

		const auto bx = cfg.x;
		const auto by = cfg.y;

		if ( cfg.style == 1 )
		{
			zdraw::rect_filled( bx, by, box_w, box_h, zdraw::rgba{ 15, 16, 22, 200 } );
			zdraw::text( bx + pad_x, by + pad_y, label, accent );
		}
		else
		{
			zdraw::rect_filled( bx, by, box_w, box_h, zdraw::rgba{ 15, 16, 22, 200 } );
			zdraw::rect( bx, by, box_w, box_h, zdraw::rgba{ 38, 38, 38, 255 } );

			const auto accent_trans = zdraw::rgba{ accent.r, accent.g, accent.b, 0 };
			const auto fade = box_w * 0.15f;
			const auto solid = box_w - fade * 2.0f;
			zdraw::rect_filled_multi_color( bx + 1.0f, by + 1.0f, fade, 1.0f, accent_trans, accent, accent, accent_trans );
			zdraw::rect_filled( bx + 1.0f + fade, by + 1.0f, solid, 1.0f, accent );
			zdraw::rect_filled_multi_color( bx + 1.0f + fade + solid, by + 1.0f, fade, 1.0f, accent, accent_trans, accent_trans, accent );

			zdraw::text( bx + pad_x, by + pad_y, label, text_col );
		}
	}

}
