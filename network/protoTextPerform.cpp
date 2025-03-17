#include <iostream>
#include <chrono>
#include "trace.pb.h"
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/text_format.h>

void benchmarkProtobufParsing(const std::string& proto_text) {
    SocketStreamedTraceSegment trace_segment;

    auto start = std::chrono::high_resolution_clock::now();
    bool success = google::protobuf::TextFormat::ParseFromString(proto_text, &trace_segment);
    auto end = std::chrono::high_resolution_clock::now();
    
    if (!success) {
        std::cerr << "Failed to parse Protobuf text format!" << std::endl;
        return;
    }

    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Protobuf Text Parsing Time: " << elapsed.count() * 1000 << " ms" << std::endl;
}

int main() {
    std::string proto_text = R"(
    socket_streamed_trace_segment {
      event {
        timestamp {
          seconds: 1741756764
          nanos: 263017000
        }
        read {
          data {
            as_bytes: "\000\000\000\004\001\000\000\000\000"
          }
        }
        connection {
          local_address {
            socket_address {
              address: "127.0.0.1"
              port_value: 8022
            }
          }
          remote_address {
            socket_address {
              address: "127.0.0.1"
              port_value: 43624
            }
          }
        }
      }
    })";

    benchmarkProtobufParsing(proto_text);
    return 0;
}
