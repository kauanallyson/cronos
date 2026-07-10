#ifndef CRONOS_H
#define CRONOS_H

#include <stddef.h>

#include "raylib.h"

#define COVER_SIZE 320

typedef struct Assets
{
    Texture2D texture;
    Music music;
} Assets;

typedef enum
{
    ASSET_KIND_NONE,
    ASSET_KIND_IMAGE,
    ASSET_KIND_MUSIC,
} AssetKind;

typedef struct Playback
{
    float musicLen;        // music length in secs
    float timePlayed;      // [0.0f..1.0f]
    float timePlayedSecs;  // playback position in secs
    const char *secsLabel; // "mm:ss" time remaining
    Vector2 secsSize;
    Vector2 secsPos;
} Playback;

typedef struct LabelLayout
{
    Font font;
    float fontSize;
    float fontSpacing;
    Vector2 wCenter;
    Vector2 textPos;
    Vector2 textSize;
} LabelLayout;

AssetKind get_asset_kind(const char *filepath);
bool set_image_asset(Assets *assets, const char *filepath);
bool set_music_asset(Assets *assets, const char *filepath);
void refresh_playback(Playback *pb, Music music, LabelLayout layout);
void load_path(const char *path, Assets *assets, bool *pause, float volume,
               Playback *pb, LabelLayout layout);

#ifdef CRONOS_IMPLEMENTATION

AssetKind get_asset_kind(const char *filepath)
{
    const char *ext = GetFileExtension(filepath);
    if (ext == NULL)
        return ASSET_KIND_NONE;

    if (TextIsEqual(ext, ".png") || TextIsEqual(ext, ".jpg") ||
        TextIsEqual(ext, ".jpeg") || TextIsEqual(ext, ".bmp") ||
        TextIsEqual(ext, ".gif") || TextIsEqual(ext, ".qoi"))
        return ASSET_KIND_IMAGE;

    if (TextIsEqual(ext, ".mp3") || TextIsEqual(ext, ".wav") ||
        TextIsEqual(ext, ".ogg") || TextIsEqual(ext, ".flac") ||
        TextIsEqual(ext, ".xm") || TextIsEqual(ext, ".mod") ||
        TextIsEqual(ext, ".qoa"))
        return ASSET_KIND_MUSIC;

    return ASSET_KIND_NONE;
}

bool set_image_asset(Assets *assets, const char *filepath)
{
    Image image = LoadImage(filepath);
    if (!IsImageValid(image))
    {
        TraceLog(LOG_WARNING, "Could not add image to assets: %s", filepath);
        return false;
    }

    int cropSize = (image.width < image.height) ? image.width : image.height;
    Rectangle crop = {
        .x = (image.width - cropSize) / 2.0f,
        .y = (image.height - cropSize) / 2.0f,
        .width = (float)cropSize,
        .height = (float)cropSize,
    };
    ImageCrop(&image, crop);
    ImageResize(&image, COVER_SIZE, COVER_SIZE);

    Texture2D newTexture = LoadTextureFromImage(image);
    UnloadImage(image);
    if (!IsTextureValid(newTexture))
    {
        TraceLog(LOG_WARNING, "Could not upload image asset to GPU: %s", filepath);
        return false;
    }

    if (IsTextureValid(assets->texture))
        UnloadTexture(assets->texture);
    assets->texture = newTexture;
    return true;
}

bool set_music_asset(Assets *assets, const char *filepath)
{
    Music newMusic = LoadMusicStream(filepath);
    if (!IsMusicValid(newMusic))
    {
        TraceLog(LOG_WARNING, "Could not add music to assets: %s", filepath);
        return false;
    }

    if (IsMusicValid(assets->music))
    {
        StopMusicStream(assets->music);
        UnloadMusicStream(assets->music);
    }
    assets->music = newMusic;
    return true;
}

static void reset_playback_time(Playback *pb, Music music)
{
    pb->musicLen = IsMusicValid(music) ? GetMusicTimeLength(music) : 0.0f;
    pb->timePlayed = 0.0f;
    pb->timePlayedSecs = 0.0f;
}

static void update_playback_label(Playback *pb, LabelLayout layout)
{
    pb->secsLabel = TextFormat("%02d:%02d", (int)pb->musicLen / 60, (int)pb->musicLen % 60);
    pb->secsSize = MeasureTextEx(layout.font, pb->secsLabel, layout.fontSize, layout.fontSpacing);
    pb->secsPos.x = layout.wCenter.x - (pb->secsSize.x / 2.0f);
    pb->secsPos.y = layout.textPos.y + layout.textSize.y;
}

void refresh_playback(Playback *pb, Music music, LabelLayout layout)
{
    reset_playback_time(pb, music);
    update_playback_label(pb, layout);
}

static void start_music_playback(Assets *assets, bool *pause, float volume, Playback *pb, LabelLayout layout)
{
    *pause = false;
    PlayMusicStream(assets->music);
    SetMusicVolume(assets->music, volume);
    refresh_playback(pb, assets->music, layout);
}

// Loads `path` as either an image or music asset, updating playback state as needed.
void load_path(const char *path, Assets *assets, bool *pause, float volume,
               Playback *pb, LabelLayout layout)
{
    AssetKind kind = get_asset_kind(path);

    if (kind == ASSET_KIND_IMAGE)
    {
        set_image_asset(assets, path);
    }
    else if (kind == ASSET_KIND_MUSIC)
    {
        if (set_music_asset(assets, path))
            start_music_playback(assets, pause, volume, pb, layout);
    }
    else
    {
        TraceLog(LOG_WARNING, "Unsupported file type: %s", path);
    }
}

#endif // CRONOS_IMPLEMENTATION

#endif // CRONOS_H
