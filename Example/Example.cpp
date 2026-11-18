
#include <ImperaEngine/ImperaEngine.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

int main(int argc, char** argv) {
	
	auto logger = spdlog::basic_logger_mt("ImperaEngineExample", "ImperaEngineExample.log.txt", true);
	spdlog::set_default_logger(logger);
	
	spdlog::info("Init SDL");
	if(!SDL_Init(SDL_INIT_VIDEO)) {
		spdlog::error("Failed to initialise SDL: {}", SDL_GetError());
		return -1;
	}
	
	spdlog::info("Create SDL_Window");
	SDL_Window* window = SDL_CreateWindow("Impera Engine Example", 1280, 720, 0);
	if(!window) {
		spdlog::error("Failed to create SDL_Window: {}", SDL_GetError());
		SDL_Quit();
		return -2;
	}
	
	spdlog::info("Create SDL_Renderer");
	SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
	if(!renderer) {
		spdlog::error("Failed to create SDL_Renderer: {}", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
	}
	
	spdlog::info("Create Impera::Map");
	impera::Map map;
	
	impera::JsonFile json;
	if(!json.open("./Data/Map/ColorToProvince.json")) {
		spdlog::error("Failed to load ColorToProvince.json");
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return -3;
	}
	
	std::unordered_map<uint32_t, impera::ProvinceId> colorToProvinceMap;
	
	if (json.contains("Entries")) {
		for (const auto& entry : json["Entries"]) {
			if (entry.contains("Color") && entry.contains("Id")) {
				const auto color = entry["Color"].get<uint32_t>();
				const auto provinceId = entry["Id"].get<impera::ProvinceId>();
				colorToProvinceMap[color] = provinceId;
			}
		}
	}
	
	if(!json.open("./Data/Data/Provinces.json")) {
		spdlog::error("Failed to load Provinces.json");
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return -3;
	}
	
	if (json.contains("Provinces")) {
		for (const auto& province : json["Provinces"]) {
			if (province.contains("Id") && province.contains("Owner")) {
				const auto id = province["Id"].get<impera::ProvinceId>();
				const auto owner = province["Owner"].get<impera::CountryId>();
				map.addProvince( {id, owner} );
			}
		}
	}
	
	if(!json.open("./Data/Data/Countries.json")) {
		spdlog::error("Failed to load Countries.json");
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return -3;
	}

	if (json.contains("Countries")) {
		for (const auto& country : json["Countries"]) {
			if (country.contains("Id") && country.contains("R") && country.contains("G") && country.contains("B")) {
				const auto id = country["Id"].get<impera::CountryId>();
				const auto r = country["R"].get<uint8_t>();
				const auto g = country["G"].get<uint8_t>();
				const auto b = country["B"].get<uint8_t>();
				impera::Country c;
				c.id = id;
				c.color = {r,g,b,255};
				map.addCountry(c);
			}
		}
	}

	spdlog::info("Compile Map");
	impera::MapCompiler mapCompiler;
	if(!mapCompiler.compile("./Data/Map/Map.bmp", colorToProvinceMap, map)) {
		spdlog::error("Failed to compile map");
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return -4;
	}
	
	spdlog::info("Create Impera::MapRenderer");
	impera::MapRenderer mapRenderer;
	if(!mapRenderer.init(&map)) {
		spdlog::error("Failed to intialise Impera::MapRenderer");
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return -4;
	}
		
	spdlog::info("Create Impera::Camera");
	impera::Camera camera;
	
	bool isOpen = true;
	SDL_Event event;
	float zoomFactor = 1.f;
	while(isOpen) {
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_EVENT_QUIT:
					isOpen = false;
				break;
				case SDL_EVENT_MOUSE_MOTION:
				{	
					impera::Province* tmp = map.getProvince( camera.screenToWorld(event.motion.x, event.motion.y) );
					mapRenderer.setHoveredProvince( tmp );
				}
				break;
				case SDL_EVENT_KEY_DOWN:
					if(event.key.key == SDLK_UP) {
						camera.y -= 15.0f;
					} else if( event.key.key == SDLK_DOWN) {
						camera.y += 15.0f;
					} else if( event.key.key == SDLK_LEFT) {
						camera.x -= 15.0f;
					} else if( event.key.key == SDLK_RIGHT) {
						camera.x += 15.0f;
					}
				break;
				case SDL_EVENT_MOUSE_WHEEL:
					if(event.wheel.y > 0) {
						zoomFactor += 0.05;
					} else {
						zoomFactor -= 0.05;
					}
					camera.zoomAt( event.wheel.mouse_x, event.wheel.mouse_y, zoomFactor);
				break;
				case SDL_EVENT_MOUSE_BUTTON_DOWN:
				{
					mapRenderer.selectProvince();
				}
				break;
			}
		}
		SDL_RenderClear(renderer);
		mapRenderer.draw(renderer, camera);
		SDL_RenderPresent(renderer);
	}
	
	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	
	return 0;
}