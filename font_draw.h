#ifndef FONT_DRAW_H
#define FONT_DRAW_H

#include <complex>
using namespace std;



#include <cstdlib>

#include "GL/glew.h"
#include <opencv2/opencv.hpp>
using namespace cv;



// http://www.songho.ca/opengl/gl_transform.html

complex<float> get_window_coords_from_ndc_coords(size_t viewport_width, size_t viewport_height, complex<float>& src_coords)
{
	float x_w = viewport_width / 2.0f * src_coords.real() + viewport_width / 2.0f;
	float y_w = viewport_height / 2.0f * src_coords.imag() + viewport_height / 2.0f;

	return complex<float>(x_w, y_w);
}

complex<float> get_ndc_coords_from_window_coords(size_t viewport_width, size_t viewport_height, complex<float>& src_coords)
{
	float x_ndc = (2.0f * src_coords.real() / viewport_width) - 1.0f;
	float y_ndc = (2.0f * src_coords.imag() / viewport_height) - 1.0f;

	return complex<float>(x_ndc, y_ndc);
}


class RGB_uchar
{
public:
	unsigned char r, g, b;
};

class font_character_image
{
public:
	size_t width;
	size_t height;
	vector<unsigned char> monochrome_data;



	GLuint tex_handle = 0, vbo_handle = 0;


	font_character_image& operator=(font_character_image rhs)
	{
		width = rhs.width;
		height = rhs.height;
		monochrome_data = rhs.monochrome_data;
		tex_handle = rhs.tex_handle;
		vbo_handle = rhs.vbo_handle;

		return *this;
	}





	void opengl_init(RGB_uchar text_colour, float alpha_max = 255)
	{
		cleanup();

		glGenBuffers(1, &vbo_handle);

		vector<unsigned char> rgba_data(4 * width * height, 0);

		for (size_t i = 0; i < width; i++)
		{
			for (size_t j = 0; j < height; j++)
			{
				size_t mono_index = j * width + i;
				size_t rgba_index = 4 * mono_index;

				rgba_data[rgba_index + 0] = text_colour.r;
				rgba_data[rgba_index + 1] = text_colour.g;
				rgba_data[rgba_index + 2] = text_colour.b;

				if (monochrome_data[mono_index] <= alpha_max)
					rgba_data[rgba_index + 3] = monochrome_data[mono_index];
				else
					rgba_data[rgba_index + 3] = static_cast<unsigned char>(alpha_max);

				//rgba_data[rgba_index + 3] = monochrome_data[mono_index];
			}
		}

		glGenTextures(1, &tex_handle);
		glBindTexture(GL_TEXTURE_2D, tex_handle);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, &rgba_data[0]);
	}

	void draw(float scale, GLuint shader_program, size_t x, size_t y, size_t win_width, size_t win_height) const
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		complex<float> v0w(static_cast<float>(x), static_cast<float>(y));
		complex<float> v1w(static_cast<float>(x), static_cast<float>(y + this->height * scale));
		complex<float> v2w(static_cast<float>(x + this->width * scale), static_cast<float>(y + this->height * scale));
		complex<float> v3w(static_cast<float>(x + this->width * scale), static_cast<float>(y));




		complex<float> v0ndc = get_ndc_coords_from_window_coords(win_width, win_height, v0w);
		complex<float> v1ndc = get_ndc_coords_from_window_coords(win_width, win_height, v1w);
		complex<float> v2ndc = get_ndc_coords_from_window_coords(win_width, win_height, v2w);
		complex<float> v3ndc = get_ndc_coords_from_window_coords(win_width, win_height, v3w);

		vector<GLfloat> vertex_data = {

			// 3D position, 2D texture coordinate
			v0ndc.real(), v0ndc.imag(), 0,   0, 1, // vertex 0
			v2ndc.real(), v2ndc.imag(), 0,   1, 0, // vertex 2
			v1ndc.real(), v1ndc.imag(), 0,   0, 0, // vertex 1
			v0ndc.real(), v0ndc.imag(), 0,   0, 1, // vertex 0
			v3ndc.real(), v3ndc.imag(), 0,   1, 1, // vertex 3
			v2ndc.real(), v2ndc.imag(), 0,   1, 0, // vertex 2
		};

		GLuint components_per_vertex = 5;
		GLuint components_per_position = 3;
		GLuint components_per_tex_coord = 2;
		GLuint num_vertices = static_cast<GLuint>(vertex_data.size()) / components_per_vertex;

		glBindBuffer(GL_ARRAY_BUFFER, vbo_handle);

		glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(GLfloat), &vertex_data[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(glGetAttribLocation(shader_program, "position"));
		glVertexAttribPointer(glGetAttribLocation(shader_program, "position"),
			components_per_position,
			GL_FLOAT,
			GL_FALSE,
			components_per_vertex * sizeof(GLfloat),
			NULL);

		glEnableVertexAttribArray(glGetAttribLocation(shader_program, "texcoord"));
		glVertexAttribPointer(glGetAttribLocation(shader_program, "texcoord"),
			components_per_tex_coord,
			GL_FLOAT,
			GL_TRUE,
			components_per_vertex * sizeof(GLfloat),
			(const GLvoid*)(components_per_position * sizeof(GLfloat)));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex_handle);
		glUniform1i(glGetUniformLocation(shader_program, "tex"), 0);

		glDrawArrays(GL_TRIANGLES, 0, num_vertices);
	}

	void cleanup(void)
	{
		if (glIsTexture(tex_handle))
			glDeleteTextures(1, &tex_handle);

		if (glIsBuffer(vbo_handle))
			glDeleteBuffers(1, &vbo_handle);
	}

	~font_character_image(void)
	{
		cleanup();
	}
};

