#ifndef COINBASE_TICKER_SUBSCRIBER
#define COINBASE_TICKER_SUBSCRIBER

#include <string>
#include <functional>
#include <vector>

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>


namespace tlogger {

using WebsocketppClient = websocketpp::client<websocketpp::config::asio_tls_client>;
using MessagePtr = websocketpp::config::asio_tls_client::message_type::ptr;
using ContextPtr = websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context>;

// This class implements a subscriber to Coinbase ticker events via Coinbase websocket interface.
// The class allows external clients subsribe on ticker events to receive ticker event messages.
class CoinbaseTickerSubscriber {

public:
    // TYPES

    // An alias for ticker events handler type
    using MessageHandler = std::function<void(const std::string&)>;

private:
    // DATA

    std::vector<std::string> m_tickers;  // tickers to subscribe

    WebsocketppClient m_client;  // websocket client
    websocketpp::connection_hdl m_hdl;  // websocket connection handle
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;  // websocket thread

    std::vector<MessageHandler> m_subscribers;  // ticker message subscribers

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
    // This method is not a thread safe, the behaviour is undefined if
    // the `subscribe` is called after the `connect` call.
    template<typename Func>
    void subscribe(Func handler);

    // Connect to a websocet with the specified `uri` and start subscription
    // for the tickers specified in the consturctor on connection open event.
    int connect(const std::string& uri);

private:
    // PRIVATE MANIPULATORS

    // Handle TLS init websocket event. Return SSL context with TSL connection 
    // parameters. The method gets connection handle as the specified 'hdl' 
    // parameter. 
    ContextPtr handleTLSInit(websocketpp::connection_hdl hdl);

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
    m_subscribers.emplace_back(handler);
}

}

#endif  // COINBASE_TICKER_SUBSCRIBER
