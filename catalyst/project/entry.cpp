#include <stdafx.hpp>

static std::filesystem::path get_config_dir( )
{
	wchar_t path[ MAX_PATH ]{};
	::GetModuleFileNameW( nullptr, path, MAX_PATH );

	auto dir = std::filesystem::path( path ).parent_path( ) / "configs";

	if ( !std::filesystem::exists( dir ) )
	{
		std::filesystem::create_directories( dir );
	}

	return dir;
}

static bool is_first_launch( )
{
	return !std::filesystem::exists( get_config_dir( ) / ".first_launch" );
}

static bool relaunch_as_random( )
{
	wchar_t path[ MAX_PATH ]{};
	::GetModuleFileNameW( nullptr, path, MAX_PATH );

	auto dir = std::filesystem::path( path ).parent_path( );

	std::mt19937 rng( static_cast< std::uint32_t >( __rdtsc( ) ) );
	std::uniform_int_distribution<int> dist( 'a', 'z' );
	std::wstring new_name{};
	for ( int i = 0; i < 10; ++i )
		new_name += static_cast< wchar_t >( dist( rng ) );
	new_name += L".exe";

	auto new_path = dir / new_name;

	if ( !::CopyFileW( path, new_path.c_str( ), FALSE ) )
	{
		return false;
	}

	{
		std::ofstream marker( get_config_dir( ) / ".first_launch" );
	}

	std::wstring cmd = L"\"" + new_path.wstring( ) + L"\" \"" + std::wstring( path ) + L"\"";

	STARTUPINFOW si{};
	si.cb = sizeof( si );
	PROCESS_INFORMATION pi{};

	if ( !::CreateProcessW( nullptr, cmd.data( ), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi ) )
	{
		return false;
	}

	::CloseHandle( pi.hThread );
	::CloseHandle( pi.hProcess );

	return true;
}

static void mutate_binary( )
{
	wchar_t path[ MAX_PATH ]{};
	::GetModuleFileNameW( nullptr, path, MAX_PATH );

	std::mt19937 rng( static_cast< std::uint32_t >( __rdtsc( ) ) );

	{
		std::ofstream file( path, std::ios::binary | std::ios::app );
		if ( file )
		{
			std::uint8_t junk[ 64 ]{};
			for ( auto& b : junk )
				b = static_cast< std::uint8_t >( rng( ) & 0xFF );

			file.write( reinterpret_cast< const char* >( junk ), sizeof( junk ) );
		}
	}

	auto dir = std::filesystem::path( path ).parent_path( );
	std::uniform_int_distribution<int> dist( 'a', 'z' );
	std::wstring new_name{};
	for ( int i = 0; i < 8; ++i )
		new_name += static_cast< wchar_t >( dist( rng ) );
	new_name += L".exe";

	auto new_path = dir / new_name;

	::MoveFileW( path, new_path.c_str( ) );
}

int main( int argc, char* argv[] )
{
	if ( is_first_launch( ) )
	{
		if ( relaunch_as_random( ) )
		{
			return 0;
		}
	}

	if ( argc > 1 )
	{
		::Sleep( 500 );

		const std::filesystem::path original( argv[ 1 ] );
		std::filesystem::remove( original );
	}

	mutate_binary( );

	{
		if ( !g::console.initialize( " :3 " ) )
		{
			return 1;
		}

		if ( !g::input.initialize( ) )
		{
			return 1;
		}

		if ( !g::memory.initialize( L"cs2.exe" ) )
		{
			return 1;
		}
	}

	{
		if ( !g::modules.initialize( ) )
		{
			return 1;
		}

		if ( !g::offsets.initialize( ) )
		{
			return 1;
		}
	}

	g::config.load_default( );

	{
		std::thread( threads::game ).detach( );
		std::thread( threads::combat ).detach( );
		std::thread( threads::movement ).detach( );

		if ( !g::render.initialize( ) )
		{
			return 1;
		}
	}

	return 0;
}
