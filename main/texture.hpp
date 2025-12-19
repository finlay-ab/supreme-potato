#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../third_party/stb/include/stb_image.h"
#include <cassert>
#include "../support/error.hpp"

GLuint load_texture_2d(char const* aPath);