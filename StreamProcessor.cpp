#include "StreamProcessor.h"
#include <iostream>

StreamProcessor::StreamProcessor(size_t maxSize)
    : maxQueueSize(maxSize), isRunning(false) {
}

bool StreamProcessor::produce(const StreamEvent& event) {
    if (eventQueue.size() >= maxQueueSize) {
        std::cerr << "Stream queue full - backpressure detected\n";
        return false;
    }
    
    eventQueue.push(event);
    return true;
}

int StreamProcessor::consume(int maxEvents) {
    if (!isRunning) return 0;
    
    int processed = 0;
    while (!eventQueue.empty() && processed < maxEvents) {
        StreamEvent event = eventQueue.front();
        eventQueue.pop();
        
        // Route to appropriate handler
        if (event.eventType == "vote" && voteHandler) {
            voteHandler(event);
        } else if (event.eventType == "proposal" && proposalHandler) {
            proposalHandler(event);
        } else if (event.eventType == "user_action" && userActionHandler) {
            userActionHandler(event);
        }
        
        processed++;
    }
    
    return processed;
}

void StreamProcessor::setVoteHandler(std::function<void(const StreamEvent&)> handler) {
    voteHandler = handler;
}

void StreamProcessor::setProposalHandler(std::function<void(const StreamEvent&)> handler) {
    proposalHandler = handler;
}

void StreamProcessor::setUserActionHandler(std::function<void(const StreamEvent&)> handler) {
    userActionHandler = handler;
}

void StreamProcessor::start() {
    isRunning = true;
    std::cout << "Stream processor started\n";
}

void StreamProcessor::stop() {
    isRunning = false;
    std::cout << "Stream processor stopped\n";
}

std::string StreamProcessor::getProductionInfo() {
    return R"(
=== Production Streaming Architecture ===

Recommended Stack:
1. Apache Kafka - Distributed streaming platform
   - High throughput (millions of messages/sec)
   - Fault tolerance with replication
   - Horizontal scalability
   - Consumer groups for parallel processing

2. AWS Kinesis - Managed streaming service
   - Auto-scaling
   - Pay-per-use pricing
   - Integration with AWS ecosystem
   - Real-time analytics

3. Redis Streams - Lightweight messaging
   - Low latency (<1ms)
   - Simple deployment
   - Good for smaller scale
   - Pub/sub patterns

Implementation Considerations:
- Event serialization (Protocol Buffers, Avro, JSON)
- Partitioning strategy (by user ID, proposal ID)
- Consumer group coordination
- Dead letter queues for failed events
- Monitoring and alerting
- Backpressure handling
- Event replay for recovery

Example Kafka Integration:
```cpp
#include <librdkafka/rdkafkacpp.h>

// Producer
RdKafka::Producer* producer = RdKafka::Producer::create(conf, errstr);
producer->produce(topic, partition, flags, payload, key, ...);

// Consumer
RdKafka::KafkaConsumer* consumer = RdKafka::KafkaConsumer::create(conf, errstr);
consumer->subscribe({topic});
RdKafka::Message* msg = consumer->consume(timeout_ms);
```

Deployment:
- Kubernetes StatefulSets for Kafka brokers
- Horizontal Pod Autoscaler for consumers
- Prometheus + Grafana for monitoring
- AlertManager for incident response
)";
}
