// Source code by Shawn Halayka
// Source code is in the public domain


#ifndef MAIN_H
#define MAIN_H

#include <Windows.h>
#include "sqlite/sqlite3.h"


#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <thread>
#include <regex>
using namespace std;

#include <GL/glew.h>

#include <glm/glm.hpp>

#include <opencv2/opencv.hpp>
using namespace cv;

#include <SDL.h>
#include <SDL_opengl.h>


#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/misc/cpp/imgui_stdlib.h"



#ifdef _MSC_VER
#pragma comment(lib, "opencv_world4100")
#pragma comment(lib, "freeglut")
#pragma comment(lib, "glew32")

#pragma comment(lib, "SDL2")
#pragma comment(lib, "SDL2main")
#pragma comment(lib, "OpenGL32")
#endif


#include "vertex_fragment_shader.h"
#include "font_draw.h"
#include "mysql_functions.h"

vertex_fragment_shader ortho_shader;



vector<string> std_strtok(const string& s, const string& regex_s)
{
	vector<string> tokens;

	regex r(regex_s);

	sregex_token_iterator iter(s.begin(), s.end(), r, -1);
	sregex_token_iterator end;

	while (iter != end)
	{
		if (*iter != "")
			tokens.push_back(*iter);

		iter++;
	}

	return tokens;
}




bool vector_screen_equals(const vector<screen>& a, const vector<screen>& b)
{
	if (a.size() == 0 || b.size() == 0)
		return false;

	if (a.size() != b.size())
		return false;

	for (size_t i = 0; i < a.size(); i++)
	{
		if (a[i].nickname != b[i].nickname)
			return false;
	}

	return true;
}




bool openFileDialog(std::string& fileName)
{
	// common dialog box structure, setting all fields to 0 is important
	OPENFILENAME ofn = { 0 };
	TCHAR szFile[9600] = { 0 };

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL; // <-- maybe needing HANDLE here ?
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"All files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST;// | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn) == TRUE)
	{
//		std::cout << "file selected : " << ofn.lpstrFile << std::endl;

		wstring f = wstring(ofn.lpstrFile);

		fileName = string(f.begin(), f.end());// ofn.lpstrFile);// [0] , ofn.lpstrFile[wcslen(ofn.lpstrFile) - 1]);
		return true;
	}

	return false;
}


std::vector<cv::Mat> splitImage(cv::Mat& image, int M, int N)
{
	// All images should be the same size ...
	int width = image.cols / M;
	int height = image.rows / N;
	// ... except for the Mth column and the Nth row
	int width_last_column = width + (image.cols % width);
	int height_last_row = height + (image.rows % height);

	std::vector<cv::Mat> result;

	for (int i = 0; i < N; ++i)
	{
		for (int j = 0; j < M; ++j)
		{
			// Compute the region to crop from
			cv::Rect roi(width * j,
				height * i,
				(j == (M - 1)) ? width_last_column : width,
				(i == (N - 1)) ? height_last_row : height);

			result.push_back(image(roi));
		}
	}

	return result;
}


cv::Mat imageCollage(std::vector<cv::Mat>& array_of_images, int M, int N)
{
	// All images should be the same size
	const cv::Size images_size = array_of_images[0].size();
	// Create a black canvas
	cv::Mat image_collage(images_size.height * N, images_size.width * M, array_of_images[0].type(), cv::Scalar(0, 0, 0));

	for (int i = 0; i < N; ++i)
	{
		for (int j = 0; j < M; ++j)
		{
			if (((i * M) + j) >= array_of_images.size())
				break;

			cv::Rect roi(images_size.width * j, images_size.height * i, images_size.width, images_size.height);
			array_of_images[(i * M) + j].copyTo(image_collage(roi));
		}
	}

	return image_collage;
}





class compute_chunk_params
{
public:
	bool previously_computed;
	int chunk_index_x;
	int chunk_index_y;
	int num_tiles_per_dimension;
	GLuint compute_shader_program;
	vector<float> output_pixels;
	Mat input_pixels;
	Mat input_light_pixels;
	Mat input_light_blocking_pixels;
	Mat input_coordinates_pixels;
	Mat output_light_pixels;
};




