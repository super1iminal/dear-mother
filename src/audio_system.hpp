#pragma once
#include "common.hpp"
#include <SDL.h>
#include <SDL_mixer.h>
#define SDL_MAIN_HANDLED

const int VOLUME_FADE_IN_TIME = 5000; // 5 seconds

enum class SOUND_ASSET_ID
{
	MELEE_SOUND = 0,
	PLAYER_SHOOTING_SOUND = MELEE_SOUND + 1,
	ENEMY_SHOOTING_SOUND = PLAYER_SHOOTING_SOUND + 1,
	PLAYER_PROJECTILE_HIT_SOUND = ENEMY_SHOOTING_SOUND + 1,
	DOOR_CHANGE_SOUND = PLAYER_PROJECTILE_HIT_SOUND + 1,
	SOUND_COUNT = DOOR_CHANGE_SOUND + 1,
};

const int sound_count = (int)SOUND_ASSET_ID::SOUND_COUNT;

enum class SONG_ASSET_ID
{
	COMBAT_MUSIC = 0,
	POST_COMBAT_MUSIC = COMBAT_MUSIC + 1,
	BOSS_ONE_MUSIC = POST_COMBAT_MUSIC + 1,
	BOSS_TWO_MUSIC = BOSS_ONE_MUSIC + 1,
	BOSS_THREE_MUSIC = BOSS_TWO_MUSIC + 1,
	SONG_COUNT = BOSS_THREE_MUSIC + 1,
};

const int song_count = (int)SONG_ASSET_ID::SONG_COUNT;

class AudioSystem
{
public:
	AudioSystem() {
		if (SDL_Init(SDL_INIT_AUDIO) < 0) {
			fprintf(stderr, "Failed to initialize SDL Audio");
			exit(1);
		}
		if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
			fprintf(stderr, "Failed to open audio device");
			exit(1);
		}

		load_sounds();
		load_songs();

		num_channels = Mix_AllocateChannels(16);
	}
	~AudioSystem() {
		free_sounds();
		free_songs();
		Mix_CloseAudio();
		SDL_Quit();
	}

	void play_sound(SOUND_ASSET_ID sound, int channel = -1);
	void play_music(SONG_ASSET_ID music);
	void stop_music();
	void set_sound_volume(float volume, int channel = -1);
	void set_music_volume(float volume);
	float get_volume() const;
private:
	void load_sounds();
	void load_songs();
	void free_sounds();
	void free_songs();

	std::vector<Mix_Chunk*> sounds;
	std::vector<Mix_Music*> songs;

	int num_channels;

	// must be in the same order as the enums above
	const std::array<std::string, sound_count> sound_paths = {
		audio_path("impactMetal_medium_003.wav"),
		audio_path("laserSmall_000.wav"),
		audio_path("laserLarge_000.wav"),
		audio_path("forceField_002.wav"),
		audio_path("doorchange.wav"),
	};
	// must be in the same order as the enums above
	const std::array<std::string, song_count> song_paths = {
		audio_path("The Death of Gods Will-[AudioTrimmer.com].wav"),
		audio_path("Factory-On-Mercury_Looping.wav"),
		audio_path("Alex Roe - Darksign - 09 Aylward and Lilura-compressed.wav"),
		audio_path("Alex Roe - Darksign - 06 Chaos Ember Dragon-compressed.wav"),
		audio_path("Alex Roe - Darksign - 05 Imprisoned Guardian-compressed.wav"),
	};

};
