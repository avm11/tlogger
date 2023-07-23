#include <coinbasetickersubscriber.h>

#include <boost/algorithm/string/join.hpp>
#include <glog/logging.h>

namespace tlogger {

CoinbaseTickerSubscriber::CoinbaseTickerSubscriber(std::vector<std::string> tickers)
: WebsocketClient<CoinbaseTickerSubscriber>()
, m_tickers(std::move(tickers))
{
}

CoinbaseTickerSubscriber::~CoinbaseTickerSubscriber()
{
}

void CoinbaseTickerSubscriber::handleOpen(websocketpp::connection_hdl hdl) {
    WebsocketppClient::connection_ptr con = m_client.get_con_from_hdl(hdl);
    auto server = con->get_response_header("Server");

    LOG(INFO) << "Connection opened to " << server;

    std::string tickersStr = "\"" + boost::algorithm::join(m_tickers, "\",\"") + "\"";
    std::string request = R"json({
    "type": "subscribe",
    "product_ids": [ )json" + tickersStr + R"json( ],
    "channels": ["ticker"]
    })json";
    LOG(INFO) << "Subscribing on " << tickersStr;

    websocketpp::lib::error_code ec;
    m_client.send(m_hdl, request, websocketpp::frame::opcode::text, ec);
    if (ec) {
        LOG(ERROR) << "Subscribing error: " << ec.message();
    }
}

void CoinbaseTickerSubscriber::handleFail(websocketpp::connection_hdl hdl) {
    WebsocketppClient::connection_ptr con = m_client.get_con_from_hdl(hdl);
    auto server = con->get_response_header("Server");
    auto error_reason = con->get_ec().message();

    LOG(ERROR) << "Connection failed to " << server << " : " << error_reason;
}

void CoinbaseTickerSubscriber::handleClose(websocketpp::connection_hdl hdl) {
     WebsocketppClient::connection_ptr con = m_client.get_con_from_hdl(hdl);
     LOG(INFO) << "Connection closed: close code: " << con->get_remote_close_code() << " (" 
          << websocketpp::close::status::get_string(con->get_remote_close_code()) 
          << "), close reason: " << con->get_remote_close_reason();
}

void CoinbaseTickerSubscriber::handleMessage(websocketpp::connection_hdl, MessagePtr message) {
    const auto& payload = message->get_payload();

    std::lock_guard<std::mutex> guard{m_subscribersLock};
    for (auto& subscriber : m_subscribers) {
        subscriber(payload);
    }
}

}
