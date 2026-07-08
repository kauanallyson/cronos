#include "raylib.h"

#define RESOURCES_DIR "../resources/"
#define BG_MUSIC RESOURCES_DIR "On & On NCS.mp3"
#define COVER RESOURCES_DIR "cover.png"

#define ZOZBLACK (Color){24, 24, 24, 255}
#define MUSGO_GREEN (Color){10, 95, 30, 255}

int main(void)
{
	const char *wTitle = "cronos";
	Vector2 wSize = {800, 600};
	Vector2 wCenter = {.x = wSize.x / 2.0f, .y = wSize.y / 2.0f};

	InitWindow((int)wSize.x, (int)wSize.y, wTitle);
	SetTargetFPS(60);
	ChangeDirectory(GetApplicationDirectory()); // set relative path

	// label
	const char *label = "cronos";
	Font font = GetFontDefault();
	const float fontSize = 40.0f;
	const float fontSpacing = 1.0f;
	const Vector2 textSize = MeasureTextEx(font, label, fontSize, fontSpacing);
	const Vector2 textPos = {
		.x = wCenter.x - (textSize.x / 2.0f),
		.y = (textSize.y / 2.0f)};

	// music
	InitAudioDevice();
	Music music = LoadMusicStream(BG_MUSIC);
	if (!IsMusicValid(music))
	{
		TraceLog(LOG_ERROR, "Failed to load music: %s", BG_MUSIC);
		CloseAudioDevice();
		CloseWindow();
		return 1;
	}
	PlayMusicStream(music);
	bool pause = false;
	float volume = 0.5f; // [0.0f..1.0f]
	SetMusicVolume(music, volume);
	const float DELTA_VOLUME = 0.025f;
	float musicLen = GetMusicTimeLength(music); // music length in secs
	float timePlayed = 0.0f;					// [0.0f..1.0f]
	float timePlayedSecs = 0.0f;				// playback position in secs
	const char *secsLabel = TextFormat("%02d:%02d", (int)musicLen / 60, (int)musicLen % 60);
	Vector2 secsSize = MeasureTextEx(font, secsLabel, fontSize, fontSpacing);
	Vector2 secsPos = {
		.x = wCenter.x - (secsSize.x / 2.0f),
		.y = textPos.y + textSize.y};

	// cover
	Texture2D texture = LoadTexture(COVER);
	if (!IsTextureValid(texture))
	{
		TraceLog(LOG_ERROR, "Failed to load image: %s", COVER);
		UnloadMusicStream(music);
		CloseAudioDevice();
		CloseWindow();
		return 1;
	}

	// lb = LoadingBar
	Vector2 lbPos = {.x = wSize.x * 0.1f, .y = wSize.y * 0.90f};
	Vector2 lbSize = {.x = wSize.x * 0.8f, .y = wSize.y * 0.025f};

	while (!WindowShouldClose())
	{
		UpdateMusicStream(music);

		// timing
		timePlayedSecs = GetMusicTimePlayed(music);
		timePlayed = timePlayedSecs / musicLen;

		float secsLeft = musicLen - timePlayedSecs;
		if (secsLeft < 0.0f)
			secsLeft = 0.0f;

		int mins = (int)secsLeft / 60;
		int secs = (int)secsLeft % 60;
		secsLabel = TextFormat("%02d:%02d", mins, secs);
		secsSize = MeasureTextEx(font, secsLabel, fontSize, fontSpacing);
		secsPos.x = wCenter.x - (secsSize.x / 2.0f);

		if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			pause = !pause;
			if (pause)
				PauseMusicStream(music);
			else
				ResumeMusicStream(music);
		}

		if (IsKeyDown(KEY_UP))
		{
			volume += DELTA_VOLUME;
			if (volume > 1.0f)
				volume = 1.0f;
			SetMusicVolume(music, volume);
		}
		else if (IsKeyDown(KEY_DOWN))
		{
			volume -= DELTA_VOLUME;
			if (volume < 0.0f)
				volume = 0.0f;
			SetMusicVolume(music, volume);
		}

		if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_R))
		{
			StopMusicStream(music);
			PlayMusicStream(music);
		}

		BeginDrawing();
		{
			ClearBackground(ZOZBLACK);
			// labels
			DrawTextEx(font, label, textPos, fontSize, fontSpacing, RAYWHITE);
			DrawTextEx(font, secsLabel, secsPos, fontSize, fontSpacing, RAYWHITE);

			// cover
			DrawTextureV(texture, (Vector2){.x = wCenter.x - (texture.width / 2), .y = wCenter.y - (texture.height / 2)}, WHITE);

			// lb
			DrawRectangleV(lbPos, lbSize, GRAY);
			DrawRectangleV(lbPos, (Vector2){.x = timePlayed * lbSize.x, .y = lbSize.y}, MUSGO_GREEN);
		}
		EndDrawing();
	}

	UnloadMusicStream(music);
	CloseAudioDevice();

	UnloadTexture(texture);

	CloseWindow();
	return 0;
}