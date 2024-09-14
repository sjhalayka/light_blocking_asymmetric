#define _CRT_SECURE_NO_WARNINGS

#include "main.h"














// https://stackoverflow.com/questions/18092240/sqlite-blob-insertion-c
int InsertFile(const string& db_name, const string &file_name)
{
	ifstream file(file_name.c_str(), ios::in | ios::binary);
	if (!file) {
		cerr << "An error occurred opening the file\n";
		return 12345;
	}
	file.seekg(0, ifstream::end);
	streampos size = file.tellg();
	file.seekg(0);

	char* buffer = new char[size];
	file.read(buffer, size);

	sqlite3* db = NULL;
	int rc = sqlite3_open_v2(db_name.c_str(), &db, SQLITE_OPEN_READWRITE, NULL);
	if (rc != SQLITE_OK) {
		cerr << "db open failed: " << sqlite3_errmsg(db) << endl;
	}
	else {
		sqlite3_stmt* stmt = NULL;
		rc = sqlite3_prepare_v2(db,
			"INSERT INTO DEMO_TABLE(DEMO_FILE)"
			" VALUES(?)",
			-1, &stmt, NULL);
		if (rc != SQLITE_OK) {
			cerr << "prepare failed: " << sqlite3_errmsg(db) << endl;
		}
		else {
			// SQLITE_STATIC because the statement is finalized
			// before the buffer is freed:
			rc = sqlite3_bind_blob(stmt, 1, buffer, size, SQLITE_STATIC);
			if (rc != SQLITE_OK) {
				cerr << "bind failed: " << sqlite3_errmsg(db) << endl;
			}
			else {
				rc = sqlite3_step(stmt);
				if (rc != SQLITE_DONE)
					cerr << "execution failed: " << sqlite3_errmsg(db) << endl;
			}
		}
		sqlite3_finalize(stmt);
	}
	sqlite3_close(db);

	delete[] buffer;
}

// https://stackoverflow.com/a/11238683/3634553





