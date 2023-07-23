#ifndef WEBSOCKET_CLIENT
#define WEBSOCKET_CLIENT

#include <functional>

#include <glog/logging.h>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

namespace tlogger {

using WebsocketppClient = websocketpp::client<websocketpp::config::asio_tls_client>;
using MessagePtr = websocketpp::config::asio_tls_client::message_type::ptr;
using ContextPtr = websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context>;

// This class implements a websocket client. It allows to connect to a websocket and
// calls event handlers of a derived Handler class on connection open, connection failed,
// connection closed and message received event.
// The derived Handler class should implement the following methods:
// 
// void handleOpen(websocketpp::connection_hdl hdl);
// void handleFail(websocketpp::connection_hdl hdl);
// void handleClose(websocketpp::connection_hdl hdl);
// void handleMessage(websocketpp::connection_hdl, MessagePtr message);
//
template<class Handler>
class WebsocketClient
{
private:
    // DATA

    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;  // websocket thread

public:
    // CREATORS

    // Create WebsocketClient object.
    WebsocketClient();

    // Delete this object.
    ~WebsocketClient();

    // NOT IMPLEMENTED

    WebsocketClient(const WebsocketClient&) = delete;
    WebsocketClient& operator=(const WebsocketClient&) = delete;

    WebsocketClient(WebsocketClient&&) noexcept = delete;
    WebsocketClient& operator=(WebsocketClient&&) noexcept = delete;

    // MANIPULATORS

    // Connect to a websocet with the specified `uri`.
    int connect(const std::string& uri);

protected:
    // DATA

    WebsocketppClient m_client;  // websocket client
    websocketpp::connection_hdl m_hdl;  // websocket connection handle

    // MANIPULATORS

    // Handle TLS init websocket event. Return SSL context with TSL connection 
    // parameters. The method gets connection handle as the specified 'hdl' 
    // parameter. 
    virtual ContextPtr handleTLSInit(websocketpp::connection_hdl hdl);
};

template<class Handler>
WebsocketClient<Handler>::WebsocketClient()
{
    m_client.clear_access_channels(websocketpp::log::alevel::all);
//    m_client.clear_access_channels(websocketpp::log::alevel::frame_payload);
    m_client.clear_error_channels(websocketpp::log::elevel::all);

    m_client.init_asio();
    m_client.start_perpetual();

    m_thread.reset(new websocketpp::lib::thread(&WebsocketppClient::run, &m_client));
}

template<class Handler>
WebsocketClient<Handler>::~WebsocketClient()
{
    m_client.stop_perpetual();

    if (m_hdl.use_count()) {
        websocketpp::lib::error_code ec;
        m_client.close(m_hdl, websocketpp::close::status::going_away, "", ec);
        if (ec) {
            LOG(ERROR) << "Error closing connection " << ec.message();
        }
    }
        
    m_thread->join();
}

template<class Handler>
int WebsocketClient<Handler>::connect(const std::string& uri)
{
    using namespace std::placeholders;

    websocketpp::lib::error_code ec;

    LOG(INFO) << "Create connection to " << uri;

    m_client.set_tls_init_handler([this](websocketpp::connection_hdl hdl) -> ContextPtr {
        return handleTLSInit(hdl);
    });

    WebsocketppClient::connection_ptr con = m_client.get_connection(uri, ec);

    if (ec) {
        LOG(ERROR) << "Connect initialization error: " << ec.message();
        return -1;
    }

    m_hdl = con->get_handle();

    con->set_open_handler(std::bind(
        &Handler::handleOpen,
        static_cast<Handler*>(this),
        _1
    ));
    con->set_fail_handler(std::bind(
        &Handler::handleFail,
        static_cast<Handler*>(this),
        _1
    ));
    con->set_close_handler(std::bind(
        &Handler::handleClose,
        static_cast<Handler*>(this),
        _1
    ));
    con->set_message_handler(std::bind(
        &Handler::handleMessage,
        static_cast<Handler*>(this),
        _1,
        _2
    ));

    m_client.connect(con);

    return 0;
}

template<class Handler>
ContextPtr WebsocketClient<Handler>::handleTLSInit(websocketpp::connection_hdl hdl)
{
    LOG(INFO) << "TLS Initialization";

    auto ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);

    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);


        ctx->set_verify_mode(boost::asio::ssl::verify_none);
    } catch (std::exception& e) {
        LOG(ERROR) << "TLS Initialization failed: " << e.what();
    }
    return ctx;
}

}

#endif  // WEBSOCKET_CLIENT
