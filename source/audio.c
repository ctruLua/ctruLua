/***
The `audio` module.
An audio channel can play only one audio object at a time.
There are 24 audio channels available, numbered from 0 to 23.
@module ctr.audio
@usage local audio = require("ctr.audio")
*/

#include <3ds.h>

#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include <lua.h>
#include <lauxlib.h>

#include <ivorbiscodec.h>
#include <ivorbisfile.h>

// Audio object type
typedef enum {
	TYPE_UNKNOWN = -1,
	TYPE_OGG = 0,
	TYPE_WAV = 1,
	TYPE_RAW = 2
} filetype;

// Audio object userdata
typedef struct {
	filetype type; // file type

	// File type specific
	union {
		// OGG Vorbis
		struct {
			OggVorbis_File vf;
			int currentSection; // section and position at the end of the initial data
			long rawPosition;
		};
		// WAV
		struct {
			FILE* file;
			long fileSize;
			long filePosition; // position at the end of the initial data
		};
	};

	// Needed for playback
	float rate; // sample rate (per channel) (Hz)
	u32 channels; // channel count
	u32 encoding; // data encoding (NDSP_ENCODING_*)

	// Initial data
	u32 nsamples; // numbers of samples in the audio (per channel, not the total)
	u32 size; // number of bytes in the audio (total, ie data size)
	char* data; // raw audio data

	// Other useful data
	u16 bytePerSample; // bytes per sample (warning: undefined for ADPCM (only for raw data))
	u32 chunkSize; // size per chunk (for streaming)
	u32 chunkNsamples; // number of samples per chunk

	// Playing parameters (type-independant)
	float mix[12]; // mix parameters
	ndspInterpType interp; // interpolation type
	double speed; // playing speed
} audio_userdata;

// Audio stream instance struct (when an audio is played; only used when streaming)
typedef struct {
	audio_userdata* audio;

	// Current position information
	union {
		// OGG
		struct {
			int currentSection;
			long rawPosition;
		};
		// WAV
		long filePosition;
	};

	double prevStartTime; // audio time when last chunk started playing
	bool eof; // if reached end of file
	bool done; // if streaming ended and the stream will be skipped on the next update
	// (the struct should be keept in memory until replaced or audio stopped or it will break audio:time())

	char* nextData; // the next data to play
	ndspWaveBuf* nextWaveBuf;

	char* prevData; // the data actually playing
	ndspWaveBuf* prevWaveBuf;
} audio_stream;

// Indicate if NDSP was initialized or not.
// NDSP doesn't work on citra yet.
// Please only throw an error related to this when using a ndsp function, so other parts of the
// audio module are still available on citra, like audio.load() and audio:info().
bool isAudioInitialized = false;

// Array of the last audio_userdata sent to each channel; channels range from 0 to 23
audio_userdata* channels[24];

// Array of the audio_instance that needs to be updated when calling audio.update (indexed per channel).
audio_stream* streaming[24];

// Stop playing audio on a channel, and stop streaming/free memory.
void stopAudio(int channel) {
	ndspChnWaveBufClear(channel);

	// Stop streaming and free data
	if (streaming[channel] != NULL) {
		audio_stream* stream = streaming[channel];
		if (stream->nextWaveBuf != NULL) {
			free(stream->nextWaveBuf);
			stream->nextWaveBuf = NULL;
		}
		if (stream->nextData != NULL) {
			linearFree(stream->nextData);
			stream->nextData = NULL;
		}
		if (stream->prevWaveBuf != NULL) {
			free(stream->prevWaveBuf);
			stream->prevWaveBuf = NULL;
		}
		if (stream->prevData != NULL) {
			linearFree(stream->prevData);
			stream->prevData = NULL;
		}
		free(stream);
		streaming[channel] = NULL;
	}
}

