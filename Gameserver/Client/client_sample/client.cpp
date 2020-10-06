#define SFML_STATIC 1
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
using namespace std;

#ifdef _DEBUG
#pragma comment (lib, "lib/sfml-graphics-s-d.lib")
#pragma comment (lib, "lib/sfml-window-s-d.lib")
#pragma comment (lib, "lib/sfml-system-s-d.lib")
#pragma comment (lib, "lib/sfml-network-s-d.lib")
#else
#pragma comment (lib, "lib/sfml-graphics-s.lib")
#pragma comment (lib, "lib/sfml-window-s.lib")
#pragma comment (lib, "lib/sfml-system-s.lib")
#pragma comment (lib, "lib/sfml-network-s.lib")
#endif
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "ws2_32.lib")

#include "..\..\protocol.h"


sf::TcpSocket socket;

constexpr auto TILE_WIDTH = 65;
constexpr auto WINDOW_WIDTH = TILE_WIDTH * SCREEN_WIDTH + 10;   // size of window
constexpr auto WINDOW_HEIGHT = TILE_WIDTH * SCREEN_HEIGHT + 10;
constexpr auto BUF_SIZE = 200;
constexpr auto MAX_USER = 10;

int g_left_x;
int g_top_y;
int g_myid;

sf::RenderWindow *g_window;

class OBJECT {
private:
	bool m_showing;
	sf::Sprite m_sprite;
public:
	int m_x, m_y;
	OBJECT(sf::Texture &t, int x, int y, int x2, int y2) {
		m_showing = false;
		m_sprite.setTexture(t);
		m_sprite.setTextureRect(sf::IntRect(x, y, x2, y2));
	}
	OBJECT() {
		m_showing = false;
	}
	void show()
	{
		m_showing = true;
	}
	void hide()
	{
		m_showing = false;
	}

	void a_move(int x, int y) {
		m_sprite.setPosition((float)x, (float)y);
	}

	void a_draw() {
		g_window->draw(m_sprite);
	}

	void move(int x, int y) {
		m_x = x;
		m_y = y;
	}
	void draw() {
		if (false == m_showing) return;
		float rx = (m_x - g_left_x) * 65.0f + 8;
		float ry = (m_y - g_top_y) * 65.0f + 8;
		m_sprite.setPosition(rx, ry);
		g_window->draw(m_sprite);
	}
};

OBJECT avatar;
OBJECT players[MAX_USER];

OBJECT white_tile;
OBJECT black_tile;

sf::Texture *board;
sf::Texture *pieces;

void client_initialize()
{
	board = new sf::Texture;
	pieces = new sf::Texture;
	board->loadFromFile("chessmap.bmp");
	pieces->loadFromFile("chess2.png");
	white_tile = OBJECT{ *board, 5, 5, TILE_WIDTH, TILE_WIDTH };
	black_tile = OBJECT{ *board, 69, 5, TILE_WIDTH, TILE_WIDTH };
	avatar = OBJECT{ *pieces, 128, 0, 64, 64 };
	for (auto& player : players)
		player = OBJECT{ *pieces, 64, 0, 64, 64 };
}

void client_finish()
{
	delete board;
	delete pieces;
}

