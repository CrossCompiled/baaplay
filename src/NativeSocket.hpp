#pragma once

#include <boost/asio.hpp>
#include <boost/function.hpp>

namespace baaplay
{

template <typename Service>
class non_closing_service : public Service
{
public:
    non_closing_service(boost::asio::io_service& io)
        : Service(io)
    {}
    void destroy(typename Service::implementation_type&)
    {
        // Do not close the file descriptor as we have no ownership of it
    }
};

//! @brief Boost.Asio wrapper for a native socket
class NativeSocket
{
    typedef non_closing_service<boost::asio::posix::stream_descriptor_service> service_type;
    typedef boost::asio::posix::basic_stream_descriptor<service_type> socket_type;

public:
    typedef socket_type::native_handle_type native_handle_type;

    NativeSocket(boost::asio::io_service& io, native_handle_type handle)
        : socket(io, handle)
    {
    }

    ~NativeSocket()
    {
        // Cancel all asynchronous requests.
        boost::system::error_code dummy; // Ignore errors
        socket.cancel(dummy);
        socket.release();
    }

    template <typename Handler>
    void async_read_event(BOOST_ASIO_MOVE_ARG(Handler) handler)
    {
        socket.async_read_some(boost::asio::null_buffers(), handler);
    }
    template <typename Handler>
    void async_write_event(BOOST_ASIO_MOVE_ARG(Handler) handler)
    {
        socket.async_write_some(boost::asio::null_buffers(), handler);
    }
    native_handle_type native_handle();

private:
    socket_type socket;
};

} // namespace baaplay
