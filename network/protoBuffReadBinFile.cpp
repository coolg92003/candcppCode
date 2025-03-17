#include <iostream>
#include <fstream>
#include "trace.pb.h"

void deserializeAndPrint() {
    trace::SocketStreamedTraceSegment segment;

    // Read from binary file
    std::ifstream input("trace_output.bin", std::ios::binary);
    if (!segment.ParseFromIstream(&input)) {
        std::cerr << "Failed to parse protobuf file!" << std::endl;
        return;
    }

    // Print the parsed data
    std::cout << "Parsed Protobuf Data:\n";
    std::cout << "Timestamp: " << segment.event().timestamp().seconds() << "s, "
              << segment.event().timestamp().nanos() << "ns\n";
    std::cout << "Local Address: " << segment.event().connection().local_address().address()
              << ":" << segment.event().connection().local_address().port_value() << "\n";
    std::cout << "Remote Address: " << segment.event().connection().remote_address().address()
              << ":" << segment.event().connection().remote_address().port_value() << "\n";
}

int main() {
    deserializeAndPrint();
    return 0;
}

