#include <stdafx.hpp>

void menu::draw( )
{
	if ( GetAsyncKeyState( VK_DELETE ) & 1 )
	{
		this->m_open = !this->m_open;

		if ( settings::g_misc.close_cs_menu )
		{
			g::input.inject_keyboard( VK_ESCAPE, true );
			g::input.inject_keyboard( VK_ESCAPE, false );
		}
	}

	const auto dt = zdraw::get_delta_time( );
	const auto& appearance = settings::g_misc.m_appearance;
	const auto anim_speed = appearance.animation_speed;
	const auto fade_target = this->m_open ? 1.0f : 0.0f;
	this->m_fade_alpha += ( fade_target - this->m_fade_alpha ) * std::min( anim_speed * dt, 1.0f );
	this->m_appearance_scale += ( fade_target - this->m_appearance_scale ) * std::min( anim_speed * dt, 1.0f );

	if ( this->m_fade_alpha < 0.01f && !this->m_open )
	{
		this->m_fade_alpha = 0.0f;
		this->m_appearance_scale = 0.0f;
		::SetLayeredWindowAttributes( g::render.hwnd( ), 0, 255, LWA_ALPHA );
		return;
	}

	const auto alpha = static_cast< BYTE >( this->m_fade_alpha * 255.0f );
	::SetLayeredWindowAttributes( g::render.hwnd( ), 0, alpha, LWA_ALPHA );

	this->m_w = settings::g_misc.menu_w;
	this->m_h = settings::g_misc.menu_h;

	auto draw_x = this->m_x;
	auto draw_y = this->m_y;
	auto draw_w = this->m_w;
	auto draw_h = this->m_h;

	if ( appearance.scale_animation )
	{
		const auto scale = 0.85f + this->m_appearance_scale * 0.15f;
		const auto sw = std::round( this->m_w * scale );
		const auto sh = std::round( this->m_h * scale );
		draw_x = std::round( this->m_x + ( this->m_w - sw ) * 0.5f );
		draw_y = std::round( this->m_y + ( this->m_h - sh ) * 0.5f );
		draw_w = sw;
		draw_h = sh;
	}

	const auto pre_x = draw_x;
	const auto pre_y = draw_y;
	const auto pre_w = draw_w;
	const auto pre_h = draw_h;

	zui::begin( );

	if ( zui::begin_window( "nyality##main", draw_x, draw_y, draw_w, draw_h, true, 480.0f, 360.0f ) )
	{
		this->m_x += draw_x - pre_x;
		this->m_y += draw_y - pre_y;
		this->m_w += draw_w - pre_w;
		this->m_h += draw_h - pre_h;
		settings::g_misc.menu_w = this->m_w;
		settings::g_misc.menu_h = this->m_h;
		const auto [avail_w, avail_h] = zui::get_content_region_avail( );

		if ( zui::begin_nested_window( "##inner", avail_w, avail_h ) )
		{
			constexpr auto header_h{ 28.0f };
			constexpr auto padding{ 6.0f };

			zui::set_cursor_pos( padding, padding );
			this->draw_header( avail_w - padding * 2.0f, header_h );

			zui::set_cursor_pos( padding, padding + header_h + padding );
			this->draw_content( avail_w - padding * 2.0f, avail_h - header_h - padding * 3.0f );

			if ( const auto win = zui::detail::get_current_window( ) )
			{
				this->draw_accent_lines( win->bounds );
			}

			zui::end_nested_window( );
		}

		if ( const auto win = zui::detail::get_current_window( ) )
		{
			win->scroll_y = 0.0f;
		}

		zui::end_window( );
	}

	zui::end( );
}

