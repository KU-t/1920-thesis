#include "stdafx.h"

sf::TcpSocket socket;
sf::RenderWindow *g_window;
sf::Font* g_font;

Terrain_Manager* TrnMgr;
TIME_MANAGER* TmrMgr;

Player avatar;
unordered_map<Type_ID,Other_Object*> objects;
unordered_set<Bullet*> bullets;
unordered_set<UI_Text*> texts;

void client_initialize()
{
	TrnMgr = new Terrain_Manager("background.bmp", "..\\..\\terrain_base.txt");
	TmrMgr = new TIME_MANAGER();

	g_font = new sf::Font;
	g_font->loadFromFile("Font\\Regular.ttf");

	Bullet::image = new sf::Texture;
	Bullet::image->loadFromFile("baseimage-png.png");

	Bullet::Init_Sprite();

	Object::image = new sf::Texture;
	Object::image->loadFromFile("baseimage-png.png");
	
	Player::Init_Sprite();
	Other_Player::Init_Sprite();
	Monster_type1::Init_Sprite();
	Monster_type2::Init_Sprite();
	Monster_type3::Init_Sprite();

}

void client_finish()
{
	socket.disconnect();
}

void ProcessPacket(char *ptr)
{
	switch (ptr[1]) {
		case SCPT_LOGIN_SUCCESS:
		{
			SCP_LOGIN_SUCCESS *packet = reinterpret_cast<SCP_LOGIN_SUCCESS *>(ptr);
			avatar.id = packet->id;
			avatar.Login__init_data(packet->x, packet->y, packet->x, packet->y,packet->vel, packet->Hp, packet->str);
			std::cout << "Login Success!" << std::endl;
		}
		break;

		case SCPT_LOGIN_FAILURE:
		{
			std::cout << "Login Fail" << std::endl;
		}
		break;

		case SCPT_SIGHT_IN:
		{
			SCP_SIGHT_IN *packet = reinterpret_cast<SCP_SIGHT_IN *>(ptr);
			Type_ID id{ packet->id };
			
			if (id == avatar.id) break;

			if (0 == objects.count(id)) {
				if (id < MAX_PLAYER)				
					objects[id] = reinterpret_cast<Other_Object*>(new Other_Player(packet->x, packet->y, packet->target_x, packet->target_y, packet->vel, packet->Hp));

				else if (id < MAX_MONSTER_TYPE1)	
					objects[id] = reinterpret_cast<Other_Object*>(new Monster_type1(packet->x, packet->y, packet->target_x, packet->target_y, packet->vel, packet->Hp));

				else if (id < MAX_MONSTER_TYPE2)	
					objects[id] = reinterpret_cast<Other_Object*>(new Monster_type2(packet->x, packet->y, packet->target_x, packet->target_y, packet->vel, packet->Hp));
				
				else if (id < MAX_MONSTER_TYPE3)
					objects[id] = reinterpret_cast<Other_Object*>(new Monster_type3(packet->x, packet->y, packet->target_x, packet->target_y, packet->vel, packet->Hp));
			}
			else {
				objects[id]->sight_in(packet->x, packet->y, packet->target_x, packet->target_y, packet->vel, packet->Hp);
			}
			//printf("RECV SIGHT IN (%f / %f)[%f / %f]\n", objects[id]->x, objects[id]->y, objects[id]->target_x, objects[id]->target_y);
		}
		break;

		case SCPT_SIGHT_OUT:
		{
			SCP_SIGHT_OUT *packet = reinterpret_cast<SCP_SIGHT_OUT *>(ptr);
			Type_ID id{ packet->id };

			if (id == avatar.id)	avatar.hide();
			else				objects[id]->hide();
			//printf("sight out \n", id);
		}
		break;
		
		case SCPT_MOVE:
		{
			SCP_MOVE *packet = reinterpret_cast<SCP_MOVE*>(ptr);
			Type_ID id{ packet->id };

			if (id == avatar.id) {
				avatar.move(packet->x, packet->y);
				break;
				//printf("RECV MOVE (%f / %f)[%f / %f]\n", packet->x, packet->y, avatar.target_x, avatar.target_y);
			}


			else  {
				if (0 < objects.count(packet->id))
					objects[id]->move(packet->x, packet->y);
			}
			
			//printf("RECV MOVE (%f / %f)[%f / %f]\n", objects[id]->x, objects[id]->y, objects[id]->target_x, objects[id]->target_y);
			//printf("move \n", id);
		}
		break;

		case SCPT_MOVE_TARGET:
		{
			SCP_MOVE_TARGET *packet = reinterpret_cast<SCP_MOVE_TARGET*>(ptr);
			Type_ID id{ packet->id };

			if (id == avatar.id) 
				avatar.move(packet->x, packet->y, packet->target_x, packet->target_y);
				
			else {
				if (0 < objects.count(packet->id))
					objects[id]->move(packet->x, packet->y, packet->target_x, packet->target_y);
			}
			//printf("RECV MOVE TARGET (%f / %f)[%f / %f]\n", objects[id]->x, objects[id]->y, objects[id]->target_x, objects[id]->target_y);
		}
		break;

		case SCPT_PLAYER_NORMAL_ATTACK_RESULT:
		{
			SCP_PLAYER_ATTACK_RESULT *packet = reinterpret_cast<SCP_PLAYER_ATTACK_RESULT*>(ptr);
			Type_ID monster_id{ packet->monster_id };
			Type_Damage damage{ packet->damage };
			
			if (0 == objects.count(packet->monster_id))	break;

			objects[monster_id]->Hurt(packet->damage); 
			texts.insert(new UI_Text(objects[monster_id], &avatar, damage));
			//printf("SCPT_PLAYER_NORMAL_ATTACK_RESULT \n");
		}
		break;

		case SCPT_PLAYER_SPELL_RESULT:
		{
			SCP_PLAYER_ATTACK_RESULT *packet = reinterpret_cast<SCP_PLAYER_ATTACK_RESULT*>(ptr);
			Type_ID monster_id{ packet->monster_id };
			Type_Damage damage{ packet->damage };

			if (0 == objects.count(packet->monster_id))	break;

			bullets.insert(new Bullet(Bullet::TYPE::PLAYER_SPELL, avatar.x, avatar.y, monster_id, damage));
			//printf("SCPT_PLAYER_SPELL_RESULT \n");
		}
		break;
		
		case SCPT_PLAYER_NORMAL_ATTACK_RESULT_KILL:
		{
			SCP_PLAYER_ATTACK_RESULT *packet = reinterpret_cast<SCP_PLAYER_ATTACK_RESULT*>(ptr);
			Type_ID monster_id{ packet->monster_id };
			Type_Damage damage{ packet->damage };

			if (0 == objects.count(packet->monster_id))	break;

			objects[monster_id]->Set_cant_move();
			objects[monster_id]->Hurt(packet->damage);
			texts.insert(new UI_Text(objects[monster_id], &avatar, damage));
			//printf("SCPT_PLAYER_NORMAL_ATTACK_RESULT \n");
		}
		break;

		case SCPT_PLAYER_SPELL_RESULT_KILL:
		{
			SCP_PLAYER_ATTACK_RESULT *packet = reinterpret_cast<SCP_PLAYER_ATTACK_RESULT*>(ptr);
			Type_ID monster_id{ packet->monster_id };
			Type_Damage damage{ packet->damage };

			if (0 == objects.count(packet->monster_id))	break;

			objects[monster_id]->Set_cant_move();
			bullets.insert(new Bullet(Bullet::TYPE::PLAYER_SPELL, avatar.x, avatar.y, monster_id, damage));
			//printf("SCPT_PLAYER_SPELL_RESULT \n");
		}
		break;

		case SCPT_PLAYER_NORMAL_ATTACK:
		{
			SCP_PLAYER_ATTACK *packet = reinterpret_cast<SCP_PLAYER_ATTACK*>(ptr);
			Type_ID player_id{ packet->player_id }, monster_id{ packet->monster_id };
			Type_Damage damage{ packet->damage };
			bool Do_draw{true};

			if (0 < objects.count(player_id)) {
				Other_Player* p_attack_player{ reinterpret_cast<Other_Player*>(objects[player_id]) };
				if (p_attack_player != nullptr) p_attack_player->Set_Normal_Attack_image();
			}
			else Do_draw = false;
			

			if (0 < objects.count(monster_id)) {
				Object* p_monster{ reinterpret_cast<Other_Player*>(objects[monster_id]) };
				if (p_monster != nullptr) {
					p_monster->Hurt(damage);

					if(true == Do_draw)
						texts.insert(new UI_Text(objects[monster_id], objects[player_id], damage));
				}
			}

			//printf("SCPT_PLAYER_NORMAL_ATTACK \n");
		}
		break;

		case SCPT_PLAYER_SPELL:
		{
			SCP_PLAYER_ATTACK *packet = reinterpret_cast<SCP_PLAYER_ATTACK*>(ptr);
			Type_ID player_id{ packet->player_id }, monster_id{ packet->monster_id };
			Type_Damage damage{ packet->damage };

			if (0 < objects.count(player_id)) {
				Other_Player* p_attack_player{ reinterpret_cast<Other_Player*>(objects[player_id]) };
				if (p_attack_player != nullptr) p_attack_player->Set_Spell_image();
			}

			if (0 < objects.count(monster_id)) 
				bullets.insert(new Bullet(Bullet::TYPE::OTHER_PLAYER_SPELL, objects[player_id]->x, objects[player_id]->y, monster_id, damage));

			//printf("SCPT_PLAYER_SPELL \n");
		}
		break;

		case SCPT_PLAYER_NORMAL_ATTACK_KILL:
		{
			SCP_PLAYER_ATTACK *packet = reinterpret_cast<SCP_PLAYER_ATTACK*>(ptr);
			Type_ID player_id{ packet->player_id }, monster_id{ packet->monster_id };
			Type_Damage damage{ packet->damage };
			bool Do_draw{ true };

			if (0 < objects.count(player_id)) {
				Other_Player* p_attack_player{ reinterpret_cast<Other_Player*>(objects[player_id]) };
				if (p_attack_player != nullptr) p_attack_player->Set_Normal_Attack_image();
			}
			else Do_draw = false;


			if (0 < objects.count(monster_id)) {
				Object* p_monster{ reinterpret_cast<Other_Player*>(objects[monster_id]) };
				if (p_monster != nullptr) {
					p_monster->Hurt(damage);

					if (true == Do_draw)
						texts.insert(new UI_Text(objects[monster_id], objects[player_id], damage));
				}
			}

			//printf("SCPT_PLAYER_NORMAL_ATTACK \n");
		}
		break;

		case SCPT_PLAYER_SPELL_KILL:
		{
			SCP_PLAYER_ATTACK *packet = reinterpret_cast<SCP_PLAYER_ATTACK*>(ptr);
			Type_ID player_id{ packet->player_id }, monster_id{ packet->monster_id };
			Type_Damage damage{ packet->damage };

			if (0 < objects.count(player_id)) {
				Other_Player* p_attack_player{ reinterpret_cast<Other_Player*>(objects[player_id]) };
				if (p_attack_player != nullptr) p_attack_player->Set_Spell_image();
			}

			if (0 < objects.count(monster_id)) {
				objects[monster_id]->Set_cant_move();
				bullets.insert(new Bullet(Bullet::TYPE::OTHER_PLAYER_SPELL, objects[player_id]->x, objects[player_id]->y, monster_id, damage));
			}

			//printf("SCPT_PLAYER_SPELL \n");
		}
		break;

		case SCPT_MONSTER_TYPE1_ATTACK:
		{
			SCP_MONSTER_ATTACK *packet = reinterpret_cast<SCP_MONSTER_ATTACK*>(ptr);
			Type_ID player_id{ packet->player_id }, monster_id{ packet->monster_id };
			Type_Damage damage{ packet->damage };
			
			bool Do_draw{ true };

			if (0 < objects.count(monster_id)) {
				Monster_type1* p_monster{ reinterpret_cast<Monster_type1*>(objects[monster_id]) };
				p_monster->move(packet->x, packet->y, packet->target_x, packet->target_y);
				p_monster->Attack();
			}
			else Do_draw = false;

			if (avatar.id == player_id) {
				avatar.Hurt(damage);
				if (true == Do_draw)
					texts.insert(new UI_Text(&avatar, objects[monster_id], damage));
			}

			else if (0 < objects.count(player_id)) {
				Other_Player* p_player{ reinterpret_cast<Other_Player*>(objects[player_id]) };
				p_player->Hurt(damage);
				if (true == Do_draw)
					texts.insert(new UI_Text(objects[player_id], objects[monster_id], damage));
			}
			printf("SCPT_MONSTER_TYPE1_ATTACK - %d \n", damage);
		}
		break;

		case SCPT_MONSTER_TYPE2_ATTACK:
		{
			SCP_MONSTER_ATTACK *packet = reinterpret_cast<SCP_MONSTER_ATTACK*>(ptr);
			Type_ID player_id{ packet->player_id }, monster_id{ packet->monster_id };
			Type_Damage damage{ packet->damage };
			
			bool Do_draw{ true };

			if (0 < objects.count(monster_id)) {
				Monster_type2* p_monster{ reinterpret_cast<Monster_type2*>(objects[monster_id]) };
				p_monster->move(packet->x, packet->y, packet->target_x, packet->target_y);
				p_monster->Attack();
			}
			else Do_draw = false;

			if (avatar.id == player_id) {
				avatar.Hurt(damage);
				if (true == Do_draw)
					texts.insert(new UI_Text(&avatar, objects[monster_id], damage));
			}

			else if (0 < objects.count(packet->player_id)) {
				Other_Player* p_player{ reinterpret_cast<Other_Player*>(objects[player_id]) };
				p_player->Hurt(damage);
				if (true == Do_draw)
					texts.insert(new UI_Text(objects[player_id], objects[monster_id], damage));
			}
			printf("SCPT_MONSTER_TYPE2_ATTACK - %d \n", damage);
		}
		break;

		case SCPT_MONSTER_TYPE3_ATTACK:
		{
			SCP_MONSTER_ATTACK *packet = reinterpret_cast<SCP_MONSTER_ATTACK*>(ptr);
			Type_ID player_id{ packet->player_id }, monster_id{ packet->monster_id };
			Type_Damage damage{ packet->damage };
			

			if (0 < objects.count(monster_id)) {
				Monster_type3* p_monster{ reinterpret_cast<Monster_type3*>(objects[monster_id]) };
				p_monster->move(packet->x, packet->y, packet->target_x, packet->target_y);
				p_monster->Attack();

				if (avatar.id == player_id) 
					bullets.insert(new Bullet(Bullet::TYPE::MONSTER_TYPE3_ATTACK, objects[monster_id]->x, objects[monster_id]->y, avatar.id, damage));

				else if (0 < objects.count(player_id)) 
					bullets.insert(new Bullet(Bullet::TYPE::MONSTER_TYPE3_ATTACK, objects[monster_id]->x, objects[monster_id]->y, player_id, damage));
				
			}
			printf("SCPT_MONSTER_TYPE3_ATTACK - %d \n", damage);
		}
		break;

		case SCPT_RESPWAN:
		{
			SCP_RESPWAN *packet = reinterpret_cast<SCP_RESPWAN*>(ptr);
			Type_ID id{ packet->id };

			if (id == avatar.id) {
				avatar.Respwan(packet->x, packet->y, packet->vel,packet->Hp,packet->str);
			}

			else if (0 < objects.count(id)) {
				objects[id]->Respwan(packet->x, packet->y, packet->vel, packet->Hp, packet->str);
			}

			else {
				if (id < MAX_PLAYER)
					objects[id] = reinterpret_cast<Other_Object*>(new Other_Player(packet->x, packet->y, packet->x, packet->y, packet->vel, packet->Hp));

				else if (id < MAX_MONSTER_TYPE1)
					objects[id] = reinterpret_cast<Other_Object*>(new Monster_type1(packet->x, packet->y, packet->x, packet->y, packet->vel, packet->Hp));

				else if (id < MAX_MONSTER_TYPE2)
					objects[id] = reinterpret_cast<Other_Object*>(new Monster_type2(packet->x, packet->y, packet->x, packet->y, packet->vel, packet->Hp));

				else if (id < MAX_MONSTER_TYPE3)
					objects[id] = reinterpret_cast<Other_Object*>(new Monster_type3(packet->x, packet->y, packet->x, packet->y, packet->vel, packet->Hp));
			}
		}
		break;

		default:
			//printf("Unknown PACKET type [%d]\n", ptr[1]);
			break;
	}
}