//
//void cpu_compute_chunk(
//	compute_chunk_params& ccp)
//{
//	const GLint tex_w_small = ccp.input_pixels.cols;
//	const GLint tex_h_small = ccp.input_pixels.rows;
//	const GLint tex_w_large = ccp.input_light_pixels.cols;
//	const GLint tex_h_large = ccp.input_light_pixels.rows;
//
//	ccp.output_pixels.resize(4 * tex_w_small * tex_h_small, 0.0f);
//
//	for (unsigned short int x = 0; x < tex_w_small; x++)
//	{
//		for (unsigned short int y = 0; y < tex_h_small; y++)
//		{
//			Vec4f pixelValue = ccp.input_pixels.at<Vec4f>(y, x);
//			const glm::vec4 image_sample = glm::vec4(pixelValue[0], pixelValue[1], pixelValue[2], pixelValue[3]);
//
//			if (image_sample.x == 0 && image_sample.y == 0 && image_sample.z == 0)
//			{
//				size_t index = 4 * (y * tex_w_small + x);
//
//				ccp.output_pixels[index + 0] = 0;
//				ccp.output_pixels[index + 1] = 0;
//				ccp.output_pixels[index + 2] = 0;
//				ccp.output_pixels[index + 3] = 0;
//
//				continue;
//			}
//
//
//			glm::ivec2 per_compute_pixel_coords = glm::ivec2(x, y);// glm::ivec2(gl_GlobalInvocationID.xy);
//
//			pixelValue = ccp.input_coordinates_pixels.at<Vec4f>(y, x);
//			const glm::vec2 coords_float = glm::vec2(pixelValue[0], pixelValue[1]);
//			const glm::ivec2 global_per_compute_pixel_coords = glm::ivec2(int(coords_float.r), int(coords_float.g));
//
//			glm::vec3 lighting = glm::vec3(0, 0, 0);
//
//			for (int i = 0; i < tex_w_large; i++)
//			{
//				for (int j = 0; j < tex_h_large; j++)
//				{
//					glm::ivec2 per_image_pixel_coords = glm::ivec2(i, j);
//
//					pixelValue = ccp.input_light_pixels.at<Vec4f>(j, i);
//					const glm::vec4 light_image_sample = glm::vec4(pixelValue[0], pixelValue[1], pixelValue[2], pixelValue[3]);
//
//					// Not a light
//					if (light_image_sample.x == 0 && light_image_sample.y == 0 && light_image_sample.z == 0)
//						continue;
//
//					pixelValue = ccp.input_light_blocking_pixels.at<Vec4f>(j, i);
//					const glm::vec4 light_blocking_image_sample = glm::vec4(pixelValue[0], pixelValue[1], pixelValue[2], pixelValue[3]);
//
//					// A light blocker doesn't receive light
//					if (light_blocking_image_sample.x == 1 && light_blocking_image_sample.y == 1 && light_blocking_image_sample.z == 1)
//						continue;
//
//					int num_steps = 0;
//
//					glm::vec2 pixel_diff;
//					pixel_diff.x = glm::distance(float(global_per_compute_pixel_coords.x), float(per_image_pixel_coords.x));
//					pixel_diff.y = glm::distance(float(global_per_compute_pixel_coords.y), float(per_image_pixel_coords.y));
//
//					if (pixel_diff.x > pixel_diff.y)
//						num_steps = int(pixel_diff.x);
//					else
//						num_steps = int(pixel_diff.y);
//
//					// Please forgive my use of a magic number
//					if (num_steps < 10)
//						num_steps = 10;
//
//					glm::vec2 light_pos = per_image_pixel_coords;
//
//					glm::ivec2 start = per_image_pixel_coords;
//					glm::ivec2 end = global_per_compute_pixel_coords;
//
//					bool found_light_blocker = false;
//					glm::vec2 curr_uv;
//					glm::ivec2 curr_uv_i;
//
//					for (int s = 0; s < num_steps; s++)
//					{
//						curr_uv = glm::mix(start, end, float(s) / float(num_steps - 1));
//
//						curr_uv_i.x = int(curr_uv.x);
//						curr_uv_i.y = int(curr_uv.y);
//
//						pixelValue = ccp.input_light_blocking_pixels.at<Vec4f>(curr_uv_i.y, curr_uv_i.x);
//						const glm::vec4 light_blocking_image_sample = glm::vec4(pixelValue[0], pixelValue[1], pixelValue[2], pixelValue[3]);
//
//						if (light_blocking_image_sample.x == 1 && light_blocking_image_sample.y == 1 && light_blocking_image_sample.z == 1)
//						{
//							found_light_blocker = true;
//							break;
//						}
//					}
//
//					if (found_light_blocker == false)
//					{
//						// Convert from screen space to some other coordinate system,
//						// so that attenuation works much better
//						const float coordinate_change = 1.0 / 5.0;
//
//						const float dist = glm::distance(light_pos, curr_uv) * coordinate_change;
//						const float dist_coefficient = 1;
//						const float dist_squared_coefficient = 1;
//
//						const float attenuation = 1.0 / (1.0f + dist_coefficient * dist + dist_squared_coefficient * dist * dist);
//						
//						lighting.x += light_image_sample.x * attenuation;
//						lighting.y += light_image_sample.y * attenuation;
//						lighting.z += light_image_sample.z * attenuation;
//					}
//				}
//			}
//
//			lighting = glm::clamp(lighting, 0.0f, 1.0f);
//
//			lighting = glm::max(lighting, 0.001f); // Throw in some ambient lighting
//
//			size_t index = 4 * (y * tex_w_small + x);
//
//			ccp.output_pixels[index + 0] = lighting.x;
//			ccp.output_pixels[index + 1] = lighting.y;
//			ccp.output_pixels[index + 2] = lighting.z;
//			ccp.output_pixels[index + 3] = 1.0;
//
//		}
//	}
//}














