#include "raylib.h"
#include "raygui.h"

#define CRONOS_IMPLEMENTATION
#include "cronos.h"

#include "nob.h"

#define ZOZBLACK (Color){24, 24, 24, 255}

int main(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "cronos");
    SetTargetFPS(60);
    ChangeDirectory(GetApplicationDirectory()); // set relative path

    // label
    const char *label = "cronos";
    Font font = GetFontDefault();
    const float fontSize = 40.0f;
    const float fontSpacing = 1.0f;
    const Vector2 textSize = MeasureTextEx(font, label, fontSize, fontSpacing);
    const Vector2 textPos = {
        .x = screenWidth / 2 - (textSize.x / 2.0f),
        .y = (textSize.y / 2.0f)};
    const Vector2 wCenter = {.x = screenWidth / 2.0f, .y = screenHeight / 2.0f};
    const LabelLayout layout = {font, fontSize, fontSpacing, wCenter, textPos, textSize};

    // music
    Assets assets = {0};
    InitAudioDevice();
    bool pause = false;
    float volume = 0.5f; // [0.0f..1.0f]
    const float DELTA_VOLUME = 0.025f;
    const float SEEK_SECONDS = 5.0f;
    Playback pb = {0};
    refresh_playback(&pb, assets.music, layout);

    // lb = LoadingBar
    Rectangle lbBounds = {
        .x = screenWidth * 0.1f,
        .y = screenHeight * 0.90f,
        .width = screenWidth * 0.8f,
        .height = screenHeight * 0.025f};

    // drop hint
    const char *dropHint = "Drop an image or a song to replace it";
    const float hintFontSize = 20.0f;
    Vector2 hintSize = MeasureTextEx(font, dropHint, hintFontSize, fontSpacing);
    Vector2 hintPos = {
        .x = screenWidth / 2 - (hintSize.x / 2.0f),
        .y = lbBounds.y - hintSize.y - 10.0f};

    while (!WindowShouldClose())
    {
        UpdateMusicStream(assets.music);

        // timing
        pb.timePlayedSecs = IsMusicValid(assets.music) ? GetMusicTimePlayed(assets.music) : 0.0f;
        pb.timePlayed = (pb.musicLen > 0.0f) ? (pb.timePlayedSecs / pb.musicLen) : 0.0f;

        float secsLeft = pb.musicLen - pb.timePlayedSecs;
        if (secsLeft < 0.0f)
            secsLeft = 0.0f;

        int mins = (int)secsLeft / 60;
        int secs = (int)secsLeft % 60;
        pb.secsLabel = TextFormat("%02d:%02d", mins, secs);
        pb.secsSize = MeasureTextEx(font, pb.secsLabel, fontSize, fontSpacing);
        pb.secsPos.x = screenWidth / 2 - (pb.secsSize.x / 2.0f);

        if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            pause = !pause;
            if (pause)
                PauseMusicStream(assets.music);
            else
                ResumeMusicStream(assets.music);
        }

        if (IsKeyDown(KEY_UP))
        {
            volume += DELTA_VOLUME;
            if (volume > 1.0f)
                volume = 1.0f;
            SetMusicVolume(assets.music, volume);
        }
        else if (IsKeyDown(KEY_DOWN))
        {
            volume -= DELTA_VOLUME;
            if (volume < 0.0f)
                volume = 0.0f;
            SetMusicVolume(assets.music, volume);
        }

        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_R))
        {
            StopMusicStream(assets.music);
            PlayMusicStream(assets.music);
        }

        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT))
        {
            float seekTo = pb.timePlayedSecs + (IsKeyPressed(KEY_RIGHT) ? SEEK_SECONDS : -SEEK_SECONDS);
            if (seekTo < 0.0f)
                seekTo = 0.0f;
            else if (seekTo > pb.musicLen)
                seekTo = pb.musicLen;
            SeekMusicStream(assets.music, seekTo);
            pb.timePlayedSecs = seekTo;
            pb.timePlayed = (pb.musicLen > 0.0f) ? (pb.timePlayedSecs / pb.musicLen) : 0.0f;
        }

        if (IsFileDropped())
        {
            FilePathList droppedFiles = LoadDroppedFiles();
            for (size_t i = 0; i < (size_t)droppedFiles.count; ++i)
                load_path(droppedFiles.paths[i], &assets, &pause, volume, &pb, layout);
            UnloadDroppedFiles(droppedFiles);
        }

        BeginDrawing();
        {
            ClearBackground(ZOZBLACK);
            // labels
            DrawTextEx(font, label, textPos, fontSize, fontSpacing, RAYWHITE);
            DrawTextEx(font, pb.secsLabel, pb.secsPos, fontSize, fontSpacing, RAYWHITE);

            // cover
            DrawTextureV(assets.texture, (Vector2){screenWidth / 2 - (assets.texture.width / 2), screenHeight / 2 - (assets.texture.height / 2)}, WHITE);

            // lb
            GuiProgressBar(lbBounds, NULL, NULL, &pb.timePlayed, 0.0f, 1.0f);

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
