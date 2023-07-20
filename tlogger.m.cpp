#include <atomic>
#include <chrono>
#include <cstdio>
#include <functional>
#include <iostream>
#include <thread>
#include <signal.h>

#include <glog/logging.h>

#include <coinbasetickersubscriber.h>
#include <csvtickerprocessor.h>

using namespace tlogger;

const std::string DEFAULT_TICKER = "BTC-USD";

const std::string COINBASE_MARKETDATA_URI = "wss://ws-feed.exchange.coinbase.com";
const std::string COINBASE_MARKETDATA_SANDBOX_URI = "wss://ws-feed-public.sandbox.exchange.coinbase.com";

const std::string DEFAULT_CSV_FILE_NAME = "data.csv";

static std::atomic<bool> exitFlag{false};

void signalHandler(int s) {
    LOG(INFO) << "Terminate signal received: " << s;
    exitFlag.store(true);
}


int main(int argc, char* argv[]) {
    using namespace std::placeholders;
    using namespace std::chrono_literals;

    FLAGS_logtostderr = true;
    FLAGS_stderrthreshold = 0;
    google::InitGoogleLogging(argv[0]);

    std::string ticker = DEFAULT_TICKER;
    LOG(INFO) << "Started with ticker " << ticker;

    CsvTickerProcessor processor{DEFAULT_CSV_FILE_NAME};
    CoinbaseTickerSubscriber subscriber{ticker};

    subscriber.subscribe(
        std::bind(&CsvTickerProcessor::handleTickerMessage, &processor, _1));

    int rc = processor.start();
    if (rc) {
        LOG(ERROR) << "Failed to start ticker processor";
        return -1;
    }

    const auto uri = COINBASE_MARKETDATA_URI;
    rc = subscriber.connect(uri);
    if (rc) {
        LOG(ERROR) << "Failed to connect to " << uri;
        return -1;
    }

    LOG(INFO) << "Press Ctrl+C to stop";
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = signalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    while (!exitFlag.load()) {
        std::this_thread::sleep_for(1s);
    }

    processor.stop();

    LOG(INFO) << "Done";

    return 0;
}