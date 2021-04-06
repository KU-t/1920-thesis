#pragma once
#include "stdafx.h"

sf::Texture* Object_Base::image = nullptr;
sf::Texture* Bullet::image = nullptr;

sf::Sprite* Bullet::sprite_player_spell = nullptr;
sf::Sprite* Bullet::sprite_other_player_spell = nullptr;
sf::Sprite* Bullet::sprite_monster_type3_attack = nullptr;

sf::Sprite* Player::sprite_normal = nullptr;
sf::Sprite* Player::sprite_normal_attack = nullptr;
sf::Sprite* Player::sprite_spell = nullptr;
sf::Sprite* Player::sprite_hurt = nullptr;
sf::Sprite* Player::sprite_die = nullptr;

sf::Sprite* Other_Player::sprite_normal = nullptr;
sf::Sprite* Other_Player::sprite_normal_attack = nullptr;
sf::Sprite* Other_Player::sprite_spell = nullptr;
sf::Sprite* Other_Player::sprite_hurt = nullptr;
sf::Sprite* Other_Player::sprite_die = nullptr;

sf::Sprite* Monster_type1::sprite_normal = nullptr;
sf::Sprite* Monster_type1::sprite_targeting = nullptr;
sf::Sprite* Monster_type1::sprite_hurt = nullptr;
sf::Sprite* Monster_type1::sprite_attack = nullptr;
sf::Sprite* Monster_type1::sprite_die = nullptr;

sf::Sprite* Monster_type2::sprite_normal = nullptr;
sf::Sprite* Monster_type2::sprite_targeting = nullptr;
sf::Sprite* Monster_type2::sprite_hurt = nullptr;
sf::Sprite* Monster_type2::sprite_attack = nullptr;
sf::Sprite* Monster_type2::sprite_die = nullptr;

sf::Sprite* Monster_type3::sprite_normal = nullptr;
sf::Sprite* Monster_type3::sprite_targeting = nullptr;
sf::Sprite* Monster_type3::sprite_hurt = nullptr;
sf::Sprite* Monster_type3::sprite_attack = nullptr;
sf::Sprite* Monster_type3::sprite_die = nullptr;

void Terrain_Manager::Terrain_Base::draw(const float& _x, const float& _y)
{
	tile.setPosition(_x, _y);
	g_window->draw(tile);
}


Type_POS Terrain_Manager::Get_terrain_height(const int& x, const int& y)
{
	return height_map[(int)(x)+((int)(y) * (int)(WORLD_MAP_SIZE_WIDTH))];
}

Type_POS Terrain_Manager::Get_obj_height(Object_Base* const obj)
{
	int xi{ (int)(obj->x) }, yi{ (int)(obj->y) };
	float xr{ obj->x - (float)(xi) }, yr{ obj->y - (float)(yi) };

	int next_xi{ xi + 1 }, next_yi{ yi + 1 };

	if (xi == WORLD_MAP_SIZE_WIDTH - 1)		next_xi = xi;
	if (yi == WORLD_MAP_SIZE_HEIGHT - 1)	next_yi = yi;

	Type_POS left_top_ter{ Get_terrain_height(xi, yi) },
		right_top_ter{ Get_terrain_height(next_xi, yi) },
		left_bottom_ter{ Get_terrain_height(xi, next_yi) },
		right_bottom_ter{ Get_terrain_height(next_xi, next_yi) };


	Type_POS height;

#ifdef __LINEAR_INTERPOLATION_RECTANGLE__
	height = (left_top_ter * (1 - xr) * (1 - yr)) + (right_top_ter * xr * (1 - yr))
		+ (left_bottom_ter * (1 - xr) * yr) + (right_bottom_ter * xr * yr);
#endif

#ifdef __LINEAR_INTERPOLATION_TRIANGLE__
	float uh, vh;

	if (xr <= 1.f - yr) {
		uh = right_bottom_ter - left_bottom_ter;
		vh = left_bottom_ter - left_top_ter;
		height = left_bottom_ter + (uh * xr) + (vh * yr);
	}
	else {
		uh = right_top_ter - left_top_ter;
		vh = right_bottom_ter - right_top_ter;
		height = right_top_ter + (uh * (1.f - xr)) + (vh * (1.f - yr));
	}

#endif

	return height;
}

Terrain_Manager::Terrain_Manager(const char* image_file, const char* height_file)
{
	texture = new sf::Texture;
	texture->loadFromFile(image_file);

	tile_image0 = new Terrain_Base{ *texture, 0, 0, (int)TILE_WIDTH_POS, (int)TILE_WIDTH_POS };
	tile_image1 = new Terrain_Base{ *texture, 64, 0, (int)TILE_WIDTH_POS, (int)TILE_WIDTH_POS };
	tile_image2 = new Terrain_Base{ *texture, 128, 0, (int)TILE_WIDTH_POS, (int)TILE_WIDTH_POS };
	tile_image3 = new Terrain_Base{ *texture, 192, 0, (int)TILE_WIDTH_POS, (int)TILE_WIDTH_POS };
	tile_image4 = new Terrain_Base{ *texture, 256, 0, (int)TILE_WIDTH_POS, (int)TILE_WIDTH_POS };
	tile_image5 = new Terrain_Base{ *texture, 320, 0, (int)TILE_WIDTH_POS, (int)TILE_WIDTH_POS };

	height_map = new float[(int)WORLD_MAP_SIZE];

	std::ifstream Terrain_File;
	Terrain_File.open(height_file);

	if (Terrain_File.is_open())
		printf("file_open_succes\n");

	for (int i = 0; i < (int)WORLD_MAP_SIZE; i++)
		Terrain_File >> height_map[i];

	Terrain_File.close();
}

