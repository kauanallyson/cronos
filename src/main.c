#include "raylib.h"

#define RESOURCES "../resources/"
#define MENU_MUSIC RESOURCES "awesomeness.wav"

#define ZOZBLACK (Color){24, 24, 24, 255}
#define MUSGO_GREEN (Color){10, 95, 30, 255}

int main(void)
{
	const char *wTitle = "cronos";
	const Vector2 wSizes = {800, 600};
	const Vector2 center = {.x = wSizes.x / 2.0f, .y = wSizes.y / 2.0f};

	InitWindow(wSizes.x, wSizes.y, wTitle);
	SetTargetFPS(60);
	ChangeDirectory(GetApplicationDirectory()); // set relative path

	const char *text = "cronos";
	const float fontSize = 40.0f;
	const float spacing = 1.0f;
	const Font font = GetFontDefault();

	InitAudioDevice();
	Music music = LoadMusicStream(MENU_MUSIC);
	PlayMusicStream(music);
	float timePlayed = 0.0f; // [0.0f..1.0f]

	const int segments = 36;
	const float radius = 100.0f;
	const float startAngle = 270.0f; // 12horas
	float endAngle = timePlayed * 360.0f + startAngle;

	while (!WindowShouldClose())
	{
		UpdateMusicStream(music);
		timePlayed = GetMusicTimePlayed(music) / GetMusicTimeLength(music);
		if (timePlayed > 1.0f)
		{
			timePlayed = 0.0f; // loop
		}

		endAngle = (timePlayed * 360.0f) + startAngle;

		Vector2 tSizes = MeasureTextEx(font, text, fontSize, spacing);
		Vector2 tPos = {
			.x = center.x - (tSizes.x / 2.0f),
			.y = (tSizes.y / 2.0f)};

		BeginDrawing();
		{
			ClearBackground(ZOZBLACK);
			DrawTextEx(font, text, tPos, fontSize, spacing, RAYWHITE);
			DrawCircleSector(center, radius, startAngle, endAngle, segments, MUSGO_GREEN);
		}
		EndDrawing();
	}

	UnloadMusicStream(music);
	CloseAudioDevice();

	UnloadFont(font);

	CloseWindow();
	return 0;
}