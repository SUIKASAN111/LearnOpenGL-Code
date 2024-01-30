#ifndef SHADER_H
#define SHADER_H

#include <string>

#include "glad/glad.h"
#include "glm\glm.hpp"
#include "glm\gtc\type_ptr.hpp"

class Shader
{
public:
	// constructor/destructor
	Shader() = default;
	Shader(const std::string& filepath);
	~Shader() = default;

	// use/unuse the shader
	Shader& Use();
	void Unuse();

	// delete shader program
	void DeleteShaderProgram();

	// utility functions
	void SetFloat(const std::string name, float value, bool useShader = false);
	void SetInteger(const std::string name, int value, bool useShader = false);
	void SetVector2f(const std::string name, float x, float y, bool useShader = false);
	void SetVector2f(const std::string name, const glm::vec2& value, bool useShader = false);
	void SetVector3f(const std::string name, float x, float y, float z, bool useShader = false);
	void SetVector3f(const std::string name, const glm::vec3& value, bool useShader = false);
	void SetVector4f(const std::string name, float x, float y, float z, float w, bool useShader = false);
	void SetVector4f(const std::string name, const glm::vec4& value, bool useShader = false);
	void SetMatrix4(const std::string name, const glm::mat4& matrix, bool useShader = false);

	inline uint32_t GetID() { return m_ID; }

private:
	// shader id
	uint32_t m_ID;
	// filepath
	std::string m_filepath;
	// shader source
	struct ShaderProgramSource
	{
		std::string vertexSource;
		std::string fragmentSource;
		std::string geometrySource;
	};

	// get ShaderProgramSource from .shader file
	ShaderProgramSource ParseShader(const std::string& filepath);

	// compiles the shader from given source code
	void Compile(Shader::ShaderProgramSource shaderProgramSrc);

	// checks if compilation or linking failed and if so, print the error logs
	void checkCompileErrors(uint32_t object, std::string type);
};

#endif // !SHADER_H