bool Terrain_Manager::Can_move(Object_Base* const obj)
{
	if (obj->x < 0.f)								return false;
	if (WORLD_MAP_SIZE_WIDTH - 1.f < obj->x)			return false;
	if (obj->y < 0.f)								return false;
	if (WORLD_MAP_SIZE_HEIGHT - 1.f < obj->y)		return false;

	//Type_POS height{ Get_obj_height(obj) };
	
	//if (height < 1.f)	return false;
	//if (4.f < height)	return false;

	return true;
}

void Terrain_Manager::delete_Obj()
{
	delete height_map;
	delete texture;
	delete tile_image1;
	delete tile_image2;
}


void Terrain_Manager::draw()
{
	float view_left{ avatar.x - PLAYER_VIEW_SIZE_WIDTH };
	float view_right{ avatar.x + PLAYER_VIEW_SIZE_WIDTH };
	float view_top{ avatar.y - PLAYER_VIEW_SIZE_HEIGHT };
	float view_bottom{ avatar.y + PLAYER_VIEW_SIZE_HEIGHT };
	float view_partx{ avatar.x - (int)avatar.x };
	float view_party{ avatar.y - (int)avatar.y };

	for (int i = (int)view_left; i < (int)view_right * 2 + 1; ++i) {
		for (int j = (int)view_top; j < (int)view_bottom * 2 + 1; ++j) {

			float x{ (i - view_left) * TILE_WIDTH_POS - view_partx - (TILE_WIDTH) };
			float y{ (j - view_top) * TILE_WIDTH_POS - view_party - (TILE_WIDTH) };

			if ((i < 0) || ((int)WORLD_MAP_SIZE_WIDTH - 1 < i))		continue;
			if ((j < 0) || ((int)WORLD_MAP_SIZE_HEIGHT - 1 < j))	continue;
			
			float height{ height_map[i + (j * (int)WORLD_MAP_SIZE_WIDTH)] };

			if (height < 0.5f)		tile_image0->draw(x, y);
			else if (height < 1.5f)	tile_image1->draw(x, y);
			else if (height < 2.5f)	tile_image2->draw(x, y);
			else if (height < 3.5f)	tile_image3->draw(x, y);
			else if (height < 4.5f)	tile_image4->draw(x, y);
			else if (height < 5.5f)	tile_image5->draw(x, y);
		}
	}
}



void Object::move_update(const duration<float, std::milli>& gap_time)
{
	if (moving == false) return;
	if (Hp < 1)			return;

	if (x == target_x && y == target_y) {
		moving = false;		
		return;
	}

	float pred_x{ x }, pred_y{ y };

	float length_rate{ vel * 0.001f * TIME_MOVE_INTERPOLATION * 2};

	float lenght_x{ server_x - x };
	float lenght_y{ server_y - y };
	
	float length{ hypotf(lenght_x, lenght_y) };

	if (length_rate < length) {
		x = server_x;
		y = server_y;
	}

	length_rate = vel * 0.001f * gap_time.count();

	lenght_x = target_x - x;
	lenght_y = target_y - y;

	length = hypotf(lenght_x, lenght_y);
	
	if (length < length_rate) {
		x = target_x;
		y = target_y;
		
		return;
	}
	
	float move_x{ length_rate * lenght_x / length };
	float move_y{ length_rate * lenght_y / length };

	x += move_x;
	y += move_y;
	server_x += move_x;
	server_y += move_y;
	
	if (false == TrnMgr->Can_move(this)) 
	{
		x = pred_x;
		y = pred_y;
		moving = false;
	}
}

void Other_Object::sight_in
	(const Type_POS& _x, const Type_POS& _y, const Type_POS& _target_x, const Type_POS& _target_y, const Type_POS& _vel, const Type_Hp& _Hp)
{
	m_showing = true;

	x = _x;
	y = _y;

	server_x = _x;
	server_y = _y;

	target_x = _target_x;
	target_y = _target_y;

	vel = _vel;
	Hp = _Hp;

	moving = true;
}

void Object::Respwan
	(const Type_POS& _x, const Type_POS& _y,const Type_POS& _vel,
		const Type_Hp& _Hp, const Type_Damage &)
{
	x = _x;
	y = _y;

	server_x = _x;
	server_y = _y;

	target_x = _x;
	target_y = _y;

	vel = _vel;
	Hp = _Hp;

	moving = true;
}