void menu::draw_header( float width, float height )
{
	if ( !zui::begin_nested_window( "##header", width, height ) )
	{
		return;
	}

	const auto current = zui::detail::get_current_window( );
	if ( !current )
	{
		zui::end_nested_window( );
		return;
	}

	const auto& style = zui::get_style( );
	const auto dt = zdraw::get_delta_time( );
	const auto bx = current->bounds.x;
	const auto by = current->bounds.y;
	const auto bw = current->bounds.w;
	const auto bh = current->bounds.h;

	zdraw::rect_filled( bx, by, bw, bh, zdraw::rgba{ 14, 14, 14, 255 } );
	zdraw::rect( bx, by, bw, bh, zdraw::rgba{ 38, 38, 38, 255 } );

	{
		constexpr auto title{ "nyality" };
		auto [tw, th] = zdraw::measure_text( title );
		zdraw::text( bx + 10.0f, by + ( bh - th ) * 0.5f, title, style.accent );
	}

	static constexpr std::pair<const char*, tab> tabs[ ]
	{
		{ "combat", tab::combat },
		{ "esp",    tab::esp    },
		{ "misc",   tab::misc   },
		{ "config", tab::config },
	};

	constexpr auto tab_count = static_cast< int >( std::size( tabs ) );
	constexpr auto tab_spacing{ 16.0f };

	struct tab_anim { float v{ 0.0f }; };
	static std::array<tab_anim, tab_count> anims{};

	auto cursor_x = bx + bw - 10.0f;

	for ( int i = tab_count - 1; i >= 0; --i )
	{
		const auto& t = tabs[ i ];
		const auto is_sel = ( this->m_tab == t.second );
		auto [tw, th] = zdraw::measure_text( t.first );

		cursor_x -= tw;

		const auto tab_rect = zui::rect{ cursor_x, by, tw, bh };
		const auto hovered = zui::detail::mouse_hovered( tab_rect ) && !zui::detail::overlay_blocking_input( );

		if ( hovered && zui::detail::mouse_clicked( ) )
		{
			this->m_tab = t.second;
		}

		auto& anim = anims[ i ];
		anim.v += ( ( is_sel ? 1.0f : 0.0f ) - anim.v ) * std::min( 10.0f * dt, 1.0f );

		const auto text_y = by + ( bh - th ) * 0.5f;
		const auto col = is_sel ? zui::lighten( style.accent, 1.0f + 0.1f * anim.v ) : zui::lerp( zdraw::rgba{ 110, 110, 110, 255 }, style.text, hovered ? 1.0f : 0.0f );

		zdraw::text( cursor_x, text_y, t.first, col );

		cursor_x -= tab_spacing;
	}

	zui::end_nested_window( );
}

void menu::draw_content( float width, float height )
{
	zui::push_style_var( zui::style_var::window_padding_x, 10.0f );
	zui::push_style_var( zui::style_var::window_padding_y, 10.0f );

	if ( !zui::begin_nested_window( "##content", width, height ) )
	{
		zui::pop_style_var( 2 );
		return;
	}

	if ( const auto win = zui::detail::get_current_window( ) )
	{
		this->draw_accent_lines( win->bounds );
	}

	switch ( this->m_tab )
	{
	case tab::combat: this->draw_combat( ); break;
	case tab::esp:    this->draw_esp( );    break;
	case tab::misc:   this->draw_misc( );   break;
	case tab::config: this->draw_config( ); break;
	default: break;
	}

	zui::pop_style_var( 2 );
	zui::end_nested_window( );
}

void menu::draw_accent_lines( const zui::rect& bounds, float fade_ratio )
{
	const auto ix = bounds.x + 1.0f;
	const auto iw = bounds.w - 2.0f;
	const auto top_y = bounds.y + 1.0f;
	const auto bot_y = bounds.y + bounds.h - 2.0f;
	const auto accent = zui::get_accent_color( );
	const auto trans = zdraw::rgba{ accent.r, accent.g, accent.b, 0 };
	const auto fade_w = iw * fade_ratio;
	const auto solid_w = iw - fade_w * 2.0f;

	for ( const auto ly : { top_y, bot_y } )
	{
		zdraw::rect_filled_multi_color( ix, ly, fade_w, 1.0f, trans, accent, accent, trans );
		zdraw::rect_filled( ix + fade_w, ly, solid_w, 1.0f, accent );
		zdraw::rect_filled_multi_color( ix + fade_w + solid_w, ly, fade_w, 1.0f, accent, trans, trans, accent );
	}
}