void gpu_compute_chunk_2(
	compute_chunk_params& ccp)
{
	const GLint tex_w_small = ccp.input_coordinates_pixels.cols;
	const GLint tex_h_small = ccp.input_coordinates_pixels.rows;

	const GLint tex_w_large = ccp.input_light_pixels.cols;
	const GLint tex_h_large = ccp.input_light_pixels.rows;

	ccp.output_pixels.resize(4 * tex_w_large * tex_h_large, 0.0f);

	size_t index = ccp.chunk_index_x * ccp.num_tiles_per_dimension + ccp.chunk_index_y;

	glEnable(GL_TEXTURE_2D);

	GLuint tex_input = 0;
	GLuint tex_light_input = 0;
	GLuint tex_light_blocking_input = 0;
//	GLuint tex_coordinates_input = 0;
	GLuint tex_light_output = 0;

	glGenTextures(1, &tex_input);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex_input);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w_large, tex_h_large, 0, GL_RGBA, GL_FLOAT, ccp.input_pixels.data);
	glBindImageTexture(1, tex_input, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glGenTextures(1, &tex_light_input);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tex_light_input);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w_large, tex_h_large, 0, GL_RGBA, GL_FLOAT, ccp.input_light_pixels.data);
	glBindImageTexture(2, tex_light_input, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glGenTextures(1, &tex_light_blocking_input);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, tex_light_blocking_input);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w_large, tex_h_large, 0, GL_RGBA, GL_FLOAT, ccp.input_light_blocking_pixels.data);
	glBindImageTexture(3, tex_light_blocking_input, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	//glGenTextures(1, &tex_coordinates_input);
	//glActiveTexture(GL_TEXTURE4);
	//glBindTexture(GL_TEXTURE_2D, tex_coordinates_input);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w_small, tex_h_small, 0, GL_RGBA, GL_FLOAT, ccp.input_coordinates_pixels.data);
	//glBindImageTexture(4, tex_coordinates_input, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glGenTextures(1, &tex_light_output);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, tex_light_output);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w_large, tex_h_large, 0, GL_RGBA, GL_FLOAT, ccp.output_light_pixels.data);
	glBindImageTexture(5, tex_light_output, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);


	GLuint tex_output = 0;

	glGenTextures(1, &tex_output);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_output);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w_large, tex_h_large, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	// Use the compute shader
	glUseProgram(ccp.compute_shader_program);

	glUniform1i(glGetUniformLocation(ccp.compute_shader_program, "output_image"), 0);
	glUniform1i(glGetUniformLocation(ccp.compute_shader_program, "input_image"), 1);
	glUniform1i(glGetUniformLocation(ccp.compute_shader_program, "input_light_image"), 2);
	glUniform1i(glGetUniformLocation(ccp.compute_shader_program, "input_light_blocking_image"), 3);
//	glUniform1i(glGetUniformLocation(ccp.compute_shader_program, "input_coordinates_image"), 4);
	glUniform1i(glGetUniformLocation(ccp.compute_shader_program, "output_light_image"), 5);


	glUniform2i(glGetUniformLocation(ccp.compute_shader_program, "u_size"), tex_w_large, tex_h_large);
	glUniform2i(glGetUniformLocation(ccp.compute_shader_program, "u_size_small"), tex_w_small, tex_h_small);
	glUniform2i(glGetUniformLocation(ccp.compute_shader_program, "u_chunk_index"), ccp.chunk_index_x, ccp.chunk_index_y);



	// Run compute shader
	glDispatchCompute((GLuint)tex_w_large / 32, (GLuint)tex_h_large / 32, 1);

	// Wait for compute shader to finish
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);




	// Copy output pixel array to CPU as texture 0
	glActiveTexture(GL_TEXTURE0);
	glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, &ccp.output_pixels[0]);


	glDeleteTextures(1, &tex_input);
	glDeleteTextures(1, &tex_light_input);
	glDeleteTextures(1, &tex_light_blocking_input);
//	glDeleteTextures(1, &tex_coordinates_input);
	glDeleteTextures(1, &tex_light_output);

	glDeleteTextures(1, &tex_output);


	//for (size_t x = 0; x < (4 * tex_w_small * tex_h_small); x += 4)
	//{
	//	cout << ccp.output_pixels[x + 0] << " " << ccp.output_pixels[x + 1] << " " << ccp.output_pixels[x + 2] << " " << ccp.output_pixels[x + 3] << endl;
	//}


	//Mat uc_output(tex_w_large, tex_h_large, CV_8UC4);

	//for (size_t x = 0; x < (4 * uc_output.rows * uc_output.cols); x += 4)
	//{
	//	//cout << ccp.output_pixels[x + 0] << " " << ccp.output_pixels[x + 1] << " " << ccp.output_pixels[x + 2] << " " << ccp.output_pixels[x + 3] << endl;

	//	uc_output.data[x + 0] = (ccp.output_pixels[x + 0] * 255.0f);
	//	uc_output.data[x + 1] = (ccp.output_pixels[x + 1] * 255.0f);
	//	uc_output.data[x + 2] = (ccp.output_pixels[x + 2] * 255.0f);
	//	uc_output.data[x + 3] = 255.0f;
	//}

	//string s = "_output_" + to_string(index) + ".png";
	//imwrite(s.c_str(), uc_output);
}




