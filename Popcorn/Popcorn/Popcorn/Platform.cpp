﻿#include "Platform.h"

// AsPlatform
//------------------------------------------------------------------------------------------------------------
AsPlatform::AsPlatform()
	: X_Pos(AsConfig::Border_X_Offset), X_Step(AsConfig::Global_Scale * 2), Platform_State(EPS_Normal), Inner_Width(21),
	  Rolling_Step(0), Width(Normal_Width), Platform_Rect{}, Prev_Platform_Rect{}, Highlight_Pen(0), Platform_Circle_Pen(0), Platform_Inner_Pen(0),
	  Platform_Circle_Brush(0), Platform_Inner_Brush(0)
{
	X_Pos = (AsConfig::Max_X_Pos - Width) / 2;
}
//------------------------------------------------------------------------------------------------------------
void AsPlatform::Init()
{
	Highlight_Pen = CreatePen(PS_SOLID, 0, RGB(255, 255, 255));

	AsConfig::Create_Pen_Brush(151, 0, 0, Platform_Circle_Pen, Platform_Circle_Brush);
	AsConfig::Create_Pen_Brush(0, 128, 192, Platform_Inner_Pen, Platform_Inner_Brush);
}
//------------------------------------------------------------------------------------------------------------
void AsPlatform::Act()
{	
	switch (Platform_State)
	{
	case EPS_Meltdown:
	case EPS_Roll_In:
	case EPS_Expand_Roll_In:
		AsPlatform::Redraw_Platform();
		break;
	}
}
//------------------------------------------------------------------------------------------------------------
void AsPlatform::Set_State(EPlatform_State new_state)
{
	if (new_state == Platform_State)
		return;

	int len;
	switch (new_state)
	{
	case EPS_Meltdown:
		Platform_State = EPS_Meltdown;
		len = sizeof(Meltdown_Platform_Y_Pos) / sizeof(Meltdown_Platform_Y_Pos[0]);
		for (int i = 0; i < len; i++)
			Meltdown_Platform_Y_Pos[i] = Platform_Rect.bottom;
		break;
	case EPS_Roll_In:
		X_Pos = AsConfig::Max_X_Pos;
		Rolling_Step = Max_Rolling_Step - 1;
		break;
	}
	Platform_State = new_state;
}
//------------------------------------------------------------------------------------------------------------
void AsPlatform::Redraw_Platform()
{
	Prev_Platform_Rect = Platform_Rect;

	int platform_width;
	if (Platform_State == EPS_Roll_In)
		platform_width = Circle_Size;
	else
		platform_width = Width;

	
	Platform_Rect.left = X_Pos * AsConfig::Global_Scale;
	Platform_Rect.top = AsConfig::Platform_Y_Pos * AsConfig::Global_Scale;
	Platform_Rect.right = Platform_Rect.left + platform_width * AsConfig::Global_Scale;
	Platform_Rect.bottom = Platform_Rect.top + Height * AsConfig::Global_Scale;


	if(Platform_State == EPS_Meltdown)
		Prev_Platform_Rect.bottom = AsConfig::Max_Y_Pos * AsConfig::Global_Scale * 2;

	InvalidateRect(AsConfig::Hwnd, &Prev_Platform_Rect, FALSE);
	InvalidateRect(AsConfig::Hwnd, &Platform_Rect, FALSE);
}
//------------------------------------------------------------------------------------------------------------
void AsPlatform::Draw(HDC hdc, RECT& paint_area)
{// Рисуем платформу
	RECT intersection_rect;

	if (!IntersectRect(&intersection_rect, &paint_area, &Platform_Rect))
		return;

	switch (Platform_State)
	{
	case EPS_Missing:
		break;
	case EPS_Normal:
		Draw_Normal_State(hdc, paint_area);
		break;
	case EPS_Meltdown:
		Draw_Meltdown_State(hdc, paint_area);
		break;
	case EPS_Roll_In:
		Draw_Roll_In_State(hdc, paint_area);
		break;
	case EPS_Expand_Roll_In:
		Draw_Expanding_Roll_In_State(hdc, paint_area);
		break;
	default:
		break;
	}
}
//------------------------------------------------------------------------------------------------------------
void AsPlatform::Clear_BG(HDC hdc)
{
	SelectObject(hdc, AsConfig::BG_Pen);
	SelectObject(hdc, AsConfig::BG_Brush);

	Rectangle(hdc, Prev_Platform_Rect.left, Prev_Platform_Rect.top, Prev_Platform_Rect.right, Prev_Platform_Rect.bottom);
}
//------------------------------------------------------------------------------------------------------------
void AsPlatform::Draw_Circle_Highlight(HDC hdc, int x, int y)
{
	SelectObject(hdc, Highlight_Pen);

	Arc(hdc, x + AsConfig::Global_Scale, y + AsConfig::Global_Scale, x + (Circle_Size-1) * AsConfig::Global_Scale, y + (Circle_Size - 1) * AsConfig::Global_Scale,
		x + 2 * AsConfig::Global_Scale, y + AsConfig::Global_Scale, x + AsConfig::Global_Scale, y + 3 * AsConfig::Global_Scale);
}
//------------------------------------------------------------------------------------------------------------
void AsPlatform::Draw_Normal_State(HDC hdc, RECT& paint_area)
{

	int x = X_Pos;
	int y = AsConfig::Platform_Y_Pos;
	

	Clear_BG(hdc);

	// 1. Рисуем боковые шарики
	SelectObject(hdc, Platform_Circle_Pen);
	SelectObject(hdc, Platform_Circle_Brush);

	Ellipse(hdc, x * AsConfig::Global_Scale, y * AsConfig::Global_Scale, (x + Circle_Size) * AsConfig::Global_Scale, (y + Circle_Size) * AsConfig::Global_Scale);
	Ellipse(hdc, (x + Inner_Width) * AsConfig::Global_Scale, y * AsConfig::Global_Scale, (x + Circle_Size + Inner_Width) * AsConfig::Global_Scale, (y + Circle_Size) * AsConfig::Global_Scale);

	// 2. Рисуем блик
	Draw_Circle_Highlight(hdc, x * AsConfig::Global_Scale, y * AsConfig::Global_Scale);

	// 3. Рисуем среднюю часть
	SelectObject(hdc, Platform_Inner_Pen);
	SelectObject(hdc, Platform_Inner_Brush);

	RoundRect(hdc, (x + 4) * AsConfig::Global_Scale, (y + 1) * AsConfig::Global_Scale, (x + 4 + Inner_Width - 1) * AsConfig::Global_Scale, (y + 1 + 5) * AsConfig::Global_Scale, 3 * AsConfig::Global_Scale, 3 * AsConfig::Global_Scale);
}
//------------------------------------------------------------------------------------------------------------
void AsPlatform::Draw_Meltdown_State(HDC hdc, RECT& paint_area)
{// Анимация расплавления платформы
	int area_width = Width * AsConfig::Global_Scale;
	int area_height = Height * AsConfig::Global_Scale + 1;
	int y_offset;
	int x, y;
	COLORREF pixel;
	COLORREF bg_pixel = RGB(AsConfig::BG_Color.R, AsConfig::BG_Color.G, AsConfig::BG_Color.B);

	for (int i = 0; i < area_width; i++)
	{
		y_offset = AsConfig::Rand(Meltdown_Speed) + 1;
		x = Platform_Rect.left + i;
		for (int j = 0; j < area_height; j++)
		{
			y = Meltdown_Platform_Y_Pos[i] - j;
			pixel = GetPixel(hdc, x, y);

			SetPixel(hdc, x, y + y_offset, pixel);
		}

		for (int j = 0; j < y_offset; j++)
		{
			y = Meltdown_Platform_Y_Pos[i] - area_height + 1 + j;
			SetPixel(hdc, x, y, bg_pixel);
		}
		Meltdown_Platform_Y_Pos[i] += y_offset;
	}
}
//------------------------------------------------------------------------------------------------------------
void AsPlatform::Draw_Roll_In_State(HDC hdc, RECT& paint_area)
{// Анимация выкатывания новой платформы
	int x = X_Pos * AsConfig::Global_Scale;
	int y = AsConfig::Platform_Y_Pos * AsConfig::Global_Scale;
	int roller_size = Circle_Size * AsConfig::Global_Scale;
	double alpha;
	XFORM xform, old_xform;

	Clear_BG(hdc);
	// 1. Рисуем боковые шарики
	SelectObject(hdc, Platform_Circle_Pen);
	SelectObject(hdc, Platform_Circle_Brush);

	Ellipse(hdc, x, y, x + roller_size, y + roller_size);

	// 2. Разделительная линия
	SetGraphicsMode(hdc, GM_ADVANCED);

	alpha = -2.0 * M_PI / (double)Max_Rolling_Step * (double)Rolling_Step;

	xform.eM11 = (float)cos(alpha);
	xform.eM12 = (float)sin(alpha);
	xform.eM21 = (float)-sin(alpha);
	xform.eM22 = (float)cos(alpha);
	xform.eDx =  (float)(x + roller_size / 2);
	xform.eDy =  (float)(y + roller_size / 2);
	GetWorldTransform(hdc, &old_xform);
	SetWorldTransform(hdc, &xform);

	SelectObject(hdc, AsConfig::BG_Pen);
	SelectObject(hdc, AsConfig::BG_Brush);

	Rectangle(hdc, -AsConfig::Global_Scale / 2, -roller_size / 2, AsConfig::Global_Scale / 2, roller_size / 2);
	
	SetWorldTransform(hdc, &old_xform);
	// 3. Блик
	Draw_Circle_Highlight(hdc, x, y);

	++Rolling_Step;

	if (Rolling_Step >= Max_Rolling_Step)
		Rolling_Step -= Max_Rolling_Step;

	X_Pos -= Rolling_Platform_Speed;

	if (X_Pos <= Roll_In_End_X_Pos)
	{
		X_Pos += Rolling_Platform_Speed;
		Platform_State = EPS_Expand_Roll_In;
		Inner_Width = 1;
	}
}
//------------------------------------------------------------------------------------------------------------
void AsPlatform::Draw_Expanding_Roll_In_State(HDC hdc, RECT& paint_area)
{
	X_Pos-=1;
	Inner_Width += 2;
	Draw_Normal_State(hdc, paint_area);

	if (Inner_Width >= Normal_Platform_Inner_Width)
	{
		Inner_Width = Normal_Platform_Inner_Width;
		Platform_State = EPS_Normal;
		Redraw_Platform();
	}
}
//------------------------------------------------------------------------------------------------------------