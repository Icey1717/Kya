#include "kya.h"
#include "renderer.h"
#include "DebugMenu.h"
#include "DebugRenderer.h"

#include "port/input.h"
#include "Texture.h"
#include "Mesh.h"

#include <csignal>
#include "log.h"

void signal_handler(int signal)
{
	Log::GetInstance().ForceFlush();
}

int main(int argc, char** argv) {
	std::signal(SIGINT, signal_handler);

	Renderer::Setup();
	DebugMenu::SetupRenderer();
	DebugMenu::Init();
	Renderer::Kya::TextureLibrary::Init();
	Renderer::Kya::MeshLibrary::Init();
	Input::SetInputFunctions(DebugMenu::GetInputFunctions());
	main_internal(argc, argv);
}