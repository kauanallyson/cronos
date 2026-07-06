#include "raylib.h"

static const Color ZOZBLACK = {
	0x18,
	0x18,
	0x18,
	0xFF};

const char *wTitle = "cronometro";
const Vector2 wSizes = {800, 600};

const float fontSize = 40.0f;
const float spacing = 1.0f;

int main(void)
{
	InitWindow(wSizes.x, wSizes.y, wTitle);
	SetTargetFPS(60);

	const Font font = GetFontDefault();

	while (!WindowShouldClose())
	{
		const char *text = TextFormat("%.2f", GetTime());

		Vector2 tSizes = MeasureTextEx(font, text, fontSize, spacing);
		Vector2 tPos = {
			.x = (wSizes.x / 2.0f) - (tSizes.x / 2.0f),
			.y = (wSizes.y / 2.0f) - (tSizes.y / 2.0f)};

		BeginDrawing();
		ClearBackground(ZOZBLACK);
		DrawTextEx(font, text, tPos, fontSize, spacing, RAYWHITE);
		EndDrawing();
	}

	CloseWindow();
	return 0;
}