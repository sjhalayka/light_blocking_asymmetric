#define _CRT_SECURE_NO_WARNINGS

#include "main.h"


#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include <opencv2/opencv.hpp>
using namespace cv;

#pragma comment(lib, "opencv_world4100.lib")



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

void RGBtoHSV(float& fR, float& fG, float &fB, float& fH, float& fS, float& fV) {
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
	cv::Mat image_collage(images_size.height * N, images_size.width * M, CV_8UC4, cv::Scalar(0, 0, 0));

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








int main(int argc, char** argv)
{
	const int lighting_tile_size = 18;
	const int background_tile_size = 36;


	// Load from file
	Mat input_mat = imread("input.png", IMREAD_UNCHANGED);

	if (input_mat.empty() || input_mat.channels() != 4)
	{
		cout << "input.png must be a 32-bit PNG" << endl;
		return -1;
	}

	const int res_x = input_mat.cols;
	const int res_y = input_mat.rows;



// https://stackoverflow.com/questions/64972297/how-to-stitch-an-array-of-images-into-an-mxn-image-collage/64973661#64973661
// https://stackoverflow.com/questions/65091012/how-to-split-an-image-into-m-x-n-tiles


	//std::vector<cv::Mat> array_of_images;
	//array_of_images.push_back(cv::imread("1.jpg"));
	//array_of_images.push_back(cv::imread("2.jpg"));
	//cv::Mat image_collage = imageCollage(array_of_images, 3, 3);

	//cv::imshow("Image Collage", image_collage);
	//cv::waitKey(0);


	int num_tiles_x = res_x / background_tile_size;
	int num_tiles_y = res_y / background_tile_size;

	std::vector<cv::Mat> array_of_images = splitImage(input_mat, num_tiles_x, num_tiles_y);

	for (size_t a = 0; a < array_of_images.size(); a++)
	{
		float r = rand() % 10 * 0.015;

		for (int i = 0; i < array_of_images[a].cols; i++)
		{
			for (int j = 0; j < array_of_images[a].rows; j++)
			{
				Vec4b pixelValue = array_of_images[a].at<Vec4b>(j, i);

				glm::vec3 colour(pixelValue[0] / 255.0f, pixelValue[1] / 255.0f, pixelValue[2] / 255.0f);
				glm::vec3 hsv;
				
				RGBtoHSV(colour.x, colour.y, colour.z, hsv.x, hsv.y, hsv.z);
				
				hsv.x += r*180.0;

				if (hsv.x > 360.0)
					hsv.x = 360.0;

				hsv.z += r;

				if (hsv.z > 1.0)
					hsv.z = 1.0;

				glm::vec3 new_colour;

				HSVtoRGB(new_colour.x, new_colour.y, new_colour.z, hsv.x, hsv.y, hsv.z);

				pixelValue[0] = new_colour.x * 255.0f;
				pixelValue[1] = new_colour.y * 255.0f;
				pixelValue[2] = new_colour.z * 255.0f;
				pixelValue[3] = 255.0f;

				array_of_images[a].at<Vec4b>(j, i) = pixelValue;
			}
		}
	}

	cout << array_of_images.size() << endl;

	cv::Mat image_collage = imageCollage(array_of_images, num_tiles_x, num_tiles_y);

	input_mat = image_collage.clone();



	int largest_dim = res_x;

	if (res_y > largest_dim)
		largest_dim = res_y;


	Mat square_mat(Size(largest_dim, largest_dim), CV_8UC4);
	square_mat = Scalar(0, 0, 0, 255);
	input_mat.copyTo(square_mat(Rect(0, 0, res_x, res_y)));
	input_mat = square_mat.clone();


	Mat input_mat_backup = input_mat.clone();


	resize(input_mat, input_mat, cv::Size(largest_dim / lighting_tile_size, largest_dim / lighting_tile_size), 0, 0, cv::INTER_NEAREST);





	//	imwrite("input_mat.png", input_mat);









	Mat input_light_mat = imread("input_light.png", IMREAD_UNCHANGED);

	if (input_light_mat.empty() || input_light_mat.channels() != 4)
	{
		cout << "input_light.png must be a 32-bit PNG" << endl;
		return -1;
	}

	vector<glm::vec3> distinct_colours;
	vector<glm::vec3> centres;

	map<glm::vec3, size_t> distinct_colour_map; // for acceleration

	for (int i = 0; i < input_light_mat.cols; i++)
	{
		for (int j = 0; j < input_light_mat.rows; j++)
		{
			Vec4b pixelValue = input_light_mat.at<Vec4b>(j, i);

			glm::vec3 colour(pixelValue[0] / 255.0f, pixelValue[1] / 255.0f, pixelValue[2] / 255.0f);

			if (distinct_colour_map[colour] == 0)
			{
				distinct_colour_map[colour]++;
				distinct_colours.push_back(colour);
				centres.push_back(glm::vec3(i, j, 0));
			}
		}
	}

//	cout << distinct_colour_map.size() << endl;

	//for (vector<glm::vec3>::const_iterator ci = distinct_colours.begin(); ci != distinct_colours.end(); ci++)
	//	cout << ci->x << " " << ci->y << " " << ci->z << endl;


	vector<glm::vec3> loop_centres;
	vector<glm::vec3> loop_colours;
			
	for (size_t i = 0; i < distinct_colours.size(); i++)
	{
		Mat mask;

		inRange(input_light_mat,
			Scalar(static_cast<unsigned char>(distinct_colours[i].r * 255.0f), static_cast<unsigned char>(distinct_colours[i].g * 255.0f), static_cast<unsigned char>(distinct_colours[i].b * 255.0f), 255),
			Scalar(static_cast<unsigned char>(distinct_colours[i].r * 255.0f), static_cast<unsigned char>(distinct_colours[i].g * 255.0f), static_cast<unsigned char>(distinct_colours[i].b * 255.0f), 255),
			mask);

		vector<vector<Point> > loop_contours;
		vector<Vec4i> loop_hierarchy;
		findContours(mask, loop_contours, loop_hierarchy, RETR_LIST, CHAIN_APPROX_NONE);

		for (size_t j = 0; j < loop_contours.size(); j++)
		{
			cv::Moments M = cv::moments(loop_contours[j]);

			if (M.m00 == 0)
				continue;

			cv::Point2f loop_centre(M.m10 / M.m00, M.m01 / M.m00);

			Vec4b pixelValue = input_light_mat.at<Vec4b>(loop_centre.y, loop_centre.x);
			loop_centres.push_back(glm::vec3(loop_centre.x, loop_centre.y, 0));
			loop_colours.push_back(glm::vec3(pixelValue[0] / 255.0f, pixelValue[1] / 255.0f, pixelValue[2] / 255.0f));
		}
	}

	Mat square_light_mat(Size(largest_dim, largest_dim), CV_8UC4);
	square_light_mat = Scalar(0, 0, 0, 255);
	input_light_mat = square_light_mat.clone();

	resize(input_light_mat, input_light_mat, cv::Size(largest_dim / lighting_tile_size, largest_dim / lighting_tile_size), 0, 0, cv::INTER_NEAREST);

	for (size_t i = 0; i < loop_centres.size(); i++)
		input_light_mat.at<Vec4b>(loop_centres[i].y / lighting_tile_size, loop_centres[i].x / lighting_tile_size) = Vec4b(loop_colours[i].r * 255.0f, loop_colours[i].g * 255.0f, loop_colours[i].b * 255.0f, 255.0f);

//	imwrite("input_light_mat.png", input_light_mat);







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

	resize(input_light_blocking_mat, input_light_blocking_mat, cv::Size(largest_dim / lighting_tile_size, largest_dim / lighting_tile_size), 0, 0, cv::INTER_NEAREST);







	vector<float> output_pixels((largest_dim / lighting_tile_size) * (largest_dim / lighting_tile_size) * 4, 1.0f);
	vector<float> input_pixels((largest_dim / lighting_tile_size) * (largest_dim / lighting_tile_size) * 4, 1.0f);
	vector<float> input_light_pixels((largest_dim / lighting_tile_size) * (largest_dim / lighting_tile_size) * 4, 1.0f);
	vector<float> input_light_blocking_pixels((largest_dim / lighting_tile_size) * (largest_dim / lighting_tile_size) * 4, 1.0f);


	for (size_t x = 0; x < 4 * ((largest_dim / lighting_tile_size) * (largest_dim / lighting_tile_size)); x += 4)
	{
		input_pixels[x + 0] = input_mat.data[x + 0] / 255.0f;
		input_pixels[x + 1] = input_mat.data[x + 1] / 255.0f;
		input_pixels[x + 2] = input_mat.data[x + 2] / 255.0f;
		input_pixels[x + 3] = 1.0;
	}

	for (size_t x = 0; x < 4 * ((largest_dim / lighting_tile_size) * (largest_dim / lighting_tile_size)); x += 4)
	{
		input_light_pixels[x + 0] = input_light_mat.data[x + 0] / 255.0f;
		input_light_pixels[x + 1] = input_light_mat.data[x + 1] / 255.0f;
		input_light_pixels[x + 2] = input_light_mat.data[x + 2] / 255.0f;
		input_light_pixels[x + 3] = 1.0;
	}

	for (size_t x = 0; x < 4 * ((largest_dim / lighting_tile_size) * (largest_dim / lighting_tile_size)); x += 4)
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
		largest_dim / lighting_tile_size, largest_dim / lighting_tile_size,
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
		largest_dim / lighting_tile_size, largest_dim / lighting_tile_size,
		compute_shader_program,
		output_pixels,
		input_pixels,
		input_light_pixels,
		input_light_blocking_pixels);

	cout << "Computing done" << endl;


	// Save output to PNG file
	Mat uc_output(largest_dim / lighting_tile_size, largest_dim / lighting_tile_size, CV_8UC4);

	for (size_t x = 0; x < 4 * ((largest_dim / lighting_tile_size) * (largest_dim / lighting_tile_size)); x += 4)
	{
		uc_output.data[x + 0] = static_cast<unsigned char>(output_pixels[x + 0] * 255.0);
		uc_output.data[x + 1] = static_cast<unsigned char>(output_pixels[x + 1] * 255.0);
		uc_output.data[x + 2] = static_cast<unsigned char>(output_pixels[x + 2] * 255.0);
		uc_output.data[x + 3] = 255;
	}

	resize(uc_output, uc_output, cv::Size(largest_dim, largest_dim), 0, 0, cv::INTER_LINEAR);

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