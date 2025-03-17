#include <iostream>
#include <fstream>
#include "trace.pb.h"

int main() {
    // Create a protobuf object and set values
    trace::SocketStreamedTraceSegment segment;
    
    trace::Event* event = segment.mutable_event();
    trace::Timestamp* timestamp = event->mutable_timestamp();
    timestamp->set_seconds(1741756764);
    timestamp->set_nanos(263017000);

    trace::Read* read = event->mutable_read();
    trace::Data* data = read->mutable_data();
    data->set_as_bytes("\000\000\000\004\001\000\000\000\000", 9);

    trace::Connection* connection = event->mutable_connection();
    trace::SocketAddress* local_addr = connection->mutable_local_address();
    local_addr->set_address("127.0.0.1");
    local_addr->set_port_value(8022);

    trace::SocketAddress* remote_addr = connection->mutable_remote_address();
    remote_addr->set_address("127.0.0.1");
    remote_addr->set_port_value(43624);

    // Serialize to binary
    std::ofstream output("trace_output.bin", std::ios::binary);
    segment.SerializeToOstream(&output);
    output.close();
    
    std::cout << "Protobuf message serialized to 'trace_output.bin'." << std::endl;

    return 0;
}