void gpu_compute_chunk(
	compute_chunk_params& ccp)
{
	const GLint tex_w_small = ccp.input_pixels.cols;
	const GLint tex_h_small = ccp.input_pixels.rows;
	const GLint tex_w_large = ccp.input_light_pixels.cols;
	const GLint tex_h_large = ccp.input_light_pixels.rows;

	ccp.output_pixels.resize(4 * tex_w_small * tex_h_small, 0.0f);

	size_t index = ccp.chunk_index_x * ccp.num_tiles_per_dimension + ccp.chunk_index_y;


	glEnable(GL_TEXTURE_2D);

	GLuint tex_input = 0;
	GLuint tex_light_input = 0;
	GLuint tex_light_blocking_input = 0;
	GLuint tex_coordinates_input = 0;

	glGenTextures(1, &tex_input);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex_input);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w_small, tex_h_small, 0, GL_RGBA, GL_FLOAT, ccp.input_pixels.data);
	glBindImageTexture(1, tex_input, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glGenTextures(1, &tex_light_input);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tex_light_input);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w_large, tex_h_large, 0, GL_RGBA, GL_FLOAT, ccp.input_light_pixels.data);
	glBindImageTexture(2, tex_light_input, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glGenTextures(1, &tex_light_blocking_input);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, tex_light_blocking_input);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w_large, tex_h_large, 0, GL_RGBA, GL_FLOAT, ccp.input_light_blocking_pixels.data);
	glBindImageTexture(3, tex_light_blocking_input, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glGenTextures(1, &tex_coordinates_input);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, tex_coordinates_input);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w_small, tex_h_small, 0, GL_RGBA, GL_FLOAT, ccp.input_coordinates_pixels.data);
	glBindImageTexture(4, tex_coordinates_input, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);



	GLuint tex_output = 0;

	glGenTextures(1, &tex_output);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_output);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w_small, tex_h_small, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	// Use the compute shader
	glUseProgram(ccp.compute_shader_program);
	glUniform1i(glGetUniformLocation(ccp.compute_shader_program, "output_image"), 0);
	glUniform1i(glGetUniformLocation(ccp.compute_shader_program, "input_image"), 1);
	glUniform1i(glGetUniformLocation(ccp.compute_shader_program, "input_light_image"), 2);
	glUniform1i(glGetUniformLocation(ccp.compute_shader_program, "input_light_blocking_image"), 3);
	glUniform1i(glGetUniformLocation(ccp.compute_shader_program, "input_coordinates_image"), 4);

	glUniform2i(glGetUniformLocation(ccp.compute_shader_program, "u_size"), tex_w_large, tex_h_large);
	glUniform2i(glGetUniformLocation(ccp.compute_shader_program, "u_size_small"), tex_w_small, tex_h_small);
	glUniform2i(glGetUniformLocation(ccp.compute_shader_program, "u_chunk_index"), ccp.chunk_index_x, ccp.chunk_index_y);



	// Run compute shader
	glDispatchCompute((GLuint)tex_w_small / 32, (GLuint)tex_h_small / 32, 1);

	// Wait for compute shader to finish
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);




	// Copy output pixel array to CPU as texture 0
	glActiveTexture(GL_TEXTURE0);
	glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, &ccp.output_pixels[0]);


	glDeleteTextures(1, &tex_input);
	glDeleteTextures(1, &tex_light_input);
	glDeleteTextures(1, &tex_light_blocking_input);
	glDeleteTextures(1, &tex_coordinates_input);
	glDeleteTextures(1, &tex_output);


	//for (size_t x = 0; x < (4 * tex_w_small * tex_h_small); x += 4)
	//{
	//	cout << ccp.output_pixels[x + 0] << " " << ccp.output_pixels[x + 1] << " " << ccp.output_pixels[x + 2] << " " << ccp.output_pixels[x + 3] << endl;
	//}


	//Mat uc_output_small(tex_w_small, tex_h_small, CV_8UC4);

	//for (size_t x = 0; x < (4 * uc_output_small.rows * uc_output_small.cols); x += 4)
	//{
	//	cout << ccp.output_pixels[x + 0] << " " << ccp.output_pixels[x + 1] << " " << ccp.output_pixels[x + 2] << " " << ccp.output_pixels[x + 3] << endl;

	//	uc_output_small.data[x + 0] = (ccp.output_pixels[x + 0] * 255.0f);
	//	uc_output_small.data[x + 1] = (ccp.output_pixels[x + 1] * 255.0f);
	//	uc_output_small.data[x + 2] = (ccp.output_pixels[x + 2] * 255.0f);
	//	uc_output_small.data[x + 3] = 255.0f;
	//}

