






// https://stackoverflow.com/questions/18092240/sqlite-blob-insertion-c
int InsertFile(const string& db_name, const string& file_name)
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







string retrieve_table_schema(const string& db_name, const string& table_name)
{
	string ret;

	sqlite3* db;
	sqlite3_stmt* stmt;
	string sql = "SELECT sql FROM sqlite_schema WHERE name = '" + table_name + "';";
	int rc = sqlite3_open(db_name.c_str(), &db);

	if (rc)
	{
		std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
		return "";
	}

	rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

	if (rc != SQLITE_OK)
	{
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
		return "";
	}

	bool done = false;

	while (!done)
	{
		switch (sqlite3_step(stmt))
		{
		case SQLITE_ROW:
		{
			const unsigned char* c = const_cast<unsigned char*>(sqlite3_column_text(stmt, 0));
			ret = reinterpret_cast<const char*>(c);
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
			cout << "Failure" << endl;
			break;
		}
		}
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	return ret;
}





string run_sql(const string& db_name, const string& sql)
{
	string ret;

	sqlite3* db;
	sqlite3_stmt* stmt;

	int rc = sqlite3_open(db_name.c_str(), &db);

	if (rc)
	{
		std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
		return "";
	}

	rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

	if (rc != SQLITE_OK)
	{
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
		return "";
	}

	bool done = false;

	while (!done)
	{
		switch (sqlite3_step(stmt))
		{
		case SQLITE_ROW:
		{
			const unsigned char* c = const_cast<unsigned char*>(sqlite3_column_text(stmt, 0));
			ret = reinterpret_cast<const char*>(c);
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
			cout << "Failure" << endl;
			break;
		}
		}
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	return ret;
}



class screen
{
public:
	size_t screen_id;
	string nickname;

	vector<unsigned char> input_image;
	vector<unsigned char> input_light_image;
	vector<unsigned char> input_light_blocker_image;
	vector<unsigned char> input_traversable_image;
	
	size_t north_neighbour_id;
	size_t east_neighbour_id;
	size_t south_neighbour_id;
	size_t west_neighbour_id;
};













bool insert_screen(const string& db_name, const screen& s)
{
	sqlite3* db;
	sqlite3_stmt* stmt;
	string sql = "INSERT INTO SCREENS (nickname, input_image, input_light_image, input_light_blocker_image, input_traversable_image, north_neighbour_id, east_neighbour_id, south_neighbour_id, west_neighbour_id) VALUES ('" + s.nickname + "', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);";
	int rc = sqlite3_open(db_name.c_str(), &db);

	if (rc)
	{
		std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}

	rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

	if (rc != SQLITE_OK)
	{
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
		return false;
	}

	bool done = false;

	while (!done)
	{
		switch (sqlite3_step(stmt))
		{
		case SQLITE_ROW:
		{
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
			cout << "Failure" << endl;
			break;
		}
		}
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	return true;
}




vector<screen> retrieve_screen_ids_and_nicknames(const string& db_name)
{
	vector<screen> ret;

	sqlite3* db;
	sqlite3_stmt* stmt;
	string sql = "SELECT screen_id, nickname FROM screens ORDER BY nickname;";
	int rc = sqlite3_open(db_name.c_str(), &db);

	if (rc)
	{
		std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
		return ret;
	}

	rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

	if (rc != SQLITE_OK)
	{
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
		return ret;
	}

	bool done = false;

	while (!done)
	{
		switch (sqlite3_step(stmt))
		{
		case SQLITE_ROW:
		{
			screen s;
			s.screen_id = sqlite3_column_int(stmt, 0);
			s.nickname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

			ret.push_back(s);
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
			cout << "Failure" << endl;
			break;
		}
		}
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	return ret;
}




screen retrieve_screen_everything(const string& db_name, size_t screen_id)
{
	screen ret;

	sqlite3* db;
	sqlite3_stmt* stmt;
	string sql = "SELECT screen_id, nickname, input_image, input_light_image, input_light_blocker_image, input_traversable_image, north_neighbour_id, east_neighbour_id, south_neighbour_id, west_neighbour_id FROM screens WHERE screen_id = " + to_string(screen_id) + ";";

	int rc = sqlite3_open(db_name.c_str(), &db);

	if (rc)
	{
		std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
		return ret;
	}

	rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

	if (rc != SQLITE_OK)
	{
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_close(db);
		return ret;
	}

	bool done = false;

	while (!done)
	{
		switch (sqlite3_step(stmt))
		{
		case SQLITE_ROW:
		{
			screen s;
			s.screen_id = sqlite3_column_int(stmt, 0);
			s.nickname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

			// input image
			int blob_size = sqlite3_column_bytes(stmt, 2);

			if (blob_size > 0)
			{
				const void* blob = sqlite3_column_blob(stmt, 2);
				std::vector<unsigned char> blobData((unsigned char*)blob, (unsigned char*)blob + blob_size);
				s.input_image = blobData;
			}

			// input_light_image
			blob_size = sqlite3_column_bytes(stmt, 3);

			if (blob_size > 0)
			{
				const void* blob = sqlite3_column_blob(stmt, 3);
				std::vector<unsigned char> blobData((unsigned char*)blob, (unsigned char*)blob + blob_size);
				s.input_light_image = blobData;
			}

			// input_light_blocker_image
			blob_size = sqlite3_column_bytes(stmt, 4);

			if (blob_size > 0)
			{
				const void* blob = sqlite3_column_blob(stmt, 4);
				std::vector<unsigned char> blobData((unsigned char*)blob, (unsigned char*)blob + blob_size);
				s.input_light_blocker_image = blobData;
			}

			// input_traversable_image
			blob_size = sqlite3_column_bytes(stmt, 5);

			if (blob_size > 0)
			{	
				const void* blob = sqlite3_column_blob(stmt, 5);
				std::vector<unsigned char> blobData((unsigned char*)blob, (unsigned char*)blob + blob_size);
				s.input_traversable_image = blobData;
			}

			s.north_neighbour_id = sqlite3_column_int(stmt, 6);
			s.east_neighbour_id = sqlite3_column_int(stmt, 7);
			s.south_neighbour_id = sqlite3_column_int(stmt, 8);
			s.west_neighbour_id = sqlite3_column_int(stmt, 9);

			ret = s;
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
			cout << "Failure" << endl;
			break;
		}
		}
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	return ret;
}









int retrieve_table_names(const string& db_name, vector<string>& names)
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

			const unsigned char* c = const_cast<unsigned char*>(sqlite3_column_text(stmt, 0));

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








int update_nickname(const string& db_name, int id, const string &new_nickname)
{


	sqlite3* db;
	sqlite3_stmt* stmt;
	string sql = "UPDATE screens SET nickname = '" + new_nickname + "' WHERE screen_id = '" + to_string(id) + "';";
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
