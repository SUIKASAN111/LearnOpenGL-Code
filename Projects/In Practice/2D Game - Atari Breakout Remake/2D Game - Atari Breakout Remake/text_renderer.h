#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <unordered_map>

#include <glad\glad.h>
#include <glm\glm.hpp>

#include "texture.h"
#include "shader.h"

// Holds all state information relevant to a character as loaded using FreeType
struct Character
{
	uint32_t TextureID; // ID handle of the glyph texture
	glm::ivec2   Size;      // size of glyph
	glm::ivec2   Bearing;   // offset from baseline to left/top of glyph
	uint32_t Advance;   // horizontal offset to advance to next glyph
};

// A renderer class for rendering text displayed by a font loaded using the 
// FreeType library. A single font is loaded, processed into a list of Character
// items for later rendering.
class TextRenderer
{
public:
	// holds a list of pre-compiled Characters
	std::unordered_map<char, Character> Characters;
	// shader used for text rendering
	Shader TextShader;
	// constuctor
	TextRenderer(uint32_t width, uint32_t height);
	// pre-compiles a list of characters from the given font
	void Load(std::string font, uint32_t fontSize);
	// renders a string of text using the precompiled list of characters
	void RenderText(std::string text, float x, float y, float scale, glm::vec3 color = glm::vec3(1.0f));
private:
	// render state
	uint32_t m_VAO, m_VBO;
};

#endif // !TEXT_RENDERER_H