void menu::draw_combat( )
{
	const auto [avail_w, avail_h] = zui::get_content_region_avail( );
	const auto col_w = ( avail_w - 8.0f ) * 0.5f;
	const auto& style = zui::get_style( );

	{
		const auto win = zui::detail::get_current_window( );
		if ( win )
		{
			constexpr auto group_spacing{ 12.0f };
			constexpr auto bar_h{ 22.0f };
			auto gx = win->bounds.x + style.window_padding_x;
			const auto gy = win->bounds.y + win->cursor_y;

			for ( int i = 0; i < 6; ++i )
			{
				auto [tw, th] = zdraw::measure_text( k_weapon_groups[ i ] );
				const auto gr = zui::rect{ gx, gy, tw, bar_h };
				const auto hov = zui::detail::mouse_hovered( gr ) && !zui::detail::overlay_blocking_input( );

				if ( hov && zui::detail::mouse_clicked( ) )
				{
					this->m_weapon_group = i;
				}

				const auto sel = ( this->m_weapon_group == i );
				const auto col = sel ? zui::get_accent_color( ) : zui::lerp( zdraw::rgba{ 100, 100, 100, 255 }, style.text, hov ? 1.0f : 0.0f );

				zdraw::text( gx, gy + ( bar_h - th ) * 0.5f, k_weapon_groups[ i ], col );

				if ( sel )
				{
					const auto accent = zui::get_accent_color( );
					const auto trans = zdraw::rgba{ accent.r, accent.g, accent.b, 0 };
					const auto fade = tw * 0.3f;
					zdraw::rect_filled_multi_color( gx, gy + bar_h - 2.0f, fade, 1.0f, trans, accent, accent, trans );
					zdraw::rect_filled( gx + fade, gy + bar_h - 2.0f, tw - fade * 2.0f, 1.0f, accent );
					zdraw::rect_filled_multi_color( gx + tw - fade, gy + bar_h - 2.0f, fade, 1.0f, accent, trans, trans, accent );
				}

				gx += tw + group_spacing;
			}

			win->cursor_y += bar_h + style.item_spacing_y;
			win->line_height = 0.0f;
		}
	}

	auto& cfg = settings::g_combat.groups[ this->m_weapon_group ];

	if ( zui::begin_group_box( "aimbot", col_w ) )
	{
		zui::checkbox( "enabled##ab", cfg.aimbot.enabled );
		zui::keybind( "key##ab", cfg.aimbot.key );
		zui::slider_int( "fov##ab", cfg.aimbot.fov, 1, 45 );
		zui::slider_int( "smoothing##ab", cfg.aimbot.smoothing, 0, 50 );
		zui::checkbox( "head only##ab", cfg.aimbot.head_only );
		zui::checkbox( "visible only##ab", cfg.aimbot.visible_only );

		if ( cfg.aimbot.visible_only )
		{
			zui::checkbox( "autowall##ab", cfg.aimbot.autowall );

			if ( cfg.aimbot.autowall )
			{
				zui::slider_float( "min damage##ab", cfg.aimbot.min_damage, 1.0f, 100.0f, "%.0f" );
			}
		}

		zui::checkbox( "predictive##ab", cfg.aimbot.predictive );
		zui::separator( );
		zui::checkbox( "draw fov##ab", cfg.aimbot.draw_fov );

		if ( cfg.aimbot.draw_fov )
		{
			zui::color_picker( "fov color##ab", cfg.aimbot.fov_color );
		}

		zui::end_group_box( );
	}

	zui::same_line( );

	if ( zui::begin_group_box( "triggerbot", col_w ) )
	{
		zui::checkbox( "enabled##tb", cfg.triggerbot.enabled );
		zui::keybind( "key##tb", cfg.triggerbot.key );
		zui::slider_float( "hitchance##tb", cfg.triggerbot.hitchance, 0.0f, 100.0f, "%.0f%%" );
		zui::slider_int( "delay (ms)##tb", cfg.triggerbot.delay, 0, 500 );
		zui::checkbox( "autowall##tb", cfg.triggerbot.autowall );

		if ( cfg.triggerbot.autowall )
		{
			zui::slider_float( "min damage##tb", cfg.triggerbot.min_damage, 1.0f, 100.0f, "%.0f" );
		}

		zui::checkbox( "autostop##tb", cfg.triggerbot.autostop );

		if ( cfg.triggerbot.autostop )
		{
			zui::checkbox( "early autostop##tb", cfg.triggerbot.early_autostop );
		}

		zui::checkbox( "predictive##tb", cfg.triggerbot.predictive );
		zui::end_group_box( );
	}

	if ( zui::begin_group_box( "other", col_w ) )
	{
		zui::checkbox( "penetration crosshair##ot", cfg.other.penetration_crosshair );

		if ( cfg.other.penetration_crosshair )
		{
			zui::color_picker( "can penetrate##ot", cfg.other.penetration_color_yes );
			zui::color_picker( "cannot penetrate##ot", cfg.other.penetration_color_no );
		}

		zui::end_group_box( );
	}

	if ( zui::begin_group_box( "humanization", col_w ) )
	{
		zui::slider_int( "amount##hz", cfg.humanization, 0, 100 );
		zui::end_group_box( );
	}
}

