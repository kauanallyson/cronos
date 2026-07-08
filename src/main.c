#include "raylib.h"
#define NOB_IMPLEMENTATION

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
		TraceLog(LOG_WARNING, "Failed to load music: %s", BG_MUSIC);
	}
	PlayMusicStream(music);
	bool pause = false;
	float volume = 0.5f;						// [0.0f..1.0f]
	float timePlayed = 0.0f;					// [0.0f..1.0f]
	float musicLen = GetMusicTimeLength(music); // Get music time length (in seconds)
	float secsLeft = musicLen - timePlayed;
	const char *secsLabel = TextFormat("%02.02f s", secsLeft);
	SetMusicVolume(music, volume);
	const float DELTA_VOLUME = 0.025f;
	Vector2 secsPos = {.x = textPos.x, .y = textPos.y + textSize.y};

	Image image = LoadImage(COVER);
	if (!IsImageValid(image))
	{
		TraceLog(LOG_WARNING, "Failed to load image: %s", COVER);
	}
	Rectangle cropRect = {
		.x = (image.width - 320.0f) / 2.0f,
		.y = (image.height - 320.0f) / 2.0f,
		.width = 320.0f,
		.height = 320.0f};
	ImageCrop(&image, cropRect);
	Rectangle imagePos = {.x = wCenter.x - 320.0f / 2.0f, .y = wCenter.y - 320.0f / 2.0f, .width = 320.0f, .height = 320.0f};
	Texture2D texture = LoadTextureFromImage(image);
	UnloadImage(image); // depois de gerar a textura pode dar free na imagem

	// lb = LoadingBar
	float lbYPos = wSize.y * 0.9f;
	float thickness = 10.0f;
	Vector2 lbStartPos = {.x = wSize.x * 0.1f, .y = lbYPos};
	Vector2 lbEndPos = {.x = wSize.x * 0.9f, .y = lbYPos};
	Vector2 lbCurrPos = {.x = lbStartPos.x, .y = lbYPos};

	while (!WindowShouldClose())
	{
		UpdateMusicStream(music);

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

		// timing
		float timePlayedSecs = GetMusicTimePlayed(music);
		timePlayed = timePlayedSecs / musicLen;
		if (timePlayed > 1.0f)
			timePlayed = 0.0f;
		float secsLeft = musicLen - timePlayedSecs;
		secsLabel = TextFormat("%02.02f s", secsLeft);
		lbCurrPos.x = timePlayed * (wSize.x * 0.8f) + lbStartPos.x;

		BeginDrawing();
		{
			ClearBackground(ZOZBLACK);
			DrawTextEx(font, label, textPos, fontSize, fontSpacing, RAYWHITE);
			DrawTextEx(font, secsLabel, secsPos, fontSize, fontSpacing, RAYWHITE);
			DrawTextureEx(texture, (Vector2){.x = imagePos.x, .y = imagePos.y}, 0, 1, WHITE);

			// lb
			DrawLineEx(lbStartPos, lbEndPos, thickness, RAYWHITE);
			DrawLineEx(lbStartPos, lbCurrPos, thickness, MUSGO_GREEN);
		}
		EndDrawing();
	}

	UnloadMusicStream(music);
	CloseAudioDevice();

	UnloadTexture(texture);

	CloseWindow();
	return 0;
}