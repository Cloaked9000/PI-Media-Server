//
// Created by fred.nicolson on 05/05/17.
//

#ifndef MEDIASERVER_MUSICPLAYER_H
#define MEDIASERVER_MUSICPLAYER_H
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "Playlist.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/samplefmt.h>
#include <ao/ao.h>
}

class MusicPlayer
{
public:
    enum State
    {
        Stopped = 0,
        Playing = 1,
        Paused = 2
    };

    MusicPlayer();
    virtual ~MusicPlayer();

    /*!
     * Converts a 'State' enum value to a printable string.
     *
     * @param state The state to convert
     * @return The printable string of it
     */
    const std::string state_to_string(State state);

    /*!
     * Loads a soundtrack from a given filepath,
     * should be followed up with a call to 'play'.
     *
     * @param filepath A valid filepath to some media that can be loaded
     * @param wait_for_thread Should play thread be stopped first?
     * @return True on success, false on failure.
     */
    bool load(const std::string &filepath, bool wait_for_thread = true);

    /*!
     * Unloads the given track
     *
     * @param wait_for_thread Should the play thread be stopped first?
     */
    void unload(bool wait_for_thread = true);

    /*!
     * Starts/resumes playing the track
     */
    void play();

    /*!
     * Pauses the track
     */
    void pause();

    /*!
     * Gets the current playing offset from the track
     *
     * @return The current playing offset as a time point
     */
    std::chrono::seconds get_offset();

    /*!
     * Sets the current playing offset of the track
     *
     * @param offset Where to continue playing from
     */
    void set_offset(std::chrono::seconds offset);

    /*!
     * Gets the track's duration.
     *
     * @return The tracks' duration.
     */
    std::chrono::seconds get_duration();

    /*!
     * Gets the playback state. Paused/playing/stopped etc.
     *
     * @return Current playback state
     */
    State get_state();

    /*!
     * Gets the filepath of the track being played
     *
     * @return The track filepath
     */
    const std::string &get_filepath();

    /*!
     * Gets the binary data behind the album cover
     *
     * @return A string containing the binary image
     */
    const std::string &get_album_cover();

    void wait()
    {
        lock.lock();
        lock.unlock();
    }

    Playlist playlist;

private:

    /*!
     * Music is played in another thread, so that calls to
     * play don't block, and the caller can keep going. This
     * occurs in this function.
     */
    void play_thread();

    /*!
     * Updates the internal play state of the player,
     * and notifies the playback thread (to let it pause,
     * resume etc if needed).
     *
     * @param play_state The state to set the play_state to
     */
    void set_state(State play_state);

    //State
    std::unique_ptr<std::thread> thread;
    std::atomic<State> play_state;
    std::string track_filepath;
    std::atomic<bool> loaded;
    std::atomic<uint32_t> play_offset;
    std::string cover_image;
    std::mutex lock;

    std::condition_variable notifier;
    std::mutex notifier_lock;

    bool is_planar = false;
    int plane_size = 0;
    AVFormatContext *av_container;
    AVCodecContext *av_codec_context;
    AVCodec *av_codec;
    int stream_id;

    int ao_driver;
    ao_device *ao_output;
    ao_sample_format ao_format;
};


#endif //MEDIASERVER_MUSICPLAYER_H
