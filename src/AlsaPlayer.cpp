#include <iostream>

#include "AlsaPlayer.hpp"

namespace baaplay
{

AlsaPlayer::AlsaPlayer(boost::asio::io_service& ioService, std::string alsaDeviceName)
    : ioService(ioService),
      buffer(10240),
      inputDescriptor(ioService),
      waitRead(false),
      waitWrite(false)
{
    int err = snd_pcm_open(&sndHandle, alsaDeviceName.c_str(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    if (err < 0)
    {
        std::cerr << "Failed to open ALSA device " << alsaDeviceName << std::endl;
        abort();
    }

    SetupAlsa();
}

void AlsaPlayer::Play(std::string filename)
{
    int fd = open(filename.c_str(), O_RDONLY);
    inputDescriptor.assign(fd);

    ReadFromFile();
}

void AlsaPlayer::SetupAlsa()
{
    int err = snd_pcm_set_params(sndHandle, 
                                 SND_PCM_FORMAT_S16_LE,         // Sample Format
                                 SND_PCM_ACCESS_RW_INTERLEAVED, // Access type
                                 2,                             // Channel Count
                                 44100,                         // Sample rate
                                 0,                             // Can't remmember :-)
                                 200000                         // ALSA buffer in microseconds
        );
    if (err < 0)
    {
        std::cerr << "Failed to initialize ALSA: " << snd_strerror(err) << std::endl;
        abort();
    }

    err = snd_pcm_prepare(sndHandle);
    if (err < 0)
    {
        std::cerr << "Failed to prepare ALSA: " << snd_strerror(err) << std::endl;
        abort();
    }

    auto count = snd_pcm_poll_descriptors_count(sndHandle);
    if (count > 0)
    {
        pollfd ufds[count];
        err = snd_pcm_poll_descriptors(sndHandle, ufds, count);
        if (err < 0)
        {
            std::cerr << "Failed to retrieve poll handles: " << snd_strerror(err) << std::endl;
            abort();
        }
        alsaSocket = std::unique_ptr<NativeSocket>(new NativeSocket(ioService, ufds[0].fd));
    }
}

void AlsaPlayer::ReadFromFile()
{
    boost::asio::async_read(inputDescriptor,
                            buffer,
                            [this](const boost::system::error_code& error, std::size_t count)
                            {
                                if (error)
                                    return;

                                AsyncWaitForAlsaReady();
                            });
}

void AlsaPlayer::AsyncWaitForAlsaReady()
{
    if (!waitRead)
    {
        waitRead = true;
        alsaSocket->async_read_event(
            [this](const boost::system::error_code& error, std::size_t)
            {
                if (!error)
                {
                    AlsaReadyForData(waitRead);
                }
            });
    }
    
    if (!waitWrite)
    {
        waitWrite = true;
        alsaSocket->async_write_event(
            [this](const boost::system::error_code& error, std::size_t)
            {
                if (!error)
                {
                    AlsaReadyForData(waitWrite);
                }
            });
    }
}

void AlsaPlayer::AlsaReadyForData(bool& waitVar)
{
    waitVar = false;

    const auto frameSize = 4; // 2 * bytes * 2 channels = 4 bytes per frame

    auto framesRemaining = buffer.size() / frameSize;

    const char* dataPtr = boost::asio::buffer_cast<const char*>(buffer.data());
    int res = snd_pcm_writei(sndHandle, dataPtr, framesRemaining);
    if (res < 0)
    {
        switch(res)
        {
        case -EAGAIN:
            AsyncWaitForAlsaReady();
            break;
        default:
            std::cerr << "Unhandled error: " << res << std::endl;
            abort();
        }
        return;
    }

    buffer.consume(res*frameSize);

    if (buffer.size() > 0)
    {
        AsyncWaitForAlsaReady();
    }
    else
    {
        ReadFromFile();
    }
}

} // namespace baaplay