//	string s = "_output_" + to_string(index) + ".png";
//	imwrite(s.c_str(), uc_output_small);
}





//
//void thread_func(
//	mutex& m,
//	vector<compute_chunk_params> &v_ccp)
//{
//	while (1)
//	{
//		bool found_work = false;
//
//		m.lock();
//
//		size_t i = 0;
//
//		for (i = 0; i < v_ccp.size(); i++)
//		{
//			if (v_ccp[i].previously_computed == false)
//			{
//				v_ccp[i].previously_computed = true;
//				found_work = true;
//				break;
//			}
//		}
//
//		m.unlock();
//
//
//		if (!found_work)
//			return;
//		else
//			cpu_compute_chunk(v_ccp[i]);
//	}
//}







Mat compute_global_coords(
	const int size_x,
	const int size_y)
{
	// Assumes that texture values are 65535 or less
	Mat output_mat(size_x, size_y, CV_16UC4);

	for (unsigned short int x = 0; x < size_x; x++)
	{
		for (unsigned short int y = 0; y < size_y; y++)
		{
			Vec4s pixelValue;
			pixelValue[0] = x;
			pixelValue[1] = y;
			pixelValue[2] = 0;
			pixelValue[3] = 0;

			output_mat.at<Vec4s>(y, x) = pixelValue;
		}
	}

	return output_mat;
}






