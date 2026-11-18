#ifndef IMPERAENGINE_MAP_MAP_HPP
#define IMPERAENGINE_MAP_MAP_HPP

#include <SDL3/SDL.h>
#include <unordered_set>
#include <vector>

namespace impera {

	using ProvinceId = uint16_t;
	using CountryId = uint16_t;
	
	// TODO: Remove this, use SDL_Color
	struct Color {
		uint8_t r,g,b,a;
	};
	
	// TODO: Change Color to SDL_Color
	struct Country {
		CountryId id;
		Color color;
	};
	
	struct Province {
		ProvinceId id;
		CountryId owner;
		float minX, minY;
		float maxX, maxY;
		std::unordered_set<int> chunks;
	};
	

	class Map {
		
		int m_Width = 0;
		int m_Height = 0;
		std::vector<ProvinceId> m_ProvinceIds;
		std::vector<Province> m_Provinces;
		std::vector<Country> m_Countries;
		
		public:
		
		int getWidth() const;
		int getHeight() const;
		
		void setProvinceId( std::size_t index, ProvinceId id);
		
		void setSize(int width, int height);
		
		void addCountry(const Country& country);
		
		void addProvince(const Province& province);
		
		const std::vector<ProvinceId>& getProvinceIds() const;
		
		const std::vector<Province>& getProvinces() const;
		
		const std::vector<Country>& getCountries() const;

		const Province* getProvince(ProvinceId id) const;
		
		Province* getProvinceById(ProvinceId id);
		
		const Country* getCountry(CountryId id) const;
		
		int getProvinceId(int x, int y) const;
		
		Province* getProvince(const SDL_FPoint& coord);
				
	};
	
}

#endif