void menu::draw_esp( )
{
	const auto [avail_w, avail_h] = zui::get_content_region_avail( );
	const auto col_w = ( avail_w - 8.0f ) * 0.5f;
	auto& p = settings::g_esp.m_player;

	if ( zui::begin_group_box( "box", col_w ) )
	{
		zui::checkbox( "enabled##bx", p.m_box.enabled );

		constexpr const char* box_styles[ ]{ "full", "cornered" };
		static int bs = static_cast< int >( p.m_box.style );

		if ( zui::combo( "style##bx", bs, box_styles, 2 ) )
		{
			p.m_box.style = static_cast< settings::esp::player::box::style0 >( bs );
		}
		else
		{
			bs = static_cast< int >( p.m_box.style );
		}

		zui::checkbox( "fill##bx", p.m_box.fill );
		zui::checkbox( "outline##bx", p.m_box.outline );

		if ( p.m_box.style == settings::esp::player::box::style0::cornered )
		{
			zui::slider_float( "corner len##bx", p.m_box.corner_length, 4.0f, 30.0f, "%.0f" );
		}

		zui::color_picker( "visible##bx", p.m_box.visible_color );
		zui::color_picker( "occluded##bx", p.m_box.occluded_color );
		zui::end_group_box( );
	}

	if ( zui::begin_group_box( "skeleton", col_w ) )
	{
		zui::checkbox( "enabled##sk", p.m_skeleton.enabled );
		zui::slider_float( "thickness##sk", p.m_skeleton.thickness, 0.5f, 4.0f, "%.1f" );
		zui::color_picker( "visible##sk", p.m_skeleton.visible_color );
		zui::color_picker( "occluded##sk", p.m_skeleton.occluded_color );
		zui::end_group_box( );
	}

	if ( zui::begin_group_box( "health bar", col_w ) )
	{
		zui::checkbox( "enabled##hb", p.m_health_bar.enabled );
		zui::checkbox( "outline##hb", p.m_health_bar.outline );
		zui::checkbox( "gradient##hb", p.m_health_bar.gradient );
		zui::checkbox( "show value##hb", p.m_health_bar.show_value );
		zui::color_picker( "full##hb", p.m_health_bar.full_color );
		zui::color_picker( "low##hb", p.m_health_bar.low_color );
		zui::end_group_box( );
	}

	if ( zui::begin_group_box( "items", col_w ) )
	{
		auto& it = settings::g_esp.m_item;
		zui::checkbox( "enabled##it", it.enabled );
		zui::slider_float( "max dist##it", it.max_distance, 5.0f, 150.0f, "%.0fm" );
		zui::checkbox( "icon##it", it.m_icon.enabled );
		zui::color_picker( "icon color##it", it.m_icon.color );
		zui::checkbox( "name##it", it.m_name.enabled );
		zui::color_picker( "name color##it", it.m_name.color );
		zui::checkbox( "ammo##it", it.m_ammo.enabled );
		zui::color_picker( "ammo color##it", it.m_ammo.color );
		zui::color_picker( "empty color##it", it.m_ammo.empty_color );
		zui::end_group_box( );
	}

	const auto [cx, cy] = zui::get_cursor_pos( );
	zui::set_cursor_pos( col_w + 8.0f + zui::get_style( ).window_padding_x, zui::get_style( ).window_padding_y );

	if ( zui::begin_group_box( "name / weapon", col_w ) )
	{
		zui::checkbox( "name##nm", p.m_name.enabled );
		zui::color_picker( "name color##nm", p.m_name.color );
		zui::separator( );
		zui::checkbox( "weapon##wp", p.m_weapon.enabled );

		constexpr const char* disp_types[ ]{ "text", "icon", "text + icon" };
		static int dt = static_cast< int >( p.m_weapon.display );

		if ( zui::combo( "display##wp", dt, disp_types, 3 ) )
		{
			p.m_weapon.display = static_cast< settings::esp::player::weapon::display_type >( dt );
		}
		else
		{
			dt = static_cast< int >( p.m_weapon.display );
		}

		zui::color_picker( "text color##wp", p.m_weapon.text_color );
		zui::color_picker( "icon color##wp", p.m_weapon.icon_color );
		zui::end_group_box( );
	}

	if ( zui::begin_group_box( "ammo bar", col_w ) )
	{
		zui::checkbox( "enabled##amb", p.m_ammo_bar.enabled );
		zui::checkbox( "outline##amb", p.m_ammo_bar.outline );
		zui::checkbox( "gradient##amb", p.m_ammo_bar.gradient );
		zui::checkbox( "show value##amb", p.m_ammo_bar.show_value );
		zui::color_picker( "full##amb", p.m_ammo_bar.full_color );
		zui::color_picker( "low##amb", p.m_ammo_bar.low_color );
		zui::end_group_box( );
	}

	if ( zui::begin_group_box( "projectiles", col_w ) )
	{
		auto& pr = settings::g_esp.m_projectile;
		zui::checkbox( "enabled##pr", pr.enabled );
		zui::checkbox( "icon##pr", pr.show_icon );
		zui::checkbox( "name##pr", pr.show_name );
		zui::checkbox( "timer bar##pr", pr.show_timer_bar );
		zui::checkbox( "inferno bounds##pr", pr.show_inferno_bounds );
		zui::separator( );
		zui::color_picker( "he##pr", pr.color_he );
		zui::color_picker( "flash##pr", pr.color_flash );
		zui::color_picker( "smoke##pr", pr.color_smoke );
		zui::color_picker( "molotov##pr", pr.color_molotov );
		zui::color_picker( "decoy##pr", pr.color_decoy );
		zui::end_group_box( );
	}

	if ( zui::begin_group_box( "item filters", col_w ) )
	{
		auto& f = settings::g_esp.m_item.m_filters;
		zui::checkbox( "rifles##f", f.rifles );
		zui::checkbox( "smgs##f", f.smgs );
		zui::checkbox( "shotguns##f", f.shotguns );
		zui::checkbox( "snipers##f", f.snipers );
		zui::checkbox( "pistols##f", f.pistols );
		zui::checkbox( "heavy##f", f.heavy );
		zui::checkbox( "grenades##f", f.grenades );
		zui::checkbox( "utility##f", f.utility );
		zui::end_group_box( );
	}
}