void compute(
	GLuint& compute_shader_program,
	GLuint& compute_shader_program2,
	vector<float>& output_pixels,
	Mat input_mat,
	Mat input_light_mat_with_dynamic_lights,
	Mat input_light_blocking_mat)
{
	int pre_pot_res_x = input_mat.cols;
	int pre_pot_res_y = input_mat.rows;

	int pot = pre_pot_res_x;

	if (pre_pot_res_y > pot)
		pot = pre_pot_res_y;

	pot = pow(2, ceil(log(pot) / log(2)));


	Mat input_square_mat(Size(pot, pot), CV_8UC4, Scalar(0, 0, 0, 255));
	input_mat.copyTo(input_square_mat(Rect(0, 0, pre_pot_res_x, pre_pot_res_y)));
	input_mat = input_square_mat.clone();

	Mat input_light_square_mat(Size(pot, pot), CV_8UC4, Scalar(0, 0, 0, 255));
	input_light_mat_with_dynamic_lights.copyTo(input_light_square_mat(Rect(0, 0, pre_pot_res_x, pre_pot_res_y)));
	input_light_mat_with_dynamic_lights = input_light_square_mat.clone();

	Mat input_light_blocking_square_mat(Size(pot, pot), CV_8UC4, Scalar(0, 0, 0, 255));
	input_light_blocking_mat.copyTo(input_light_blocking_square_mat(Rect(0, 0, pre_pot_res_x, pre_pot_res_y)));
	input_light_blocking_mat = input_light_blocking_square_mat.clone();

	Mat input_mat_float(pot, pot, CV_32FC4);
	input_mat.convertTo(input_mat_float, CV_32FC4, 1.0 / 255.0);

	Mat input_light_mat_float(pot, pot, CV_32FC4);
	input_light_mat_with_dynamic_lights.convertTo(input_light_mat_float, CV_32FC4, 1.0 / 255.0);

	Mat input_light_blocking_mat_float(pot, pot, CV_32FC4);
	input_light_blocking_mat.convertTo(input_light_blocking_mat_float, CV_32FC4, 1.0 / 255.0);


	const int num_tiles_per_dimension = 1; // this squared is the number of tiles

	Mat uc_output_large = compute_global_coords(pot, pot);

	std::vector<cv::Mat> array_of_input_coordinate_mats = splitImage(uc_output_large, num_tiles_per_dimension, num_tiles_per_dimension);
	std::vector<cv::Mat> array_of_input_mats = splitImage(input_mat, num_tiles_per_dimension, num_tiles_per_dimension);
	std::vector<cv::Mat> array_of_output_mats(num_tiles_per_dimension * num_tiles_per_dimension);

	vector<compute_chunk_params> v_ccp;

	for (size_t x = 0; x < num_tiles_per_dimension; x++)
	{
		for (size_t y = 0; y < num_tiles_per_dimension; y++)
		{
			size_t index = x * num_tiles_per_dimension + y;

			Mat input_mat_float(array_of_input_mats[index].rows, array_of_input_mats[index].cols, CV_32FC4);
			array_of_input_mats[index].convertTo(input_mat_float, CV_32FC4, 1.0 / 255.0);

			Mat input_coordinates_mat_float(array_of_input_coordinate_mats[index].rows, array_of_input_coordinate_mats[index].cols, CV_32FC4);
			array_of_input_coordinate_mats[index].convertTo(input_coordinates_mat_float, CV_32FC4, 1.0);

			compute_chunk_params ccp;

			ccp.previously_computed = false;
			ccp.chunk_index_x = x;
			ccp.chunk_index_y = y;
			ccp.num_tiles_per_dimension = num_tiles_per_dimension;
			ccp.compute_shader_program = compute_shader_program;
			ccp.input_pixels = input_mat_float;
			ccp.input_light_pixels = input_light_mat_float;
			ccp.input_light_blocking_pixels = input_light_blocking_mat_float;
			ccp.input_coordinates_pixels = input_coordinates_mat_float;

			v_ccp.push_back(ccp);
		}
	}


	//mutex m;
	//vector<thread> threads;
	//int num_cpu_threads = 0;// std::thread::hardware_concurrency();

	//for(int i = 0; i < num_cpu_threads; i++)
	//	threads.push_back(thread(thread_func, ref(m), ref(v_ccp)));

	while (1)
	{
		bool all_done = true;

		//m.lock();

		size_t i = 0;

		for (i = 0; i < v_ccp.size(); i++)
		{
			if (v_ccp[i].previously_computed == false)
			{
				v_ccp[i].previously_computed = true;
				all_done = false;
				break;
			}
		}

		//m.unlock();

		if (all_done)
			break;
		else
			gpu_compute_chunk(v_ccp[i]);
	}

	//for (int i = 0; i < num_cpu_threads; i++)
	//	threads[i].join();

	for (size_t i = 0; i < v_ccp.size(); i++)
	{
		size_t index = v_ccp[i].chunk_index_x * v_ccp[i].num_tiles_per_dimension + v_ccp[i].chunk_index_y;

		Mat uc_output_small(v_ccp[i].input_pixels.rows, v_ccp[i].input_pixels.cols, CV_8UC4);

		for (size_t x = 0; x < (4 * uc_output_small.rows * uc_output_small.cols); x += 4)
		{
			uc_output_small.data[x + 0] = static_cast<unsigned char>(v_ccp[i].output_pixels[x + 0] * 255.0f);
			uc_output_small.data[x + 1] = static_cast<unsigned char>(v_ccp[i].output_pixels[x + 1] * 255.0f);
			uc_output_small.data[x + 2] = static_cast<unsigned char>(v_ccp[i].output_pixels[x + 2] * 255.0f);
			uc_output_small.data[x + 3] = 255.0f;
		}

		array_of_output_mats[index] = uc_output_small;
	}


	cv::Mat uc_output = imageCollage(array_of_output_mats, num_tiles_per_dimension, num_tiles_per_dimension);


	float gi_factor = 0;

	if (gi_factor > 0.0f)
	{
		// Do indirect lighting
		// 
		Mat light_mat_float(uc_output.rows, uc_output.cols, CV_32FC4);
		uc_output.convertTo(light_mat_float, CV_32FC4, 1.0 / 255.0);

		v_ccp[0].compute_shader_program = compute_shader_program2;
		v_ccp[0].input_pixels = input_mat_float;
		//v_ccp[0].input_light_pixels = input_light_mat_float; // Not needed in shader2.comp
		v_ccp[0].input_light_blocking_pixels = input_light_blocking_mat_float;
		v_ccp[0].output_light_pixels = light_mat_float;

		gpu_compute_chunk_2(v_ccp[0]);

		for (size_t x = 0; x < (4 * uc_output.rows * uc_output.cols); x += 4)
		{
			glm::vec4 uc_data(
				uc_output.data[x + 0] / 255.0f,
				uc_output.data[x + 1] / 255.0f,
				uc_output.data[x + 2] / 255.0f,
				1.0);

			glm::vec4 gi_data(
				v_ccp[0].output_pixels[x + 0],
				v_ccp[0].output_pixels[x + 1],
				v_ccp[0].output_pixels[x + 2],
				1.0);

			glm::vec4 output = glm::mix(uc_data, gi_data, gi_factor);

			output_pixels[x + 0] = output.r;
			output_pixels[x + 1] = output.g;
			output_pixels[x + 2] = output.b;
			output_pixels[x + 3] = 1.0f;
		}
	}
	else
	{
		for (size_t x = 0; x < (4 * uc_output.rows * uc_output.cols); x += 4)
		{
			output_pixels[x + 0] = uc_output.data[x + 0] / 255.0f;
			output_pixels[x + 1] = uc_output.data[x + 1] / 255.0f;
			output_pixels[x + 2] = uc_output.data[x + 2] / 255.0f;
			output_pixels[x + 3] = 1.0f;
		}
	}

	


}






bool compile_and_link_compute_shader(const char* const file_name, GLuint& program)
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
	//glutInit(&argc, argv);

	//glutInitDisplayMode(GLUT_RGBA);
	//int glut_window_handle = glutCreateWindow("");

	//if (!(GLEW_OK == glewInit() &&
	//	GLEW_VERSION_4_3))
	//{
	//	return false;
	//}

	return true;
}

