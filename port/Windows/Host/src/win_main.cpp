#include "kya.h"
#include "renderer.h"
#include "DebugMenu.h"
#include "DebugRenderer.h"

int main(int argc, char** argv) {
	Renderer::Setup();
	DebugMenu::SetupRenderer();
	DebugMenu::Init();
	main_internal(argc, argv);
}