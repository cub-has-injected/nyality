#pragma once

class config
{
public:
	bool save( const std::string& name ) const;
	bool load( const std::string& name );
	bool remove( const std::string& name ) const;
	void reset( );

	[[nodiscard]] std::vector<std::string> list( ) const;

	bool set_default( const std::string& name ) const;
	[[nodiscard]] std::string get_default( ) const;
	bool load_default( );

private:
	[[nodiscard]] std::filesystem::path get_config_dir( ) const;
	[[nodiscard]] std::filesystem::path get_config_path( const std::string& name ) const;

	static constexpr std::uint32_t k_magic{ 0x4E594C54 };
	static constexpr std::uint32_t k_version{ 1 };

	struct header
	{
		std::uint32_t magic;
		std::uint32_t version;
		std::uint32_t combat_size;
		std::uint32_t esp_size;
		std::uint32_t misc_size;
	};
};
