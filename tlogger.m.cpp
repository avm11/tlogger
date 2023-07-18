#include <iostream>
#include <cstdio>

#include <coinbasetickersubscriber.h>

using namespace tlogger;

const std::string DEFAULT_TICKER = "BTC-USD";

const std::string COINBASE_MARKETDATA_URI = "wss://ws-feed.exchange.coinbase.com";
const std::string COINBASE_MARKETDATA_SANDBOX_URI = "wss://ws-feed-public.sandbox.exchange.coinbase.com";

int main(void) {
    CoinbaseTickerSubscriber subscriber{DEFAULT_TICKER};

    std::cout << "Started\n";
    const auto uri = COINBASE_MARKETDATA_SANDBOX_URI;
    int rc = subscriber.connect(uri);
    if (rc) {
        std::cout << "Failed to connect to " << uri << "\n";
        return -1;
    }

    std::getchar();

    std::cout << "Done\n";

    return 0;
}