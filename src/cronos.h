#ifndef CRONOS_H
#define CRONOS_H

#include "raylib.h"
#include "nob.h"
#include <stddef.h>

typedef struct
{
    char **items;
    size_t count;
    size_t capacity;
} CrPlaylist;

void cr_playlist_push(CrPlaylist *pl, const char *path);
void cr_playlist_free(CrPlaylist *pl);

typedef struct
{
    Music music;
    bool hasMusic;
    int currentIndex; // -1 base value

    Texture2D cover;
    bool hasCover;

    bool paused;
    float volume; // [0..1]

    CrPlaylist playlist;
} CrPlayer;

void cr_player_reset(CrPlayer *p, float initialVolume);
void cr_player_unload(CrPlayer *p);

void cr_player_add_track(CrPlayer *p, const char *path);

void cr_player_set_cover(CrPlayer *p, const char *path);

void cr_player_play_index(CrPlayer *p, int index);
void cr_player_next(CrPlayer *p);
void cr_player_prev(CrPlayer *p);
void cr_player_toggle_pause(CrPlayer *p);

void cr_player_set_volume(CrPlayer *p, float volume); // clamped to [0..1]
void cr_player_seek(CrPlayer *p, float seconds);      // absolute, clamped
void cr_player_seek_by(CrPlayer *p, float deltaSeconds);

void cr_player_update(CrPlayer *p);

float cr_player_get_progress(CrPlayer *p);         // [0..1]
const char *cr_player_get_track_name(CrPlayer *p); // filename without ext

const char *cr_format_time(float seconds); // "MM:SS"

void cr_draw_texture_fit(Texture2D texture, Rectangle bounds, Color tint);

#ifdef CRONOS_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

void cr_playlist_push(CrPlaylist *pl, const char *path)
{
    size_t len = strlen(path);
    char *copy = malloc(len + 1);
    memcpy(copy, path, len + 1);
    nob_da_append(pl, copy);
}

void cr_playlist_free(CrPlaylist *pl)
{
    nob_da_foreach(char *, it, pl)
        free(*it);
    nob_da_free(*pl);
    *pl = (CrPlaylist){0};
}

void cr_player_reset(CrPlayer *p, float initialVolume)
{
    *p = (CrPlayer){0};
    p->currentIndex = -1;
    p->volume = initialVolume;
}

void cr_player_unload(CrPlayer *p)
{
    if (p->hasMusic)
        UnloadMusicStream(p->music);
    if (p->hasCover)
        UnloadTexture(p->cover);
    cr_playlist_free(&p->playlist);
    cr_player_reset(p, p->volume);
}

void cr_player_add_track(CrPlayer *p, const char *path)
{
    cr_playlist_push(&p->playlist, path);
    if (p->currentIndex < 0)
        cr_player_play_index(p, (int)p->playlist.count - 1);
}

void cr_player_set_cover(CrPlayer *p, const char *path)
{
    if (p->hasCover)
        UnloadTexture(p->cover);
    p->cover = LoadTexture(path);
    p->hasCover = IsTextureValid(p->cover);
}

void cr_player_play_index(CrPlayer *p, int index)
{
    if (index < 0 || (size_t)index >= p->playlist.count)
        return;

    if (p->hasMusic)
        UnloadMusicStream(p->music);

    p->music = LoadMusicStream(p->playlist.items[index]);
    p->hasMusic = IsMusicValid(p->music);
    p->currentIndex = index;

    if (p->hasMusic)
    {
        p->music.looping = false;
        SetMusicVolume(p->music, p->volume);
        PlayMusicStream(p->music);
        p->paused = false;
    }
}

void cr_player_next(CrPlayer *p)
{
    if (p->playlist.count == 0)
        return;
    int next = (p->currentIndex + 1) % (int)p->playlist.count;
    cr_player_play_index(p, next);
}

void cr_player_prev(CrPlayer *p)
{
    if (p->playlist.count == 0)
        return;
    int prev = (p->currentIndex - 1 + (int)p->playlist.count) % (int)p->playlist.count;
    cr_player_play_index(p, prev);
}

void cr_player_toggle_pause(CrPlayer *p)
{
    if (!p->hasMusic)
        return;
    p->paused = !p->paused;
    if (p->paused)
        PauseMusicStream(p->music);
    else
        ResumeMusicStream(p->music);
}

void cr_player_set_volume(CrPlayer *p, float volume)
{
    if (volume < 0.0f)
        volume = 0.0f;
    else if (volume > 1.0f)
        volume = 1.0f;
    p->volume = volume;
    if (p->hasMusic)
        SetMusicVolume(p->music, volume);
}

void cr_player_seek(CrPlayer *p, float seconds)
{
    if (!p->hasMusic)
        return;
    float len = GetMusicTimeLength(p->music);
    if (seconds < 0.0f)
        seconds = 0.0f;
    else if (seconds > len)
        seconds = len;
    SeekMusicStream(p->music, seconds);
}

void cr_player_seek_by(CrPlayer *p, float deltaSeconds)
{
    if (!p->hasMusic)
        return;
    cr_player_seek(p, GetMusicTimePlayed(p->music) + deltaSeconds);
}

void cr_player_update(CrPlayer *p)
{
    if (!p->hasMusic)
        return;

    UpdateMusicStream(p->music);

    if (!p->paused)
    {
        float len = GetMusicTimeLength(p->music);
        float played = GetMusicTimePlayed(p->music);
        if (len > 0.0f && played >= len - 0.05f)
            cr_player_next(p);
    }
}

float cr_player_get_progress(CrPlayer *p)
{
    if (!p->hasMusic)
        return 0.0f;
    float len = GetMusicTimeLength(p->music);
    if (len <= 0.0f)
        return 0.0f;
    return GetMusicTimePlayed(p->music) / len;
}

const char *cr_player_get_track_name(CrPlayer *p)
{
    if (p->currentIndex < 0)
        return NULL;
    return GetFileNameWithoutExt(p->playlist.items[p->currentIndex]);
}

const char *cr_format_time(float seconds)
{
    if (seconds < 0.0f)
        seconds = 0.0f;
    int mins = (int)seconds / 60;
    int secs = (int)seconds % 60;
    return TextFormat("%02d:%02d", mins, secs);
}

void cr_draw_texture_fit(Texture2D texture, Rectangle bounds, Color tint)
{
    if (texture.id == 0)
        return;

    float scaleX = bounds.width / (float)texture.width;
    float scaleY = bounds.height / (float)texture.height;
    float scale = scaleX < scaleY ? scaleX : scaleY;

    float w = texture.width * scale;
    float h = texture.height * scale;

    Rectangle src = {0, 0, (float)texture.width, (float)texture.height};
    Rectangle dst = {bounds.x + (bounds.width - w) / 2.0f, bounds.y + (bounds.height - h) / 2.0f, w, h};
    DrawTexturePro(texture, src, dst, (Vector2){0, 0}, 0.0f, tint);
}

#endif // CRONOS_IMPLEMENTATION

#endif // CRONOS_H