void menu::draw_misc( )
{
	const auto [avail_w, avail_h] = zui::get_content_region_avail( );
	const auto col_w = ( avail_w - 8.0f ) * 0.5f;
	const auto& style = zui::get_style( );

	auto& gr = settings::g_misc.m_grenades;

	if ( zui::begin_group_box( "grenade prediction", col_w ) )
	{
		zui::checkbox( "enabled##gr", gr.enabled );
		zui::checkbox( "local only##gr", gr.local_only );
		zui::slider_float( "line thickness##gr", gr.line_thickness, 0.5f, 5.0f, "%.1f" );
		zui::checkbox( "gradient line##gr", gr.line_gradient );
		zui::color_picker( "line color##gr", gr.line_color );
		zui::separator( );
		zui::checkbox( "show bounces##gr", gr.show_bounces );
		zui::color_picker( "bounce color##gr", gr.bounce_color );
		zui::slider_float( "bounce size##gr", gr.bounce_size, 1.0f, 8.0f, "%.1f" );
		zui::separator( );
		zui::color_picker( "detonate color##gr", gr.detonate_color );
		zui::slider_float( "detonate size##gr", gr.detonate_size, 1.0f, 10.0f, "%.1f" );
		zui::slider_float( "fade duration##gr", gr.fade_duration, 0.0f, 2.0f, "%.2f" );
		zui::end_group_box( );
	}

	if ( zui::begin_group_box( "movement", col_w ) )
	{
		auto& mv = settings::g_misc.m_movement;
		zui::checkbox( "bunny hop##mv", mv.bhop_enabled );
		zui::keybind( "key##bhop", mv.bhop_key );
		zui::checkbox( "quick stop##mv", mv.quick_stop );
		zui::end_group_box( );
	}

	if ( zui::begin_group_box( "watermark", col_w ) )
	{
		auto& wm = settings::g_misc.m_watermark;
		zui::checkbox( "enabled##wm", wm.enabled );

		{
			static const char* wm_styles[] = { "boxed", "minimal" };
			zui::combo( "style##wm", wm.style, wm_styles, 2 );
		}

		this->m_watermark_text = std::string( wm.custom_text );

		if ( zui::text_input( "text##wm", this->m_watermark_text, 63 ) )
		{
			std::memset( wm.custom_text, 0, sizeof( wm.custom_text ) );
			std::memcpy( wm.custom_text, this->m_watermark_text.c_str( ), std::min( this->m_watermark_text.size( ), std::size_t{ 63 } ) );
		}

		zui::checkbox( "show fps##wm", wm.show_fps );
		zui::end_group_box( );
	}

	zui::set_cursor_pos( col_w + 8.0f + style.window_padding_x, style.window_padding_y );

	if ( zui::begin_group_box( "per type colors", col_w ) )
	{
		zui::checkbox( "enabled##ptc", gr.per_type_colors );
		if ( gr.per_type_colors )
		{
			zui::color_picker( "he##ptc", gr.color_he );
			zui::color_picker( "flash##ptc", gr.color_flash );
			zui::color_picker( "smoke##ptc", gr.color_smoke );
			zui::color_picker( "molotov##ptc", gr.color_molotov );
			zui::color_picker( "decoy##ptc", gr.color_decoy );
		}

		zui::end_group_box( );
	}

	if ( zui::begin_group_box( "crosshair", col_w ) )
	{
		auto& ch = settings::g_misc.m_crosshair;
		zui::checkbox( "enabled##ch", ch.enabled );
		zui::checkbox( "sniper only##ch", ch.sniper_only );
		zui::slider_int( "size##ch", ch.size, 1, 10 );
		zui::slider_int( "gap##ch", ch.gap, 0, 8 );
		zui::slider_int( "thickness##ch", ch.thickness, 1, 5 );
		zui::color_picker( "color##ch", ch.color );
		zui::separator( );
		zui::checkbox( "hitmarker##ch", ch.hitmarker );

		if ( ch.hitmarker )
		{
			zui::color_picker( "hitmarker color##ch", ch.hitmarker_color );
			zui::slider_int( "hitmarker size##ch", ch.hitmarker_size, 2, 20 );
			zui::slider_int( "hitmarker thickness##ch", ch.hitmarker_thickness, 1, 5 );
			zui::slider_float( "hitmarker duration##ch", ch.hitmarker_duration, 0.1f, 1.0f, "%.2fs" );
		}

		zui::end_group_box( );
	}

	if ( zui::begin_group_box( "appearance", col_w ) )
	{
		zui::color_picker( "accent color##app", settings::g_misc.accent_color );
		zui::checkbox( "scale animation##app", settings::g_misc.m_appearance.scale_animation );
		zui::slider_float( "animation speed##app", settings::g_misc.m_appearance.animation_speed, 1.0f, 20.0f, "%.1f" );
		zui::checkbox( "anti screenshare##app", settings::g_misc.anti_screenshare );
		zui::checkbox( "close cs menu##app", settings::g_misc.close_cs_menu );
		zui::end_group_box( );
	}

	if ( zui::begin_group_box( "indicators", col_w ) )
	{
		auto& ind = settings::g_misc.m_indicators;

		zui::checkbox( "key display##ind", ind.keys_enabled );

		if ( ind.keys_enabled )
		{
			zui::checkbox( "show speed##ind", ind.show_speed );
			zui::slider_int( "size##ind", ind.keys_size, 12, 40 );
			zui::color_picker( "key color##ind", ind.key_color );
			zui::color_picker( "active color##ind", ind.key_color_active );
		}

		zui::end_group_box( );
	}
}