void Object::Set_Hurt_image()
{
	draw_hurt_timer.Set_time(COOLTIME__DRAW_HURT);
}

void Object::Hurt(const Type_Damage& damage)
{
	if (0 == Hp)	return;

	Set_Hurt_image();
	Hp -= damage;

	if (Hp < 0) Hp = 0;
}

void Object::Set_cant_move()
{
	moving = false;
}

void Object::Die()
{
	Hp = 0;
	moving = false;
}

bool Object::Is_Die()
{
	if (0 < Hp)	return false;
	return true;
}


bool Other_Object::Check_target(const Type_POS& mouse_x, const Type_POS& mouse_y)
{
	if (true == Is_Die())	return false;

	Type_POS size{ Get_Size() };
	if (size < mouse_x - x)	return false;
	if (size < mouse_y - y)	return false;
	if (size < x - mouse_x)	return false;
	if (size < y - mouse_y)	return false;

	float distance{ sqrtf(pow(mouse_x - x, 2) + pow(mouse_y - y, 2)) };
	if (size < distance)	return false;

	return true;
}






Player::Player()
{
	m_font.loadFromFile("Font\\Bold.ttf");
	
	UI_Hp.setFont(m_font);
	UI_Str.setFont(m_font);
	UI_Vel.setFont(m_font);

	moving = false;
	Is_Time__send_position = false;
	Is_Modify__target_position = false;

	target_object.p_object = nullptr;
	target_object.id = -1;

	timer_normal_attack.Set_time(0);
	timer_spell.Set_time(0);
}

void Player::Init_Sprite()
{
	sprite_normal = new sf::Sprite;
	sprite_normal->setTexture(*image);
	sprite_normal->setTextureRect(sf::IntRect(0, 64, 64, 64));

	sprite_hurt = new sf::Sprite;
	sprite_hurt->setTexture(*image);
	sprite_hurt->setTextureRect(sf::IntRect(64, 64, 64, 64));

	sprite_normal_attack = new sf::Sprite;
	sprite_normal_attack->setTexture(*image);
	sprite_normal_attack->setTextureRect(sf::IntRect(128, 64, 64, 64));

	sprite_spell = new sf::Sprite;
	sprite_spell->setTexture(*image);
	sprite_spell->setTextureRect(sf::IntRect(192, 64, 64, 64));

	sprite_die = new sf::Sprite;
	sprite_die->setTexture(*image);
	sprite_die->setTextureRect(sf::IntRect(448, 64, 64, 64));
}

void Player::move_update(const duration<float, std::milli>& gap_time)
{
	if (moving == false) return;
	if (Hp < 1)			return;

	float pred_x{ x }, pred_y{ y };

	float length_rate{ vel * 0.001f * TIME_MOVE_INTERPOLATION * 2 };

	float lenght_x{ server_x - x };
	float lenght_y{ server_y - y };

	float length{ hypotf(lenght_x, lenght_y) };

	if (length_rate < length) {
		x = server_x;
		y = server_y;
	}

	length_rate = vel * 0.001f * gap_time.count();

	lenght_x = target_x - x;
	lenght_y = target_y - y;

	length = hypotf(lenght_x, lenght_y);

	if (length < length_rate) {
		x = target_x;
		y = target_y;
		moving = false;
		return;
	}

	float move_x{ length_rate * lenght_x / length };
	float move_y{ length_rate * lenght_y / length };

	x += move_x;
	y += move_y;
	server_x += move_x;
	server_y += move_y;

	if (false == TrnMgr->Can_move(this))
	{
		x = pred_x;
		y = pred_y;
		moving = false;
	}

	if (false == Is_Time__send_position) 
		Is_Time__send_position = true;
	
}

void Player::draw()
{
	float rx = WINDOW_WIDTH / 2 - TILE_WIDTH;
	float ry = WINDOW_HEIGHT / 2 - TILE_WIDTH;
	
	g_window->draw(UI_Hp);
	g_window->draw(UI_Str);
	g_window->draw(UI_Vel);

	if (Hp < 1) {
		sprite_die->setPosition(rx, ry);
		g_window->draw(*sprite_die);
		return;
	}

	sprite_normal->setPosition(rx, ry);
	g_window->draw(*sprite_normal);

	if (true == draw_hit_timer.Is_running_time()) {
		
		if (NORMAL_ATTACK == attack_type) {
			sprite_normal_attack->setPosition(rx, ry);
			g_window->draw(*sprite_normal_attack);
		}
		else if (SPELL == attack_type) {
			sprite_spell->setPosition(rx, ry);
			g_window->draw(*sprite_spell);
		}
	}
	
	if (true == draw_hurt_timer.Is_running_time()) {
		sprite_hurt->setPosition(rx, ry);
		g_window->draw(*sprite_hurt);
	}
}

Type_POS Player::Get_Size()
{
	return SIZE_PLAYER;
}

