#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return {_sender._stream.remaining_capacity()}; } // 남은용량 반환

size_t TCPConnection::bytes_in_flight() const { return {_sender.bytes_in_flight()}; } // 보내는 중인데이터

size_t TCPConnection::unassembled_bytes() const { return {_receiver.unassembled_bytes()}; } // 

size_t TCPConnection::time_since_last_segment_received() const { return {_last_received_time}; } // 마지막으 받고 초기화되고 ticker가 돌아가고있음

void TCPConnection::segment_received(const TCPSegment &seg) { // 수신시
    _receiver.segment_received(seg); 
    _last_received_time = 0; // 받았으니까 이거 초기화
    if (seg.header().syn) {

    }
}

bool TCPConnection::active() const { return {_active}; }

size_t TCPConnection::write(const string &data) { // 발송시
    DUMMY_CODE(data);
    return {};
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) { DUMMY_CODE(ms_since_last_tick); 
    _last_received_time += ms_since_last_tick; // 시간흐르도록
    _sender.tick(ms_since_last_tick); // 여기도 시간흐르도록
    if (consecutive_retransmissions < TCPConfig::MAX_RETX_ATTEMPTS){
        

    }
}

void TCPConnection::end_input_stream() {
    _sender.stream.end_input(); // 종료를 위해 이걸 호출
}

void TCPConnection::connect() { // 연결하는 함수 3hand shake

}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