void menu::draw_config( )
{
	const auto [avail_w, avail_h] = zui::get_content_region_avail( );
	const auto col_w = ( avail_w - 8.0f ) * 0.5f;

	if ( zui::begin_group_box( "save config", col_w ) )
	{
		zui::text_input( "name##cfg", this->m_config_name, 32, "config name" );

		if ( zui::button( "save##cfg", col_w - 20.0f, 24.0f ) )
		{
			if ( !this->m_config_name.empty( ) )
			{
				g::config.save( this->m_config_name );
			}
		}

		zui::end_group_box( );
	}

	zui::same_line( );

	if ( zui::begin_group_box( "load config", col_w ) )
	{
		const auto configs = g::config.list( );
		const auto default_name = g::config.get_default( );

		if ( !configs.empty( ) )
		{
			std::vector<const char*> items{};
			items.reserve( configs.size( ) );

			for ( const auto& c : configs )
			{
				items.push_back( c.c_str( ) );
			}

			if ( this->m_config_selected >= static_cast< int >( configs.size( ) ) )
			{
				this->m_config_selected = 0;
			}

			zui::combo( "##cfglist", this->m_config_selected, items.data( ), static_cast< int >( items.size( ) ) );

			const auto& selected = configs[ this->m_config_selected ];

			if ( zui::button( "load##cfg", col_w - 20.0f, 24.0f ) )
			{
				g::config.load( selected );
			}

			if ( zui::button( "save##cfgoverwrite", col_w - 20.0f, 24.0f ) )
			{
				g::config.save( selected );
			}

			if ( zui::button( "delete##cfg", col_w - 20.0f, 24.0f ) )
			{
				g::config.remove( selected );

				if ( this->m_config_selected >= static_cast< int >( configs.size( ) ) - 1 )
				{
					this->m_config_selected = std::max( 0, static_cast< int >( configs.size( ) ) - 2 );
				}
			}

			if ( zui::button( "set default##cfg", col_w - 20.0f, 24.0f ) )
			{
				g::config.set_default( selected );
			}

			if ( !default_name.empty( ) )
			{
				zui::separator( );
				zui::text( std::string( "default: " ) + default_name );
			}
		}
		else
		{
			zui::text( "no configs saved" );
		}

		zui::end_group_box( );
	}

	if ( zui::begin_group_box( "reset", col_w ) )
	{
		if ( zui::button( "reset to defaults##cfg", col_w - 20.0f, 24.0f ) )
		{
			g::config.reset( );
		}

		zui::end_group_box( );
	}

	if ( zui::begin_group_box( "unload", col_w ) )
	{
		if ( zui::button( "exit nyality##unload", col_w - 20.0f, 24.0f ) )
		{
			::PostQuitMessage( 0 );
		}

		zui::end_group_box( );
	}
}