void Player::Respwan
	(const Type_POS& _x, const Type_POS& _y, const Type_POS& _vel,
		const Type_Hp& _Hp, const Type_Damage& _str) 
{
	moving = false;

	x = _x;
	y = _y;

	server_x = _x;
	server_y = _y;

	target_x = _x;
	target_y = _y;

	Hp = _Hp;
	vel = _vel;
	str = _str;

	char buf[SIZE_BUFFER_TEXT_UI];

	sprintf_s(buf, "HP : %d", Hp);
	UI_Hp.setString(buf);
}

void Player::Hurt(const Type_Damage& damage)
{
	if (0 == Hp)	return;

	Set_Hurt_image();
	Hp -= damage;

	if (Hp < 0) Hp = 0;

	char buf[SIZE_BUFFER_TEXT_UI];

	sprintf_s(buf, "HP : %d", Hp);
	UI_Hp.setString(buf);
}


void Player::Login__init_data
	(const Type_POS& _x, const Type_POS& _y, const Type_POS& _target_x, const Type_POS& _target_y, const Type_POS& _vel, const Type_Hp& _Hp, const Type_POS& _str)
{
	x = _x;
	y = _y;

	server_x = _x;
	server_y = _y;

	target_x = _target_x;
	target_y = _target_y;

	Hp = _Hp;
	vel = _vel;
	str = _str;

	char buf[SIZE_BUFFER_TEXT_UI];

	sprintf_s(buf, "HP : %d", Hp);
	UI_Hp.setString(buf);
	UI_Hp.setPosition(10, WINDOW_HEIGHT - 150);

	sprintf_s(buf, "STR : %d", str);
	UI_Str.setString(buf);
	UI_Str.setPosition(10, WINDOW_HEIGHT - 100);

	sprintf_s(buf, "VEL : %f", vel);
	UI_Vel.setString(buf);
	UI_Vel.setPosition(10, WINDOW_HEIGHT - 50);
}

void Player::Set_target_position(const Type_POS& new_target_x, const Type_POS& new_target_y)
{
	target_x = new_target_x;
	target_y = new_target_y;

	server_x = x;
	server_y = y;

	moving = true;
	Is_Modify__target_position = true;
}

bool Player::Is_time__Send_Position()
{
	if (false == Is_Time__send_position)	return false;

	if (true == timer_send_move.Is_running_time())	return false;

	Is_Time__send_position = false;
	timer_send_move.Set_time(CS_TIME_SEND_PLAYER_MOVE);

	return true;
}

bool Player::Get_send_target_position()
{
	if (false == Is_Modify__target_position)	return false;

	Is_Modify__target_position = false;

	return true;
}

void Player::Set_target_object(Object* const p_object, const Type_ID& id)
{
	target_object.p_object = p_object;
	target_object.id = id;
}

Object* Player::Get_target_object()
{
	return target_object.p_object;
}

bool Player::Can_normal_attack()
{
	if (nullptr == target_object.p_object) return false;

	Type_POS distance{ sqrtf(pow(target_object.p_object->x - x, 2) + pow(target_object.p_object->y - y, 2)) };

	if (Get_Size() + target_object.p_object->Get_Size() + DISTANCE_PLAYER_NORMAL_ATTACK < distance) return false;

	bool result{ timer_normal_attack.Is_running_time() };

	if (true == result) return false;

	draw_hit_timer.Set_time(COOLTIME__DRAW_PLAYER_NORMAL_ATTACK);
	timer_normal_attack.Set_time(COOLTIME_PLAYER_NORMAL_ATTACK);
	attack_type = NORMAL_ATTACK;

	return true;
}

bool Player::Can_spell()
{
	if (nullptr == target_object.p_object) return false;

	float distance{ sqrtf(pow(target_object.p_object->x - x, 2) + pow(target_object.p_object->y - y, 2)) };

	if (DISTANCE_PLAYER_SPELL < distance) return false;
	
	if (true == timer_spell.Is_running_time()) return false;

	draw_hit_timer.Set_time(COOLTIME__DRAW_PLAYER_SPELL);
	timer_spell.Set_time(COOLTIME_PLAYER_SPELL);
	attack_type = SPELL;

	return true;
}

void TIME_MANAGER::Set_draw_time()
{
	pred_draw_time = high_resolution_clock::now();
}

nanoseconds TIME_MANAGER::Get_gap_time()
{
	high_resolution_clock::time_point curr_time = high_resolution_clock::now();
	nanoseconds gap_time = curr_time - pred_draw_time;
	pred_draw_time = curr_time;

	return gap_time;
}

bool TIME_MANAGER::Is_time__Update()
{
	if (pred_draw_time + milliseconds(C_TIME_UPDATE_OBJECT) < high_resolution_clock::now())
		return true;
	
	return false;
}




void Update()
{
	Update_Objects_State();
	Draw_Objects();
}

