#define _CRT_SECURE_NO_WARNINGS

#include "main.h"

#include <opencv2/opencv.hpp>
using namespace cv;

#pragma comment(lib, "opencv_world4100.lib")




int main(int argc, char** argv)
{
	const int tile_size = 18;

	// Load from file
	Mat input_mat = imread("input.png", IMREAD_UNCHANGED);

	if (input_mat.empty() || input_mat.channels() != 4)
	{
		cout << "input.png must be a 32-bit PNG" << endl;
		return -1;
	}





	// Must be widescreen
	const int res_x = input_mat.cols;
	const int res_y = input_mat.rows;

	int largest_dim = res_x;

	if (res_y > largest_dim)
		largest_dim = res_y;


	Mat square_mat(Size(largest_dim, largest_dim), CV_8UC4);
	square_mat = Scalar(0, 0, 0, 255);
	input_mat.copyTo(square_mat(Rect(0, 0, res_x, res_y)));
	input_mat = square_mat.clone();

	Mat input_mat_backup = input_mat.clone();

	resize(input_mat, input_mat, cv::Size(largest_dim / tile_size, largest_dim / tile_size), 0, 0, cv::INTER_AREA);

	imwrite("input_mat.png", input_mat);




	Mat input_light_mat = imread("input_light.png", IMREAD_UNCHANGED);

	if (input_light_mat.empty() || input_light_mat.channels() != 4)
	{
		cout << "input_light.png must be a 32-bit PNG" << endl;
		return -1;
	}

	Mat square_light_mat(Size(largest_dim, largest_dim), CV_8UC4);
	square_light_mat = Scalar(0, 0, 0, 255);
	input_light_mat.copyTo(square_light_mat(Rect(0, 0, res_x, res_y)));
	input_light_mat = square_light_mat.clone();


	resize(input_light_mat, input_light_mat, cv::Size(largest_dim / tile_size, largest_dim / tile_size), 0, 0, cv::INTER_AREA);




	Mat input_light_blocking_mat = imread("input_light_blocking.png", IMREAD_UNCHANGED);

	if (input_light_blocking_mat.empty() || input_light_blocking_mat.channels() != 4)
	{
		cout << "input_light_blocking.png must be a 32-bit PNG" << endl;
		return -1;
	}

	Mat square_light_blocking_mat(Size(largest_dim, largest_dim), CV_8UC4);
	square_light_blocking_mat = Scalar(0, 0, 0, 255);
	input_light_blocking_mat.copyTo(square_light_blocking_mat(Rect(0, 0, res_x, res_y)));
	input_light_blocking_mat = square_light_blocking_mat.clone();

//	imwrite("resized_light_mat.png", input_light_mat);

	resize(input_light_blocking_mat, input_light_blocking_mat, cv::Size(largest_dim / tile_size, largest_dim / tile_size), 0, 0, cv::INTER_AREA);









	imwrite("resized_light_blocking_mat.png", input_light_blocking_mat);





	vector<float> output_pixels((largest_dim / tile_size) * (largest_dim / tile_size) * 4, 1.0f);
	vector<float> input_pixels((largest_dim / tile_size) * (largest_dim / tile_size) * 4, 1.0f);
	vector<float> input_light_pixels((largest_dim / tile_size) * (largest_dim / tile_size) * 4, 1.0f);
	vector<float> input_light_blocking_pixels((largest_dim / tile_size) * (largest_dim / tile_size) * 4, 1.0f);


	for (size_t x = 0; x < 4 * ((largest_dim / tile_size) * (largest_dim / tile_size)); x += 4)
	{
		input_pixels[x + 0] = input_mat.data[x + 0] / 255.0f;
		input_pixels[x + 1] = input_mat.data[x + 1] / 255.0f;
		input_pixels[x + 2] = input_mat.data[x + 2] / 255.0f;
		input_pixels[x + 3] = 1.0;
	}

	for (size_t x = 0; x < 4 * ((largest_dim / tile_size) * (largest_dim / tile_size)); x += 4)
	{
		input_light_pixels[x + 0] = input_light_mat.data[x + 0] / 255.0f;
		input_light_pixels[x + 1] = input_light_mat.data[x + 1] / 255.0f;
		input_light_pixels[x + 2] = input_light_mat.data[x + 2] / 255.0f;
		input_light_pixels[x + 3] = 1.0;
	}

	for (size_t x = 0; x < 4 * ((largest_dim / tile_size) * (largest_dim / tile_size)); x += 4)
	{
		input_light_blocking_pixels[x + 0] = input_light_blocking_mat.data[x + 0] / 255.0f;
		input_light_blocking_pixels[x + 1] = input_light_blocking_mat.data[x + 1] / 255.0f;
		input_light_blocking_pixels[x + 2] = input_light_blocking_mat.data[x + 2] / 255.0f;
		input_light_blocking_pixels[x + 3] = 1.0;
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
		largest_dim / tile_size, largest_dim / tile_size,
		compute_shader_program))
	{
		cout << "Aborting" << endl;
		return -1;
	}

	cout << "Computing " << endl;

	compute(tex_output,
		tex_input,
		tex_light_input,
		tex_light_blocking_input,
		largest_dim / tile_size, largest_dim / tile_size,
		compute_shader_program,
		output_pixels,
		input_pixels,
		input_light_pixels,
		input_light_blocking_pixels);

	cout << "Computing done" << endl;


	// Save output to PNG file
	Mat uc_output(largest_dim / tile_size, largest_dim / tile_size, CV_8UC4);

	for (size_t x = 0; x < 4 * ((largest_dim / tile_size) * (largest_dim / tile_size)); x += 4)
	{
		uc_output.data[x + 0] = static_cast<unsigned char>(output_pixels[x + 0] * 255.0);
		uc_output.data[x + 1] = static_cast<unsigned char>(output_pixels[x + 1] * 255.0);
		uc_output.data[x + 2] = static_cast<unsigned char>(output_pixels[x + 2] * 255.0);
		uc_output.data[x + 3] = 255;
	}

	resize(uc_output, uc_output, cv::Size(largest_dim, largest_dim), 0, 0, cv::INTER_AREA);

	for (size_t x = 0; x < (largest_dim * largest_dim * 4); x += 4)
	{
		uc_output.data[x + 0] = static_cast<unsigned char>(255.0 * (input_mat_backup.data[x + 0] / 255.0 * uc_output.data[x + 0] / 255.0));
		uc_output.data[x + 1] = static_cast<unsigned char>(255.0 * (input_mat_backup.data[x + 1] / 255.0 * uc_output.data[x + 1] / 255.0));
		uc_output.data[x + 2] = static_cast<unsigned char>(255.0 * (input_mat_backup.data[x + 2] / 255.0 * uc_output.data[x + 2] / 255.0));
		uc_output.data[x + 3] = 255;
	}

	// Crop
	uc_output = uc_output(Range(0, res_y), Range(0, res_x));

	imwrite("out.png", uc_output);



	// Clean up all memory
	delete_all(tex_output, tex_input, tex_light_input, tex_light_blocking_input, compute_shader_program);



	return 0;
}