void ProcessPacket(char *ptr)
{
	//static bool first_time = true;
	std::cout << "recv" << endl;

	switch (ptr[1]) {
	case SC_PLAYER_LOGIN:
	{
		SC_PACKET_PLAYER_LOGIN *packet = reinterpret_cast<SC_PACKET_PLAYER_LOGIN *>(ptr);
		g_myid = packet->id;
		std::cout << "Login ID : " << g_myid << endl;
		avatar.move(packet->x, packet->y);
		avatar.show();
	}
	break;
	//case SC_PUT_PLAYER:
	//{
	//	sc_packet_put_player *my_packet = reinterpret_cast<sc_packet_put_player *>(ptr);
	//	int id = my_packet->id;

	//	if (id == g_myid) {
	//		avatar.move(my_packet->x, my_packet->y);
	//		avatar.show();
	//	}
	//	else if (id < MAX_USER) {
	//		players[id].move(my_packet->x, my_packet->y);
	//		players[id].show();
	//	}
	//	else {
	//		//npc[id - NPC_START].x = my_packet->x;
	//		//npc[id - NPC_START].y = my_packet->y;
	//		//npc[id - NPC_START].attr |= BOB_ATTR_VISIBLE;
	//	}
	//	break;
	//}
	case SC_PLAYER_POSITION:
	{
		SC_PACKET_PLAYER_POSITION *my_packet = reinterpret_cast<SC_PACKET_PLAYER_POSITION *>(ptr);



		int other_id = my_packet->id;

		if (other_id == g_myid) {
			g_left_x = my_packet->x - SCREEN_WIDTH / 2;
			g_top_y = my_packet->y - SCREEN_HEIGHT / 2;
			avatar.move(my_packet->x, my_packet->y);
		}

		else if (other_id < MAX_USER) {
			players[other_id].move(my_packet->x, my_packet->y);
		}

		else {
			//npc[other_id - NPC_START].x = my_packet->x;
			//npc[other_id - NPC_START].y = my_packet->y;
		}

		break;
	}

	//case SC_REMOVE_PLAYER:
	//{
	//	sc_packet_remove_player *my_packet = reinterpret_cast<sc_packet_remove_player *>(ptr);
	//	int other_id = my_packet->id;
	//	if (other_id == g_myid) {
	//		avatar.hide();
	//	}
	//	else if (other_id < MAX_USER) {
	//		players[other_id].hide();
	//	}
	//	else {
	//		//		npc[other_id - NPC_START].attr &= ~BOB_ATTR_VISIBLE;
	//	}
	//	break;
	//}
	default:
		printf("Unknown PACKET type [%d]\n", ptr[1]);
		break;
	}
}

void process_data(char *net_buf, size_t io_byte)
{
	char *ptr = net_buf;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (0 != io_byte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (io_byte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			io_byte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
}

void client_main()
{
	char net_buf[BUF_SIZE];
	size_t	received;

	auto recv_result = socket.receive(net_buf, BUF_SIZE, received);
	if (recv_result == sf::Socket::Error)
	{
		wcout << L"Recv 에러!";
		while (true);
	}
	if (recv_result != sf::Socket::NotReady)
		if (received > 0) process_data(net_buf, received);

	for (int i = 0; i < SCREEN_WIDTH; ++i)
		for (int j = 0; j < SCREEN_HEIGHT; ++j)
		{
			int tile_x = i + g_left_x;
			int tile_y = j + g_top_y;
			if ((tile_x < 0) || (tile_y < 0)) continue;
			if ((WORLD_WIDTH < tile_x) || (WORLD_HEIGHT < tile_y)) continue;
			if (((tile_x + tile_y) % 8) < 2) {
				white_tile.a_move(TILE_WIDTH * i + 7, TILE_WIDTH * j + 7);
				white_tile.a_draw();
			}
			else
			{
				black_tile.a_move(TILE_WIDTH * i + 7, TILE_WIDTH * j + 7);
				black_tile.a_draw();
			}
		}
	avatar.draw();

	for (auto &player : players)
		player.draw();
}

void send_packet_position(char p_type)
{
	CS_PACKET_POSITION packet;
	packet.size = sizeof(packet);
	packet.type = p_type;
	size_t sent = 0;


	socket.send(&packet, sizeof(packet), sent);
}

int main()
{
	wcout.imbue(locale("korean"));
	sf::Socket::Status status = socket.connect("127.0.0.1", SERVER_PORT);
	socket.setBlocking(false);

	if (status != sf::Socket::Done) {
		wcout << L"서버와 연결할 수 없습니다.\n";
		while (true);
	}

	client_initialize();

	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2D CLIENT");
	g_window = &window;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed) {

				switch (event.key.code) {

				case sf::Keyboard::Up:
					cout << "UP" << endl;
					send_packet_position(CS_UP);
					break;

				case sf::Keyboard::Down:
					cout << "DOWN" << endl;
					send_packet_position(CS_DOWN);
					break;

				case sf::Keyboard::Left:
					cout << "LEFT" << endl;
					send_packet_position(CS_LEFT);
					break;

				case sf::Keyboard::Right:
					cout << "RIGHT" << endl;
					send_packet_position(CS_RIGHT);
					break;

				case sf::Keyboard::Escape:
					window.close();
					break;
				}
			}
		}

		window.clear();
		client_main();
		window.display();
	}
	client_finish();

	return 0;
}