void Update_Objects_State()
{
	duration<float, std::milli> gap_time{ TmrMgr->Get_gap_time() };
	
	sf::Vector2i mouse_pos{ sf::Mouse::getPosition(*g_window) };
	Type_POS mouse_pos_x{ avatar.x + ((float)(mouse_pos.x - (WINDOW_WIDTH / 2)) / TILE_WIDTH_POS) };
	Type_POS mouse_pos_y{ avatar.y + ((float)(mouse_pos.y - (WINDOW_HEIGHT / 2)) / TILE_WIDTH_POS) };
	
	Type_ID target_object_id{ -1 };

	for (auto& object : objects) {
		object.second->move_update(gap_time);

		if (true == object.second->Check_target(mouse_pos_x, mouse_pos_y)) {
			target_object_id = object.first;
		}
	}

	if (-1 != target_object_id)
		avatar.Set_target_object(objects[target_object_id], target_object_id);

	else
		avatar.Set_target_object(nullptr, -1);
	
	avatar.move_update(gap_time);

	vector<UI_Text*> vec_delete_text;

	for (UI_Text* text : texts) {
		if (true == text->Can_destroy()) {
			vec_delete_text.emplace_back(text);
		}
	}

	vector<Bullet*> vec_delete_bullet;

	for (Bullet* bullet : bullets) {
		if (false == bullet->move_update(gap_time)) {
			bullet->attack();
			vec_delete_bullet.emplace_back(bullet);
		}
	}

	for (UI_Text* text : vec_delete_text) {
		texts.erase(text);
		delete text;
	}
	

	for (Bullet* bullet : vec_delete_bullet) {
		bullets.erase(bullet);
		delete bullet;
	}
}

void Draw_Objects()
{
	g_window->clear();

	TrnMgr->draw();

	for (auto &object : objects)
		object.second->draw();

	avatar.draw();

	for (auto& bullet : bullets)
		bullet->draw();
			
	for (auto& text : texts)
		text->draw();

	g_window->display();
}


void Send_move()
{
	if (true == avatar.Get_send_target_position()) {
		CSP_MOVE_TARGET packet;
		packet.size = sizeof(packet);
		packet.type = CSPT_MOVE_TARGET;
		packet.x = avatar.x;
		packet.y = avatar.y;
		packet.target_x = avatar.target_x;
		packet.target_y = avatar.target_y;
		size_t sent = 0;
		socket.send(&packet, sizeof(packet), sent);
		//printf("SEND MOVE TARGET (%f / %f)[%f / %f]\n", avatar.x, avatar.y, avatar.target_x, avatar.target_y);
	}

	else {
		CSP_MOVE packet;
		packet.size = sizeof(packet);
		packet.type = CSPT_MOVE;
		packet.x = avatar.x;
		packet.y = avatar.y;
		size_t sent = 0;
		socket.send(&packet, sizeof(packet), sent);
		//printf("SEND MOVE (%f / %f)[%f / %f]\n", avatar.x, avatar.y, avatar.target_x, avatar.target_y);
	}
}

void Send_normal_attack()
{
	CSP_PLAYER_ATTACK packet;
	packet.size = sizeof(packet);
	packet.type = CSPT_PLAYER_NORMAL_ATTACK;
	packet.monster_id = avatar.target_object.id;
	size_t sent = 0;
	socket.send(&packet, sizeof(packet), sent);
	//printf("SEND Normal attack\n");
}

void Send_spell()
{
	CSP_PLAYER_ATTACK packet;
	packet.size = sizeof(packet);
	packet.type = CSPT_PLAYER_SPELL;
	packet.monster_id = avatar.target_object.id;
	size_t sent = 0;
	socket.send(&packet, sizeof(packet), sent);
	//printf("SEND spell\n");
}


Object_Base::Object_Base()
{
	m_showing = true;
}

void Object_Base::move(const Type_POS& _x, const Type_POS& _y, const Type_POS& _target_x, const Type_POS& _target_y)
{
	server_x = _x;
	server_y = _y;

	target_x = _target_x;
	target_y = _target_y;
	moving = true;
}

void Object_Base::move(const Type_POS& _x, const Type_POS& _y)
{
	server_x = _x;
	server_y = _y;
}

void Other_Player::Init_Sprite()
{
	sprite_normal = new sf::Sprite;
	sprite_normal->setTexture(*image);
	sprite_normal->setTextureRect(sf::IntRect(0, 128, 64, 64));

	sprite_hurt = new sf::Sprite;
	sprite_hurt->setTexture(*image);
	sprite_hurt->setTextureRect(sf::IntRect(64, 128, 64, 64));

	sprite_normal_attack = new sf::Sprite;
	sprite_normal_attack->setTexture(*image);
	sprite_normal_attack->setTextureRect(sf::IntRect(128, 128, 64, 64));

	sprite_spell = new sf::Sprite;
	sprite_spell->setTexture(*image);
	sprite_spell->setTextureRect(sf::IntRect(192, 128, 64, 64));

	sprite_die = new sf::Sprite;
	sprite_die->setTexture(*image);
	sprite_die->setTextureRect(sf::IntRect(448, 128, 64, 64));
}

