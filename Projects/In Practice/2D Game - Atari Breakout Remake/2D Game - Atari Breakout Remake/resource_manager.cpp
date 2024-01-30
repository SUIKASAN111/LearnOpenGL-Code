#include "resource_manager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Instantiate static variables
std::unordered_map<std::string, Shader>		ResourceManager::Shaders;
std::unordered_map<std::string, Texture2D>	ResourceManager::Textures;

Shader ResourceManager::LoadShader(const std::string& shadersFile, std::string name)
{
	Shaders[name] = loadShaderFromFile(shadersFile);
	return Shaders[name];
}

Shader& ResourceManager::GetShader(std::string name)
{
	return Shaders[name];
}

Texture2D ResourceManager::LoadTexture(const std::string& textureFile, bool alpha, std::string name)
{
	Textures[name] = loadTextureFromFile(textureFile, alpha);
	return Textures[name];
}

Texture2D& ResourceManager::GetTexture(std::string name)
{
	return Textures[name];
}

void ResourceManager::Clear()
{
	// (properly) delete all shaders
	for (auto it : Shaders)
	{
		it.second.~Shader();
	}
	// (properly) delete all textures
	for (auto it : Textures)
	{
		it.second.~Texture2D();
	}
}

Shader ResourceManager::loadShaderFromFile(const std::string& shadersFile)
{
	return Shader(shadersFile);
}

Texture2D ResourceManager::loadTextureFromFile(const std::string& textureFile, bool alpha)
{
	// create texture object
	Texture2D texture;
	if (alpha)
	{
		texture.SetInternalFormat(GL_RGBA);
		texture.SetImageFormat(GL_RGBA);
	}
	// load image
	int width, height, nrChannels;
	unsigned char* data = stbi_load(textureFile.c_str(), &width, &height, &nrChannels, 0);
	// now generate texture
	texture.Generate(width, height, data);
	// and finallt free image data
	stbi_image_free(data);
	return texture;
}
