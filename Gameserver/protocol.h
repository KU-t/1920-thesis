#pragma once

// ���� ũ�� 
constexpr int MAX_BUF = 128;

// ��Ŷ ũ��


#define SERVER_PORT		3500

#define WORLD_WIDTH		800
#define WORLD_HEIGHT	800

#define SCREEN_WIDTH	16
#define SCREEN_HEIGHT	16

#define NEAR_WIDTH		7
#define NEAR_HEIGHT		7

//

#define SC_PLAYER_LOGIN		11	// �α���
#define SC_PLAYER_POSITION	12
#define SC_PLAYER_IN		13
#define SC_PLAYER_OUT		14


#define CS_UP				211
#define CS_DOWN				212
#define CS_LEFT				213
#define CS_RIGHT			214


#pragma pack(push ,1)	// �����Ϸ��� �� �ɼ��� ������ ������ �ض�

struct SC_PACKET_PLAYER_LOGIN {
	char size;
	char type;
	int id;
	int x, y;
};

struct SC_PACKET_PLAYER_POSITION {
	char size;
	char type;
	int id;
	int x, y;
};

struct SC_PACKET_PLAYER_IN {
	char size;
	char type;
	int id;
};


struct SC_PACKET_PLAYER_OUT {
	char size;
	char type;
	int id;
	int x, y;
	// ����������
};





struct CS_PACKET_POSITION {
	char	size;
	char	type;
};


#pragma pack (pop)