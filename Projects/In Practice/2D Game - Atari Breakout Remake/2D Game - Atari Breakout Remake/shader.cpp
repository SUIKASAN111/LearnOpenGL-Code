#include "shader.h"

#include <iostream>
#include <fstream>
#include <sstream>


Shader::Shader(const std::string& filepath)
	: m_ID(0), m_filepath(filepath)
{
	Compile(ParseShader(m_filepath));
}

Shader& Shader::Use()
{
	glUseProgram(this->m_ID);
	return *this;
}

void Shader::Unuse()
{
	glUseProgram(0);
}

void Shader::DeleteShaderProgram()
{
	glDeleteProgram(this->m_ID);
}

Shader::ShaderProgramSource Shader::ParseShader(const std::string& filepath)
{
	std::ifstream stream(filepath);

	enum class ShaderType
	{
		NONE = -1, VERTEX = 0, FRAGMENT = 1, GEOMETRY = 2
	};

	std::string line;
	std::stringstream ss[3];
	ShaderType type = ShaderType::NONE;
	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
				// set mode to vertex
				type = ShaderType::VERTEX;
			else if (line.find("fragment") != std::string::npos)
				// set mode to fragment
				type = ShaderType::FRAGMENT;
			else if(line.find("geometry") != std::string::npos)
				// set mode to geometry
				type = ShaderType::GEOMETRY;
		}
		else
		{
			ss[static_cast<int>(type)] << line << '\n';
		}
	}

	return { ss[0].str(), ss[1].str(), ss[2].str() };
}

void Shader::Compile(Shader::ShaderProgramSource shaderProgramSrc)
{
	uint32_t vert, frag, geo;
	// src
	const char* src[3] = {
		shaderProgramSrc.vertexSource.c_str(),
		shaderProgramSrc.fragmentSource.c_str(),
		shaderProgramSrc.geometrySource.empty() ? nullptr : shaderProgramSrc.geometrySource.c_str()
	};

	// vertex shader
	vert = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert, 1, &src[0], nullptr);
	glCompileShader(vert);
	checkCompileErrors(vert, "VERTEX");
	// fragment shader
	frag = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag, 1, &src[1], nullptr);
	glCompileShader(frag);
	checkCompileErrors(frag, "FRAGMENT");
	// geometry shader
	if (src[2] != nullptr)
	{
		geo = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geo, 1, &src[2], nullptr);
		glCompileShader(geo);
		checkCompileErrors(geo, "GEOMETRY");
	}
	// shader program
	this->m_ID = glCreateProgram();
	glAttachShader(this->m_ID, vert);
	glAttachShader(this->m_ID, frag);
	if (src[2] != nullptr)
	{
		glAttachShader(this->m_ID, geo);
	}
	glLinkProgram(this->m_ID);
	checkCompileErrors(this->m_ID, "PROGRAM");
	// delete the shaders as they're linked into our program now and no longer necessary
	glDeleteShader(vert);
	glDeleteShader(frag);
	if (src[2] != nullptr)
	{
		glDeleteShader(geo);
	}
	//std::cout << "success" << std::endl;
}

void Shader::SetFloat(const std::string name, float value, bool useShader)
{
	if (useShader)
		this->Use();
	glUniform1f(glGetUniformLocation(this->m_ID, name.c_str()), value);
}

void Shader::SetInteger(const std::string name, int value, bool useShader)
{
	if (useShader)
		this->Use();
	glUniform1i(glGetUniformLocation(this->m_ID, name.c_str()), value);
}

void Shader::SetVector2f(const std::string name, float x, float y, bool useShader)
{
	if (useShader)
		this->Use();
	glUniform2f(glGetUniformLocation(this->m_ID, name.c_str()), x, y);
}

void Shader::SetVector2f(const std::string name, const glm::vec2& value, bool useShader)
{
	SetVector2f(name, value.x, value.y, useShader);
}

void Shader::SetVector3f(const std::string name, float x, float y, float z, bool useShader)
{
	if (useShader)
		this->Use();
	glUniform3f(glGetUniformLocation(this->m_ID, name.c_str()), x, y, z);
}

void Shader::SetVector3f(const std::string name, const glm::vec3& value, bool useShader)
{
	SetVector3f(name, value.x, value.y, value.z, useShader);
}

void Shader::SetVector4f(const std::string name, float x, float y, float z, float w, bool useShader)
{
	if (useShader)
		this->Use();
	glUniform4f(glGetUniformLocation(this->m_ID, name.c_str()), x, y, z, w);
}

void Shader::SetVector4f(const std::string name, const glm::vec4& value, bool useShader)
{
	SetVector4f(name, value.x, value.y, value.z, value.w, useShader);
}

void Shader::SetMatrix4(const std::string name, const glm::mat4& matrix, bool useShader)
{
	if (useShader)
		this->Use();
	glUniformMatrix4fv(glGetUniformLocation(this->m_ID, name.c_str()), 1, false, glm::value_ptr(matrix));
}

void Shader::checkCompileErrors(uint32_t object, std::string type)
{
	int success;
	char infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(object, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(object, 1024, nullptr, infoLog);
			std::cout << "| ERROR::SHADER: Compile-time error: Type: " << type << "\n"
				<< infoLog << "\n -- ------------------------------------------------------ --"
				<< std::endl;
		}
	}
	else
	{
		glGetProgramiv(object, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(object, 1024, nullptr, infoLog);
			std::cout << "| ERROR::SHADER: Link-time error: Type: " << type << "\n"
				<< infoLog << "\n -- ------------------------------------------------------ --"
				<< std::endl;
		}
	}
}