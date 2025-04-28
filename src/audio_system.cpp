#include "audio_system.hpp"

void AudioSystem::play_sound(SOUND_ASSET_ID sound, int channel) {
	if (static_cast<int>(sound) >= sound_count) {
		fprintf(stderr, "Invalid sound ID: %d\n", static_cast<int>(sound));
		return;
	}
	if (!(((channel >= 0) && (channel < num_channels)) || channel == -1)) {
		fprintf(stderr, "Invalid channel ID: %d\n", channel);
		return;
	}
	if (Mix_PlayChannel(channel, sounds[static_cast<int>(sound)], 0) == -1) {
		fprintf(stderr, "Failed to play sound: %s\n", Mix_GetError());
		return;
	}
}
void AudioSystem::play_music(SONG_ASSET_ID music) {
	if (static_cast<int>(music) >= song_count) {
		fprintf(stderr, "Invalid music ID: %d\n", static_cast<int>(music));
		return;
	}
	if (Mix_FadeInMusic(songs[static_cast<int>(music)], -1, VOLUME_FADE_IN_TIME) == -1) {
		fprintf(stderr, "Failed to play music: %s\n", Mix_GetError());
		return;
	}
}
void AudioSystem::stop_music() {
	if (Mix_PlayingMusic()) {
		Mix_HaltMusic();
	}
	else {
		fprintf(stderr, "No music is currently playing\n");
	}
}

void AudioSystem::set_sound_volume(float volume, int channel) {
	if (!(((channel >= 0) && (channel < num_channels)) || channel == -1)) {
		fprintf(stderr, "Invalid channel ID: %d\n", channel);
		return;
	}
	if (Mix_Volume(channel, static_cast<int>(volume * MIX_MAX_VOLUME)) == -1) {
		fprintf(stderr, "Failed to set sound volume: %s\n", Mix_GetError());
		return;
	}
}
void AudioSystem::set_music_volume(float volume) {
	if (Mix_VolumeMusic(static_cast<int>(volume * MIX_MAX_VOLUME)) == -1) {
		fprintf(stderr, "Failed to set music volume: %s\n", Mix_GetError());
		return;
	}
}

float AudioSystem::get_volume() const {
	return static_cast<float>(Mix_Volume(-1, -1)) / MIX_MAX_VOLUME;
}

void AudioSystem::load_sounds() {
	for (int i = 0; i < sound_count; ++i) {
		sounds[i] = Mix_LoadWAV(sound_paths[i].c_str());
		if (!sounds[i]) {
			fprintf(stderr, "Failed to load sound: %s\n", Mix_GetError());
			exit(1);
		}
	}
}

void AudioSystem::load_songs() {
	for (int i = 0; i < song_count; ++i) {
		songs[i] = Mix_LoadMUS(song_paths[i].c_str());
		if (!songs[i]) {
			fprintf(stderr, "Failed to load song: %s\n", Mix_GetError());
			exit(1);
		}
	}
}

void AudioSystem::free_sounds() {
	for (int i = 0; i < sound_count; ++i) {
		if (sounds[i]) {
			Mix_FreeChunk(sounds[i]);
			sounds[i] = nullptr;
		}
	}
}
void AudioSystem::free_songs() {
	for (int i = 0; i < song_count; ++i) {
		if (songs[i]) {
			Mix_FreeMusic(songs[i]);
			songs[i] = nullptr;
		}
	}
}