/***
Load an audio file.
OGG Vorbis and PCM WAV file format are currently supported.
(Most WAV files use the PCM encoding).
NOTE: audio streaming doesn't use threading for now, this means that the decoding will be done on the main thread.
It should work fine with WAVs, but with OGG files you may suffer slowdowns when a new chunk of data is decoded.
To avoid that, you can either reduce the chunkDuration or disable streaming, but be careful if you do so, audio files
can fill the memory really quickly.
@function load
@tparam string path path to the file or the data if type is raw
@tparam[opt=0.1] number chunkDuration if set to -1, streaming will be disabled (all data is loaded in memory at once)
                                     Other values are the stream chunk duration in seconds (ctrµLua will load
                                     the audio per chunk of x seconds). Note that you need to call audio.update() each
                                     frame in order for ctµLua to load new data from audio streams. Two chunks of data
                                     will be loaded at the same time at most (one playing, the other ready to be played).
@tparam[opt=detect] string type file type, `"ogg"` or `"wav"`.
                                If set to `"detect"`, will try to deduce the type from the filename.
@treturn[1] audio the loaded audio object
@treturn[2] nil if a error happened
@treturn[2] string error message
*/
static int audio_load(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);
	double streamChunk = luaL_optnumber(L, 2, 0.1);
	const char* argType = luaL_optstring(L, 3, "detect");

	// Create userdata
	audio_userdata *audio = lua_newuserdata(L, sizeof(*audio));
	luaL_getmetatable(L, "LAudio");
	lua_setmetatable(L, -2);
	for (int i=0; i<12; i++) audio->mix[i] = 1;
	audio->interp = NDSP_INTERP_LINEAR;
	audio->speed = 1;

	// Get file type
	filetype type = TYPE_UNKNOWN;
	if (strcmp(argType, "detect") == 0) {
		const char *dot = strrchr(path, '.');
		if (!dot || dot == path) dot = "";
		const char *ext = dot + 1;
		if (strncmp(ext, "ogg", 3) == 0) type = TYPE_OGG;
		else if (strncmp(ext, "wav", 3) == 0) type = TYPE_WAV;
	} else if (strcmp(argType, "ogg") == 0) {
		type = TYPE_OGG;
	} else if (strcmp(argType, "wav") == 0) {
		type = TYPE_WAV;
	} else if (strcmp(argType, "raw") == 0) {
		type = TYPE_RAW;
	}

	// Open and read file
	if (type == TYPE_OGG) {
		audio->type = TYPE_OGG;

		// Load audio file
		FILE *file = fopen(path, "rb");
		if (ov_open(file, &audio->vf, NULL, 0) < 0) {
			lua_pushnil(L);
			lua_pushstring(L, "input does not appear to be a valid ogg vorbis file or doesn't exist");
			return 2;
		}

		// Decoding Ogg Vorbis bitstream
		vorbis_info* vi = ov_info(&audio->vf, -1);
		if (vi == NULL) luaL_error(L, "could not retrieve ogg audio stream informations");

		audio->rate = vi->rate;
		audio->channels = vi->channels;
		audio->encoding = NDSP_ENCODING_PCM16;
		audio->nsamples = ov_pcm_total(&audio->vf, -1);
		audio->size = audio->nsamples * audio->channels * 2; // *2 because output is PCM16 (2 bytes/sample)
		audio->bytePerSample = 2;

		// Streaming
		if (streamChunk < 0) {
			audio->chunkNsamples = audio->nsamples;
			audio->chunkSize = audio->size;
		} else {
			audio->chunkNsamples = round(streamChunk * audio->rate);
			audio->chunkSize = audio->chunkNsamples * audio->channels * 2;
		}

		// Allocate
		if (linearSpaceFree() < audio->chunkSize) luaL_error(L, "not enough linear memory available");
		audio->data = linearAlloc(audio->chunkSize);

		// Decoding loop
		int offset = 0;
		int eof = 0;
		while (!eof && offset < audio->chunkSize) {
			long ret = ov_read(&audio->vf, &audio->data[offset], fmin(audio->chunkSize - offset, 4096), &audio->currentSection);

			if (ret == 0) {
				eof = 1;
			} else if (ret < 0) {
				ov_clear(&audio->vf);
				linearFree(audio->data);
				luaL_error(L, "error in the ogg vorbis stream");
				return 0;
			} else {
				// TODO handle multiple links (http://xiph.org/vorbis/doc/vorbisfile/decoding.html)
				offset += ret;
			}
		}
		audio->rawPosition = ov_raw_tell(&audio->vf);

		return 1;

	} else if (type == TYPE_WAV) {
		audio->type = TYPE_WAV;

		// Used this as a reference for the WAV format: http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html

		// Load file
		FILE *file = fopen(path, "rb");
		if (file) {
			bool valid = true; // if something goes wrong, this will be false

			char buff[8];

			// Master chunk
			fread(buff, 4, 1, file); // ckId
			if (strncmp(buff, "RIFF", 4) != 0) valid = false;

			fseek(file, 4, SEEK_CUR); // skip ckSize

			fread(buff, 4, 1, file); // WAVEID
			if (strncmp(buff, "WAVE", 4) != 0) valid = false;

			// fmt Chunk
			fread(buff, 4, 1, file); // ckId
			if (strncmp(buff, "fmt ", 4) != 0) valid = false;

			fread(buff, 4, 1, file); // ckSize
			if (*buff != 16) valid = false; // should be 16 for PCM format

			fread(buff, 2, 1, file); // wFormatTag
			if (*buff != 0x0001) valid = false; // PCM format

			u16 channels;
			fread(&channels, 2, 1, file); // nChannels
			audio->channels = channels;
			
			u32 rate;
			fread(&rate, 4, 1, file); // nSamplesPerSec
			audio->rate = rate;

			fseek(file, 4, SEEK_CUR); // skip nAvgBytesPerSec

			u16 byte_per_block; // 1 block = 1*channelCount samples
			fread(&byte_per_block, 2, 1, file); // nBlockAlign

			u16 byte_per_sample;
			fread(&byte_per_sample, 2, 1, file); // wBitsPerSample
			byte_per_sample /= 8; // bits -> bytes

			// There may be some additionals chunks between fmt and data
			// TODO handle some usefull chunks that may be here
			fread(&buff, 4, 1, file); // ckId
			while (strncmp(buff, "data", 4) != 0) {
				u32 size;
				fread(&size, 4, 1, file); // ckSize

				fseek(file, size, SEEK_CUR); // skip chunk

				int i = fread(&buff, 4, 1, file); // next chunk ckId

				if (i < 4) { // reached EOF before finding a data chunk
					valid = false;
					break;
				}
			}

			// data Chunk (ckId already read)
			u32 size;
			fread(&size, 4, 1, file); // ckSize
			audio->size = size;

			audio->nsamples = audio->size / byte_per_block;

			if (byte_per_sample == 1) audio->encoding = NDSP_ENCODING_PCM8;
			else if (byte_per_sample == 2) audio->encoding = NDSP_ENCODING_PCM16;
			else luaL_error(L, "unknown encoding, needs to be PCM8 or PCM16");

			if (!valid) {
				fclose(file);
				luaL_error(L, "invalid PCM wav file");
				return 0;
			}

			audio->bytePerSample = byte_per_sample / audio->channels;

			// Streaming
			if (streamChunk < 0) {
				audio->chunkNsamples = audio->nsamples;
				audio->chunkSize = audio->size;
			} else {
				audio->chunkNsamples = round(streamChunk * audio->rate);
				audio->chunkSize = audio->chunkNsamples * audio->channels * audio->bytePerSample;
			}

			// Read data
			if (linearSpaceFree() < audio->chunkSize) luaL_error(L, "not enough linear memory available");
			audio->data = linearAlloc(audio->chunkSize);

			fread(audio->data, audio->chunkSize, 1, file);

			audio->file = file;
			audio->filePosition = ftell(file);

			fseek(file, 0, SEEK_END);
			audio->fileSize = ftell(file);

			return 1;

		} else {
			lua_pushnil(L);
			lua_pushfstring(L, "error while opening wav file: %s", strerror(errno));;
			return 2;
		}
	}

	luaL_error(L, "unknown audio type");
	return 0;
}

