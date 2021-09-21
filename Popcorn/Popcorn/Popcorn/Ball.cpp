#include "Ball.h"

// ABall
//------------------------------------------------------------------------------------------------------------
ABall::ABall()
	:Ball_State(EBS_Normal), Ball_Pen(0), Ball_Brush(0), Ball_X_Pos(0), Ball_Y_Pos(181), Ball_Speed(3.0), Ball_Direction(M_PI - M_PI_4), Ball_Rect{}, Prev_Ball_Rect{}
{
}
//------------------------------------------------------------------------------------------------------------
void ABall::Init(int x_pos)
{
	Ball_X_Pos = (double)x_pos - (double)AsConfig::Ball_Size / 2;
	AsConfig::Create_Pen_Brush(255, 255, 255, Ball_Pen, Ball_Brush);
}
//------------------------------------------------------------------------------------------------------------
void ABall::Draw(HDC hdc, RECT& paint_area)
{
	RECT intersection_rect;

	// 1. Очищаем фон
	if (IntersectRect(&intersection_rect, &paint_area, &Prev_Ball_Rect))
	{
		SelectObject(hdc, AsConfig::BG_Pen);
		SelectObject(hdc, AsConfig::BG_Brush);

		Ellipse(hdc, Prev_Ball_Rect.left, Prev_Ball_Rect.top, Prev_Ball_Rect.right - 1, Prev_Ball_Rect.bottom - 1);
	}

	// 2. Рисуем шарик
	if (IntersectRect(&intersection_rect, &paint_area, &Ball_Rect))
	{
		SelectObject(hdc, Ball_Pen);
		SelectObject(hdc, Ball_Brush);

		Ellipse(hdc, Ball_Rect.left, Ball_Rect.top, Ball_Rect.right - 1, Ball_Rect.bottom - 1);
	}
}
//------------------------------------------------------------------------------------------------------------
void ABall::Move(ALevel* level, int platform_x_pos, int platform_width)
{
	double next_x_pos, next_y_pos;
	int max_x_pos = AsConfig::Max_X_Pos - AsConfig::Ball_Size;
	int max_y_pos = AsConfig::Max_Y_Pos - AsConfig::Ball_Size;
	int platform_y_pos = AsConfig::Platform_Y_Pos - AsConfig::Ball_Size;

	if (Ball_State != EBS_Normal)
		return;

	Prev_Ball_Rect = Ball_Rect;

	next_x_pos = Ball_X_Pos + Ball_Speed * cos(Ball_Direction);
	next_y_pos = Ball_Y_Pos - Ball_Speed * sin(Ball_Direction);

	// Корректируем позицию при отражении от рамки
	if (next_x_pos < AsConfig::Border_X_Offset)
	{
		next_x_pos = AsConfig::Level_X_Offset - (next_x_pos - AsConfig::Level_X_Offset);
		Ball_Direction = M_PI - Ball_Direction;
	}

	if (next_y_pos < AsConfig::Border_Y_Offset)
	{
		next_y_pos = AsConfig::Border_Y_Offset - (next_y_pos - AsConfig::Border_Y_Offset);
		Ball_Direction = -Ball_Direction;
	}

	if (next_x_pos > max_x_pos)
	{
		next_x_pos = max_x_pos - (next_x_pos - max_x_pos);
		Ball_Direction = M_PI - Ball_Direction;
	}

	if (next_y_pos > max_y_pos)
	{
		if (level->Has_Floar)
		{
			next_y_pos = max_y_pos - (next_y_pos - max_y_pos); 
			Ball_Direction = -Ball_Direction;
		}
		else 
			if(next_y_pos > (double)max_y_pos + (double)AsConfig::Ball_Size * 2.0) // чтобы шарик смог улететь ниже полаб проверяем его
				Ball_State = EBS_Lost;
	}

	// Корректируем позицию при отражении от платформы
	if (next_y_pos > platform_y_pos)
	{
		if (next_x_pos >= platform_x_pos && next_x_pos <= (double)platform_x_pos + (double)platform_width)
		{
			next_y_pos = platform_y_pos - (next_y_pos - platform_y_pos);
			Ball_Direction = M_PI + (M_PI - Ball_Direction);
		}
	}

	// Корректируем позицию при отражении от кирпичей
	level->Check_Level_Brick_Hit(next_y_pos, Ball_Direction);

	// Смещаем шарик
	Ball_X_Pos = next_x_pos;
	Ball_Y_Pos = next_y_pos;

	Ball_Rect.left   = (int)Ball_X_Pos * AsConfig::Global_Scale;
	Ball_Rect.top    = (int)Ball_Y_Pos * AsConfig::Global_Scale;
	Ball_Rect.right  = (int)Ball_Rect.left + AsConfig::Ball_Size * AsConfig::Global_Scale + 1;
	Ball_Rect.bottom = (int)Ball_Rect.top + AsConfig::Ball_Size * AsConfig::Global_Scale + 1;

	InvalidateRect(AsConfig::Hwnd, &Prev_Ball_Rect, FALSE);
	InvalidateRect(AsConfig::Hwnd, &Ball_Rect, FALSE);
}
//------------------------------------------------------------------------------------------------------------