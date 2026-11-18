#include "Map.hpp"
#include <assert.h>
#include <iostream>

namespace impera {

	int Map::getWidth() const {
		return m_Width;
	}
	
	int Map::getHeight() const {
		return m_Height;
	}
	
	void Map::setProvinceId( std::size_t index, ProvinceId id) {
		m_ProvinceIds[index] = id;
	}
		
	void Map::setSize(int width, int height) {
		m_Width = width;
		m_Height = height;
		m_ProvinceIds.resize(width*height);
	}
		
	void Map::addCountry(const Country& country) {
		m_Countries.push_back(country);
	}
		
	void Map::addProvince(const Province& province) {
		m_Provinces.push_back(province);
	}
		
	const std::vector<ProvinceId>& Map::getProvinceIds() const {
		return m_ProvinceIds;
	}
		
	const std::vector<Province>& Map::getProvinces() const {
		return m_Provinces;
	}
		
	const std::vector<Country>& Map::getCountries() const {
		return m_Countries;
	}

	const Province* Map::getProvince(ProvinceId id) const {
		if(id == 0 || id > m_Provinces.size()) {
			return nullptr;
		}
		const auto& p = m_Provinces[id - 1];
		return p.id == id ? &p : nullptr;
	}
		
	Province* Map::getProvinceById(ProvinceId id) {
		if(id == 0 || id > m_Provinces.size()) {
			return nullptr;
		}
		auto& p = m_Provinces[id - 1];
		return p.id == id ? &p : nullptr;
	}
		
	const Country* Map::getCountry(CountryId id) const {
		for(const auto& c: m_Countries) {
			if(c.id == id) {
				return &c;
			}
		}
		return nullptr;
	}
		
	int Map::getProvinceId(int x, int y) const {
		if(x < 0 || x >= m_Width) {
			return -1;
		}
		if(y < 0 || y >= m_Height) {
			return -1;
		}
		return m_ProvinceIds[y*m_Width+x];
	}
		
	Province* Map::getProvince(const SDL_FPoint& coord) {
		auto id = getProvinceId(coord.x,coord.y);
		if(id <= 0 || id > m_Provinces.size()) {
			return nullptr;
		}
		auto& p = m_Provinces[id - 1];
		return p.id == id ? &p : nullptr;
	}
}