/***
Load raw audio data from a string.
No streaming.
@function loadRaw
@tparam string data raw audio data
@tparam number rate sampling rate
@tparam string encoding audio encoding, can be `"PCM8"`, `"PCM16"` or `"ADPCM"`
@tparam[opt=1] number channels audio channels count
@treturn[1] audio the loaded audio object
@treturn[2] nil if a error happened
@treturn[2] string error message
*/
static int audio_loadRaw(lua_State *L) {
	size_t dataSize;
	char* data = (char*)luaL_checklstring(L, 1, &dataSize);
	float rate = luaL_checkinteger(L, 2);
	const char* argEncoding = luaL_checkstring(L, 3);
	u32 channels = luaL_optinteger(L, 4, 1);
	
	audio_userdata *audio = lua_newuserdata(L, sizeof(*audio));
	luaL_getmetatable(L, "LAudio");
	lua_setmetatable(L, -2);
	
	audio->type = TYPE_RAW;
	audio->rate = rate;
	audio->channels = channels;
	
	u8 sampleSize = 2; // default to 2
	if (strcmp(argEncoding, "PCM8")) {
		audio->encoding = NDSP_ENCODING_PCM8;
		audio->bytePerSample = 1;
		sampleSize = 1;
	} else if (strcmp(argEncoding, "PCM16")) {
		audio->encoding = NDSP_ENCODING_PCM16;
		audio->bytePerSample = 2;
	} else if (strcmp(argEncoding, "ADPCM")) {
		audio->encoding = NDSP_ENCODING_ADPCM;
	} else {
		lua_pushnil(L);
		lua_pushstring(L, "Wrong format");
		return 2;
	}
	
	audio->nsamples = dataSize/sampleSize;
	audio->size = dataSize;
	audio->data = data;

	audio->chunkSize = audio->size;
	audio->chunkNsamples = audio->nsamples;
	
	audio->speed = 1.0;
	
	return 1;
}

