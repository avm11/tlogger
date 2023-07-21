#include <atomic>
#include <chrono>
#include <cstdio>
#include <functional>
#include <iostream>
#include <thread>
#include <signal.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/program_options.hpp>
#include <glog/logging.h>

#include <coinbasetickersubscriber.h>
#include <csvtickerprocessor.h>

using namespace tlogger;

namespace po = boost::program_options;

const std::string DEFAULT_TICKER = "BTC-USD";

const std::string COINBASE_MARKETDATA_URI = "wss://ws-feed.exchange.coinbase.com";
const std::string COINBASE_MARKETDATA_SANDBOX_URI = "wss://ws-feed-public.sandbox.exchange.coinbase.com";

const std::string DEFAULT_CSV_FILE_NAME = "data.csv";

static std::atomic<bool> exitFlag{false};

void signalHandler(int s) 
{
    LOG(INFO) << "Terminate signal received: " << s;
    exitFlag.store(true);
}

po::options_description makeCommandLineOptions() 
{
    po::options_description desc{"Program arguments:"};
    desc.add_options()
        ("help", "Show help message")
        ("output-file,o", 
         po::value<std::string>()->default_value(DEFAULT_CSV_FILE_NAME),
         "Output file name")
         ("ticker", 
          po::value<std::vector<std::string>>(), 
          "Ticker to subscribe");
    return desc;
}

po::variables_map parseCommandLine(int argc, char* argv[], const po::options_description& desc) 
{
    po::variables_map cmdVars;

    po::positional_options_description posArgs;
    posArgs.add("ticker", -1);

    po::store(po::command_line_parser(argc, argv).options(desc).positional(posArgs).run(), cmdVars);
    po::notify(cmdVars);
    return cmdVars;
}


int main(int argc, char* argv[]) {
    using namespace std::placeholders;
    using namespace std::chrono_literals;

    FLAGS_logtostderr = true;
    FLAGS_stderrthreshold = 0;
    google::InitGoogleLogging(argv[0]);

    auto desc = makeCommandLineOptions();
    auto cmdVars = parseCommandLine(argc, argv, desc);
    if (cmdVars.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    std::vector<std::string> tickers;
    if (cmdVars.count("ticker")) {
        tickers = cmdVars["ticker"].as<std::vector<std::string>>();
    }
    else {
        tickers.emplace_back(DEFAULT_TICKER);
    }
    LOG(INFO) << "Subscription tickers " << boost::algorithm::join(tickers, ", ");

    std::string csvFileName = cmdVars["output-file"].as<std::string>();
    LOG(INFO) << "Output file " << csvFileName;

    CsvTickerProcessor processor{csvFileName};
    CoinbaseTickerSubscriber subscriber{tickers};

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