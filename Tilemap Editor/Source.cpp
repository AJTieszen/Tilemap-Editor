// DON'T FORGET TO COPY SFML DLL'S INTO PROJECT FOLDER!

#include<SFML/Graphics.hpp>
#include<SFML/Audio.hpp>
#include<iostream>
#include<fstream>
#include<sstream>
using namespace std;

string tileset, ld_tilemap;


int mode, width = 16, height = 14, tile_id, layer, mouse_x, mouse_y, scroll_x, scroll_y, frame_count, grid_trans = 128;
int tilemap[4][64][64];
bool show_layer[4] = { true, true, true, true };

sf::Texture tiles, ui_txt, ui_txt_2;
sf::RenderTexture tilemap_buffer, tileset_buffer, preview_buffer;
sf::Sprite tile, ui_txt_obj, ui_txt_2_obj, tilemap_buffer_obj, tileset_buffer_obj, preview_buffer_obj;
sf::RectangleShape viewport, active_layer;
sf::RectangleShape visible_layer[4];

void drawTileMenu();
void drawSelectedTile();
void drawTilemap();
void drawGrid();

void updatePreview();

void readMouse();
void swapLayers();
void selectTile();
void scrollTilemap();
void placeTile();

int main()
{
	// Set up editor
	{
		// Setup Blank Tilemap
		cout << "Initializing Blank Tilemap...";
		for (int l = 0; l < 4; l++)
		{
			for (int y = 0; y < 64; y++)
			{
				for (int x = 0; x < 64; x++)
				{
					if (l == 0) tilemap[l][y][x] = 0;
					else tilemap[l][y][x] = 512;
				}
			}
		}
		cout << "Done. \n";
		// Load Graphics
		{
			cout << "Enter Tileset Filename (include extension). \n";
			cin >> tileset;

			tiles.loadFromFile(tileset);
			ui_txt.loadFromFile("UI Text.png");
			ui_txt_2.loadFromFile("UI Text 2.png");
			tile.setTexture(tiles);
			ui_txt_obj.setTexture(ui_txt);
			ui_txt_2_obj.setTexture(ui_txt_2);
		}

		cout << "What would you like to do (enter number)? \n"
			"     1. Load Existing Tilemap \n"
			"     2. Create Small Tilemap (16 x 14) \n"
			"     3. Create Large Tilemap (64 x 64) \n"
			"     4. Exit \n";
		cin >> mode;

		// Load
		if (mode == 1)
		{
			string filename, line;
			int temp, x;


			for (int l = 0; l < 4; l++)
			{
				cout << "Enter Tilemap Filename (include extension) or  's' to skip. Layer = " << l + 1 << "\n";
				cin >> filename;

				if (filename.length() >= 5)
				{
					ifstream file(filename);
					for (int y = 0; y < height; y++)
					{
						x = 0;
						getline(file, line);
						stringstream conv(line);

						while (conv >> temp)
						{
							tilemap[l][y][x] = temp;
							if (conv.peek() == ',') conv.ignore();
							x++;
						}
						if (x > 16)
						{
							width = 64;
							height = 64;
						}
						else
						{
							width = 16;
							height = 14;
						}
					}

					file.close();
				}
			}
		}
		// Small
		else if (mode == 2)
		{
			width = 16;
			height = 14;
		}
		// Large
		else if (mode == 3)
		{
			width = height = 64;
		}
		// Exit
		else return 0;

		// Create Buffers
		tileset_buffer.create(272, 1080);
		tilemap_buffer.create(256, 224);
		preview_buffer.create(256, 256);

		// Set Static Positions and Data
		tileset_buffer_obj.setPosition(1376, 0);
		tilemap_buffer_obj.setPosition(320, 92);
		preview_buffer_obj.setPosition(30, 92);
		ui_txt_2_obj.setPosition(30, 500);

		viewport.setSize(sf::Vector2f(64, 56));
		viewport.setOutlineColor(sf::Color::Black);
		viewport.setOutlineThickness(1);
		viewport.setFillColor(sf::Color(0, 0, 0, 0));

		active_layer.setSize(sf::Vector2f(170, 34));
		active_layer.setOutlineColor(sf::Color::Black);
		active_layer.setOutlineThickness(2);
		active_layer.setFillColor(sf::Color(0, 0, 0, 0));

		for (int i = 0; i < 4; i++)
		{
			visible_layer[i].setSize(sf::Vector2f(18, 18));
			visible_layer[i].setFillColor(sf::Color::Red);
			visible_layer[i].setPosition(170, 504 + 43 * i);
		}
	}

	sf::RenderWindow window(sf::VideoMode(1920, 1080), "Tile Map Editor", sf::Style::Fullscreen);
	window.setFramerateLimit(60);

	while (window.isOpen())
	{
		sf::Event event;
		window.pollEvent(event);
		if (event.type == sf::Event::Closed) { window.close(); } // Program pauses to handle events

		// exit
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) window.close();

		// adjust grid transparency
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Equal)) grid_trans += 4;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Dash)) grid_trans -= 4;
		if (grid_trans > 255) grid_trans = 255;
		if (grid_trans < 0) grid_trans = 0;

		// Editor
		readMouse();
		swapLayers();

		drawTileMenu();
		drawSelectedTile();
		drawTilemap();
		drawGrid();

		selectTile();
		placeTile();
		updatePreview();

		// scroll tilemap
		scrollTilemap();

		// Draw Window
		{
			window.clear(sf::Color::Cyan);

			// Tileset Buffer
			tileset_buffer.display();
			tileset_buffer.setSmooth(false);
			tileset_buffer_obj.setScale(2, 2);
			tileset_buffer_obj.setTexture(tileset_buffer.getTexture());

			// Tilemap Buffer
			tilemap_buffer.display();
			tilemap_buffer.setSmooth(false);
			tilemap_buffer_obj.setScale(4, 4);
			tilemap_buffer_obj.setTexture(tilemap_buffer.getTexture());

			// Preview Buffer
			preview_buffer.display();
			preview_buffer_obj.setTexture(preview_buffer.getTexture());

			// Show viewport
			viewport.setPosition(30 + scroll_x / 4, 92 + scroll_y / 4);

			// Layer Selection
			active_layer.setPosition(25, 498 + layer * 43);

			// Draw window
			window.draw(tileset_buffer_obj);
			window.draw(tilemap_buffer_obj);
			window.draw(preview_buffer_obj);
			window.draw(ui_txt_2_obj);
			window.draw(viewport);
			window.draw(active_layer);

			for (int i = 0; i < 4; i++)
			{
				if (show_layer[i]) window.draw(visible_layer[i]);
			}

			window.display();
		}

		frame_count++;
	}

	// Save
	{
		cout << "What would you like to save (enter number)? \n"
			"1. All layers \n"
			"2. Active layers only \n"
			"3. Nothing \n";
		cin >> mode;
		if (mode == 3) return 0;

		string filename;

		for (int l = 0; l < 4; l++)
		{
			if ((mode == 1) || show_layer[l])
			{
				cout << "Enter a filename for layer " << l + 1 << " (include .csv extension):";
				cin >> filename;
				ofstream file(filename);
				for (int y = 0; y < height; y++)
				{
					for (int x = 0; x < width; x++)
					{
						file << tilemap[l][y][x] << ",";
					}
					file << '\n';
				}
				file.close();
			}
		}
	}

	return 0;
}