/***
Check if audio is currently playing on a channel.
@function playing
@tparam[opt] integer channel number; if `nil` will search the first channel playing an audio
@treturn boolean `true` if the channel is currently playing the audio, `false` otherwise.
                 If channel is not set, `false` means no audio is playing at all.
*/
static int audio_playing(lua_State *L) {
	if (!isAudioInitialized) {
		lua_pushboolean(L, false);
		return 1;
	}

	int channel = luaL_optinteger(L, 1, -1);
	if (channel < -1 || channel > 23) luaL_error(L, "channel number must be between 0 and 23");

	// Search a channel playing audio
	if (channel == -1) {
		for (int i = 0; i <= 23; i++) {
			if (ndspChnIsPlaying(i)) {
				lua_pushboolean(L, true);
				return 1;
			}
		}

		lua_pushboolean(L, false);
		return 1;

	} else {
		lua_pushboolean(L, ndspChnIsPlaying(channel));
		return 1;
	}

	return 0;
}

/***
Set the mix parameters (volumes) of a channel.
Volumes go from 0 (0%) to 1 (100%).
Note that when a new audio object will play on this channel, theses parameters will be
reset with the new audio object defaults (set in `audio:mix()`).
@function mix
@tparam[opt] integer channel the channel number, if `nil` will change the mix parmaters of all channels
@tparam[opt=1] number frontLeft front left volume
@tparam[opt=frontLeft] number frontRight front right volume
@tparam[opt=frontLeft] number backLeft back left volume
@tparam[opt=frontRight] number backRight back right volume
*/
static int audio_mix(lua_State *L) {
	if (!isAudioInitialized) luaL_error(L, "audio wasn't initialized correctly");

	int channel = luaL_optinteger(L, 1, -1);
	if (channel < -1 || channel > 23) luaL_error(L, "channel number must be between 0 and 23");

	float mix[12];
	mix[0] = luaL_optnumber(L, 2, 1);
	mix[1] = luaL_optnumber(L, 3, mix[0]);
	mix[2] = luaL_optnumber(L, 4, mix[0]);
	mix[3] = luaL_optnumber(L, 5, mix[2]);

	if (channel == -1) {
		for (int i=0; i<=23; i++) ndspChnSetMix(i, mix);
	} else {
		ndspChnSetMix(channel, mix);
	}

	return 0;
}

/***
Set the interpolation type of a channel.
Note that when a new audio object will play on this channel, this parameter will be
reset with the new audio object default (set in `audio:interpolation()`).
@function interpolation
@tparam[opt] integer channel stop playing audio on this channel; if `nil` will change interpolation type on all channels
@tparam[opt=linear] string "none", "linear" or "polyphase"
*/
static int audio_interpolation(lua_State *L) {
	if (!isAudioInitialized) luaL_error(L, "audio wasn't initialized correctly");

	int channel = luaL_optinteger(L, 1, -1);
	if (channel < -1 || channel > 23) luaL_error(L, "channel number must be between 0 and 23");

	const char* interpArg = luaL_optstring(L, 2, "linear");

	ndspInterpType interp;
	if (strcmp(interpArg, "none") == 0)
		interp = NDSP_INTERP_NONE;
	else if (strcmp(interpArg, "linear") == 0)
		interp = NDSP_INTERP_LINEAR;
	else if (strcmp(interpArg, "polyphase") == 0)
		interp = NDSP_INTERP_POLYPHASE;
	else {
		luaL_error(L, "unknown interpolation type");
		return 0;
	}

	if (channel == -1) {
		for (int i=0; i<=23; i++) ndspChnSetInterp(i, interp);
	} else {
		ndspChnSetInterp(channel, interp);
	}

	return 0;
}

/***
Set the speed of the audio playing in a channel.
Speed is expressed as a percentage of the normal playing speed.
1 is 100% speed and 2 is 200%, etc.
Note that when a new audio object will play on this channel, this parameter will be
reset with the new audio object default (set in `audio:speed()`).
@function speed
@tparam[opt] integer channel stop playing audio on this channel; if `nil` will change interpolation type on all channels
@tparam[opt=1] number speed percentage
*/
static int audio_speed(lua_State *L) {
	if (!isAudioInitialized) luaL_error(L, "audio wasn't initialized correctly");

	int channel = luaL_optinteger(L, 1, -1);
	if (channel < -1 || channel > 23) luaL_error(L, "channel number must be between 0 and 23");

	double speed = luaL_optnumber(L, 2, 1);

	if (channel == -1) {
		for (int i=0; i<=23; i++) {
			if (channels[i]) ndspChnSetRate(i, channels[i]->rate * speed);
		}
	} else {
		if (channels[channel]) ndspChnSetRate(channel, channels[channel]->rate * speed);
	}

	return 0;
}

/***
Stop playing all audio on all channels or a specific channel.
@function stop
@tparam[opt] integer channel stop playing audio on this channel; if `nil` will stop audio on all channels
@treturn integer number of channels where audio was stopped
*/
static int audio_stop(lua_State *L) {
	if (!isAudioInitialized) {
		lua_pushinteger(L, 0);
		return 1;
	}

	int channel = luaL_optinteger(L, 1, -1);

	int n = 0;

	if (channel == -1) {
		for (int i = 0; i <= 23; i++) {
			if (ndspChnIsPlaying(i)) {
				stopAudio(i);
				n++;
			}
		}
	} else if (channel < 0 || channel > 23) {
		luaL_error(L, "channel number must be between 0 and 23");
	} else {
		if (ndspChnIsPlaying(channel)) {
			stopAudio(channel);
			n++;
		}
	}
	
	lua_pushinteger(L, n);

	return 1;
}