bool init_gl(int argc, char** argv,
	//	GLint tex_w, GLint tex_h,
	GLuint& compute_shader_program,
	GLuint& compute_shader_program2)
{
	if (GLEW_OK != glewInit())
	{
		cout << "glew failure" << endl;
		return false;
	}

	else
	{
		cout << "OpenGL 4.3 initialization OK" << endl;
	}




	// Initialize the compute shader
	compute_shader_program = 0;

	if (false == compile_and_link_compute_shader("shader.comp", compute_shader_program))
	{
		cout << "Failed to initialize compute shader" << endl;
		return false;
	}

	compute_shader_program2 = 0;

	if (false == compile_and_link_compute_shader("shader2.comp", compute_shader_program2))
	{
		cout << "Failed to initialize compute shader 2" << endl;
		return false;
	}

	
	//cout << "Texture size: " << tex_w << "x" << tex_h << endl;

	//// Check that the global workgrounp count is greater than or equal to the input/output textures
	//GLint global_workgroup_count[2];
	//glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &global_workgroup_count[0]);
	//glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &global_workgroup_count[1]);

	//cout << "Max global workgroup size: " << global_workgroup_count[0] << "x" << global_workgroup_count[1] << endl;

	//if (tex_w > global_workgroup_count[0])
	//{
	//	cout << "Texture width " << tex_w << " is larger than max " << global_workgroup_count[0] << endl;
	//	return false;
	//}

	//if (tex_h > global_workgroup_count[1])
	//{
	//	cout << "Texture height " << tex_h << " is larger than max " << global_workgroup_count[1] << endl;
	//	return false;
	//}



	return true;
}

void delete_all(GLuint& tex_output, GLuint& tex_input, GLuint& tex_light_input, GLuint& tex_light_blocking_input, GLuint& compute_shader_program)
{

	glDeleteProgram(compute_shader_program);
}




namespace glm
{
	bool operator< (const glm::vec3& lhs, const glm::vec3& rhs)
	{
		return glm::all(glm::lessThan(lhs, rhs));
	}

	bool operator== (const glm::vec3& lhs, const glm::vec3& rhs)
	{
		return glm::all(glm::lessThan(lhs, rhs));
	}
}

namespace std
{
	bool operator==(const pair<glm::vec3, size_t>& lhs, const pair<glm::vec3, size_t>& rhs)
	{
		return lhs.first == rhs.first && lhs.second == rhs.second;
	}
}

/*! \brief Convert RGB to HSV color space

  Converts a given set of RGB values `r', `g', `b' into HSV
  coordinates. The input RGB values are in the range [0, 1], and the
  output HSV values are in the ranges h = [0, 360], and s, v = [0,
  1], respectively.

  \param fR Red component, used as input, range: [0, 1]
  \param fG Green component, used as input, range: [0, 1]
  \param fB Blue component, used as input, range: [0, 1]
  \param fH Hue component, used as output, range: [0, 360]
  \param fS Hue component, used as output, range: [0, 1]
  \param fV Hue component, used as output, range: [0, 1]
 */

void RGBtoHSV(float& fR, float& fG, float& fB, float& fH, float& fS, float& fV) {
	float fCMax = max(max(fR, fG), fB);
	float fCMin = min(min(fR, fG), fB);
	float fDelta = fCMax - fCMin;

	if (fDelta > 0) {
		if (fCMax == fR) {
			fH = 60 * (fmod(((fG - fB) / fDelta), 6));
		}
		else if (fCMax == fG) {
			fH = 60 * (((fB - fR) / fDelta) + 2);
		}
		else if (fCMax == fB) {
			fH = 60 * (((fR - fG) / fDelta) + 4);
		}

		if (fCMax > 0) {
			fS = fDelta / fCMax;
		}
		else {
			fS = 0;
		}

		fV = fCMax;
	}
	else {
		fH = 0;
		fS = 0;
		fV = fCMax;
	}

	if (fH < 0) {
		fH = 360 + fH;
	}
}


/*! \brief Convert HSV to RGB color space

  Converts a given set of HSV values `h', `s', `v' into RGB
  coordinates. The output RGB values are in the range [0, 1], and
  the input HSV values are in the ranges h = [0, 360], and s, v =
  [0, 1], respectively.

  \param fR Red component, used as output, range: [0, 1]
  \param fG Green component, used as output, range: [0, 1]
  \param fB Blue component, used as output, range: [0, 1]
  \param fH Hue component, used as input, range: [0, 360]
  \param fS Hue component, used as input, range: [0, 1]
  \param fV Hue component, used as input, range: [0, 1]

*/
void HSVtoRGB(float& fR, float& fG, float& fB, float& fH, float& fS, float& fV) {
	float fC = fV * fS; // Chroma
	float fHPrime = fmod(fH / 60.0, 6);
	float fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
	float fM = fV - fC;

	if (0 <= fHPrime && fHPrime < 1) {
		fR = fC;
		fG = fX;
		fB = 0;
	}
	else if (1 <= fHPrime && fHPrime < 2) {
		fR = fX;
		fG = fC;
		fB = 0;
	}
	else if (2 <= fHPrime && fHPrime < 3) {
		fR = 0;
		fG = fC;
		fB = fX;
	}
	else if (3 <= fHPrime && fHPrime < 4) {
		fR = 0;
		fG = fX;
		fB = fC;
	}
	else if (4 <= fHPrime && fHPrime < 5) {
		fR = fX;
		fG = 0;
		fB = fC;
	}
	else if (5 <= fHPrime && fHPrime < 6) {
		fR = fC;
		fG = 0;
		fB = fX;
	}
	else {
		fR = 0;
		fG = 0;
		fB = 0;
	}

	fR += fM;
	fG += fM;
	fB += fM;
}








