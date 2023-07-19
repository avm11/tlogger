#include <iostream>
#include <cstdio>

#include <glog/logging.h>

#include <coinbasetickersubscriber.h>

using namespace tlogger;

const std::string DEFAULT_TICKER = "BTC-USD";

const std::string COINBASE_MARKETDATA_URI = "wss://ws-feed.exchange.coinbase.com";
const std::string COINBASE_MARKETDATA_SANDBOX_URI = "wss://ws-feed-public.sandbox.exchange.coinbase.com";


int main(int argc, char* argv[]) {
    FLAGS_logtostderr = true;
    FLAGS_stderrthreshold = 0;
    google::InitGoogleLogging(argv[0]);

    std::string ticker = DEFAULT_TICKER;
    LOG(INFO) << "Started with ticker " << ticker;

    CoinbaseTickerSubscriber subscriber{ticker};

    const auto uri = COINBASE_MARKETDATA_SANDBOX_URI;
    int rc = subscriber.connect(uri);
    if (rc) {
        LOG(ERROR) << "Failed to connect to " << uri;
        return -1;
    }

    std::getchar();

    LOG(INFO) << "Done";

    return 0;
}