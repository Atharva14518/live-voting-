#ifndef STREAM_PROCESSOR_H
#define STREAM_PROCESSOR_H

#include <string>
#include <vector>
#include <queue>
#include <functional>
#include <memory>
#include <chrono>

/**
 * StreamProcessor - Conceptual stub for real-time event streaming
 * 
 * In production, this would integrate with:
 * - Apache Kafka for distributed streaming
 * - AWS Kinesis for managed streaming
 * - Redis Streams for lightweight messaging
 * 
 * This stub simulates the core concepts:
 * - Event buffering
 * - Stream partitioning
 * - Consumer groups
 * - Backpressure handling
 */

struct StreamEvent {
    std::string eventId;
    std::string eventType;      // "vote", "proposal", "user_action"
    std::string payload;         // JSON or serialized data
    std::chrono::system_clock::time_point timestamp;
    std::string partitionKey;    // For consistent hashing to partitions
    
    StreamEvent() {}
    StreamEvent(const std::string& type, const std::string& data)
        : eventType(type), payload(data), 
          timestamp(std::chrono::system_clock::now()) {}
};

class StreamProcessor {
private:
    std::queue<StreamEvent> eventQueue;
    size_t maxQueueSize;
    bool isRunning;
    
    // Callback handlers
    std::function<void(const StreamEvent&)> voteHandler;
    std::function<void(const StreamEvent&)> proposalHandler;
    std::function<void(const StreamEvent&)> userActionHandler;
    
public:
    StreamProcessor(size_t maxSize = 10000);
    
    /**
     * Produce (publish) an event to the stream
     * @param event Event to publish
     * @return True if successfully queued
     */
    bool produce(const StreamEvent& event);
    
    /**
     * Consume (process) events from the stream
     * @param maxEvents Maximum events to process in batch
     * @return Number of events processed
     */
    int consume(int maxEvents = 100);
    
    /**
     * Register event handlers
     */
    void setVoteHandler(std::function<void(const StreamEvent&)> handler);
    void setProposalHandler(std::function<void(const StreamEvent&)> handler);
    void setUserActionHandler(std::function<void(const StreamEvent&)> handler);
    
    /**
     * Start/stop processing
     */
    void start();
    void stop();
    
    /**
     * Get queue statistics
     */
    size_t getQueueSize() const { return eventQueue.size(); }
    bool hasBackpressure() const { return eventQueue.size() > maxQueueSize * 0.8; }
    
    /**
     * Conceptual info about production deployment
     */
    static std::string getProductionInfo();
};

#endif // STREAM_PROCESSOR_H
