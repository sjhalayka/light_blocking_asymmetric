#define _CRT_SECURE_NO_WARNINGS

#include "main.h"

#include <opencv2/opencv.hpp>
using namespace cv;

#pragma comment(lib, "opencv_world4100.lib")




int main(int argc, char** argv)
{
	int tile_size = 18;

	int num_input_channels = 4;

	// Load from file
	Mat input_mat = imread("input.png", IMREAD_UNCHANGED);

	if (input_mat.empty() || input_mat.channels() != 4)
	{
		cout << "input.png must be a 32-bit PNG" << endl;
		return -1;
	}

	const int res_x = input_mat.cols;
	const int res_y = input_mat.rows;

	Mat input_mat_backup = input_mat.clone();

	resize(input_mat, input_mat, cv::Size(res_x / tile_size, res_y / tile_size), 0, 0, cv::INTER_NEAREST);







	Mat input_light_mat = imread("input_light.png", IMREAD_UNCHANGED);

	if (input_light_mat.empty() || input_light_mat.channels() != 4)
	{
		cout << "input_light.png must be a 32-bit PNG" << endl;
		return -1;
	}

	resize(input_light_mat, input_light_mat, cv::Size(res_x / tile_size, res_y / tile_size), 0, 0, cv::INTER_NEAREST);




	Mat input_light_blocking_mat = imread("input_light_blocking.png", IMREAD_UNCHANGED);

	if (input_light_blocking_mat.empty() || input_light_blocking_mat.channels() != 4)
	{
		cout << "input_light_blocking.png must be a 32-bit PNG" << endl;
		return -1;
	}

	resize(input_light_blocking_mat, input_light_blocking_mat, cv::Size(res_x / tile_size, res_y / tile_size), 0, 0, cv::INTER_NEAREST);









	int num_output_channels = 4;

	vector<float> output_pixels(res_x / tile_size * res_y / tile_size * num_output_channels, 0.0f);
	vector<float> input_pixels(res_x / tile_size * res_y / tile_size * num_input_channels, 0.0f);
	vector<float> input_light_pixels(res_x / tile_size * res_y / tile_size * num_input_channels, 0.0f);
	vector<float> input_light_blocking_pixels(res_x / tile_size * res_y / tile_size * num_input_channels, 0.0f);

	for (size_t y = 0; y < res_y / tile_size; y++)
	{
		for (size_t x = 0; x < res_x / tile_size; x++)
		{
			size_t uc_index = 4 * (x * res_y / tile_size + y);

			Vec4b pixelValue = input_mat.at<Vec4b>(x, y);

			input_pixels[uc_index + 0] = pixelValue[0] / 255.0f;// input_mat.data[uc_index + 0] / 255.0f;
			input_pixels[uc_index + 1] = pixelValue[1] / 255.0f;// input_mat.data[uc_index + 1] / 255.0f;
			input_pixels[uc_index + 2] = pixelValue[2] / 255.0f;// input_mat.data[uc_index + 2] / 255.0f;
			input_pixels[uc_index + 3] = 1.0;
		}
	}


	for (size_t y = 0; y < res_y / tile_size; y++)
	{
		for (size_t x = 0; x < res_x / tile_size; x++)
		{
			size_t uc_index = 4 * (x * res_y / tile_size + y);

			Vec4b pixelValue = input_light_mat.at<Vec4b>(x, y);

			input_light_pixels[uc_index + 0] = pixelValue[0] / 255.0f;// input_mat.data[uc_index + 0] / 255.0f;
			input_light_pixels[uc_index + 1] = pixelValue[1] / 255.0f;// input_mat.data[uc_index + 1] / 255.0f;
			input_light_pixels[uc_index + 2] = pixelValue[2] / 255.0f;// input_mat.data[uc_index + 2] / 255.0f;
			input_light_pixels[uc_index + 3] = 0.0;
		}
	}


	for (size_t y = 0; y < res_y / tile_size; y++)
	{
		for (size_t x = 0; x < res_x / tile_size; x++)
		{
			size_t uc_index = 4 * (x * res_y / tile_size + y);

			Vec4b pixelValue = input_light_blocking_mat.at<Vec4b>(x, y);

			input_light_blocking_pixels[uc_index + 0] = pixelValue[0] / 255.0f;// input_mat.data[uc_index + 0] / 255.0f;
			input_light_blocking_pixels[uc_index + 1] = pixelValue[1] / 255.0f;// input_mat.data[uc_index + 1] / 255.0f;
			input_light_blocking_pixels[uc_index + 2] = pixelValue[2] / 255.0f;// input_mat.data[uc_index + 2] / 255.0f;
			input_light_blocking_pixels[uc_index + 3] = 1.0;
		}
	}














	// Initialize OpenGL / compute shader / textures
	GLuint compute_shader_program = 0;
	GLuint tex_output = 0, tex_input = 0, tex_light_input = 0, tex_light_blocking_input = 0;




	if (false == init_all(
		argc, argv,
		tex_output,
		tex_input,
		tex_light_input,
		tex_light_blocking_input,
		res_x / tile_size, res_y / tile_size,
		compute_shader_program))
	{
		cout << "Aborting" << endl;
		return -1;
	}

	cout << "Computing " << endl;


	// Run the compute shader
	compute(tex_output,
		tex_input,
		tex_light_input,
		tex_light_blocking_input,
		res_x / tile_size, res_y / tile_size,
		compute_shader_program,
		output_pixels,
		input_pixels,
		input_light_pixels,
		input_light_blocking_pixels);

	cout << "Computing done" << endl;


	// Save output to PNG file
	Mat uc_output(res_x / tile_size, res_y / tile_size, CV_8UC4);

	for (size_t x = 0; x < 4 * ((res_x / tile_size) * (res_y / tile_size)); x += 4)
	{
		uc_output.data[x + 0] = static_cast<unsigned char>(output_pixels[x + 0] * 255.0);
		uc_output.data[x + 1] = static_cast<unsigned char>(output_pixels[x + 1] * 255.0);
		uc_output.data[x + 2] = static_cast<unsigned char>(output_pixels[x + 2] * 255.0);
		uc_output.data[x + 3] = 255;
	}

	resize(uc_output, uc_output, cv::Size(res_x, res_y), 0, 0, cv::INTER_LINEAR);

	for (size_t x = 0; x < (res_x * res_y * 4); x += 4)
	{
		uc_output.data[x + 0] = static_cast<unsigned char>(255.0 * (input_mat_backup.data[x + 0] / 255.0 * uc_output.data[x + 0] / 255.0));
		uc_output.data[x + 1] = static_cast<unsigned char>(255.0 * (input_mat_backup.data[x + 1] / 255.0 * uc_output.data[x + 1] / 255.0));
		uc_output.data[x + 2] = static_cast<unsigned char>(255.0 * (input_mat_backup.data[x + 2] / 255.0 * uc_output.data[x + 2] / 255.0));
		uc_output.data[x + 3] = 255;
	}

	imwrite("out.png", uc_output);

	//cout << "Save done" << endl;



	// Clean up all memory
	delete_all(tex_output, tex_input, tex_light_input, tex_light_blocking_input, compute_shader_program);



	return 0;
}