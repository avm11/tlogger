#ifndef COINBASE_TICKER_SUBSCRIBER
#define COINBASE_TICKER_SUBSCRIBER

#include <string>
#include <functional>

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>


namespace tlogger {

using WebsocketppClient = websocketpp::client<websocketpp::config::asio_tls_client>;
using MessagePtr = websocketpp::config::asio_tls_client::message_type::ptr;
using ContextPtr = websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context>;

class CoinbaseTickerSubscriber {

public:
    // TYPES
    using MessageHandler = std::function<void(const std::string&)>;

private:
    // DATA
    std::string m_ticker;  // ticker to subscribe

    WebsocketppClient m_client;
    websocketpp::connection_hdl m_hdl;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;

    std::vector<MessageHandler> m_subscribers;  // ticker message subscribers

public:
    // CREATORS
    // Create CoinbaseTickerSubscriber object.
    explicit CoinbaseTickerSubscriber(std::string ticker);

    // Delete this object.
    ~CoinbaseTickerSubscriber();

    // NOT IMPLEMENTED
    CoinbaseTickerSubscriber(const CoinbaseTickerSubscriber&) = delete;
    CoinbaseTickerSubscriber& operator=(const CoinbaseTickerSubscriber&) = delete;

    CoinbaseTickerSubscriber(CoinbaseTickerSubscriber&&) noexcept = delete;
    CoinbaseTickerSubscriber& operator=(CoinbaseTickerSubscriber&&) noexcept = delete;

    // MANIPULATORS
    template<typename Func>
    void subscribe(Func handler);

    int connect(const std::string& uri);

private:
    // PRIVATE MANIPULATORS
    ContextPtr handleTLSInit(websocketpp::connection_hdl);

    void handleOpen(websocketpp::connection_hdl hdl);

    void handleFail(websocketpp::connection_hdl hdl);

    void handleClose(websocketpp::connection_hdl hdl);

    void handleMessage(websocketpp::connection_hdl, MessagePtr msg);
};

template<typename Func>
void CoinbaseTickerSubscriber::subscribe(Func handler) {
    m_subscribers.emplace_back(handler);
}

}

#endif  // COINBASE_TICKER_SUBSCRIBER
