#include <iostream>
#include "rapidjson/document.h"

int main() {
    // JSON 数据
    const char* json = R"({
        "socket_streamed_trace_segment": {
            "trace_id": "1",
            "event": [
                {
                    "timestamp": "2025-02-14T03:24:42.972926Z",
                    "write": {
                        "data": {
                            "as_bytes": "writeAAAMBAAAAAAAAAQ/////AAMAAAEAAAAABAEAAAAAAAAECAAAAAAAP/8AAAAAHgEEAAAAASAIghADDxKWw2G+lAtKYSxqCAJtQDNwTVxoSmLRvwAAAAABAAAAAQ=="
                        }
                    }
                },
                {
                    "timestamp": "2025-02-14T03:24:43.972926Z",
                    "read": {
                        "data": {
                            "as_bytes": "readAAAMBAAAAAAAAAQ/////AAMAAAEAAAAABAEAAAAAAAAECAAAAAAAP/8AAAAAHgEEAAAAASAIghADDxKWw2G+lAtKYSxqCAJtQDNwTVxoSmLRvwAAAAABAAAAAQ=="
                        }
                    }
                }
            ]
        }
    })";

    // 解析 JSON
    rapidjson::Document doc;
    doc.Parse(json);

    // 检查 "event" 是否存在
    if (doc.HasMember("socket_streamed_trace_segment") &&
        doc["socket_streamed_trace_segment"].HasMember("event") &&
        doc["socket_streamed_trace_segment"]["event"].IsArray()) {
        
        // 获取 "event" 数组
        const rapidjson::Value& events = doc["socket_streamed_trace_segment"]["event"];

        // 遍历数组
        for (rapidjson::SizeType i = 0; i < events.Size(); i++) {
            const auto& event = events[i];

            // 打印 timestamp
            if (event.HasMember("timestamp") && event["timestamp"].IsString()) {
                std::cout << "Timestamp: " << event["timestamp"].GetString() << std::endl;
            }

            // 读取 as_bytes（可能是 "read" 或 "write"）
            if (event.HasMember("read") && event["read"].HasMember("data") &&
                event["read"]["data"].HasMember("as_bytes")) {
                std::cout << "Read as_bytes: \n\t" << event["read"]["data"]["as_bytes"].GetString() << std::endl;
            }

            if (event.HasMember("write") && event["write"].HasMember("data") &&
                event["write"]["data"].HasMember("as_bytes")) {
                std::cout << "Write as_bytes:\n\t" << event["write"]["data"]["as_bytes"].GetString() << std::endl;
            }

            std::cout << "---------------------------------" << std::endl;
        }
    } else {
        std::cerr << "Error: 'event' field not found or not an array!" << std::endl;
    }

    return 0;
}