void process_data(char *net_buf, size_t io_byte)
{
	char *ptr{ net_buf };
	static size_t in_packet_size{ 0 };
	static size_t saved_packet_size{ 0 };
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

	auto recv_result{ socket.receive(net_buf, BUF_SIZE, received) };

	if (recv_result == sf::Socket::Error)
	{
		wcout << L"Recv 에러!";
		while (true);
	}

	if (recv_result != sf::Socket::NotReady)
		if (received > 0) process_data(net_buf, received);

	if (true == TmrMgr->Is_time__Update())			Update();
	if (true == avatar.Is_time__Send_Position())	Send_move();
}



int main()
{
	client_initialize();

	wcout.imbue(locale("korean"));
	sf::Socket::Status status = socket.connect("127.0.0.1", SERVER_PORT);
	socket.setBlocking(false);

	if (status != sf::Socket::Done) {
		wcout << L"서버와 연결할 수 없습니다.\n";
		while (true);
	}


	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2D CLIENT");
	g_window = &window;
	
	window.display();

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyReleased || event.type == sf::Event::MouseButtonReleased) {
				if(event.key.code == sf::Keyboard::Escape)	window.close();
				if (avatar.Hp == 0)	continue;

				switch (event.key.code) {
					case sf::Mouse::Right:
					{
						sf::Vector2i click_pos{ sf::Mouse::getPosition(window) };
						Type_POS new_target_x{ avatar.x + ((float)(click_pos.x - (WINDOW_WIDTH / 2)) / TILE_WIDTH_POS) };
						Type_POS new_target_y{ avatar.y + ((float)(click_pos.y - (WINDOW_HEIGHT / 2)) / TILE_WIDTH_POS) };
						avatar.Set_target_position(new_target_x, new_target_y);
					}
					break;

					case sf::Keyboard::Escape:
					{
						window.close();
					}
					break;

					case sf::Keyboard::A:
					{
						if (false == avatar.Can_normal_attack()) break;
						Send_normal_attack();
					}
					break;

					case sf::Keyboard::S:
					{
						if (false == avatar.Can_spell()) break;
						Send_spell();
					}
					break;
				}
			}
		}

		client_main();
	}
	client_finish();

	return 0;
}