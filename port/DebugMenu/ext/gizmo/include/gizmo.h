#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <memory>

namespace Gizmo {
	class Widget {
		std::string label;
		void* state;

	public:
		Widget(const char* inLabel);

		glm::mat4 Show();
	};
}