#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <unordered_map>
#include <string>

#include <glad\glad.h>

#include "shader.h"
#include "texture.h"


// A static singleton ResourceManager class that hosts several
// functions to load Textures and Shaders. Each loaded texture
// and/or shader is also stored for future reference by string
// handles. All functions and resources are static and no 
// public constructor is defined.
class ResourceManager
{
public:
	// resource storage
	static std::unordered_map<std::string, Shader>	  Shaders;
	static std::unordered_map<std::string, Texture2D> Textures;
	// loads (and generates) a shader program from file loading shaders' source code. If #shader geometry exists, it also loads a geometry shader
	static Shader	  LoadShader(const std::string& shadersFile, std::string name);
	// retrieves a stored shader
	static Shader&	  GetShader(std::string name);
	// loads (and generates) a texture from file
	static Texture2D  LoadTexture(const std::string& textureFile, bool alpha, std::string name);
	// retrieves a stored texture
	static Texture2D& GetTexture(std::string name);
	// properly de-allocates all loaded resources
	static void		  Clear();
private:
	// private constructor, that is we do not want any actual resource manager objects. Its members and functions should be publicly available (static).
	ResourceManager() = delete;
	// loads and generates a shader from file
	static Shader	 loadShaderFromFile(const std::string& shadersFile);
	// loads a single texture from file
	static Texture2D loadTextureFromFile(const std::string& textureFile, bool alpha);
};

#endif // !RESOURCE_MANAGER_H