/***
Update all the currently playing audio streams.
Must be called every frame if you want to use audio with streaming.
@function update
*/
static int audio_update(lua_State *L) {
	if (!isAudioInitialized) luaL_error(L, "audio wasn't initialized correctly");

	for (int i = 0; i <= 23; i++) {
		if (streaming[i] == NULL) continue;
		audio_stream* stream = streaming[i];
		if (stream->done) continue;
		audio_userdata* audio = stream->audio;

		// If the next chunk started to play, load the next one
		if (stream->nextWaveBuf != NULL && ndspChnGetWaveBufSeq(i) == stream->nextWaveBuf->sequence_id) {
			if (stream->prevWaveBuf) stream->prevStartTime = stream->prevStartTime + (double)(audio->chunkNsamples) / audio->rate;

			if (!stream->eof) {
				// Swap buffers
				char* prevData = stream->prevData; // doesn't contain important data, can rewrite
				char* nextData = stream->nextData; // contains the data that started playing
				stream->prevData = nextData; // buffer in use
				stream->nextData = prevData; // now contains an available buffer
				stream->prevWaveBuf = stream->nextWaveBuf;

				// Decoding loop
				u32 chunkNsamples = audio->chunkNsamples; // chunk nsamples and size may be lower than the defaults if reached EOF
				u32 chunkSize = audio->chunkSize;
				if (audio->type == TYPE_OGG) {
					if (ov_seekable(&audio->vf) && ov_raw_tell(&audio->vf) != stream->rawPosition)
						ov_raw_seek(&audio->vf, stream->rawPosition); // goto last read end (audio file may be played multiple times at one)

					int offset = 0;
					while (!stream->eof && offset < audio->chunkSize) {
						long ret = ov_read(&audio->vf, &stream->nextData[offset], fmin(audio->chunkSize - offset, 4096), &stream->currentSection);
						if (ret == 0) {
							stream->eof = 1;
						} else if (ret < 0) {
							luaL_error(L, "error in the ogg vorbis stream");
							return 0;
						} else {
							offset += ret;
						}
					}
					stream->rawPosition = ov_raw_tell(&audio->vf);
					chunkSize = offset;
					chunkNsamples = chunkSize / audio->channels / audio->bytePerSample;

				} else if (audio->type == TYPE_WAV) {
					chunkSize = fmin(audio->fileSize - stream->filePosition, audio->chunkSize);
					chunkNsamples = chunkSize / audio->channels / audio->bytePerSample;

					fseek(audio->file, stream->filePosition, SEEK_SET); // goto last read end (audio file may be played multiple times at one)
					fread(stream->nextData, chunkSize, 1, audio->file);
					stream->filePosition = ftell(audio->file);
					if (stream->filePosition == audio->fileSize) stream->eof = 1;

				} else luaL_error(L, "unknown audio type");

				// Send & play audio data
				ndspWaveBuf* waveBuf = calloc(1, sizeof(ndspWaveBuf));

				waveBuf->data_vaddr = stream->nextData;
				waveBuf->nsamples = chunkNsamples;
				waveBuf->looping = false;

				DSP_FlushDataCache((u32*)stream->nextData, chunkSize);

				ndspChnWaveBufAdd(i, waveBuf);

				stream->nextWaveBuf = waveBuf;
			}
		}

		// Free the last chunk if it's no longer played
		if (stream->prevWaveBuf != NULL && ndspChnGetWaveBufSeq(i) != stream->prevWaveBuf->sequence_id) {
			free(stream->prevWaveBuf);
			stream->prevWaveBuf = NULL;
		}

		// We're done
		if (stream->prevWaveBuf == NULL && stream->nextWaveBuf != NULL && ndspChnGetWaveBufSeq(i) != stream->nextWaveBuf->sequence_id && stream->eof) {
			free(stream->nextWaveBuf);
			stream->nextWaveBuf = NULL;
			linearFree(stream->prevData);
			stream->prevData = NULL;
			linearFree(stream->nextData);
			stream->nextData = NULL;
			stream->done = true;
		}
	}

	return 0;
}

/***
audio object
@section Methods
*/

/***
Returns the audio object duration.
@function :duration
@treturn number duration in seconds
*/
static int audio_object_duration(lua_State *L) {
	audio_userdata *audio = luaL_checkudata(L, 1, "LAudio");

	lua_pushnumber(L, (double)(audio->nsamples) / audio->rate);

	return 1;
}

