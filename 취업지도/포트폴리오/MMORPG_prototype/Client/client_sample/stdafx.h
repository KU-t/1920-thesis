#define SFML_STATIC 1
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <fstream>
#include <cmath>
#include <chrono>

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

using namespace std;
using namespace chrono;

constexpr float TILE_WIDTH_POS{ 64.f};
constexpr float TILE_WIDTH_POS_PER{ 2.f };
constexpr float TILE_WIDTH{ TILE_WIDTH_POS / TILE_WIDTH_POS_PER };

constexpr auto WINDOW_WIDTH{ 1280 };
constexpr auto WINDOW_HEIGHT{ 800 };
constexpr auto BUF_SIZE{ 200 };
constexpr auto TIME_MOVE_INTERPOLATION{ duration<float, std::milli>(std::chrono::milliseconds(S_TIME_UPDATE_OBJECT_MOVE)).count() };

constexpr size_t SIZE_BUFFER_TEXT_UI{64};

constexpr auto COOLTIME__DRAW_PLAYER_NORMAL_ATTACK{ COOLTIME_PLAYER_NORMAL_ATTACK / 2 };
constexpr auto COOLTIME__DRAW_PLAYER_SPELL{ COOLTIME_PLAYER_SPELL / 2 };
constexpr auto COOLTIME__DRAW_MONSTER_ATTACK{ COOLTIME_MONSTER_ATTACK / 2 };
constexpr auto COOLTIME__DRAW_HURT{ 500 };

constexpr auto COOLTIME__DRAW_TEXT_ATTACK{ 300 };


class CoolTime {
private:
	high_resolution_clock::time_point time;

public:
	CoolTime();
	void Set_time(int);
	bool Is_running_time();
};

class Object;

class Bullet {
public:
	enum TYPE {PLAYER_SPELL, OTHER_PLAYER_SPELL, MONSTER_TYPE3_ATTACK};

protected:
	TYPE type;
	Object* p_target;
	Type_ID	target_id;
	Type_POS x, y;
	Type_POS target_x, target_y;
	Type_Damage damage;

public:
	static sf::Texture *image;
	static sf::Sprite* sprite_player_spell;
	static sf::Sprite* sprite_other_player_spell;
	static sf::Sprite* sprite_monster_type3_attack;

public:
	Bullet() = delete;
	Bullet(const TYPE&, const Type_POS&, const Type_POS&, const Type_ID&, const Type_Damage&);

	static void Init_Sprite();
	
	bool move_update(const duration<float, std::milli>&);
	void attack();
	void draw();
};

class UI_Text {
protected:
	high_resolution_clock::time_point time;
	sf::Text text;
	Type_POS x, y;

public:
	UI_Text() = delete;
	UI_Text(const Type_POS&, const Type_POS&, const Type_Damage&);
	UI_Text(Object* const, Object* const, const Type_Damage&);
	void draw();
	bool Can_destroy();

	bool operator<(const UI_Text& other) const {
		return time > other.time;
	}
};

class Object_Base {
protected:
	bool m_showing;

	enum PLAYER_ATTACK_TYPE { NORMAL_ATTACK, SPELL };

public:
	static sf::Texture *image;

	Type_POS target_x, target_y;
	Type_POS server_x, server_y;
	Type_POS x, y;
	Type_POS vel;
	Type_Hp Hp;
	bool	 moving;

	CoolTime draw_hit_timer;
	CoolTime draw_hurt_timer;

public:
	Object_Base();

	void show() { m_showing = true; }
	void hide() { m_showing = false; }

	void move(const Type_POS&, const Type_POS&, const Type_POS&, const Type_POS&);
	void move(const Type_POS&, const Type_POS&);

	virtual void move_update(const duration<float, std::milli>&) = 0;
	virtual void Respwan(const Type_POS&, const Type_POS&, const Type_POS&, const Type_Hp&, const Type_Damage&) = 0;

	virtual void draw() = 0;
	virtual Type_POS Get_Size() = 0;
};


class Object : public Object_Base 
{
public:
	
	virtual void move_update(const duration<float, std::milli>&) override;

	virtual void draw() override {};
	virtual Type_POS Get_Size() override { return 0.f; };
	virtual void Respwan(const Type_POS&, const Type_POS&, const Type_POS&, const Type_Hp&, const Type_Damage&) override;

	void Set_Hurt_image();
	virtual void Hurt(const Type_Damage&);
	
	void Set_cant_move();
	void Die();
	bool Is_Die();

};

class Other_Object : public Object
{
public:
	void sight_in(const Type_POS&, const Type_POS&, const Type_POS&, const Type_POS&, const Type_POS&, const Type_Hp&);
	bool Check_target(const Type_POS&, const Type_POS&);
};

class Other_Player : public Other_Object
{
private:
	static sf::Sprite* sprite_normal;
	static sf::Sprite* sprite_normal_attack;
	static sf::Sprite* sprite_spell;
	static sf::Sprite* sprite_hurt;
	static sf::Sprite* sprite_die;

	PLAYER_ATTACK_TYPE attack_type;
public:
	static void Init_Sprite();

	Other_Player() = delete;
	Other_Player(const Type_POS&, const Type_POS&, const Type_POS&, const Type_POS&, const Type_POS&, const Type_Hp&);

public:
	virtual void draw() override final;
	virtual Type_POS Get_Size() override final;
	
