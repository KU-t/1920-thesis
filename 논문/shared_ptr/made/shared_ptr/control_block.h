#pragma once

class control_block{
private:
	int strong_ref;
	int weak_ref;

public:
	control_block();
	~control_block();

public:
	int incre_s_ref() { return ++strong_ref; }
	int decre_s_ref() { return --strong_ref; }
	int incre_w_ref() { return ++weak_ref; }
	int decre_w_ref() { return --weak_ref; }

public:
	int Getstrongref() { return strong_ref; }
	int Getweakref() { return weak_ref; }
};