/***
Returns the current playing position.
@function :time
@tparam[opt] integer channel number; if `nil` will use the first channel found which played this audio
@treturn number time in seconds
*/
static int audio_object_time(lua_State *L) {
	audio_userdata *audio = luaL_checkudata(L, 1, "LAudio");

	int channel = luaL_optinteger(L, 2, -1);
	if (channel < -1 || channel > 23) luaL_error(L, "channel number must be between 0 and 23");

	// Search a channel playing the audio object
	if (channel == -1) {
		for (int i = 0; i <= 23; i++) {
			if (channels[i] == audio) {
				channel = i;
				break;
			}
		}
	}

	if (channel == -1 || channels[channel] != audio || !isAudioInitialized) // audio not playing
		lua_pushnumber(L, 0);
	else {
		double additionnalTime = 0;
		if (streaming[channel] != NULL) additionnalTime = streaming[channel]->prevStartTime;
		lua_pushnumber(L, (double)(ndspChnGetSamplePos(channel)) / audio->rate + additionnalTime);
	}

	return 1;
}

/***
Check if the audio is currently playing.
@function :playing
@tparam[opt] integer channel channel number; if `nil` will search the first channel playing this audio
@treturn boolean true if the channel is currently playing the audio, false otherwise
*/
static int audio_object_playing(lua_State *L) {
	if (!isAudioInitialized) {
		lua_pushboolean(L, false);
		return 1;
	}

	audio_userdata *audio = luaL_checkudata(L, 1, "LAudio");

	int channel = luaL_optinteger(L, 2, -1);
	if (channel < -1 || channel > 23) luaL_error(L, "channel number must be between 0 and 23");

	// Search a channel playing the audio object
	if (channel == -1) {
		for (int i = 0; i <= 23; i++) {
			if (channels[i] == audio && ndspChnIsPlaying(i)) {
				lua_pushboolean(L, true);
				return 1;
			}
		}

		lua_pushboolean(L, false);
		return 1;

	} else {
		lua_pushboolean(L, channels[channel] == audio && ndspChnIsPlaying(channel));
		return 1;
	}

	return 0;
}

/***
Set the mix parameters (volumes) of the audio.
Volumes go from 0 (0%) to 1 (100%).
@function :mix
@tparam[opt=1] number frontLeft front left volume
@tparam[opt=frontLeft] number frontRight front right volume
@tparam[opt=frontLeft] number backLeft back left volume
@tparam[opt=frontRight] number backRight back right volume
*/
static int audio_object_mix(lua_State *L) {
	audio_userdata *audio = luaL_checkudata(L, 1, "LAudio");

	audio->mix[0] = luaL_optnumber(L, 2, 1);
	audio->mix[1] = luaL_optnumber(L, 3, audio->mix[0]);
	audio->mix[2] = luaL_optnumber(L, 4, audio->mix[0]);
	audio->mix[3] = luaL_optnumber(L, 5, audio->mix[2]);

	return 0;
}

/***
Set the interpolation type of the audio.
@function :interpolation
@tparam[opt=linear] string "none", "linear" or "polyphase"
*/
static int audio_object_interpolation(lua_State *L) {
	audio_userdata *audio = luaL_checkudata(L, 1, "LAudio");

	const char* interp = luaL_optstring(L, 2, "linear");

	if (strcmp(interp, "none") == 0)
		audio->interp = NDSP_INTERP_NONE;
	else if (strcmp(interp, "linear") == 0)
		audio->interp = NDSP_INTERP_LINEAR;
	else if (strcmp(interp, "polyphase") == 0)
		audio->interp = NDSP_INTERP_POLYPHASE;
	else {
		luaL_error(L, "unknown interpolation type");
		return 0;
	}

	return 0;
}

/***
Set the speed of the audio.
Speed is expressed as a percentage of the normal playing speed.
1 is 100% speed and 2 is 200%, etc.
@function :speed
@tparam[opt=1] number speed percentage
*/
static int audio_object_speed(lua_State *L) {
	audio_userdata *audio = luaL_checkudata(L, 1, "LAudio");

	audio->speed = luaL_optnumber(L, 2, 1);

	return 0;
}