int retrieve_table_schema(const string& db_name, const string &table_name)
{
	sqlite3* db;
	sqlite3_stmt* stmt;
	string sql = "SELECT sql FROM sqlite_schema WHERE name = '" + table_name + "';";
	int rc = sqlite3_open(db_name.c_str(), &db);

	if (rc)
	{
		std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
		return rc;
	}

	rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

	if (rc != SQLITE_OK)
	{
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
		return rc;
	}

	bool done = false;
	unsigned char* text = 0;

	while (!done)
	{
		switch (sqlite3_step(stmt))
		{
		case SQLITE_ROW:
			text = const_cast<unsigned char*>(sqlite3_column_text(stmt, 0));
			cout << text << endl;
			break;

		case SQLITE_DONE:
			done = true;
			break;

		default:
			done = true;
			cout << "Failure" << endl;
			break;
		}
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	return 0;
}



int retrieve_table_names(const string& db_name, vector<string> &names)
{
	sqlite3* db;
	sqlite3_stmt* stmt;
	string sql = "SELECT name FROM sqlite_master WHERE type='table';";
	int rc = sqlite3_open(db_name.c_str(), &db);

	if (rc)
	{
		std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
		return rc;
	}

	rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

	if (rc != SQLITE_OK)
	{
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
		return rc;
	}

	bool done = false;
	unsigned char* text = 0;

	while (!done) 
	{
		switch (sqlite3_step(stmt))
		{
			case SQLITE_ROW:
			{
				const int s = sqlite3_column_bytes(stmt, 0);

				const unsigned char* c = const_cast<unsigned char *>(sqlite3_column_text(stmt, 0));

				names.push_back(reinterpret_cast<const char*>(c));

				break;
			}
			case SQLITE_DONE:
			{
				done = true;
				break;
			}
			default:
			{
				done = true;
				// failure
				break;
			}
		}
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	return 0;
}













int retrieve_file(const string& db_name, const string& file_name)
{
	sqlite3* db;
	sqlite3_stmt* stmt;
	string sql = "SELECT demo_file, id FROM demo_table WHERE id = ?";
	int rc = sqlite3_open(db_name.c_str(), &db);

	if (rc) 
	{
		std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
		return rc;
	}

	rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

	if (rc != SQLITE_OK) 
	{
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
		return rc;
	}

	int param_value = 1; // Example value
	sqlite3_bind_int(stmt, 1, param_value);

	rc = sqlite3_step(stmt);

	if (rc == SQLITE_ROW) 
	{
		const void* blob = sqlite3_column_blob(stmt, 0);
		int blob_size = sqlite3_column_bytes(stmt, 0);

		std::vector<unsigned char> blobData((unsigned char*)blob, (unsigned char*)blob + blob_size);

		ofstream f(file_name.c_str(), ios_base::binary);
		f.write(reinterpret_cast<char*>(&blobData[0]), blobData.size() * sizeof(unsigned char));
		f.close();

		cout << "ID: " << sqlite3_column_text(stmt, 1) << endl;

	}
	else if (rc == SQLITE_DONE) 
	{
		std::cout << "No rows found." << std::endl;
	}
	else 
	{
		std::cerr << "Failed to step statement: " << sqlite3_errmsg(db) << std::endl;
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	return 0;
}



int main(int argc, char** argv)
{
	//string filename = "";

	//if(openFileDialog(filename))
	//	cout << filename << endl;	

	//InsertFile("test.db", "C:/temp/1.png");
	//int x = retrieve_file("test.db", "C:/temp/1_out.png");

	//cout << x << endl;

	//return 0;

	

	vector<pair<string, string>> table_name_code_pairs;

	pair<string, string> s;


	s.first = "screens";
	s.second = "";
	s.second += "CREATE TABLE screens\n";
	s.second += "(\n";
	s.second += "	screen_id INTEGER PRIMARY KEY NOT NULL,\n";
	s.second += "\n";
	s.second += "	input_image BLOB,\n";
	s.second += "	input_light_image BLOB,\n";
	s.second += "	input_light_blocker_image BLOB,\n";
	s.second += "	input_traversable_image BLOB,\n";
	s.second += "\n";
	s.second += "	nickname TEXT,\n";
	s.second += "\n";
	s.second += "	north_neighbour_id INTEGER,\n";
	s.second += "	east_neighbour_id INTEGER,\n";
	s.second += "	south_neighbour_id INTEGER,\n";
	s.second += "	west_neighbour_id INTEGER,\n";
	s.second += "\n";
	s.second += "	FOREIGN KEY(north_neighbour_id) REFERENCES screens(screen_id),\n";
	s.second += "	FOREIGN KEY(east_neighbour_id) REFERENCES screens(screen_id),\n";
	s.second += "	FOREIGN KEY(south_neighbour_id) REFERENCES screens(screen_id),\n";
	s.second += "	FOREIGN KEY(west_neighbour_id) REFERENCES screens(screen_id)\n";
	s.second += ");";

	table_name_code_pairs.push_back(s);


	s.first = "portals";
	s.second = "";
	s.second += "CREATE TABLE portals\n";
	s.second += "(\n";
	s.second += "portal_id INTEGER PRIMARY KEY NOT NULL,\n";
	s.second += "\n";
	s.second += "	nickname TEXT,\n";
	s.second += "\n";
	s.second += "	coordinate_x INTEGER,\n";
	s.second += "	coordinate_y INTEGER,\n";
	s.second += "\n";
	s.second += "	screen_id INTEGER NOT NULL,\n";
	s.second += "\n";
	s.second += "	FOREIGN KEY(screen_id) REFERENCES screens(screen_id)\n";
	s.second += ");\n";

	table_name_code_pairs.push_back(s);



	s.first = "portal_pairs";
	s.second = "";
	s.second += "CREATE TABLE portal_pairs\n";
	s.second += "(\n";
	s.second += "	portal_pair_id INTEGER PRIMARY KEY NOT NULL,\n";
	s.second += "\n";
	s.second += "	nickname TEXT,\n";
	s.second += "\n";
	s.second += "	portal_pointer_A INTEGER NOT NULL,\n";
	s.second += "	portal_pointer_B INTEGER NOT NULL,\n";
	s.second += "\n";
	s.second += "	FOREIGN KEY(portal_pointer_A) REFERENCES portals(portal_id),\n";
	s.second += "	FOREIGN KEY(portal_pointer_B) REFERENCES portals(portal_id)\n";
	s.second += ");\n";





	vector<string> found_table_names;
	retrieve_table_names("test.db", found_table_names);

	for (size_t i = 0; i < found_table_names.size(); i++)
	{
		retrieve_table_schema("test.db", found_table_names[i]);
	}


	return 0;





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


	cv::Mat image_collage = imageCollage(array_of_images, num_tiles_x, num_tiles_y);

	input_mat = image_collage.clone();



	int largest_dim = res_x;

	if (res_y > largest_dim)
		largest_dim = res_y;

	GLuint compute_shader_program = 0;
	GLuint compute_shader_program2 = 0;


	if (false == init_gl(
		argc, argv,
		compute_shader_program,
		compute_shader_program2))
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





	if (false == init_character_set())
	{
		cout << "Could not initialize font" << endl;
		return 0;
	}


	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	//ImGuiIO& io = ImGui::GetIO();
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	////io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	//io.WantCaptureMouse = true;

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
	io.WantCaptureMouse = true;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init(glsl_version);

	//SDL_CaptureMouse();



	bool done = false;

	while (false == done)
	{
		int window_w = 0, window_h = 0;
		SDL_GetWindowSize(window, &window_w, &window_h);


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

		//int window_w = 0, window_h = 0;
		//SDL_GetWindowSize(window, &window_w, &window_h);

		float res_x_ratio = res_x * (1.0 / window_w);
		float res_y_ratio = res_y * (1.0 / window_h);

		dynamic_centres.push_back(glm::vec3(mouse_x * res_x_ratio, mouse_y * res_y_ratio, 0.0f));
		dynamic_colours.push_back(glm::vec3(1.0, 0.5, 0.0)); // blue in BGR

		Mat input_light_mat_with_dynamic_lights = input_light_mat.clone();

		// Add in the extra lighting
		for (size_t i = 0; i < dynamic_centres.size(); i++)
			input_light_mat_with_dynamic_lights.at<Vec4b>(dynamic_centres[i].y / lighting_tile_size, dynamic_centres[i].x / lighting_tile_size) += Vec4b(dynamic_colours[i].r * 255.0f, dynamic_colours[i].g * 255.0f, dynamic_colours[i].b * 255.0f, 255.0f);




		int pre_pot_res_x = input_mat.cols;
		int pre_pot_res_y = input_mat.rows;

		int pot = pre_pot_res_x;

		if (pre_pot_res_y > largest_dim)
			pot = pre_pot_res_y;

		pot = pow(2, ceil(log(pot) / log(2)));

		vector<float> output_pixels(4 * pot * pot);

		compute(
			compute_shader_program,
			compute_shader_program2,
			output_pixels,
			input_mat,
			input_light_mat_with_dynamic_lights,
			input_light_blocking_mat);

		Mat uc_output(pot, pot, CV_8UC4);

		for (size_t x = 0; x < (4 * uc_output.rows * uc_output.cols); x += 4)
		{
			uc_output.data[x + 0] = static_cast<unsigned char>(output_pixels[x + 0] * 255.0);
			uc_output.data[x + 1] = static_cast<unsigned char>(output_pixels[x + 1] * 255.0);
			uc_output.data[x + 2] = static_cast<unsigned char>(output_pixels[x + 2] * 255.0);
			uc_output.data[x + 3] = 255;
		}

		//imwrite("_big.png", uc_output);

		// Crop
		uc_output = uc_output(Range(0, pre_pot_res_y), Range(0, pre_pot_res_x));







		uc_output = anti_alias_mat(uc_output);

		resize(uc_output, uc_output, cv::Size(largest_dim, largest_dim), 0, 0, cv::INTER_LINEAR);




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
		


		// Draw
		glViewport(0, 0, window_w, window_h);
		glClearColor(1.0, 0.5, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);




		GLuint tex_uc_output = 0;
		glGenTextures(1, &tex_uc_output);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex_uc_output);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, uc_output.cols, uc_output.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, &uc_output.data[0]);
		glBindImageTexture(0, tex_uc_output, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);

		draw_full_screen_tex(0, tex_uc_output);

		glDeleteTextures(1, &tex_uc_output);




		size_t sentence_width = get_sentence_width(1, mimgs, "Test");

		print_sentence(1.0f, mimgs, ortho_shader.get_program(), window_w, window_h, window_w / 2 - sentence_width / 2, window_h / 3, "Test");





		auto end_time = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float, std::milli> elapsed = end_time - start_time;

		cout << "Computing duration: " << elapsed.count() / 1000.0f << " seconds" << endl;





		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window);
		ImGui::NewFrame();

		ImGui::Begin("Debug");

		// Todo: use tree to list screens, etc.
		if (ImGui::BeginTabBar("##tabbar"), ImGuiTabBarFlags_::ImGuiTabBarFlags_NoTooltip)
		{
			ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

			if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
			{
				if (ImGui::BeginTabItem("Screens"))
				{
					ImGui::Text("Screens tab");
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Characters"))
				{
					ImGui::Text("Characters tab");
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Portal Pairs"))
				{
					ImGui::Text("Portal pairs tab");
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Cinematics"))
				{
					ImGui::Text("Cinematics");
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Global booleans"))
				{
					ImGui::Text("Global booleans");
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
		}

		ImGui::End();





	//	ImGui::ShowDemoWindow();





		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		SDL_GL_SwapWindow(window);


	}

	// Clean up all memory
	glDeleteProgram(compute_shader_program);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}