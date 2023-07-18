#include <string>

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>


namespace tlogger {

using WebsocketppClient = websocketpp::client<websocketpp::config::asio_tls_client>;
using MessagePtr = websocketpp::config::asio_tls_client::message_type::ptr;
using ContextPtr = websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context>;

class CoinbaseTickerSubscriber {

private:
    // TYPES

    // DATA
    std::string m_ticker;  // ticker to subscribe

    WebsocketppClient m_client;
    websocketpp::connection_hdl m_hdl;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;

public:
    // CREATORS
    // Create Feed object.
    explicit CoinbaseTickerSubscriber(std::string ticker);

    // Delete this object.
    ~CoinbaseTickerSubscriber();

    // NOT IMPLEMENTED
    CoinbaseTickerSubscriber(const CoinbaseTickerSubscriber&) = delete;
    CoinbaseTickerSubscriber& operator=(const CoinbaseTickerSubscriber&) = delete;

    CoinbaseTickerSubscriber(CoinbaseTickerSubscriber&&) noexcept = delete;
    CoinbaseTickerSubscriber& operator=(CoinbaseTickerSubscriber&&) noexcept = delete;

    // // MANIPULATORS
    int connect(const std::string& uri);

private:
    ContextPtr handleTLSInit(websocketpp::connection_hdl);

    void handleOpen(websocketpp::connection_hdl hdl);

    void handleFail(websocketpp::connection_hdl hdl);

    void handleClose(websocketpp::connection_hdl hdl);

    void handleMessage(websocketpp::connection_hdl, MessagePtr msg);
};


}
