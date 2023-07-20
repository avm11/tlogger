#ifndef CSV_TICKER_PROCESSOR
#define CSV_TICKER_PROCESSOR

#include <atomic>
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>


namespace tlogger {

class CsvTickerProcessor {

private:
    // DATA
    std::ofstream m_csvFileStream;

    std::thread m_processingThread;

    std::mutex m_msgQueueMutex;
    std::condition_variable m_msgQueueCond;
    std::queue<std::string> m_messageQueue;

    std::atomic<bool> m_stopThread;

public:
    // CREATORS
    // Create CsvTickerProcessor object.
    explicit CsvTickerProcessor(const std::string& csvFileName);

    // Delete this object.
    ~CsvTickerProcessor();

    // NOT IMPLEMENTED
    CsvTickerProcessor(const CsvTickerProcessor&) = delete;
    CsvTickerProcessor& operator=(const CsvTickerProcessor&) = delete;

    CsvTickerProcessor(CsvTickerProcessor&&) noexcept = delete;
    CsvTickerProcessor& operator=(CsvTickerProcessor&&) noexcept = delete;

    // MANIPULATORS
    int start();

    void stop();

    void handleTickerMessage(const std::string& payload);

private:
    // PRIVATE MANIPULATORS
    void run();

    void processTickerMessage(const std::string& payload);
};

}

#endif // CSV_TICKER_PROCESSOR
