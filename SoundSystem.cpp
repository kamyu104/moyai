#include "client.h"


#include "SoundSystem.h"
#include "Sound.h"
#include "Remote.h"
#ifdef USE_OPENAL
#include "ALSound.h"
#endif

SoundSystem::SoundSystem()  : id_gen(1), remote_head(0), sys(0), soundPlayCallback(0), soundStopCallback(0) {
#ifdef USE_FMOD    
	FMOD_RESULT r;
	r = FMOD_System_Create(&sys);
	FMOD_ERRCHECK(r);

	unsigned int version;
	r = FMOD_System_GetVersion(sys, &version);
	FMOD_ERRCHECK(r);
	if(version < FMOD_VERSION ){
		print("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
		return;
	}
	r = FMOD_System_Init( sys, 32, FMOD_INIT_NORMAL, NULL );
	FMOD_ERRCHECK(r);
#endif
#ifdef USE_UNTZ
	UNTZ::System::initialize( 44100, 512, 0 );
#endif
#ifdef USE_OPENAL
    if(alutInit(0,NULL)==AL_FALSE) {
        print("alutInit failed! error:%s", alutGetErrorString(alutGetError()));
        assert(false);
    } else {
        print("alutInit success!");
    }
#endif    
    for(int i=0;i<elementof(sounds);i++) sounds[i] = NULL;
}

Sound *SoundSystem::newSound( const char *path, float vol ) {
#if !defined(__linux__)    
    const char *cpath = platformCStringPath(path);
#endif
    
#ifdef USE_FMOD    
    FMOD_RESULT r;    
	FMOD_SOUND *s;
	r = FMOD_System_CreateSound(sys, cpath, FMOD_SOFTWARE, 0, &s );
    if( r != FMOD_OK ) {
        print("newSound: can't create sound:'%s'", cpath );
    }
	FMOD_ERRCHECK(r);
	FMOD_Sound_SetMode( s, FMOD_LOOP_OFF );
#endif
#ifdef USE_UNTZ
    UNTZ::Sound *s = UNTZ::Sound::create( cpath, true );
#endif
#ifdef USE_OPENAL
    ALSound *s = ALSound::create( cpath );
#endif
#ifdef __linux__
    void *s = this; // TODO: implement virtual sound
#endif    
    if(!s) {
        print("newSound failed for '%s'", path );
        return 0;
    }
    Sound *out = new Sound(this);
	out->sound = s;    
	out->default_volume = vol;
    out->id = id_gen;
    strncpy( out->last_load_file_path, path, sizeof(out->last_load_file_path) );
    id_gen++;
    append(out);
	return out;
}

Sound *SoundSystem::newSoundFromMemory( float *samples, int samples_num ) {
    Sound *out = new Sound(this);
#ifdef USE_FMOD    
    FMOD_RESULT r;
    FMOD_SOUND *s;
    FMOD_CREATESOUNDEXINFO exinfo;

    memset( &exinfo, 0, sizeof(exinfo) );
    exinfo.cbsize = sizeof(exinfo);
    exinfo.length = sizeof(float) * samples_num;
    exinfo.defaultfrequency = 44100;
    exinfo.numchannels = 1;
    exinfo.format = FMOD_SOUND_FORMAT_PCMFLOAT;
    
    r = FMOD_System_CreateSound( sys, (const char*) samples, FMOD_SOFTWARE | FMOD_OPENMEMORY | FMOD_OPENRAW, &exinfo, &s );
    FMOD_ERRCHECK(r);
    FMOD_Sound_SetMode( s, FMOD_LOOP_OFF );
#endif
#ifdef USE_UNTZ
    UNTZ::SoundInfo info;
    memset(&info,0,sizeof(info));
    info.mBitsPerSample = 32;
    info.mSampleRate = 44100;
    info.mChannels = 1;
    info.mTotalFrames = samples_num / 1; // 1 for num of channels
	info.mLength = (double)samples_num / 1.0f / 44100.0f; // 1 for num of channels
    UNTZ::Sound *s = UNTZ::Sound::create( info, samples, true ); // ownsdata: copy samples to mem
#endif
#ifdef USE_OPENAL
    ALSoundInfo info;
    info.mSampleRate = 44100;
    info.mChannels = 1;
    info.mTotalFrames = samples_num / 1;
    ALSound *s = ALSound::create( info, samples );
#endif
#ifdef __linux__
    void *s = this; // TODO: implement virtual sound
#endif        
    out->sound = s;
    out->default_volume = 1;
    out->id = id_gen;
    out->updateLastSamples(samples, samples_num);
    id_gen++;
    append(out);
    return out;
}
// don't create actual sound object, just store waves in memory. Used for repreproxy.
Sound *SoundSystem::newSoundFromMemoryVirtual(float *samples, int samples_num ) {
    Sound *out = new Sound(this);
    out->sound = NULL;
    out->default_volume = 1;
    out->id = id_gen++;
    out->updateLastSamples(samples,samples_num);
    append(out);
    return out;
}

void SoundSystem::append( Sound *s ) {
    for(int i=0;i<elementof(sounds);i++) {
        if( sounds[i] == NULL ) {
            sounds[i] = s;
            return;
        }
    }
    assertmsg(false, "sound full");
}

Sound *SoundSystem::getById( int id ) {
    for(int i=0;i<elementof(sounds);i++) {
        if( sounds[i] && sounds[i]->id == id ) {
            return sounds[i];
        }
    }
    return NULL;
}

#ifdef USE_UNTZ
// TODO: Currently UNTZ output callback doesn't support user data pointer
RemoteHead *g_audiocallback_rh = NULL;
// [numsamples of float values for ch1][numsamples of float values for ch2]
void untz_output_callback( UInt32 numChannels, float *interleavedSamples, UInt32 numSamples ) {
    //    print("audioout: %d %d %f", numChannels, numSamples, interleavedSamples[0] );
    assert(g_audiocallback_rh);
    // numChannels is always 2, so dont send it.
    g_audiocallback_rh->appendAudioSamples(numChannels, interleavedSamples, numSamples );
} 

#endif


void SoundSystem::setRemoteHead(RemoteHead*rh) {
    remote_head = rh;
#ifdef USE_UNTZ    
    g_audiocallback_rh = rh;
    UNTZ::System::setOutputCallback(untz_output_callback);
#else
    print("Warning: setRemoteHead works only with UNTZ");
#endif    
};   


void SoundSystem::setVolume(float v ) {
#if defined(USE_UNTZ)
    UNTZ::System::get()->setVolume(v);    
#else
    assertmsg(false, "not implemented");
#endif    
}
