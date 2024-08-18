#define _CRT_SECURE_NO_WARNINGS

#include "main.h"









int main(int argc, char** argv)
{
	// Load from file
	//Mat aliased_mat = imread("aliased.png", IMREAD_UNCHANGED);

	//if (aliased_mat.empty() || aliased_mat.channels() != 4)
	//{
	//	cout << "aliased.png must be a 32-bit PNG" << endl;
	//	return -1;
	//}



	//Mat anti_aliased_mat = anti_alias_mat(aliased_mat);


	//imwrite("anti_aliased.png", anti_aliased_mat);

	// 0;













	// Setup SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
	{
		printf("Error: %s\n", SDL_GetError());
		return -1;
	}

	// GL 4.3 + GLSL 430
	const char* glsl_version = "#version 430";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	// From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

	// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Window* window = SDL_CreateWindow("World Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, window_flags);
	if (window == nullptr)
	{
		printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
		return -1;
	}

	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, gl_context);
	SDL_GL_SetSwapInterval(1); // Enable vsync






















	const int lighting_tile_size = 9;
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


	int num_tiles_x = res_x / background_tile_size;
	int num_tiles_y = res_y / background_tile_size;

	std::vector<cv::Mat> array_of_images = splitImage(input_mat, num_tiles_x, num_tiles_y);

	for (size_t a = 0; a < array_of_images.size(); a++)
	{
		float r_brightness = rand() % 10 * 0.015;
		float r_hue = rand() % 10 * 0.015;

		for (int i = 0; i < array_of_images[a].cols; i++)
		{
			for (int j = 0; j < array_of_images[a].rows; j++)
			{
				Vec4b pixelValue = array_of_images[a].at<Vec4b>(j, i);

				glm::vec3 colour(pixelValue[0] / 255.0f, pixelValue[1] / 255.0f, pixelValue[2] / 255.0f);
				glm::vec3 hsv;

				RGBtoHSV(colour.x, colour.y, colour.z, hsv.x, hsv.y, hsv.z);

				hsv.x += r_hue * 360.0;

				if (hsv.x > 360.0)
					hsv.x = 360.0;

				hsv.z += r_brightness;

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

	//	cout << array_of_images.size() << endl;

	cv::Mat image_collage = imageCollage(array_of_images, num_tiles_x, num_tiles_y);

	input_mat = image_collage.clone();



	int largest_dim = res_x;

	if (res_y > largest_dim)
		largest_dim = res_y;

	GLuint compute_shader_program = 0;

	if (false == init_gl(
		argc, argv,
		largest_dim / lighting_tile_size, largest_dim / lighting_tile_size,
		compute_shader_program))
	{
		cout << "Aborting" << endl;
		return -1;
	}

	if (false == ortho_shader.init("ortho.vs.glsl", "ortho.fs.glsl"))
	{
		cout << "Could not load ortho shader" << endl;
		return false;
	}

	uniforms.ortho_shader_uniforms.tex = glGetUniformLocation(ortho_shader.get_program(), "tex");






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

	// Static lights
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


	GLuint tex_output = 0, tex_input = 0, tex_light_input = 0, tex_light_blocking_input = 0;
	init_textures(tex_output, tex_input, tex_light_input, tex_light_blocking_input, largest_dim / lighting_tile_size, largest_dim / lighting_tile_size);







	bool done = false;

	while (false == done)
	{
		auto start_time = std::chrono::high_resolution_clock::now();

		SDL_Event event;

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				done = true;
			}

			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
			{
				done = true;
			}
		}


		vector<glm::vec3> dynamic_centres;
		vector<glm::vec3> dynamic_colours;

		int mouse_x = 0, mouse_y = 0;
		SDL_GetMouseState(&mouse_x, &mouse_y);

		int window_w = 0, window_h = 0;
		SDL_GetWindowSize(window, &window_w, &window_h);

		float res_x_ratio = res_x * (1.0 / window_w);
		float res_y_ratio = res_y * (1.0 / window_h);

		dynamic_centres.push_back(glm::vec3(mouse_x * res_x_ratio, mouse_y * res_y_ratio, 0.0f));
		dynamic_colours.push_back(glm::vec3(0.0, 0.5, 1.0));

		Mat input_light_mat_with_dynamic_lights = input_light_mat.clone();

		// Add in the extra lighting (not using +=, but = )
		for (size_t i = 0; i < dynamic_centres.size(); i++)
			input_light_mat_with_dynamic_lights.at<Vec4b>(dynamic_centres[i].y / lighting_tile_size, dynamic_centres[i].x / lighting_tile_size) = Vec4b(dynamic_colours[i].r * 255.0f, dynamic_colours[i].g * 255.0f, dynamic_colours[i].b * 255.0f, 255.0f);

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
			input_light_pixels[x + 0] = input_light_mat_with_dynamic_lights.data[x + 0] / 255.0f;
			input_light_pixels[x + 1] = input_light_mat_with_dynamic_lights.data[x + 1] / 255.0f;
			input_light_pixels[x + 2] = input_light_mat_with_dynamic_lights.data[x + 2] / 255.0f;
			input_light_pixels[x + 3] = 1.0;
		}

		for (size_t x = 0; x < 4 * ((largest_dim / lighting_tile_size) * (largest_dim / lighting_tile_size)); x += 4)
		{
			input_light_blocking_pixels[x + 0] = input_light_blocking_mat.data[x + 0] / 255.0f;
			input_light_blocking_pixels[x + 1] = input_light_blocking_mat.data[x + 1] / 255.0f;
			input_light_blocking_pixels[x + 2] = input_light_blocking_mat.data[x + 2] / 255.0f;
			input_light_blocking_pixels[x + 3] = 1.0;
		}


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


/*
		vector<float> float_data(3 * (largest_dim / lighting_tile_size) * (largest_dim / lighting_tile_size));

		for (size_t i = 0; i < largest_dim / lighting_tile_size; i++)
		{
			for (size_t j = 0; j < largest_dim / lighting_tile_size; j++)
			{
				size_t float_index = 3 * (j * (largest_dim / lighting_tile_size) + i);
				size_t index = 4 * (j * (largest_dim / lighting_tile_size) + i);

				float_data[float_index + 0] = output_pixels[index + 0];
				float_data[float_index + 1] = output_pixels[index + 1];
				float_data[float_index + 2] = output_pixels[index + 2];
			}
		}

		oidn::DeviceRef dev = oidn::newDevice();
		dev.commit();

		oidn::BufferRef colorBuf = dev.newBuffer((largest_dim / lighting_tile_size) * (largest_dim / lighting_tile_size) * 3 * sizeof(float));
		colorBuf.write(0, (largest_dim / lighting_tile_size)* (largest_dim / lighting_tile_size) * 3 * sizeof(float), &float_data[0]);

		oidn::FilterRef filter = dev.newFilter("RT");

		filter.setImage("color", colorBuf, oidn::Format::Float3, (largest_dim / lighting_tile_size), (largest_dim / lighting_tile_size));
		filter.setImage("output", colorBuf, oidn::Format::Float3, (largest_dim / lighting_tile_size), (largest_dim / lighting_tile_size));
		filter.set("hdr", false); // Do not enable this, or the lights will look strange
		filter.commit();
		filter.execute();

		// Check for errors
		const char* errorMessage;
		if (dev.getError(errorMessage) != oidn::Error::None)
			cout << errorMessage << endl;// MessageBox(NULL, errorMessage, "Error", MB_OK);

		colorBuf.read(0, (largest_dim / lighting_tile_size) * (largest_dim / lighting_tile_size) * 3 * sizeof(float), &float_data[0]);

		for (size_t i = 0; i < (largest_dim / lighting_tile_size); i++)
		{
			for (size_t j = 0; j < (largest_dim / lighting_tile_size); j++)
			{
				size_t uc_index = 4 * (j * (largest_dim / lighting_tile_size) + i);
				size_t data_index = 3 * (j * (largest_dim / lighting_tile_size) + i);

				output_pixels[uc_index + 0] = float_data[data_index + 0];
				output_pixels[uc_index + 1] = float_data[data_index + 1];
				output_pixels[uc_index + 2] = float_data[data_index + 2];
				output_pixels[uc_index + 3] = 1.0f;
			}
		}

*/





		// convert output_pixels to mat, anti-alias, then convert back to float






		Mat uc_output(largest_dim / lighting_tile_size, largest_dim / lighting_tile_size, CV_8UC4);

		for (size_t x = 0; x < 4 * ((largest_dim / lighting_tile_size) * (largest_dim / lighting_tile_size)); x += 4)
		{
			uc_output.data[x + 0] = static_cast<unsigned char>(output_pixels[x + 0] * 255.0);
			uc_output.data[x + 1] = static_cast<unsigned char>(output_pixels[x + 1] * 255.0);
			uc_output.data[x + 2] = static_cast<unsigned char>(output_pixels[x + 2] * 255.0);
			uc_output.data[x + 3] = 255;
		}


		uc_output = anti_alias_mat(uc_output);



		resize(uc_output, uc_output, cv::Size(largest_dim, largest_dim), 0,0, cv::INTER_LINEAR);




		for (size_t x = 0; x < (largest_dim * largest_dim * 4); x += 4)
		{
			uc_output.data[x + 0] = static_cast<unsigned char>(255.0 * (input_mat_backup.data[x + 0] / 255.0 * uc_output.data[x + 0] / 255.0));
			uc_output.data[x + 1] = static_cast<unsigned char>(255.0 * (input_mat_backup.data[x + 1] / 255.0 * uc_output.data[x + 1] / 255.0));
			uc_output.data[x + 2] = static_cast<unsigned char>(255.0 * (input_mat_backup.data[x + 2] / 255.0 * uc_output.data[x + 2] / 255.0));
			uc_output.data[x + 3] = 255;

			unsigned char temp = uc_output.data[x + 0];
			uc_output.data[x + 0] = uc_output.data[x + 2];
			uc_output.data[x + 2] = temp;
			
		}





		// Crop
		uc_output = uc_output(Range(0, res_y), Range(0, res_x));







		glViewport(0, 0, window_w, window_h);
		glClearColor(1.0, 0.5, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		GLuint tex_uc_output = 0;

		glGenTextures(1, &tex_uc_output);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, tex_uc_output);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glActiveTexture(GL_TEXTURE4);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, uc_output.cols, uc_output.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, &uc_output.data[0]);
		glBindImageTexture(4, tex_uc_output, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);

		draw_full_screen_tex(4, tex_uc_output);

		glDeleteTextures(1, &tex_uc_output);

		auto end_time = std::chrono::high_resolution_clock::now();

		std::chrono::duration<float, std::milli> elapsed = end_time - start_time;

		//cout << "Computing duration: " << elapsed.count() / 1000.0f << " seconds" << endl;
		//imwrite("out.png", uc_output);

		SDL_GL_SwapWindow(window);
	}

	// Clean up all memory
	delete_all(tex_output, tex_input, tex_light_input, tex_light_blocking_input, compute_shader_program);

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}