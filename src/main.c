#include "raylib.h"

#define RESOURCES "../resources/"
#define MENU_MUSIC RESOURCES "awesomeness.wav"

#define ZOZBLACK (Color){24, 24, 24, 255}
#define MUSGO_GREEN (Color){10, 95, 30, 255}

int main(void)
{
	const char *W_TITLE = "cronos";
	const Vector2 W_SIZE = {800, 600};
	const Vector2 SCREEN_CENTER = {.x = W_SIZE.x / 2.0f, .y = W_SIZE.y / 2.0f};

	InitWindow(W_SIZE.x, W_SIZE.y, W_TITLE);
	SetTargetFPS(60);
	ChangeDirectory(GetApplicationDirectory()); // set relative path

	const char *CRONOS_LABEL = "cronos";
	const Font FONT = GetFontDefault();
	const float FONT_SIZE = 40.0f;
	const float FONT_SPACING = 1.0f;

	const Vector2 TEXT_SIZE = MeasureTextEx(FONT, CRONOS_LABEL, FONT_SIZE, FONT_SPACING);
	const Vector2 TEXT_POS = {
		.x = SCREEN_CENTER.x - (TEXT_SIZE.x / 2.0f),
		.y = (TEXT_SIZE.y / 2.0f)};

	InitAudioDevice();
	const Music MUSIC = LoadMusicStream(MENU_MUSIC);
	PlayMusicStream(MUSIC);
	bool pause = false;
	float timePlayed = 0.0f; // [0.0f..1.0f]
	float volume = 0.8f;	 // [0.0f..1.0f]
	const float DELTA_VOLUME = 0.025f;
	SetMusicVolume(MUSIC, volume);

	const int SEGMENTS = 36;
	const float RADIUS = 100.0f;
	const float START_ANGLE = 270.0f;					// 12horas
	float endAngle = timePlayed * 360.0f + START_ANGLE; // offset

	// lb = LoadingBar
	const float NINTH_Y = W_SIZE.y * 0.9f;
	const float LB_THICKNESS = 5.0f;
	const Vector2 LB_START_POS = {.x = W_SIZE.x * 0.1f, .y = NINTH_Y};
	const Vector2 LB_END_POS = {.x = W_SIZE.x * 0.9f, .y = NINTH_Y};
	Vector2 lbCurrPos = {.x = LB_START_POS.x, .y = NINTH_Y};

	while (!WindowShouldClose())
	{
		UpdateMusicStream(MUSIC);

		if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			pause = !pause;
			if (pause)
				PauseMusicStream(MUSIC);
			else
				ResumeMusicStream(MUSIC);
		}

		if (IsKeyDown(KEY_UP))
		{
			volume += DELTA_VOLUME;
			if (volume > 1.0f)
				volume = 1.0f;
			SetMusicVolume(MUSIC, volume);
		}
		else if (IsKeyDown(KEY_DOWN))
		{
			volume -= DELTA_VOLUME;
			if (volume < 0.0f)
				volume = 0.0f;
			SetMusicVolume(MUSIC, volume);
		}

		if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_R))
		{
			StopMusicStream(MUSIC);
			PlayMusicStream(MUSIC);
		}

		endAngle = (timePlayed * 360.0f) + START_ANGLE;
		lbCurrPos.x = timePlayed * (W_SIZE.x * 0.8f) + LB_START_POS.x;

		timePlayed = GetMusicTimePlayed(MUSIC) / GetMusicTimeLength(MUSIC);

		if (timePlayed > 1.0f)
			timePlayed = 0.0f; // loop

		BeginDrawing();
		{
			ClearBackground(ZOZBLACK);
			DrawTextEx(FONT, CRONOS_LABEL, TEXT_POS, FONT_SIZE, FONT_SPACING, RAYWHITE);

			// pie loader
			DrawCircleSector(SCREEN_CENTER, RADIUS, START_ANGLE, endAngle, SEGMENTS, MUSGO_GREEN);
			DrawRingLines(SCREEN_CENTER, RADIUS - 1, RADIUS, START_ANGLE, endAngle, SEGMENTS, RAYWHITE);

			// lb
			DrawLineEx(LB_START_POS, LB_END_POS, LB_THICKNESS, RAYWHITE);
			DrawLineEx(LB_START_POS, lbCurrPos, LB_THICKNESS, MUSGO_GREEN);
		}
		EndDrawing();
	}

	UnloadMusicStream(MUSIC);
	CloseAudioDevice();

	UnloadFont(FONT);

	CloseWindow();
	return 0;
}