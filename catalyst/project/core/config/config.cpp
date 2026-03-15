#include <stdafx.hpp>

std::filesystem::path config::get_config_dir( ) const
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

std::filesystem::path config::get_config_path( const std::string& name ) const
{
	return this->get_config_dir( ) / ( name + ".cfg" );
}

bool config::save( const std::string& name ) const
{
	if ( name.empty( ) )
		return false;

	const auto path = this->get_config_path( name );

	std::ofstream file( path, std::ios::binary );
	if ( !file )
	{
		g::console.warn( "failed to save config: {}", name );
		return false;
	}

	header hdr{};
	hdr.magic = k_magic;
	hdr.version = k_version;
	hdr.combat_size = static_cast< std::uint32_t >( sizeof( settings::combat ) );
	hdr.esp_size = static_cast< std::uint32_t >( sizeof( settings::esp ) );
	hdr.misc_size = static_cast< std::uint32_t >( sizeof( settings::misc ) );

	file.write( reinterpret_cast< const char* >( &hdr ), sizeof( hdr ) );
	file.write( reinterpret_cast< const char* >( &settings::g_combat ), sizeof( settings::combat ) );
	file.write( reinterpret_cast< const char* >( &settings::g_esp ), sizeof( settings::esp ) );
	file.write( reinterpret_cast< const char* >( &settings::g_misc ), sizeof( settings::misc ) );

	g::console.success( "config saved: {}", name );
	return true;
}

bool config::load( const std::string& name )
{
	if ( name.empty( ) )
		return false;

	const auto path = this->get_config_path( name );

	std::ifstream file( path, std::ios::binary );
	if ( !file )
	{
		g::console.warn( "config not found: {}", name );
		return false;
	}

	header hdr{};
	file.read( reinterpret_cast< char* >( &hdr ), sizeof( hdr ) );

	if ( hdr.magic != k_magic )
	{
		g::console.warn( "invalid config file: {}", name );
		return false;
	}

	if ( hdr.version != k_version ||
		 hdr.combat_size != sizeof( settings::combat ) ||
		 hdr.esp_size != sizeof( settings::esp ) ||
		 hdr.misc_size != sizeof( settings::misc ) )
	{
		g::console.warn( "outdated config: {}", name );
		return false;
	}

	file.read( reinterpret_cast< char* >( &settings::g_combat ), sizeof( settings::combat ) );
	file.read( reinterpret_cast< char* >( &settings::g_esp ), sizeof( settings::esp ) );
	file.read( reinterpret_cast< char* >( &settings::g_misc ), sizeof( settings::misc ) );

	g::console.success( "config loaded: {}", name );
	return true;
}

bool config::remove( const std::string& name ) const
{
	if ( name.empty( ) )
		return false;

	const auto path = this->get_config_path( name );

	if ( !std::filesystem::exists( path ) )
		return false;

	std::filesystem::remove( path );

	const auto def = this->get_default( );
	if ( def == name )
	{
		const auto default_path = this->get_config_dir( ) / ".default";
		std::filesystem::remove( default_path );
	}

	g::console.print( "config deleted: {}", name );
	return true;
}

void config::reset( )
{
	settings::g_combat = settings::combat{};
	settings::g_esp = settings::esp{};
	settings::g_misc = settings::misc{};

	g::console.print( "settings reset to defaults." );
}

std::vector<std::string> config::list( ) const
{
	std::vector<std::string> configs{};
	const auto dir = this->get_config_dir( );

	for ( const auto& entry : std::filesystem::directory_iterator( dir ) )
	{
		if ( entry.is_regular_file( ) && entry.path( ).extension( ) == ".cfg" )
		{
			configs.push_back( entry.path( ).stem( ).string( ) );
		}
	}

	std::sort( configs.begin( ), configs.end( ) );
	return configs;
}

bool config::set_default( const std::string& name ) const
{
	if ( name.empty( ) )
		return false;

	const auto path = this->get_config_dir( ) / ".default";

	std::ofstream file( path );
	if ( !file )
		return false;

	file << name;

	g::console.success( "default config set: {}", name );
	return true;
}

std::string config::get_default( ) const
{
	const auto path = this->get_config_dir( ) / ".default";

	std::ifstream file( path );
	if ( !file )
		return {};

	std::string name{};
	std::getline( file, name );
	return name;
}

bool config::load_default( )
{
	const auto name = this->get_default( );
	if ( name.empty( ) )
		return false;

	return this->load( name );
}