	void Set_Normal_Attack_image();
	void Set_Spell_image();
};

class Monster_type1 : public Other_Object
{
private:
	static sf::Sprite* sprite_normal;
	static sf::Sprite* sprite_targeting;
	static sf::Sprite* sprite_hurt;
	static sf::Sprite* sprite_attack;
	static sf::Sprite* sprite_die;

public:
	static void Init_Sprite();

	Monster_type1() = delete;
	Monster_type1(const Type_POS&, const Type_POS&, const Type_POS&, const Type_POS&, const Type_POS&, const Type_Hp&);

public:
	virtual void draw() override final;
	virtual Type_POS Get_Size() override final;
	
	void Attack();
};

class Monster_type2 : public Other_Object
{
private:
	static sf::Sprite* sprite_normal;
	static sf::Sprite* sprite_targeting;
	static sf::Sprite* sprite_hurt;
	static sf::Sprite* sprite_attack;
	static sf::Sprite* sprite_die;

public:
	static void Init_Sprite();

	Monster_type2() = delete;
	Monster_type2(const Type_POS&, const Type_POS&, const Type_POS&, const Type_POS&, const Type_POS&, const Type_Hp&);

public:
	virtual void draw() override final;
	virtual Type_POS Get_Size() override final;
	
	void Attack();
};

class Monster_type3 : public Other_Object
{
private:
	static sf::Sprite* sprite_normal;
	static sf::Sprite* sprite_targeting;
	static sf::Sprite* sprite_hurt;
	static sf::Sprite* sprite_attack;
	static sf::Sprite* sprite_die;

public:
	static void Init_Sprite();

	Monster_type3() = delete;
	Monster_type3(const Type_POS&, const Type_POS&, const Type_POS&, const Type_POS&, const Type_POS&, const Type_Hp&);

public:
	virtual void draw() override final;
	virtual Type_POS Get_Size() override final;
	
	void Attack();
};

class Player : public Object
{
private:
	class Target_Object
	{
	public:
		Object* p_object;
		Type_ID id;
	};
	sf::Font m_font;
	sf::Text UI_Hp;
	sf::Text UI_Str;
	sf::Text UI_Vel;
	
	Type_Damage str;

public:
	Type_ID id;
	
private:
	friend void Send_normal_attack();
	friend void Send_spell();

	static sf::Sprite* sprite_normal;
	static sf::Sprite* sprite_normal_attack;
	static sf::Sprite* sprite_spell;
	static sf::Sprite* sprite_hurt;
	static sf::Sprite* sprite_die;

	Target_Object target_object;

	PLAYER_ATTACK_TYPE attack_type;

	CoolTime timer_send_move;
	CoolTime timer_normal_attack;
	CoolTime timer_spell;
	
	bool Is_Time__send_position;
	bool Is_Modify__target_position;

public:	
	Player();

	static void Init_Sprite();

	virtual void move_update(const duration<float, std::milli>&);

	virtual void draw() override final;
	virtual Type_POS Get_Size() override final;
	virtual void Respwan(const Type_POS&, const Type_POS&, const Type_POS&, const Type_Hp&, const Type_Damage&) override final;

	virtual void Hurt(const Type_Damage&) override final;

	void Login__init_data(const Type_POS&, const Type_POS&, const Type_POS&, const Type_POS&, const Type_POS&, const Type_Hp&, const Type_POS&);
	void Set_target_position(const Type_POS&, const Type_POS&);

	bool Is_time__Send_Position();
	bool Get_send_target_position();
	
	void Set_target_object(Object* const, const Type_ID&);
	Object* Get_target_object();

	
	bool Can_normal_attack();
	bool Can_spell();
};

class Terrain_Manager {
private:
	class Terrain_Base {
	public:
		sf::Sprite tile;

		Terrain_Base() = delete;
		Terrain_Base(sf::Texture & t, int x, int y, int x2, int y2)
		{
			tile.setTexture(t);
			tile.setTextureRect(sf::IntRect(x, y, x2, y2));
		}

		void draw(const float&, const float&);
	};

	float* height_map;
	sf::Texture* texture;
	Terrain_Base* tile_image0, *tile_image1, *tile_image2, *tile_image3, *tile_image4, *tile_image5;

private:
	Type_POS Get_terrain_height(const int&, const int&);
	Type_POS Get_obj_height(Object_Base* const);

public:
	Terrain_Manager() = delete;
	Terrain_Manager(const char*, const char*);

	bool Can_move(Object_Base* const);
	void delete_Obj();

	void draw();
};

class TIME_MANAGER {
private:
	high_resolution_clock::time_point pred_draw_time;

public:
	TIME_MANAGER() = default;
	void Set_draw_time();
	nanoseconds Get_gap_time();
	bool Is_time__Update();
};

extern sf::TcpSocket socket;
extern sf::RenderWindow *g_window;
extern sf::Font* g_font;
extern Terrain_Manager* TrnMgr;
extern TIME_MANAGER* TmrMgr;
extern Player avatar;
extern unordered_map<Type_ID, Other_Object*> objects;
extern unordered_set<Bullet*> bullets;
extern unordered_set<UI_Text*> texts;

void Update();
void Update_Objects_State();
void Draw_Objects();

void Send_move();
void Send_normal_attack();
void Send_spell();