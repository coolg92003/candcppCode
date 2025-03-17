#include <iostream>
#include <chrono>
#include <rapidjson/document.h>

void benchmarkRapidJsonParsing(const std::string& json_text) {
    rapidjson::Document document;

    auto start = std::chrono::high_resolution_clock::now();
    if (document.Parse(json_text.c_str()).HasParseError()) {
        std::cerr << "Failed to parse JSON!" << std::endl;
        return;
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "RapidJSON Parsing Time: " << elapsed.count() * 1000 << " ms" << std::endl;
}

int main() {
    std::string json_text = R"({
        "socket_streamed_trace_segment": {
            "event": {
                "timestamp": {
                    "seconds": 1741756766,
                    "nanos": 231973000
                },
                "read": {
                    "data": {
                        "as_bytes": "CFX_read"
                    }
                },
                "connection": {
                    "local_address": {
                        "socket_address": {
                            "address": "127.0.0.1",
                            "port_value": 8022
                        }
                    },
                    "remote_address": {
                        "socket_address": {
                            "address": "127.0.0.1",
                            "port_value": 43624
                        }
                    }
                }
            }
        }
    })";

    benchmarkRapidJsonParsing(json_text);
    return 0;
}

