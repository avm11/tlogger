#include <coinbasetickersubscriber.h>

#include <iostream>

#include <boost/algorithm/string/join.hpp>
#include <glog/logging.h>

namespace tlogger {

CoinbaseTickerSubscriber::CoinbaseTickerSubscriber(std::vector<std::string> tickers)
: m_tickers(std::move(tickers))
{
    m_client.clear_access_channels(websocketpp::log::alevel::all);
//    m_client.clear_access_channels(websocketpp::log::alevel::frame_payload);
    m_client.clear_error_channels(websocketpp::log::elevel::all);


    m_client.init_asio();
    m_client.start_perpetual();

    m_thread.reset(new websocketpp::lib::thread(&WebsocketppClient::run, &m_client));
}

CoinbaseTickerSubscriber::~CoinbaseTickerSubscriber()
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

int CoinbaseTickerSubscriber::connect(const std::string& uri) {
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

    con->set_open_handler([this](websocketpp::connection_hdl hdl){
        handleOpen(hdl);
    });
    con->set_fail_handler([this](websocketpp::connection_hdl hdl){
        handleFail(hdl);
    });
    con->set_close_handler([this](websocketpp::connection_hdl hdl){
        handleClose(hdl);
    });
    con->set_message_handler([this](websocketpp::connection_hdl hdl, MessagePtr msg){
        handleMessage(hdl, msg);
    });

    m_client.connect(con);

    return 0;
}

ContextPtr CoinbaseTickerSubscriber::handleTLSInit(websocketpp::connection_hdl) {
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
    for (auto& subscriber : m_subscribers) {
        subscriber(payload);
    }
}

}