void drawTileMenu()
{
	tileset_buffer.clear(sf::Color::Cyan);

	ui_txt_obj.setTextureRect(sf::IntRect(0, 0, 177, 16));
	ui_txt_obj.setPosition(2, 2);
	tileset_buffer.draw(ui_txt_obj);

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			tile.setTextureRect(sf::IntRect(x * 16, y * 16, 16, 16));
			tile.setPosition(x * 17, y * 17 + 20);

			tileset_buffer.draw(tile);
		}
	}

	ui_txt_obj.setTextureRect(sf::IntRect(0, 20, 177, 16));
	ui_txt_obj.setPosition(2, 158);
	tileset_buffer.draw(ui_txt_obj);

	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			tile.setTextureRect(sf::IntRect(x * 16, y * 16 + 128, 16, 16));
			tile.setPosition(x * 17, y * 17 + 176);

			tileset_buffer.draw(tile);
		}
	}

	ui_txt_obj.setTextureRect(sf::IntRect(0, 40, 177, 16));
	ui_txt_obj.setPosition(2, 314);
	tileset_buffer.draw(ui_txt_obj);

	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			tile.setTextureRect(sf::IntRect(x * 16, y * 16 + 256, 16, 16));
			tile.setPosition(x * 17, y * 17 + 330);

			tileset_buffer.draw(tile);
		}
	}

	ui_txt_obj.setTextureRect(sf::IntRect(0, 60, 177, 16));
	ui_txt_obj.setPosition(2, 398);
	tileset_buffer.draw(ui_txt_obj);

	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 16; x++)
		{
			tile.setTextureRect(sf::IntRect(x * 16, y * 16 + 320, 16, 16));
			tile.setPosition(x * 17, y * 17 + 412);

			tileset_buffer.draw(tile);
		}
	}

	ui_txt_obj.setTextureRect(sf::IntRect(0, 78, 177, 14));
	ui_txt_obj.setPosition(2, 524);
	tileset_buffer.draw(ui_txt_obj);
}
void drawSelectedTile()
{
	int x = tile_id % 16, y = (tile_id - x) / 16;

	tile.setTextureRect(sf::IntRect(x * 16, y * 16, 16, 16));
	tile.setPosition(120, 486);
	tile.setScale(2, 2);
	tileset_buffer.draw(tile);
	tile.setScale(1, 1);
}
void drawTilemap()
{
	int id, tile_x, tile_y, map_x, map_y;

	tilemap_buffer.clear(sf::Color::Cyan);

	for (int l = 0; l < 4; l++)
	{
		if (show_layer[l])
		{
			for (int y = 0; y < height; y++)
			{
				map_y = y * 16 - scroll_y;

				if (map_y < -32 || map_y > 224) y++;
				else
				{
					for (int x = 0; x < width; x++)
					{
						map_x = x * 16 - scroll_x;

						if (map_x < -32 || map_x > 256) x++;
						else
						{
							id = tilemap[l][y][x];
							tile_x = id % 16;
							tile_y = (id - tile_x) / 16;

							tile.setPosition(map_x, map_y);
							tile.setTextureRect(sf::IntRect(tile_x * 16, tile_y * 16, 16, 16));
							tilemap_buffer.draw(tile);
						}
					}
				}
			}
		}
	}
}
void drawGrid()
{
	sf::RectangleShape vert_line;
	vert_line.setSize(sf::Vector2f(1, 224));
	vert_line.setFillColor(sf::Color(0, 0, 0, grid_trans));

	sf::RectangleShape horiz_line;
	horiz_line.setSize(sf::Vector2f(256, 1));
	horiz_line.setFillColor(sf::Color(0, 0, 0, grid_trans));

	for (int i = 0; i < 16; i++)
	{
		vert_line.setPosition(16 - scroll_x % 16 + 16 * i, 0);
		horiz_line.setPosition(0, 16 - scroll_y % 16 + 16 * i);
		tilemap_buffer.draw(vert_line);
		tilemap_buffer.draw(horiz_line);
	}
}

