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

		/*
		
			Desc: Loads A Shader Source File's Contents Into Memory Utilizing A std::string
			Buffer.

			Preconditions:
				1.) filePath Is A Valid .vert Or .frag File Path

			Postconditions:
				1.) Will Return A String Containing The Text Inside The filePath
				2.) If A Invalid File, Will Return & Notify

		*/
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


		/*

			Desc: Will Compile A Shader Into Executable Code For The GPU Based
			On Provided source Code As Well As The shaderType It'll Be Used For.

			Preconditions:
				1.) source Is Valid .vert Or .frag Shader Source Code
				2.) shaderType Is Either The GL_VERTEX Or GL_FRAGMENT GLenum Type

			Postconditions:
				1.) If Compilation Succeeds, Will Return A GLuint Pointing To Where The Shader Is Held
				2.) Compiled Shader Code Will Now Be Saved In Memory
				3.) If Compilation Fails, Will Return & Notify

		*/
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
		
		/*

			Desc: Main Initialization Function Which Will Ingest The Paths To Both
			Our .vert & .frag Shader And Will Grab The Source Code From The Given Files
			Then Compile The Source Code Into Executable Logic For The GPU.

			Preconditions:
				1.) vertexPath & fragmentPath Are Both Valid Files
				2.) vertexPath & fragmentPath Both Contain Syntactically Correct Shader Code

			Postconditions:
				1.) Both Our .vert & .frag Shader Will Be Live And Associated With A Shader
				2.) If Loading Fails, Will Return & Notify
		
		*/
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


		/*

			Desc: Returns The Position In Which The Shader Is Held Within Memory In OpenGL.
		
			Preconditions:
				1.) The Shader Has Been Initialized And Compiled
				
			Postconditions:
				1.) Return The Location In Where Our Shader Resides In Memory

		*/
		GLuint getProgID() const
		{

			return this->progID;

		}

		
		/*
		
			Desc: Will Activate The Given Shader To Be Utilized In The Rendering Of
			Vertex Data.

			Preconditions:
				1.) The Shader Has Been Initialized And Compiled
		
			Postconditions:
				1.) Rendering Will Now Utilize This Given Shader For It's Rendering

		*/
		void use()
		{

			glUseProgram(this->progID);

		}


		/*

			Desc: Cleanup Routine For Shader Instance In Which We Delete Our Shader
			Process.

			Preconditions:
				1.) The Shader Has Been Initialized And Compiled

			Postconditions:
				1.) Will Delete The Given Compiled Shader Code From Memory

		*/
		~Shader()
		{

			glDeleteProgram(this->progID);

		}

};