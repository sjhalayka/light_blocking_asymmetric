// Source code by Shawn Halayka
// Source code is in the public domain


#ifndef MAIN_H
#define MAIN_H


#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <cstdlib>
using namespace std;

#include <GL/glew.h>
#include <GL/glut.h>

#ifdef _MSC_VER
#pragma comment(lib, "freeglut")
#pragma comment(lib, "glew32")
#endif

void compute(GLuint& tex_output, 
	GLuint& tex_input,
	GLuint& tex_light_input,
	GLuint& tex_light_blocking_input,
	GLint tex_w, GLint tex_h, 
	GLuint& compute_shader_program, 
	vector<float> &output_pixels, 
	const vector<float> &input_pixels,
	const vector<float>& input_light_pixels,
	const vector<float>& input_light_blocking_pixels)
{
	glEnable(GL_TEXTURE_2D);


	glActiveTexture(GL_TEXTURE1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, &input_pixels[0]);
	glBindImageTexture(1, tex_input, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glActiveTexture(GL_TEXTURE2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, &input_light_pixels[0]);
	glBindImageTexture(2, tex_light_input, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glActiveTexture(GL_TEXTURE3);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, &input_light_blocking_pixels[0]);
	glBindImageTexture(3, tex_light_blocking_input, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glUniform1i(glGetUniformLocation(compute_shader_program, "input_image"), 1); 
	glUniform1i(glGetUniformLocation(compute_shader_program, "input_light_image"), 2);
	glUniform1i(glGetUniformLocation(compute_shader_program, "input_light_blocking_image"), 3);
	glUniform2i(glGetUniformLocation(compute_shader_program, "u_size"), tex_w, tex_h);

	// Run compute shader
	glDispatchCompute((GLuint)tex_w, (GLuint)tex_h, 1);

	// Wait for compute shader to finish
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	// Copy output pixel array to CPU as texture 0
	glActiveTexture(GL_TEXTURE0);
	glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, &output_pixels[0]);
}

void init_textures(GLuint &tex_output, GLuint &tex_input, GLuint &tex_light_input, GLuint &tex_light_blocking_input, GLuint tex_w, GLuint tex_h)
{
	// Generate and allocate output texture
	glGenTextures(1, &tex_output);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_output);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	// Generate input texture
	glGenTextures(1, &tex_input);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex_input);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &tex_light_input);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tex_light_input);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &tex_light_blocking_input);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, tex_light_blocking_input);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}



bool compile_and_link_compute_shader(const char *const file_name, GLuint &program)
{
	// Read in compute shader contents
	ifstream infile(file_name);

	if (infile.fail())
	{
		cout << "Could not open " << file_name << endl;
		return false;
	}

	string shader_code;
	string line;

	while (getline(infile, line))
	{
		shader_code += line;
		shader_code += "\n";
	}

	// Compile compute shader
	const char* cch = 0;
	GLint status = GL_FALSE;

	GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(shader, 1, &(cch = shader_code.c_str()), NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (GL_FALSE == status)
	{
		string status_string = "Compute shader compile error.\n";
		vector<GLchar> buf(4096, '\0');
		glGetShaderInfoLog(shader, 4095, 0, &buf[0]);

		for (size_t i = 0; i < buf.size(); i++)
			if ('\0' != buf[i])
				status_string += buf[i];

		status_string += '\n';

		cout << status_string << endl;

		glDeleteShader(shader);

		return false;
	}

	// Link compute shader
	program = glCreateProgram();
	glAttachShader(program, shader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	if (GL_FALSE == status)
	{
		string status_string = "Program link error.\n";
		vector<GLchar> buf(4096, '\0'); 
		glGetShaderInfoLog(program, 4095, 0, &buf[0]);

		for (size_t i = 0; i < buf.size(); i++)
			if ('\0' != buf[i])
				status_string += buf[i];

		status_string += '\n';

		cout << status_string << endl;

		glDetachShader(program, shader);
		glDeleteShader(shader);
		glDeleteProgram(program);

		return false;
	}

	// The shader is no longer needed now that the program
	// has been linked
	glDetachShader(program, shader);
	glDeleteShader(shader);

	return true;
}

bool init_opengl_4_3(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA);
	int glut_window_handle = glutCreateWindow("");

	if (!(GLEW_OK == glewInit() &&
		GLEW_VERSION_4_3))
	{
		return false;
	}

	return true;
}

bool init_gl(int argc, char** argv, 
	GLint tex_w, GLint tex_h, 
	GLuint& compute_shader_program)
{
	// Initialize OpenGL
	if (false == init_opengl_4_3(argc, argv))
	{
		cout << "OpenGL 4.3 initialization failure" << endl;
		return false;
	}
	else
	{
		cout << "OpenGL 4.3 initialization OK" << endl;
	}

	if (!GLEW_ARB_texture_non_power_of_two)
	{
		cout << "System does not support non-POT textures" << endl;
		return false;
	}
	else
	{
		cout << "System supports non-POT textures" << endl;
	}

	if (!GLEW_ARB_texture_rectangle)
	{
		cout << "System does not support rectangular textures" << endl;
		return false;
	}
	else
	{
		cout << "System supports rectangular textures" << endl;
	}

	



	// Initialize the compute shader
	compute_shader_program = 0;

	if (false == compile_and_link_compute_shader("shader.comp", compute_shader_program))
	{
		cout << "Failed to initialize compute shader" << endl;
		return false;
	}


	cout << "Texture size: " << tex_w << "x" << tex_h << endl;

	// Check that the global workgrounp count is greater than or equal to the input/output textures
	GLint global_workgroup_count[2];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &global_workgroup_count[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &global_workgroup_count[1]);

	cout << "Max global workgroup size: " << global_workgroup_count[0] << "x" << global_workgroup_count[1] << endl;

	if (tex_w > global_workgroup_count[0])
	{
		cout << "Texture width " << tex_w << " is larger than max " << global_workgroup_count[0] << endl;
		return false;
	}

	if (tex_h > global_workgroup_count[1])
	{
		cout << "Texture height " << tex_h << " is larger than max " << global_workgroup_count[1] << endl;
		return false;
	}

	
	// Use the compute shader
	glUseProgram(compute_shader_program);

	return true;
}

void delete_all(GLuint& tex_output, GLuint& tex_input, GLuint& tex_light_input, GLuint& tex_light_blocking_input, GLuint& compute_shader_program)
{
	glDeleteTextures(1, &tex_output);
	glDeleteTextures(1, &tex_input);
	glDeleteTextures(1, &tex_light_input);
	glDeleteTextures(1, &tex_light_blocking_input);

	glDeleteProgram(compute_shader_program);
}

#endif