/***
Plays the audio file.
@function :play
@tparam[opt=false] boolean loop if the audio should loop or not
@tparam[opt] integer channel the channel to play the audio on (0-23); if `nil` will use the first available channel.
                                If the channel was playing another audio, it will be stopped and replaced by this audio.
                                If not set and no channel is available, will return nil plus an error message.
@treturn[1] integer channel number the audio is playing on
@treturn[2] nil an error happened and the audio was not played
@treturn[2] error the error message
*/
static int audio_object_play(lua_State *L) {
	if (!isAudioInitialized) luaL_error(L, "audio wasn't initialized correctly");

	audio_userdata *audio = luaL_checkudata(L, 1, "LAudio");
	bool loop = lua_toboolean(L, 2);
	int channel = luaL_optinteger(L, 3, -1);

	// Find a free channel
	if (channel == -1) {
		for (int i = 0; i <= 23; i++) {
			if (!ndspChnIsPlaying(i)) {
				channel = i;
				break;
			}
		}
	}
	if (channel == -1) {
		lua_pushnil(L);
		lua_pushstring(L, "no audio channel is currently available");
		return 2;
	}
	if (channel < 0 || channel > 23) luaL_error(L, "channel number must be between 0 and 23");

	// Set channel parameters
	stopAudio(channel);
	ndspChnReset(channel);
	ndspChnInitParams(channel);
	ndspChnSetMix(channel, audio->mix);
	ndspChnSetInterp(channel, audio->interp);
	ndspChnSetRate(channel, audio->rate * audio->speed); // maybe hackish way to set a different speed, but it works
	ndspChnSetFormat(channel, NDSP_CHANNELS(audio->channels) | NDSP_ENCODING(audio->encoding));

	// Send & play audio initial data
	ndspWaveBuf* waveBuf = calloc(1, sizeof(ndspWaveBuf));

	waveBuf->data_vaddr = audio->data;
	waveBuf->nsamples = audio->chunkNsamples;
	waveBuf->looping = loop;
	
	DSP_FlushDataCache((u32*)audio->data, audio->chunkSize);
	
	ndspChnWaveBufAdd(channel, waveBuf);
	channels[channel] = audio;

	lua_pushinteger(L, channel);

	// Remove last audio stream
	if (streaming[channel] != NULL) {
		free(streaming[channel]);
		streaming[channel] = NULL;
	}

	// Stream the rest of the audio
	if (audio->chunkSize < audio->size) {
		audio_stream* stream = calloc(1, sizeof(audio_stream));
		stream->audio = audio;
		stream->nextWaveBuf = waveBuf;

		// Allocate buffers
		if (linearSpaceFree() < audio->chunkSize*2) luaL_error(L, "not enough linear memory available");
		stream->nextData = linearAlloc(audio->chunkSize);
		stream->prevData = linearAlloc(audio->chunkSize);

		// Init stream values
		if (audio->type == TYPE_OGG) {
			stream->currentSection = audio->currentSection;
			stream->rawPosition = audio->rawPosition;
		} else if (audio->type == TYPE_WAV) stream->filePosition = audio->filePosition;

		streaming[channel] = stream;
	}

	return 1;
}

/***
Stop playing an audio object.
@function :stop
@tparam[opt] integer channel stop playing the audio on this channel; if `nil` will stop all channels playing this audio.
                                  If the channel is playing another audio object, this function will do nothing.
@treturn integer number of channels where this audio was stopped
*/
static int audio_object_stop(lua_State *L) {
	if (!isAudioInitialized) {
		lua_pushinteger(L, 0);
		return 1;
	}

	audio_userdata *audio = luaL_checkudata(L, 1, "LAudio");
	int channel = luaL_optinteger(L, 2, -1);

	int n = 0;

	if (channel == -1) {
		for (int i = 0; i <= 23; i++) {
			if (channels[i] == audio && ndspChnIsPlaying(i)) {
				stopAudio(i);
				n++;
			}
		}
	} else if (channel < 0 || channel > 23) {
		luaL_error(L, "channel number must be between 0 and 23");
	} else {
		if (channels[channel] == audio && ndspChnIsPlaying(channel)) {
			stopAudio(channel);
			n++;
		}
	}
	
	lua_pushinteger(L, n);

	return 1;
}

/***
Returns the audio object type.
@function :type
@treturn string "ogg", "wav" or "raw"
*/
static int audio_object_type(lua_State *L) {
	audio_userdata *audio = luaL_checkudata(L, 1, "LAudio");
	
	if (audio->type == TYPE_OGG)
		lua_pushstring(L, "ogg");
	else if (audio->type == TYPE_WAV)
		lua_pushstring(L, "wav");
	else if (audio->type == TYPE_RAW)
		lua_pushstring(L, "raw");
	else
		lua_pushstring(L, "unknown");

	return 1;
}

/***
Unload an audio object.
@function :unload
*/
static int audio_object_unload(lua_State *L) {
	audio_userdata *audio = luaL_checkudata(L, 1, "LAudio");

	// Stop playing the audio
	if (isAudioInitialized) {
		for (int i = 0; i <= 23; i++) {
			if (channels[i] == audio) {
				stopAudio(i);
			}
		}
	}

	if (audio->type == TYPE_OGG) ov_clear(&audio->vf);
	else if (audio->type == TYPE_WAV) fclose(audio->file);

	// Free memory
	linearFree(audio->data);
	
	return 0;
}

