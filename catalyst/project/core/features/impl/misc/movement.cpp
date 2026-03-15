#include <stdafx.hpp>

namespace features::misc {

	void movement::tick( )
	{
		const auto& cfg = settings::g_misc.m_movement;

		static auto cs2 = ::FindWindowA( nullptr, "Counter-Strike 2" );
		if ( !cs2 )
			cs2 = ::FindWindowA( nullptr, "Counter-Strike 2" );

		const auto fg = ::GetForegroundWindow( );
		if ( !cs2 || fg != cs2 )
		{
			if ( this->m_stopping )
			{
				for ( const auto key : this->m_stop_keys )
				{
					g::input.inject_keyboard( key, false );
				}

				this->m_stop_keys.clear( );
				this->m_stopping = false;
			}

			if ( this->m_bhop_state == 1 )
			{
				::PostMessage( cs2, WM_KEYUP, VK_SPACE, 0 );
			}
			this->m_bhop_state = 0;

			return;
		}

		if ( !systems::g_local.alive( ) )
		{
			this->m_last_on_ground = false;

			if ( this->m_stopping )
			{
				for ( const auto key : this->m_stop_keys )
				{
					g::input.inject_keyboard( key, false );
				}

				this->m_stop_keys.clear( );
				this->m_stopping = false;
			}

			return;
		}

		const auto pawn = systems::g_local.pawn( );
		if ( !pawn )
			return;

		const auto flags = g::memory.read<std::uint32_t>( pawn + SCHEMA( "C_BaseEntity", "m_fFlags"_hash ) );
		const auto on_ground = ( flags & 1 ) != 0;

		if ( cfg.bhop_enabled )
		{
			const auto key_held = ( GetAsyncKeyState( cfg.bhop_key ) & 0x8000 ) != 0;

			if ( key_held )
			{
				if ( this->m_bhop_state == 1 )
				{
					::PostMessage( cs2, WM_KEYUP, VK_SPACE, 0 );
					this->m_bhop_state = 0;
				}
				else if ( on_ground )
				{
					::PostMessage( cs2, WM_KEYDOWN, VK_SPACE, 0 );
					this->m_bhop_state = 1;
				}
			}
			else
			{
				if ( this->m_bhop_state == 1 )
				{
					::PostMessage( cs2, WM_KEYUP, VK_SPACE, 0 );
				}
				this->m_bhop_state = 0;
			}
		}

		this->m_last_on_ground = on_ground;

		if ( cfg.quick_stop && on_ground )
		{
			const auto w = ( GetAsyncKeyState( 'W' ) & 0x8000 ) != 0;
			const auto s = ( GetAsyncKeyState( 'S' ) & 0x8000 ) != 0;
			const auto a = ( GetAsyncKeyState( 'A' ) & 0x8000 ) != 0;
			const auto d = ( GetAsyncKeyState( 'D' ) & 0x8000 ) != 0;
			const auto space = ( GetAsyncKeyState( VK_SPACE ) & 0x8000 ) != 0;
			const auto any_key = w || s || a || d || space;

			if ( this->m_stopping )
			{
				const auto velocity = g::memory.read<math::vector3>( pawn + SCHEMA( "C_BaseEntity", "m_vecAbsVelocity"_hash ) );
				const auto elapsed = std::chrono::duration<float>( std::chrono::steady_clock::now( ) - this->m_stop_start ).count( );

				if ( any_key || velocity.length_2d( ) <= 15.0f || elapsed >= 0.15f )
				{
					for ( const auto key : this->m_stop_keys )
					{
						g::input.inject_keyboard( key, false );
					}

					this->m_stop_keys.clear( );
					this->m_stopping = false;
				}
			}

			if ( !this->m_stopping && !any_key )
			{
				const auto velocity = g::memory.read<math::vector3>( pawn + SCHEMA( "C_BaseEntity", "m_vecAbsVelocity"_hash ) );

				if ( velocity.length_2d( ) > 15.0f )
				{
					const auto angles = systems::g_view.angles( );
					math::vector3 forward{}, right{};
					angles.to_directions( &forward, &right, nullptr );

					const auto vel_forward = velocity.x * forward.x + velocity.y * forward.y;
					const auto vel_right = velocity.x * right.x + velocity.y * right.y;

					this->m_stop_keys.clear( );

					if ( vel_forward > 15.0f )
						this->m_stop_keys.push_back( 'S' );
					else if ( vel_forward < -15.0f )
						this->m_stop_keys.push_back( 'W' );

					if ( vel_right > 15.0f )
						this->m_stop_keys.push_back( 'A' );
					else if ( vel_right < -15.0f )
						this->m_stop_keys.push_back( 'D' );

					if ( !this->m_stop_keys.empty( ) )
					{
						for ( const auto key : this->m_stop_keys )
						{
							g::input.inject_keyboard( key, true );
						}

						this->m_stopping = true;
						this->m_stop_start = std::chrono::steady_clock::now( );
					}
				}
			}
		}
		else
		{
			if ( this->m_stopping )
			{
				for ( const auto key : this->m_stop_keys )
				{
					g::input.inject_keyboard( key, false );
				}

				this->m_stop_keys.clear( );
				this->m_stopping = false;
			}
		}
	}

}
