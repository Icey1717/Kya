#include "kya.h"
#include "renderer.h"
#include "DebugRenderer.h"

int main(int argc, char** argv) {
	Renderer::Setup();
	DebugMenu::SetupRenderer();
	main_internal(argc, argv);
}