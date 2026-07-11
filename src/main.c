#include "raylib.h"
#include "raymath.h"
#include "raygui.h"

#define CRONOS_IMPLEMENTATION
#include "cronos.h"

#include "nob.h"

#define ZOZBLACK (Color){24, 24, 24, 255}

int main(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

    const int screenWidth = 1024;
    const int screenHeight = 768;

    // SetConfigFlags(FLAG_WINDOW_UNDECORATED);
    InitWindow(screenWidth, screenHeight, "cronos");
    SetTargetFPS(60);

    // label
    const char *label = "cronos";
    Font font = GetFontDefault();
    const float fontSize = 40.0f;
    const float fontSpacing = 1.0f;
    InitAudioDevice();

    CrPlayer player;
    cr_player_reset(&player, 0.5f);
    const float SEEK_SECONDS = 5.0f;

    bool seeking = false;
    float seekPreview = 0.0f;

    while (!WindowShouldClose())
    {
        const int sw = GetScreenWidth();
        const int sh = GetScreenHeight();

        const Vector2 textSize = MeasureTextEx(font, label, fontSize, fontSpacing);
        const Vector2 textPos = {.x = sw / 2 - (textSize.x / 2.0f), .y = sh * 0.02f};

        const Rectangle coverBounds = {(sw - 300) / 2.0f, sh / 2.0f - 150.0f, 300.0f, 300.0f};
        const Rectangle seekBounds = {sw * 0.1f, sh * 0.79f, sw * 0.8f, sh * 0.025f};
        const Rectangle prevBounds = {sw / 2.0f - 90.0f, sh * 0.86f, 50.0f, 46.0f};
        const Rectangle playBounds = {sw / 2.0f - 30.0f, sh * 0.86f, 60.0f, 46.0f};
        const Rectangle nextBounds = {sw / 2.0f + 40.0f, sh * 0.86f, 50.0f, 46.0f};
        const Rectangle volumeBounds = {sw / 2.0f - 110.0f, sh * 0.94f, 220.0f, 20.0f};

        if (!seeking)
            cr_player_update(&player);
        else
            UpdateMusicStream(player.music);

        if (IsFileDropped())
        {
            FilePathList files = LoadDroppedFiles();
            for (size_t i = 0; i < files.count; i++)
            {
                const char *p = files.paths[i];
                if (IsFileExtension(p, ".mp3;.ogg;.wav;.qoa"))
                    cr_player_add_track(&player, p);
                else if (IsFileExtension(p, ".png;.jpg;.jpeg;.bmp"))
                    cr_player_set_cover(&player, p);
            }
            UnloadDroppedFiles(files);
        }

        if (IsKeyPressed(KEY_F11) || IsKeyPressed(KEY_F))
            ToggleFullscreen();

        if (IsKeyPressed(KEY_SPACE))
            cr_player_toggle_pause(&player);

        if (IsKeyDown(KEY_UP))
            cr_player_set_volume(&player, player.volume + 0.025f);
        else if (IsKeyDown(KEY_DOWN))
            cr_player_set_volume(&player, player.volume - 0.025f);

        if (IsKeyPressed(KEY_RIGHT))
            cr_player_seek_by(&player, SEEK_SECONDS);
        else if (IsKeyPressed(KEY_LEFT))
            cr_player_seek_by(&player, -SEEK_SECONDS);

        float musicLen = player.hasMusic ? GetMusicTimeLength(player.music) : 0.0f;
        float timePlayed = player.hasMusic ? GetMusicTimePlayed(player.music) : 0.0f;

        BeginDrawing();
        {
            ClearBackground(ZOZBLACK);

            DrawTextEx(font, label, textPos, fontSize, fontSpacing, RAYWHITE);

            const char *trackName = cr_player_get_track_name(&player);
            if (trackName != NULL)
            {
                Vector2 nameSize = MeasureTextEx(font, trackName, 20.0f, fontSpacing);
                DrawTextEx(font, trackName, (Vector2){sw / 2 - nameSize.x / 2.0f, sh * 0.10f}, 20.0f, fontSpacing, RAYWHITE);
            }

            cr_draw_texture_fit(player.cover, coverBounds, WHITE);

            const char *timeLabel = TextFormat("%s / %s", cr_format_time(timePlayed), cr_format_time(musicLen));
            Vector2 timeSize = MeasureTextEx(font, timeLabel, 20.0f, fontSpacing);
            DrawTextEx(font, timeLabel, (Vector2){sw / 2 - timeSize.x / 2.0f, seekBounds.y - 28.0f}, 20.0f, fontSpacing, RAYWHITE);

            // seek bar
            float sliderValue = seeking ? seekPreview : cr_player_get_progress(&player);
            bool mouseOverSeek = CheckCollisionPointRec(GetMousePosition(), seekBounds);

            GuiSlider(seekBounds, NULL, NULL, &sliderValue, 0.0f, 1.0f);

            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && (mouseOverSeek || seeking) && player.hasMusic)
            {
                seeking = true;
                seekPreview = sliderValue;
            }
            else if (seeking)
            {
                seeking = false;
                cr_player_seek(&player, seekPreview * musicLen);
            }

            // playback controls
            if (GuiButton(prevBounds, "<<"))
                cr_player_prev(&player);
            if (GuiButton(playBounds, player.paused ? ">" : "||"))
                cr_player_toggle_pause(&player);
            if (GuiButton(nextBounds, ">>"))
                cr_player_next(&player);

            // volume
            float volumeValue = player.volume;
            GuiSlider(volumeBounds, "Vol", NULL, &volumeValue, 0.0f, 1.0f);
            if (volumeValue != player.volume)
                cr_player_set_volume(&player, volumeValue);
        }
        EndDrawing();
    }

    cr_player_unload(&player);
    CloseAudioDevice();

    CloseWindow();
    return 0;
}
