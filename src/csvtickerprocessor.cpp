#include <csvtickerprocessor.h>

#include <chrono>
#include <iostream>

#include <glog/logging.h>

using namespace std::chrono_literals;

namespace tlogger {

constexpr size_t MESSAGE_QUEUE_SIZE = 256;

enum TickerParserState {
    TPS_WaitingForJsonStart,
    TPS_WaitingForFieldSeparator,
    TPS_ProcessingFieldValue,
    TPS_ProcessingDone,
    TPS_ProcessingError,
};

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
    std::string payload;

    while (!m_stopThread.load(std::memory_order_relaxed)) {
        {
            std::unique_lock<std::mutex> lock{m_msgQueueMutex};
            if (!m_msgQueueCond.wait_for(
                    lock, 1s, [this](){return !m_messageQueue.empty();})) {
                continue;
            }
            
            payload = m_messageQueue.front();
            m_messageQueue.pop();
        }

        processTickerMessage(payload);
    }
}

void CsvTickerProcessor::processTickerMessage(const std::string& payload)
{
    if (payload.find(R"("type":"ticker")") == std::string::npos) {
        return;
    }

    std::string csv_row;
    csv_row.reserve(payload.size());

    TickerParserState state = TPS_WaitingForJsonStart;
    for (char c: payload) {
        switch (state)
        {
            case TPS_WaitingForJsonStart: {
                if (c == '{') {
                    state = TPS_WaitingForFieldSeparator;
                }
                else {
                    state = TPS_ProcessingError;
                }
            } break;
            case TPS_WaitingForFieldSeparator: {
                if (c == '}') {
                    state = TPS_ProcessingDone;
                } else if (c == ':') {
                    state = TPS_ProcessingFieldValue;
                }
            } break;
            case TPS_ProcessingFieldValue: {
                if (c == '}') {
                    state = TPS_ProcessingDone;
                } else if (c == ',') {
                    csv_row.push_back(c);
                    state = TPS_WaitingForFieldSeparator;
                } else {
                    csv_row.push_back(c);
                }
            } break;
            case TPS_ProcessingDone: {
                state = TPS_ProcessingError;
            } break;
            case TPS_ProcessingError: {
            } break;
        }

        if (state == TPS_ProcessingError) {
            break;
        }
    }

    if (state != TPS_ProcessingDone) {
        LOG(ERROR) << "Failed to parse JSON: " << payload;
        return;
    }

    m_csvFileStream << csv_row << "\n";
}


}
