
#include <ImperaEngine/ImperaEngine.hpp>

int main(int argc, char** argv) {
	
	impera::ApplicationDefinition applicationDefinition;
	applicationDefinition.name = "ImperaEngine Example";
	
	return impera::ImperaEngine().run(applicationDefinition, argc, argv);

}