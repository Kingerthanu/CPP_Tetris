#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>

class Shader
{

	private:
		GLuint progID;

		std::string loadShaderSource(const std::string& filePath)
		{

			std::ifstream file(filePath);

			if (!file.is_open())
			{
				std::cerr << "Failed To Open Shader Source File: " << filePath << std::endl;
				return "";
			}

			std::stringstream buffer;

			buffer << file.rdbuf();

			return buffer.str();

		}

		GLuint compileShader(const std::string& source, const GLenum shaderType)
		{

			GLuint shader = glCreateShader(shaderType);

			const char* src = source.c_str();
			
			glShaderSource(shader, 1, &src, nullptr);
			glCompileShader(shader);

			// Check Compilation 
			GLint success;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

			if (!success)
			{
				char logReport[512];
				glGetShaderInfoLog(shader, 512, nullptr, logReport);
				std::cerr << "Shader Compilation Failed: " <<logReport << std::endl;
			}

			return shader;

		}

		
	public:
		bool loadShader(const std::string& vertexPath, const std::string& fragmentPath)
		{

			std::string vertexSource = loadShaderSource(vertexPath);
			std::string fragmentSource = loadShaderSource(fragmentPath);

			if (vertexSource.empty() || fragmentSource.empty())
			{
				return false;
			}

			GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
			GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

			// Create Program
			this->progID = glCreateProgram();
			glAttachShader(this->progID, vertexShader);
			glAttachShader(this->progID, fragmentShader);
			glLinkProgram(this->progID);

			// Check Linking 
			GLint success;
			glGetProgramiv(this->progID, GL_LINK_STATUS, &success);

			if (!success)
			{
	
				char logReport[512];
				glGetProgramInfoLog(this->progID, 512, nullptr, logReport);
				std::cerr << "Program Linking Failed: " << logReport << std::endl;

				glDeleteShader(vertexShader);
				glDeleteShader(fragmentShader);
				glDeleteProgram(this->progID);

				return false;

			}

			// Cleanup Intermediate Shaders
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			return true;

		}

		GLuint getProgID() const
		{
			return this->progID;
		}

		void use()
		{

			glUseProgram(this->progID);

		}

		~Shader()
		{

			glDeleteProgram(this->progID);

		}

};