Other_Player::Other_Player
	(const Type_POS& _x, const Type_POS& _y, const Type_POS& _target_x, const Type_POS& _target_y, const Type_POS& _vel, const Type_Hp& _Hp)
{
	x = _x;
	y = _y;

	server_x = _x;
	server_y = _y;

	target_x = _target_x;
	target_y = _target_y;

	vel = _vel;
	Hp = _Hp;

	moving = true;
}

void Other_Player::Set_Normal_Attack_image()
{
	draw_hit_timer.Set_time(COOLTIME__DRAW_PLAYER_NORMAL_ATTACK);
	attack_type = NORMAL_ATTACK;
}

void Other_Player::Set_Spell_image()
{
	draw_hit_timer.Set_time(COOLTIME__DRAW_PLAYER_SPELL);
	attack_type = SPELL;
}

void Other_Player::draw()
{
	if (false == m_showing) return;

	float rx = (x - avatar.x + PLAYER_VIEW_SIZE_WIDTH) * TILE_WIDTH_POS - TILE_WIDTH;
	float ry = (y - avatar.y + PLAYER_VIEW_SIZE_HEIGHT) * TILE_WIDTH_POS - TILE_WIDTH;
	
	if (Hp < 1) {
		sprite_die->setPosition(rx, ry);
		g_window->draw(*sprite_die);
		return;
	}

	sprite_normal->setPosition(rx, ry);
	g_window->draw(*sprite_normal);

	if (true == draw_hit_timer.Is_running_time()) {

		if (NORMAL_ATTACK == attack_type) {
			printf("다른애 Draw NA\n");
			sprite_normal_attack->setPosition(rx, ry);
			g_window->draw(*sprite_normal_attack);
		}

		else if (SPELL == attack_type) {
			printf("다른애 Draw Spell\n");
			sprite_spell->setPosition(rx, ry);
			g_window->draw(*sprite_spell);
		}
	}

	if (true == draw_hurt_timer.Is_running_time()) {
		sprite_hurt->setPosition(rx, ry);
		g_window->draw(*sprite_hurt);
	}
}

Type_POS Other_Player::Get_Size()
{
	return SIZE_PLAYER;
}

void Monster_type1::Init_Sprite()
{
	sprite_normal = new sf::Sprite;
	sprite_normal->setTexture(*image);
	sprite_normal->setTextureRect(sf::IntRect(0, 192, 64, 64));

	sprite_targeting = new sf::Sprite;
	sprite_targeting->setTexture(*image);
	sprite_targeting->setTextureRect(sf::IntRect(64, 192, 64, 64));

	sprite_hurt = new sf::Sprite;
	sprite_hurt->setTexture(*image);
	sprite_hurt->setTextureRect(sf::IntRect(128, 192, 64, 64));

	sprite_attack = new sf::Sprite;
	sprite_attack->setTexture(*image);
	sprite_attack->setTextureRect(sf::IntRect(192, 192, 64, 64));

	sprite_die = new sf::Sprite;
	sprite_die->setTexture(*image);
	sprite_die->setTextureRect(sf::IntRect(448, 192, 64, 64));
}

Monster_type1::Monster_type1
	(const Type_POS& _x, const Type_POS& _y, const Type_POS& _target_x, const Type_POS& _target_y, const Type_POS& _vel, const Type_Hp& _Hp)
{
	x = _x;
	y = _y;

	server_x = _x;
	server_y = _y;

	target_x = _target_x;
	target_y = _target_y;

	vel = _vel;
	Hp = _Hp;

	moving = true;
}

void Monster_type1::draw()
{
	if (false == m_showing)	return;
	
	float rx = (x - avatar.x + PLAYER_VIEW_SIZE_WIDTH) * TILE_WIDTH_POS - TILE_WIDTH;
	float ry = (y - avatar.y + PLAYER_VIEW_SIZE_HEIGHT) * TILE_WIDTH_POS - TILE_WIDTH;

	if (Hp < 1) {
		sprite_die->setPosition(rx, ry);
		g_window->draw(*sprite_die);
		return;
	}

	sprite_normal->setPosition(rx, ry);
	g_window->draw(*sprite_normal);

	if (true == draw_hit_timer.Is_running_time()) {
		sprite_attack->setPosition(rx, ry);
		g_window->draw(*sprite_attack);
	}


	if (this == avatar.Get_target_object()) {
		sprite_targeting->setPosition(rx, ry);
		g_window->draw(*sprite_targeting);
	}

	if (true == draw_hurt_timer.Is_running_time()) {
		sprite_hurt->setPosition(rx, ry);
		g_window->draw(*sprite_hurt);
	}
}

Type_POS Monster_type1::Get_Size()
{
	return SIZE_MONSTER_TYPE1;
}

void Monster_type1::Attack()
{
	draw_hit_timer.Set_time(COOLTIME__DRAW_MONSTER_ATTACK);
}

