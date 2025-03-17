#include <iostream>
#include "trace.pb.h"
#include <google/protobuf/text_format.h>
#include <google/protobuf/util/json_util.h>

int main() {
    // Step 1: Create and populate the protobuf message
    std::cout << "main: enter" << std::endl;
    trace::SocketStreamedTraceSegment segment;
    segment.mutable_event()->mutable_timestamp()->set_seconds(1741756764);
    segment.mutable_event()->mutable_timestamp()->set_nanos(263017000);

    trace::Read* read = segment.mutable_event()->mutable_read();
    trace::Data* data = read->mutable_data();
    data->set_as_bytes("\000\000\000\004\001\000\000\000\000", 9);

    trace::Connection* connection = segment.mutable_event()->mutable_connection();
    trace::SocketAddress* local_addr = connection->mutable_local_address();
    local_addr->set_address("127.0.0.1");
    local_addr->set_port_value(8022);

    trace::SocketAddress* remote_addr = connection->mutable_remote_address();
    remote_addr->set_address("127.0.0.1");
    remote_addr->set_port_value(43624);

    // Step 2: Serialize the message into a buffer
    int size = segment.ByteSizeLong();
    char* buffer = new char[size];

    if (!segment.SerializeToArray(buffer, size)) {
        std::cerr << "Serialization failed!" << std::endl;
        delete[] buffer;
        return 1;
    }

    // Step 3.1: proto_text
    std::cout << "main: output as proto_text" << std::endl;
    std::string text_output;
    google::protobuf::TextFormat::PrintToString(segment, &text_output);
    std::cout << "\tText Format Output, SIZE=" << text_output.size() << std::endl;
    std::cout << "\t\tText Format Output---------------:\n" << text_output << std::endl;
    std::cout << "\t\tText Format Output---------------\n" << std::endl;

    // Step 3.2: json
    std::cout << "main: output as proto_json" << std::endl;
    std::string json_output;
    auto j_result = google::protobuf::util::MessageToJsonString(segment, &json_output);
    if (j_result.ok()) {
    	std::cout << "JSON Output: OK \n";
    }
    std::cout << "JSON Output:\n" << std::endl;
    std::cout << "\tJSON Format Output, SIZE=" << json_output.size() << std::endl;
    std::cout << "\t\tJSON Format Output---------------:\n" << json_output << std::endl;
    std::cout << "\t\tJSON Format Output---------------\n" << std::endl;


    // Step 3.3: Deserialize the message from the buffer
    trace::SocketStreamedTraceSegment new_segment;
    if (!new_segment.ParseFromArray(buffer, size)) {
        std::cerr << "Deserialization failed!" << std::endl;
        delete[] buffer;
        return 1;
    }

    delete[] buffer; // Free allocated memory

    // Step 4: Output each field from the deserialized message
    std::cout << "===== Parsed Protobuf Data =====" << std::endl;
    std::cout << "main: encoded as protobuff, segment.SIZE=" << size  << std::endl;
    std::cout << "Timestamp: " << new_segment.event().timestamp().seconds() 
              << "s, " << new_segment.event().timestamp().nanos() << "ns" << std::endl;

    std::cout << "Local Address: " << new_segment.event().connection().local_address().address()
              << ":" << new_segment.event().connection().local_address().port_value() << std::endl;

    std::cout << "Remote Address: " << new_segment.event().connection().remote_address().address()
              << ":" << new_segment.event().connection().remote_address().port_value() << std::endl;

    std::string as_bytes = new_segment.event().read().data().as_bytes();
    std::cout << "Read Data (as_bytes): ";
    for (unsigned char c : as_bytes) {
        printf("\\x%02x", c);  // Print bytes in hex format
    }
    std::cout << std::endl;

    return 0;
}

