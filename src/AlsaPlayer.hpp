#pragma once

#include <alsa/asoundlib.h>

#include <boost/asio/io_service.hpp>

#include "NativeSocket.hpp"

namespace baaplay
{

class AlsaPlayer
{
public:
    AlsaPlayer(boost::asio::io_service& ioService, std::string alsaDeviceName);

    void Play(std::string filename);

private:
    void SetupAlsa();
    void ReadFromFile();
    void AsyncWaitForAlsaReady();
    void AlsaReadyForData(bool& waitVar);

private:
    boost::asio::io_service& ioService;
    boost::asio::streambuf buffer;
    snd_pcm_t* sndHandle;

    std::unique_ptr<NativeSocket> alsaSocket;
    boost::asio::posix::stream_descriptor inputDescriptor;

    bool waitRead;
    bool waitWrite;
    std::size_t bufferWritten;
};

} // namespace baaplay