vector<font_character_image> mimgs;


const size_t num_chars = 256;
const size_t image_width = 512;
const size_t image_height = 512;
const size_t char_width = 32;
const size_t char_height = 32;
const size_t num_chars_wide = image_width / char_width;
const size_t num_chars_high = image_height / char_height;





void print_char(float scale, const vector<font_character_image>& mimgs, GLuint shader_program, const size_t fb_width, const size_t fb_height, const size_t char_x_pos, const size_t char_y_pos, const unsigned char c)
{
	mimgs[c].draw(scale, shader_program, char_x_pos, char_y_pos, fb_width, fb_height);
}

size_t get_sentence_width(float scale, const vector<font_character_image>& mimgs, const string s)
{
	float sentence_width = 0;

	for (size_t i = 0; i < s.size(); i++)
		sentence_width += mimgs[s[i]].width * scale + 2;

	// Remove trailing space
	sentence_width -= 2;

	return static_cast<size_t>(sentence_width);
}


void print_sentence(float scale, const vector<font_character_image>& mimgs, GLuint shader_program, const size_t fb_width, const size_t fb_height, size_t char_x_pos, size_t char_y_pos, const string s)
{
	char_y_pos = fb_height - char_y_pos;

	for (size_t i = 0; i < s.size(); i++)
	{
		print_char(scale, mimgs, shader_program, fb_width, fb_height, char_x_pos, char_y_pos, s[i]);

		size_t char_width = mimgs[s[i]].width * scale;

		char_x_pos += char_width + 2;
	}
}

bool is_all_zeroes(size_t width, size_t height, const vector<unsigned char>& pixel_data)
{
	bool all_zeroes = true;

	for (size_t i = 0; i < width * height; i++)
	{
		if (pixel_data[i] != 0)
		{
			all_zeroes = false;
			break;
		}
	}

	return all_zeroes;
}

bool is_column_all_zeroes(size_t column, size_t width, size_t height, const vector<unsigned char>& pixel_data)
{
	bool all_zeroes = true;

	for (size_t y = 0; y < height; y++)
	{
		size_t index = y * width + column;

		if (pixel_data[index] != 0)
		{
			all_zeroes = false;
			break;
		}
	}

	return all_zeroes;
}



bool init_character_set(void)
{

	Mat font_mat = imread("font.png", IMREAD_UNCHANGED);

	if (font_mat.empty() || font_mat.channels() != 4)
	{
		cout << "font.png must be a 32-bit PNG" << endl;
		return false;
	}

	size_t char_index = 0;

	vector< vector<GLubyte> > char_data;
	vector<unsigned char> char_template(char_width * char_height);
	char_data.resize(num_chars, char_template);

	for (size_t i = 0; i < num_chars_wide; i++)
	{
		for (size_t j = 0; j < num_chars_high; j++)
		{
			size_t left = i * char_width;
			size_t right = left + char_width - 1;
			size_t top = j * char_height;
			size_t bottom = top + char_height - 1;

			for (size_t k = left, x = 0; k <= right; k++, x++)
			{
				for (size_t l = top, y = 0; l <= bottom; l++, y++)
				{
					size_t img_pos = 4 * (k * image_height + l);
					size_t sub_pos = x * char_height + y;

					char_data[char_index][sub_pos] = font_mat.data[img_pos + 0]; // Assume grayscale, only use r component
				}
			}

			char_index++;
		}
	}



	for (size_t n = 0; n < num_chars; n++)
	{
//		cout << "test" << n << endl;


		if (is_all_zeroes(char_width, char_height, char_data[n]))
		{
			font_character_image img;

			img.width = char_width / 4;
			img.height = char_height;

			img.monochrome_data.resize(img.width * img.height, 0);

			mimgs.push_back(img);


		}
		else
		{
			size_t first_non_zeroes_column = 0;
			size_t last_non_zeroes_column = char_width - 1;

			for (size_t x = 0; x < char_width; x++)
			{
				bool all_zeroes = is_column_all_zeroes(x, char_width, char_height, char_data[n]);

				if (false == all_zeroes)
				{
					first_non_zeroes_column = x;
					break;
				}
			}

			for (size_t x = first_non_zeroes_column + 1; x < char_width; x++)
			{
				bool all_zeroes = is_column_all_zeroes(x, char_width, char_height, char_data[n]);

				if (false == all_zeroes)
				{
					last_non_zeroes_column = x;
				}
			}


			size_t cropped_width = last_non_zeroes_column - first_non_zeroes_column + 1;

			font_character_image img;
			img.width = cropped_width;
			img.height = char_height;
			img.monochrome_data.resize(img.width * img.height, 0);

			for (size_t i = 0; i < num_chars_wide; i++)
			{
				for (size_t j = 0; j < num_chars_high; j++)
				{
					const size_t left = first_non_zeroes_column;
					const size_t right = left + cropped_width - 1;
					const size_t top = 0;
					const size_t bottom = char_height - 1;

					for (size_t k = left, x = 0; k <= right; k++, x++)
					{
						for (size_t l = top, y = 0; l <= bottom; l++, y++)
						{
							const size_t img_pos = l * char_width + k;
							const size_t sub_pos = y * cropped_width + x;

							img.monochrome_data[sub_pos] = char_data[n][img_pos];
						}
					}
				}
			}

			mimgs.push_back(img);
		}
	}


	RGB_uchar text_colour;

	text_colour.r = 255;
	text_colour.g = 255;
	text_colour.b = 255;


	for (size_t i = 0; i < mimgs.size(); i++)
		mimgs[i].opengl_init(text_colour);

	return true;
}






#endif
