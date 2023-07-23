#ifndef COINBASE_TICKER_SUBSCRIBER
#define COINBASE_TICKER_SUBSCRIBER

#include <websocketclient.h>

#include <functional>
#include <mutex>
#include <string>
#include <vector>

#include <websocketpp/config/asio_client.hpp>

namespace tlogger {

using MessagePtr = websocketpp::config::asio_tls_client::message_type::ptr;

// This class implements a subscriber to Coinbase ticker events via Coinbase websocket interface.
// The class allows external clients subsribe on ticker events to receive ticker event messages.
class CoinbaseTickerSubscriber : public WebsocketClient<CoinbaseTickerSubscriber> {

public:
    // TYPES

    // An alias for ticker events handler type
    using MessageHandler = std::function<void(const std::string&)>;

private:
    // DATA

    std::vector<std::string> m_tickers;  // tickers to subscribe

    std::vector<MessageHandler> m_subscribers;  // ticker message subscribers
    std::mutex m_subscribersLock;  // guard for ticker subscribers list

public:
    // CREATORS

    // Create CoinbaseTickerSubscriber object with the specified list
    // of `tickers` to subscribe.
    explicit CoinbaseTickerSubscriber(std::vector<std::string> tickers);

    // Delete this object.
    ~CoinbaseTickerSubscriber();

    // NOT IMPLEMENTED
    CoinbaseTickerSubscriber(const CoinbaseTickerSubscriber&) = delete;
    CoinbaseTickerSubscriber& operator=(const CoinbaseTickerSubscriber&) = delete;

    CoinbaseTickerSubscriber(CoinbaseTickerSubscriber&&) noexcept = delete;
    CoinbaseTickerSubscriber& operator=(CoinbaseTickerSubscriber&&) noexcept = delete;

    // MANIPULATORS

    // Add the specified 'handler' to the ticker events subscription list.
    // The 'Func'type should be convertable to the 'MessageHandler' class.
    template<typename Func>
    void subscribe(Func handler);

    // Handle websocket open event. Send suscribe message to the webscoket to
    // start subscription on the ticker events. The method gets connection handle 
    // as the specified 'hdl' parameter.  
    void handleOpen(websocketpp::connection_hdl hdl);

    // Handle websocket connection failed event. Log the corresponding error
    // to the application log. The method gets connection handle as the specified 'hdl'
    // parameter.
    void handleFail(websocketpp::connection_hdl hdl);

    // Handle websocket connection closed event. Log the corresponding message
    // to the application log. The method gets connection handle as the specified 'hdl'
    // parameter.
    void handleClose(websocketpp::connection_hdl hdl);

    // Handle websocket message received event. Call ticker message subscribers with 
    // the specified 'message' payload. The method also gets connection handle as the 
    // specified 'hdl' parameter.
    void handleMessage(websocketpp::connection_hdl, MessagePtr message);
};

template<typename Func>
void CoinbaseTickerSubscriber::subscribe(Func handler) {
    std::lock_guard<std::mutex> guard{m_subscribersLock};
    m_subscribers.emplace_back(handler);
}

}

#endif  // COINBASE_TICKER_SUBSCRIBER
