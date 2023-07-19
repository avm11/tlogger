#include <csvtickerprocessor.h>

#include <chrono>
#include <iostream>

#include <glog/logging.h>

using namespace std::chrono_literals;

namespace tlogger {

constexpr size_t MESSAGE_QUEUE_SIZE = 256;

CsvTickerProcessor::CsvTickerProcessor(const std::string& csvFileName)
:m_csvFileStream{csvFileName, std::ios_base::app}
,m_stopThread{false}
{
}

CsvTickerProcessor::~CsvTickerProcessor()
{
    stop();
}

int CsvTickerProcessor::start() 
{
    if (!m_csvFileStream) {
        LOG(ERROR) << "Failed to open csv file";
        return 1;
    }

    m_processingThread = std::thread{&CsvTickerProcessor::run, this};

    return 0;
}

void CsvTickerProcessor::stop()
{
    if (m_processingThread.joinable()) {
        m_stopThread.store(true, std::memory_order_relaxed);
        m_processingThread.join();
    }
}

void CsvTickerProcessor::handleTickerMessage(const std::string& payload)
{
    {
        std::lock_guard<std::mutex> lock{m_msgQueueMutex};
        m_messageQueue.push(payload);
    }
    m_msgQueueCond.notify_one();    
}

void CsvTickerProcessor::run()
{
    while (!m_stopThread.load(std::memory_order_relaxed)) {
        std::string payload;

        {
            std::unique_lock<std::mutex> lock{m_msgQueueMutex};
            if (!m_msgQueueCond.wait_for(
                    lock, 1s, [this](){return !m_messageQueue.empty();})) {
                continue;
            }
            
            payload = m_messageQueue.front();
            m_messageQueue.pop();
        }

        std::cout << payload << "\n";
    }
}


}
