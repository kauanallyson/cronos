#include "raylib.h"
#define CRONOS_IMPLEMENTATION
#include "cronos.h"

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
        .y = (textSize.y / 2.0f)
    };

    Assets assets = {0};

    // music
    InitAudioDevice();
    if (!set_music_asset(&assets, BG_MUSIC)) {
        TraceLog(LOG_ERROR, "Failed to load music: %s", BG_MUSIC);
        CloseAudioDevice();
        CloseWindow();
        return 1;
    }
    PlayMusicStream(assets.music);
    bool pause = false;
    float volume = 0.5f; // [0.0f..1.0f]
    SetMusicVolume(assets.music, volume);
    const float DELTA_VOLUME = 0.025f;
    const float SEEK_SECONDS = 5.0f;
    Playback pb = {0};
    refresh_playback_geometry(&pb, assets.music, font, fontSize, fontSpacing, wCenter, textPos, textSize);

    // cover
    if (!set_image_asset(&assets, COVER)) {
        TraceLog(LOG_ERROR, "Failed to load image: %s", COVER);
        UnloadMusicStream(assets.music);
        CloseAudioDevice();
        CloseWindow();
        return 1;
    }

    // lb = LoadingBar
    Vector2 lbPos = {.x = wSize.x * 0.1f, .y = wSize.y * 0.90f};
    Vector2 lbSize = {.x = wSize.x * 0.8f, .y = wSize.y * 0.025f};

    // drop hint
    const char *dropHint = "Drop an image or a song to replace it";
    const float hintFontSize = 20.0f;
    Vector2 hintSize = MeasureTextEx(font, dropHint, hintFontSize, fontSpacing);
    Vector2 hintPos = {
        .x = wCenter.x - (hintSize.x / 2.0f),
        .y = lbPos.y - hintSize.y - 10.0f
    };

    while (!WindowShouldClose()) {
        UpdateMusicStream(assets.music);

        // timing
        pb.timePlayedSecs = GetMusicTimePlayed(assets.music);
        pb.timePlayed = pb.timePlayedSecs / pb.musicLen;

        float secsLeft = pb.musicLen - pb.timePlayedSecs;
        if (secsLeft < 0.0f)
            secsLeft = 0.0f;

        int mins = (int)secsLeft / 60;
        int secs = (int)secsLeft % 60;
        pb.secsLabel = TextFormat("%02d:%02d", mins, secs);
        pb.secsSize = MeasureTextEx(font, pb.secsLabel, fontSize, fontSpacing);
        pb.secsPos.x = wCenter.x - (pb.secsSize.x / 2.0f);

        if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            pause = !pause;
            if (pause)
                PauseMusicStream(assets.music);
            else
                ResumeMusicStream(assets.music);
        }

        if (IsKeyDown(KEY_UP)) {
            volume += DELTA_VOLUME;
            if (volume > 1.0f)
                volume = 1.0f;
            SetMusicVolume(assets.music, volume);
        } else if (IsKeyDown(KEY_DOWN)) {
            volume -= DELTA_VOLUME;
            if (volume < 0.0f)
                volume = 0.0f;
            SetMusicVolume(assets.music, volume);
        }

        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_R)) {
            StopMusicStream(assets.music);
            PlayMusicStream(assets.music);
        }

        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT)) {
            float seekTo = pb.timePlayedSecs + (IsKeyPressed(KEY_RIGHT) ? SEEK_SECONDS : -SEEK_SECONDS);
            if (seekTo < 0.0f)
                seekTo = 0.0f;
            else if (seekTo > pb.musicLen)
                seekTo = pb.musicLen;
            SeekMusicStream(assets.music, seekTo);
            pb.timePlayedSecs = seekTo;
            pb.timePlayed = pb.timePlayedSecs / pb.musicLen;
        }

        if (IsFileDropped()) {
            FilePathList droppedFiles = LoadDroppedFiles();
            for (size_t i = 0; i < (size_t)droppedFiles.count; ++i)
                load_path(droppedFiles.paths[i], &assets, &pause, volume, &pb, font, fontSize, fontSpacing, wCenter, textPos, textSize);
            UnloadDroppedFiles(droppedFiles);
        }

        BeginDrawing();
        {
            ClearBackground(ZOZBLACK);
            // labels
            DrawTextEx(font, label, textPos, fontSize, fontSpacing, RAYWHITE);
            DrawTextEx(font, pb.secsLabel, pb.secsPos, fontSize, fontSpacing, RAYWHITE);

            // cover
            DrawTextureV(assets.texture, (Vector2) {
                wCenter.x - (assets.texture.width / 2), wCenter.y - (assets.texture.height / 2)
            }, WHITE);

            // lb
            DrawRectangleV(lbPos, lbSize, GRAY);
            DrawRectangleV(lbPos, (Vector2) {
                pb.timePlayed * lbSize.x, lbSize.y
            }, MUSGO_GREEN);

            // drop
            DrawTextEx(font, dropHint, hintPos, hintFontSize, fontSpacing, RAYWHITE);
        }
        EndDrawing();
    }

    UnloadMusicStream(assets.music);
    CloseAudioDevice();

    UnloadTexture(assets.texture);

    CloseWindow();
    return 0;
}