void updatePreview()
{
	// Draw Full Tilemap (BG only)
	int y = frame_count % 64, tile_x, tile_y, id;
	for (int x = 0; x < 64; x++)
	{
		id = tilemap[0][y][x];

		tile_x = id % 16;
		tile_y = (id - tile_x) / 16;

		tile.setTextureRect(sf::IntRect(16 * tile_x, 16 * tile_y, 4, 4));
		tile.setPosition(4 * x, 4 * y);
		preview_buffer.draw(tile);
	}
}

void readMouse()
{
	mouse_x = sf::Mouse::getPosition().x;
	mouse_y = sf::Mouse::getPosition().y;
}
void swapLayers()
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1)) layer = 0;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2)) layer = 1;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3)) layer = 2;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num4)) layer = 3;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F1))
	{
		show_layer[0] = !show_layer[0];
		sf::sleep(sf::seconds(0.2));
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F2))
	{
		show_layer[1] = !show_layer[1];
		sf::sleep(sf::seconds(0.2));
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F3))
	{
		show_layer[2] = !show_layer[2];
		sf::sleep(sf::seconds(0.2));
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F4))
	{
		show_layer[3] = !show_layer[3];
		sf::sleep(sf::seconds(0.2));
	}
}
void selectTile()
{
	if (mouse_x < 1376 || mouse_y > 958) return;

	const int x_ofs = 1376;
	int id = -1, x, y, y_ofs = 0;

	if (mouse_y > 39 && mouse_y < 310) y_ofs = 39;
	if (mouse_y > 351 && mouse_y < 622) y_ofs = 80;
	if (mouse_y > 659 && mouse_y < 794) y_ofs = 117;
	if (mouse_y > 823 && mouse_y < 958) y_ofs = 146;

	x = (mouse_x - x_ofs) / 34;
	y = (mouse_y - y_ofs) / 34;
	id = 16 * y + x;

	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
	{
		if (y_ofs != 0) tile_id = id;
		else tile_id = 512;
	}
}
void scrollTilemap()
{
	if (width != 64) return;
	int scroll_speed;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) scroll_speed = 8;
	else scroll_speed = 1;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) scroll_y -= scroll_speed;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) scroll_y += scroll_speed;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) scroll_x -= scroll_speed;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) scroll_x += scroll_speed;

	if (scroll_y > 802) scroll_y = 802;
	if (scroll_y < -2) scroll_y = -2;
	if (scroll_x > 770) scroll_x = 770;
	if (scroll_x < -2) scroll_x = -2;
}
void placeTile()
{
	if (mouse_x < 320 || mouse_x > 1344 || mouse_y < 92 || mouse_y > 988) return;

	int x = (mouse_x - 320 + scroll_x * 4) / 64;
	int y = (mouse_y - 92 + scroll_y * 4) / 64;

	if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
	{
		tilemap[layer][y][x] = tile_id;
	}
}