void Monster_type2::Init_Sprite()
{
	sprite_normal = new sf::Sprite;
	sprite_normal->setTexture(*image);
	sprite_normal->setTextureRect(sf::IntRect(0, 256, 64, 64));

	sprite_targeting = new sf::Sprite;
	sprite_targeting->setTexture(*image);
	sprite_targeting->setTextureRect(sf::IntRect(64, 256, 64, 64));

	sprite_hurt = new sf::Sprite;
	sprite_hurt->setTexture(*image);
	sprite_hurt->setTextureRect(sf::IntRect(128, 256, 64, 64));

	sprite_attack = new sf::Sprite;
	sprite_attack->setTexture(*image);
	sprite_attack->setTextureRect(sf::IntRect(192, 256, 64, 64));

	sprite_die = new sf::Sprite;
	sprite_die->setTexture(*image);
	sprite_die->setTextureRect(sf::IntRect(448, 256, 64, 64));
}

Monster_type2::Monster_type2
	(const Type_POS& _x, const Type_POS& _y, const Type_POS& _target_x, const Type_POS& _target_y, const Type_POS& _vel, const Type_Hp& _Hp)
{
	x = _x;
	y = _y;

	server_x = _x;
	server_y = _y;

	target_x = _target_x;
	target_y = _target_y;

	vel = _vel;
	Hp = _Hp;

	moving = true;
}

void Monster_type2::draw()
{
	if (false == m_showing) return;

	float rx = (x - avatar.x + PLAYER_VIEW_SIZE_WIDTH) * TILE_WIDTH_POS - TILE_WIDTH;
	float ry = (y - avatar.y + PLAYER_VIEW_SIZE_HEIGHT) * TILE_WIDTH_POS - TILE_WIDTH;

	if (Hp < 1) {
		sprite_die->setPosition(rx, ry);
		g_window->draw(*sprite_die);
		return;
	}

	sprite_normal->setPosition(rx, ry);
	g_window->draw(*sprite_normal);

	if (true == draw_hit_timer.Is_running_time()) {
		sprite_attack->setPosition(rx, ry);
		g_window->draw(*sprite_attack);
	}

	if (this == avatar.Get_target_object()) {
		sprite_targeting->setPosition(rx, ry);
		g_window->draw(*sprite_targeting);
	}

	if (true == draw_hurt_timer.Is_running_time()) {
		sprite_hurt->setPosition(rx, ry);
		g_window->draw(*sprite_hurt);
	}
}

Type_POS Monster_type2::Get_Size()
{
	return SIZE_MONSTER_TYPE2;
}

void Monster_type2::Attack()
{
	draw_hit_timer.Set_time(COOLTIME__DRAW_MONSTER_ATTACK);
}

CoolTime::CoolTime()
{
	time = high_resolution_clock::now();
}

void CoolTime::Set_time(int gap_time)
{
	time = high_resolution_clock::now() + milliseconds(gap_time);
}

bool CoolTime::Is_running_time()
{
	return high_resolution_clock::now() < time;
}

Bullet::Bullet(const TYPE& _type, const Type_POS& _x, const Type_POS& _y, const Type_ID& _id, const Type_Damage& _damage)
{
	type = _type;
	x = _x;
	y = _y;

	target_id = _id;
	damage = _damage;

	if (target_id == avatar.id) {
		target_x = avatar.x;
		target_y = avatar.y;
		p_target = &avatar;
	}

	else {
		target_x = objects[target_id]->x;
		target_y = objects[target_id]->y;
		p_target = objects[target_id];
	}
}

void Bullet::Init_Sprite()
{
	sprite_player_spell = new sf::Sprite;
	sprite_player_spell->setTexture(*image);
	sprite_player_spell->setTextureRect(sf::IntRect(256, 64, 64, 64));

	sprite_other_player_spell = new sf::Sprite;
	sprite_other_player_spell->setTexture(*image);
	sprite_other_player_spell->setTextureRect(sf::IntRect(256, 128, 64, 64));

	sprite_monster_type3_attack = new sf::Sprite;
	sprite_monster_type3_attack->setTexture(*image);
	sprite_monster_type3_attack->setTextureRect(sf::IntRect(256, 320, 64, 64));
}

bool Bullet::move_update(const duration<float, std::milli>& gap_time)
{
	Type_POS length_rate{ VELOCITY_PLAYER_SPELL * 0.001f * gap_time.count() };
	
	if (avatar.id == target_id) {
		target_x = avatar.x;
		target_y = avatar.y;
	}

	else if (0 < objects.count(target_id)) {
		target_x = objects[target_id]->x;
		target_y = objects[target_id]->y;
	}

	else {
		p_target = nullptr;
		return false;
	}
	

	Type_POS length_target_x{ target_x - x };
	Type_POS length_target_y{ target_y - y };

	Type_POS length_target{ hypotf(length_target_x, length_target_y) };
	
	if (nullptr != p_target) {
		if (length_target - p_target->Get_Size() < length_rate) return false;
	}
	
	Type_POS move_x{ length_rate * length_target_x / length_target };
	Type_POS move_y{ length_rate * length_target_y / length_target };

	x += move_x;
	y += move_y;

	return true;
}

