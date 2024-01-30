#ifndef TEXTURE_H
#define TEXTURE_H

#include <cstdint>

#include <glad\glad.h>

// Texture2D is able to store and configure a texture in OpenGL.
// It also hosts utility functions for easy management.
class Texture2D
{
public:
	// constructor (set default texture modes)
	Texture2D();
	~Texture2D() = default;
	// generates texture from image data
	void Generate(uint32_t width, uint32_t height, unsigned char* data);
	// binds the texture as the current active GL_TEXTURE_2D texture object
	void Bind() const;

	inline uint32_t& GetInternalFormat()
	{
		return this->Internal_Format;
	}

	inline void SetInternalFormat(uint32_t texFormat)
	{
		this->Internal_Format = texFormat;
	}

	inline uint32_t& GetImageFormat()
	{
		return this->Image_Format;
	}

	inline void SetImageFormat(uint32_t imgFormat)
	{
		this->Image_Format = imgFormat;
	}

	inline uint32_t GetID() { return m_ID; }

private:
	// holds the ID of the texture object, used for all texture operations to reference to this particular texture
	uint32_t m_ID;
	// texture image dimensions
	uint32_t Width, Height;
	// texture Format
	uint32_t Internal_Format; // format of texture object
	uint32_t Image_Format; // format of loaded image
	// texture configuration
	uint32_t Wrap_S; // wrapping mode on S axis
	uint32_t Wrap_T; // wrapping mode on T axis
	uint32_t Filter_Min; // filter mode if texture pixels < screen pixels
	uint32_t Filter_Max; // filter mode if texture pixels > screen pixels
};

#endif // !TEXTURE_H