/***
audio object (ogg-only).
Ogg Vorbis files specific methods.
Using one of theses methods will throw an error if used on an non-ogg audio object.
@section Methods
*/

/***
Returns basic information about the audio in a vorbis bitstream.
@function :info
@treturn infoTable information table
*/
static int audio_object_info(lua_State *L) {
	audio_userdata *audio = luaL_checkudata(L, 1, "LAudio");

	if (audio->type != TYPE_OGG) luaL_error(L, "only avaible on OGG audio objects");

	vorbis_info* vi = ov_info(&audio->vf, -1);
	if (vi == NULL) luaL_error(L, "could not retrieve audio stream informations");

	lua_createtable(L, 0, 6);

	lua_pushinteger(L, vi->version);
	lua_setfield(L, -2, "version");

	lua_pushinteger(L, vi->channels);
	lua_setfield(L, -2, "channels");

	lua_pushinteger(L, vi->rate);
	lua_setfield(L, -2, "rate");

	lua_pushinteger(L, vi->bitrate_upper);
	lua_setfield(L, -2, "bitrateUpper");

	lua_pushinteger(L, vi->bitrate_nominal);
	lua_setfield(L, -2, "bitrateNominal");

	lua_pushinteger(L, vi->bitrate_lower);
	lua_setfield(L, -2, "bitrateLower");

	return 1;
}

/***
Returns the Ogg Vorbis bitstream comment.
@function :comment
@treturn commentTable comment table
*/
static int audio_object_comment(lua_State *L) {
	audio_userdata *audio = luaL_checkudata(L, 1, "LAudio");

	if (audio->type != TYPE_OGG) luaL_error(L, "only avaible on OGG audio objects");

	vorbis_comment *vc = ov_comment(&audio->vf, -1);

	if (vc == NULL) luaL_error(L, "could not retrieve audio stream comment");

	lua_createtable(L, 0, 5);

	lua_newtable(L);
	for (int i=0; i<vc->comments; i++) {
		lua_pushstring(L, vc->user_comments[i]);
		lua_seti(L, -2, i+1);
	}
	lua_setfield(L, -2, "userComments");

	lua_pushstring(L, vc->vendor);
	lua_setfield(L, -2, "vendor");

	return 1;
}

/***
Tables return.
The detailled table structures returned by some methods of audio objects.
@section
*/

/***
Vorbis bitstream information, returned by audio:info().
If bitrateLower == bitrateNominal == bitrateUpper, the stream is fixed bitrate.
@table infoTable
@tfield integer version Vorbis encoder version used to create this bitstream
@tfield integer channels number of channels in bitstream
@tfield integer rate sampling rate of the bitstream
@tfield integer bitrateUpper the upper limit in a VBR bitstream; may be unset if no limit exists
@tfield integer bitrateNominal the average bitrate for a VBR bitstream; may be unset
@tfield integer bitrateLower the lower limit in a VBR bitstream; may be unset if no limit exists
*/

/***
Vorbis bitstream comment, returned by audio:comment().
@table commentTable
@tfield table userComments list of all the user comment
@tfield string vendor information about the Vorbis implementation that encoded the file
*/

// Audio object methods
static const struct luaL_Reg audio_object_methods[] = {
	// common
	{ "duration",      audio_object_duration      },
	{ "time",          audio_object_time          },
	{ "playing",       audio_object_playing       },
	{ "mix",           audio_object_mix           },
	{ "interpolation", audio_object_interpolation },
	{ "speed",         audio_object_speed         },
	{ "play",          audio_object_play          },
	{ "stop",          audio_object_stop          },
	{ "type",          audio_object_type          },
	{ "unload",        audio_object_unload        },
	{ "__gc",          audio_object_unload        },
	// ogg only
	{ "info",          audio_object_info          },
	{ "comment",       audio_object_comment       },
	{ NULL, NULL }
};

// Library functions
static const struct luaL_Reg audio_lib[] = {
	{ "load",          audio_load          },
	{ "loadRaw",       audio_loadRaw       },
	{ "playing",       audio_playing       },
	{ "mix",           audio_mix           },
	{ "interpolation", audio_interpolation },
	{ "speed",         audio_speed         },
	{ "stop",          audio_stop          },
	{ "update",        audio_update        },
	{ NULL, NULL }
};

int luaopen_audio_lib(lua_State *L) {
	luaL_newmetatable(L, "LAudio");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, audio_object_methods, 0);

	luaL_newlib(L, audio_lib);

	return 1;
}

void load_audio_lib(lua_State *L) {
	if (!isAudioInitialized) isAudioInitialized = !ndspInit(); // ndspInit returns 0 in case of success

	luaL_requiref(L, "ctr.audio", luaopen_audio_lib, false);
}

void unload_audio_lib(lua_State *L) {
	if (isAudioInitialized) ndspExit();
}