void Bullet::attack()
{
	if (nullptr == p_target) return;

	p_target->Hurt(damage);

	if (true == p_target->Is_Die())
		p_target->Set_cant_move();

	texts.insert(new UI_Text(x, y, damage));
}

void Bullet::draw()
{
	float rx = (x - avatar.x + PLAYER_VIEW_SIZE_WIDTH) * TILE_WIDTH_POS - TILE_WIDTH;
	float ry = (y - avatar.y + PLAYER_VIEW_SIZE_HEIGHT) * TILE_WIDTH_POS - TILE_WIDTH;

	switch (type)
	{
	case PLAYER_SPELL: 
	{
		sprite_player_spell->setPosition(rx, ry);
		g_window->draw(*sprite_player_spell);
	}
		break;

	case OTHER_PLAYER_SPELL: 
	{
		sprite_other_player_spell->setPosition(rx, ry);
		g_window->draw(*sprite_other_player_spell);
	}
		break;
	case MONSTER_TYPE3_ATTACK:
	{
		sprite_monster_type3_attack->setPosition(rx, ry);
		g_window->draw(*sprite_monster_type3_attack);
	}
	break;
		
	default:
		break;
	}
}

void Monster_type3::Init_Sprite()
{
	sprite_normal = new sf::Sprite;
	sprite_normal->setTexture(*image);
	sprite_normal->setTextureRect(sf::IntRect(0, 320, 64, 64));

	sprite_targeting = new sf::Sprite;
	sprite_targeting->setTexture(*image);
	sprite_targeting->setTextureRect(sf::IntRect(64, 320, 64, 64));

	sprite_hurt = new sf::Sprite;
	sprite_hurt->setTexture(*image);
	sprite_hurt->setTextureRect(sf::IntRect(128, 320, 64, 64));

	sprite_attack = new sf::Sprite;
	sprite_attack->setTexture(*image);
	sprite_attack->setTextureRect(sf::IntRect(192, 320, 64, 64));

	sprite_die = new sf::Sprite;
	sprite_die->setTexture(*image);
	sprite_die->setTextureRect(sf::IntRect(448, 320, 64, 64));
}

Monster_type3::Monster_type3(const Type_POS& _x, const Type_POS& _y, const Type_POS& _target_x, const Type_POS& _target_y, const Type_POS& _vel, const Type_Hp& _Hp)
{
	x = _x;
	y = _y;

	server_x = _x;
	server_y = _y;

	target_x = _target_x;
	target_y = _target_y;

	vel = _vel;
	Hp = _Hp;

	moving = true;
}

void Monster_type3::draw()
{
	if (false == m_showing) return;

	float rx = (x - avatar.x + PLAYER_VIEW_SIZE_WIDTH) * TILE_WIDTH_POS - TILE_WIDTH;
	float ry = (y - avatar.y + PLAYER_VIEW_SIZE_HEIGHT) * TILE_WIDTH_POS - TILE_WIDTH;

	if (Hp < 1) {
		sprite_die->setPosition(rx, ry);
		g_window->draw(*sprite_die);
		return;
	}

	sprite_normal->setPosition(rx, ry);
	g_window->draw(*sprite_normal);

	if (true == draw_hit_timer.Is_running_time()) {
		sprite_attack->setPosition(rx, ry);
		g_window->draw(*sprite_attack);
	}

	if (this == avatar.Get_target_object()) {
		sprite_targeting->setPosition(rx, ry);
		g_window->draw(*sprite_targeting);
	}

	if (true == draw_hurt_timer.Is_running_time()) {
		sprite_hurt->setPosition(rx, ry);
		g_window->draw(*sprite_hurt);
	}
}

Type_POS Monster_type3::Get_Size()
{
	return SIZE_MONSTER_TYPE3;
}

void Monster_type3::Attack()
{
	draw_hit_timer.Set_time(COOLTIME__DRAW_MONSTER_ATTACK);
}

UI_Text::UI_Text(const Type_POS& _x, const Type_POS& _y, const Type_Damage& _damage)
{
	char buf[SIZE_BUFFER_TEXT_UI];

	x = _x;
	y = _y;

	text.setFont(*g_font);
	sprintf_s(buf, "%d", _damage);
	text.setString(buf);

	time = high_resolution_clock::now() + milliseconds(COOLTIME__DRAW_TEXT_ATTACK);
}

UI_Text::UI_Text(Object* const p_hurt_object, Object* const p_attack_object, const Type_Damage& _damage)
	:	UI_Text((p_hurt_object->x + p_attack_object->x) / 2, (p_hurt_object->y + p_attack_object->y) / 2, _damage)
{}

void UI_Text::draw()
{
	float rx = (x - avatar.x + PLAYER_VIEW_SIZE_WIDTH) * TILE_WIDTH_POS - TILE_WIDTH;
	float ry = (y - avatar.y + PLAYER_VIEW_SIZE_HEIGHT) * TILE_WIDTH_POS - TILE_WIDTH;

	text.setPosition(rx, ry);
	g_window->draw(text);
}

bool UI_Text::Can_destroy()
{
	return time < high_resolution_clock::now();
}