struct
{
	struct
	{
		GLint tex;
	}
	ortho_shader_uniforms;
}
uniforms;


bool draw_full_screen_tex(GLint tex_slot, GLint tex_handle)//, long signed int win_width, long signed int win_height)
{
	complex<float> v0ndc(-1, -1);
	complex<float> v1ndc(-1, 1);
	complex<float> v2ndc(1, 1);
	complex<float> v3ndc(1, -1);

	static GLuint vao = 0, vbo = 0, ibo = 0;

	if (!glIsVertexArray(vao))
	{
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ibo);
	}

	const GLfloat vertexData[] = {
		//	  X             Y             Z		  U  V     
			  v0ndc.real(), v0ndc.imag(), 0,      0, 1,
			  v1ndc.real(), v1ndc.imag(), 0,      0, 0,
			  v2ndc.real(), v2ndc.imag(), 0,      1, 0,
			  v3ndc.real(), v3ndc.imag(), 0,      1, 1
	};

	// https://raw.githubusercontent.com/progschj/OpenGL-Examples/master/03texture.cpp

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * 5, vertexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (char*)0 + 0 * sizeof(GLfloat));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (char*)0 + 3 * sizeof(GLfloat));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	static const GLuint indexData[] = {
		3,1,0,
		2,1,3,
	};

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 2 * 3, indexData, GL_STATIC_DRAW);

	glBindVertexArray(0);

	glUseProgram(ortho_shader.get_program());

	glActiveTexture(GL_TEXTURE0 + tex_slot);
	glBindTexture(GL_TEXTURE_2D, tex_handle);

	glUniform1i(uniforms.ortho_shader_uniforms.tex, tex_slot);
	//glUniform1i(uniforms.ortho_shader_uniforms.viewport_width, win_width);
	//glUniform1i(uniforms.ortho_shader_uniforms.viewport_height, win_height);


	glBindVertexArray(vao);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	return true;
}



Mat anti_alias_mat(Mat aliased_mat)
{
	Mat anti_aliased_mat(aliased_mat.rows, aliased_mat.cols, CV_8UC4);

	for (signed int i = 0; i < aliased_mat.cols; i++)
	{
		for (signed int j = 0; j < aliased_mat.rows; j++)
		{
			int horizontal_begin = i - 1;
			int vertical_begin = j - 1;

			if (horizontal_begin < 0)
				horizontal_begin = 0;

			if (vertical_begin < 0)
				vertical_begin = 0;

			int horizontal_end = i + 1;
			int vertical_end = j + 1;

			if (horizontal_end >= aliased_mat.cols)
				horizontal_end = aliased_mat.cols - 1;

			if (vertical_end >= aliased_mat.rows)
				vertical_end = aliased_mat.rows - 1;

			Vec4b pixelValue_centre = aliased_mat.at<Vec4b>(j, i);

			Vec4b pixelValue_left = aliased_mat.at<Vec4b>(j, horizontal_begin);
			Vec4b pixelValue_right = aliased_mat.at<Vec4b>(j, horizontal_end);

			Vec4b pixelValue_up = aliased_mat.at<Vec4b>(vertical_begin, i);
			Vec4b pixelValue_down = aliased_mat.at<Vec4b>(vertical_end, i);

			Vec4f horizontal_avg = (pixelValue_left + pixelValue_right);
			Vec4f vertical_avg = (pixelValue_up + pixelValue_down);

			Vec4f avg;

			avg[0] = (1.0 / 3.0) * (pixelValue_centre[0] + horizontal_avg[0] + vertical_avg[0]);
			avg[1] = (1.0 / 3.0) * (pixelValue_centre[1] + horizontal_avg[1] + vertical_avg[1]);
			avg[2] = (1.0 / 3.0) * (pixelValue_centre[2] + horizontal_avg[2] + vertical_avg[2]);
			avg[3] = 255;

			Vec4b avg_b;
			avg_b[0] = avg[0];
			avg_b[1] = avg[1];
			avg_b[2] = avg[2];
			avg_b[3] = avg[3];

			anti_aliased_mat.at<Vec4b>(j, i) = avg_b;
		}
	}

	return anti_aliased_mat;

}

#endif