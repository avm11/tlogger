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

// The class implements parsing Coinbase ticker events and writing the events to a csv
// file. The class runs a thread for processing events.
class CsvTickerProcessor {

private:
    // DATA

    std::ofstream m_csvFileStream;  // csv file stream
    std::string   m_csvRowBuffer;  // buffer for a csv row

    std::thread m_processingThread;  // ticker events processing thread

    std::queue<std::string> m_messageQueue;  // ticker events message queue
    std::mutex m_msgQueueMutex;  // mutex to protect the message queue
    std::condition_variable m_msgQueueCond;  // conditional variable that the queue is not empty

    std::atomic<bool> m_stopThread;  // flag to stop processing thread

public:
    // CREATORS

    // Create CsvTickerProcessor object to write ticker events to
    // a csv file with the specified 'csvFileName'.
    explicit CsvTickerProcessor(const std::string& csvFileName);

    // Delete this object.
    ~CsvTickerProcessor();

    // NOT IMPLEMENTED
    CsvTickerProcessor(const CsvTickerProcessor&) = delete;
    CsvTickerProcessor& operator=(const CsvTickerProcessor&) = delete;

    CsvTickerProcessor(CsvTickerProcessor&&) noexcept = delete;
    CsvTickerProcessor& operator=(CsvTickerProcessor&&) noexcept = delete;

    // MANIPULATORS

    // Start the ticker events processing thread.
    int start();

    // Stop the ticker events processing thread.
    void stop();

    // Push the specified message 'payload' to the processing queue.
    // The message will be processed by the processing thread.
    void handleTickerMessage(const std::string& payload);

private:
    // PRIVATE MANIPULATORS

    // Processing thread main loop. Gets messages from the processing
    // queue and calls `processTickerMessage` for each message.
    void run();

    // Convert the specified message 'payload' from JSON format to
    // a csv row and write the row to the csv file.
    void processTickerMessage(const std::string& payload);
};

}

#endif // CSV_TICKER_PROCESSOR
