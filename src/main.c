#include <stdio.h>

#include "raylib.h"

#define RESOURCES "../resources/"
#define DEFAULT_FONT RESOURCES "Audiowide-Regular.ttf"
#define MENU_MUSIC RESOURCES "awesomeness.wav"

static const Color ZOZBLACK = {
	0x18,
	0x18,
	0x18,
	0xFF};

int main(void)
{
	const char *wTitle = "cronos";
	const Vector2 wSizes = {800, 600};

	InitWindow(wSizes.x, wSizes.y, wTitle);
	SetTargetFPS(60);
	ChangeDirectory(GetApplicationDirectory()); // set relative path

	const char *text = "Olá mundo!";
	const float fontSize = 40.0f;
	const float spacing = 1.0f;
	const Font font = LoadFontEx(DEFAULT_FONT, fontSize, 0, 255);

	InitAudioDevice();
	Music music = LoadMusicStream(MENU_MUSIC);
	PlayMusicStream(music);
	float timePlayed = 0.0f; // [0.0f..1.0f]

	while (!WindowShouldClose())
	{
		UpdateMusicStream(music);
		timePlayed = GetMusicTimePlayed(music) / GetMusicTimeLength(music);
		if (timePlayed > 1.0f)
		{
			timePlayed = 0.0f;
		}

		Vector2 tSizes = MeasureTextEx(font, text, fontSize, spacing);
		Vector2 tPos = {
			.x = (wSizes.x / 2.0f) - (tSizes.x / 2.0f),
			.y = (wSizes.y / 2.0f) - (tSizes.y / 2.0f)};

		BeginDrawing();
		{
			ClearBackground(ZOZBLACK);
			DrawTextEx(font, text, tPos, fontSize, spacing, RAYWHITE);
		}
		EndDrawing();
	}

	UnloadMusicStream(music);
	CloseAudioDevice();

	UnloadFont(font);
	CloseWindow();
	return 0;
}