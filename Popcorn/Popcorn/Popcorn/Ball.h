#pragma once

#include "Config.h"
#include "Level.h"

//------------------------------------------------------------------------------------------------------------
enum EBall_State
{
	EBS_Normal,
	EBS_Lost,
	EBS_On_Platform
};
//------------------------------------------------------------------------------------------------------------
class ABall
{
public:
	ABall();

	void Init(int x_pos);

	void Draw(HDC hdc, RECT& paint_area);
	void Move(ALevel* level, int platform_x_pos, int platform_width);

	EBall_State Ball_State;
private:

	HPEN Ball_Pen;
	HBRUSH Ball_Brush;
	double Ball_Direction;

	double Ball_X_Pos, Ball_Y_Pos;
	double Ball_Speed;

	RECT Ball_Rect, Prev_Ball_Rect;
};
//------------------------------------------------------------------------------------------------------------
