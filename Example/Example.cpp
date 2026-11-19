
#include <ImperaEngine/ImperaEngine.hpp>


// This custom map mode renderer draws every province by the color of the owning country.
// It also draws the borders and changes the colors if the province is hovered or selected
// This implementation is more or less the default MapModeRenderer who gets used if you do not set a custom map mode renderer
class CustomMapModeRenderer : public impera::MapModeRenderer {

	impera::Color m_ProvinceBorderColor = {25,25,25,255};
	impera::Color m_ProvinceBorderColorHovered = {50,50,50,255};
	impera::Color m_ProvinceBorderColorSelected = {100,100,100,255};
	impera::Color m_CountryBorderColor = {150,150,150,255};
	impera::Color m_CountryBorderColorHovered = {200,200,200,255};
	impera::Color m_CountryBorderColorSelected = {225,225,225,255};

	public:

	virtual impera::Color getPixelColor(int x, int y, 
								const impera::MapChunk* chunk, 
								const impera::Province* province, 
								const impera::Map* map, 
								int chunkSize,
								bool hovered,
								bool selected) const {
			impera::Color color = {0,0,0,255};
			// Calculate the index of the border buffer
			const std::size_t borderIndex = (std::size_t(y) * chunkSize + x);
			// Only set a different color if there is a valid province
			if(province) {
				// If the pixel is a CountryBorder set the country border colors
				if( chunk->borderBuffer[borderIndex] == impera::COUNTRY_BORDER) {
					if(selected && province->enabled) {
						color = m_CountryBorderColorSelected;
					} else if(hovered && province->enabled) {
						color = m_CountryBorderColorHovered;
					} else {
						color = m_CountryBorderColor;
					}
				// If the pixel is a ProvinceBorder set the country border colors
				} else if(chunk->borderBuffer[borderIndex] == impera::PROVINCE_BORDER) {
					if(selected && province->enabled) {
						color = m_ProvinceBorderColorSelected;
					} else if(hovered && province->enabled) {
						color = m_ProvinceBorderColorHovered;
					} else {
						color = m_ProvinceBorderColor;
					}
				// If the pixel is no border, set the country colors
				} else if(chunk->borderBuffer[borderIndex] == impera::NO_BORDER) {
					
					// Get the owning country of the province
					const impera::Country* country = map->getCountry(province->owner);
				
					if(country) {
						color = country->color;
					}
					if(selected && province->enabled) {
						color.r += 50;
						color.g += 50;
						color.b += 50;
					}else if(hovered && province->enabled) {
						color.r += 25;
						color.g += 25;
						color.b += 25;
					}
				}
			}
			// If there is no valid province draw a default color
			return color;
			
		}


};

class MainGameState : public impera::GameState {

	public:
	
	virtual bool init() {
		// You can disable the drawing of the map with shouldDrawMap(false) ( IntroScreen a.s.o )
		shouldDrawMap(true);
		return true;
	}

};

// This function should check, if pixel [x,y] on [map] is a province border
bool my_province_border_check_func( impera::ProvinceId id, int x, int y, const impera::Map* map) {
	return false;
}

// This function should check, if pixel [x,y] on [map] is a country border
bool my_country_border_check_func( const impera::Country* country, int x, int y, const impera::Map* map) {
	return false;
}	

int main(int argc, char** argv) {
	
	// Create the engine object
	impera::ImperaEngine engine;
	engine.setTitle("ImperaEngine Example"); //Optional
	//engine.setProvinceBorderCheckFunction( &my_province_border_check_func ); //Optional
	//engine.setCountryBorderCheckFunction( &my_country_border_check_func ); //Optional
	
	// You need at least one MapModeRenderer and set it as default MapModeRenderer
	engine.registerMapModeRenderer<CustomMapModeRenderer>();
	engine.setDefaultMapModeRenderer<CustomMapModeRenderer>();
	
	// If you want to use the default map mode renderer:
	//engine.registerMapModeRenderer<impera::DefaultMapModeRenderer>();
	//engine.setDefaultMapModeRenderer<impera::DefaultMapModeRenderer>();
	
	// Set the active game state (not optional)
	engine.setActiveGameState<MainGameState>(); 
		
	// Run the engine (not optional)	
	return engine.run(argc, argv);

}