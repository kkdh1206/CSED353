#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    DUMMY_CODE(seg);
    TCPHeader header = seg.header();
    Buffer payload = seg.payload();
    if (header.syn) {         // 시작
        _isn = header.seqno;  // 상대 sender에 의해서 isn 결정되고 그걸 그대로 가지고있음
    }
    if (!_isn.has_value()) {
        return;  // syn안왓으면 그냥 종료 이거없으면 순서 알지도 못함
    }

    uint64_t checkpoint =
        _reassembler.stream_out()
            .bytes_written();  // 어차피 checkpoint는 오차 엄청커도되서 window안에만있으면되서 이걸로둠 - byte_written이
                               // 사실상 index랑 가까울거임 0부터 시작해서 한친구라서
    uint64_t absolute_seqno =
        unwrap(header.seqno, _isn.value(), checkpoint);  // seqno를 64비트에 SYN까지 고려해서 만들어줌
    uint64_t index;
    if (header.syn) {
        index = absolute_seqno;  // index는 지금 초기 시작위치를 찍어주는거기때문
    } else {
        index = absolute_seqno - 1;  // 이전에 SYN때문에 1칸당겨진 상태라서 쭉 뒤에서부터는 index를 한칸씩 당겨야함
    }

    _reassembler.push_substring(payload.copy(), index, header.fin);  // buffer 타입이라
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!_isn.has_value()) {
        return {};  // syn도 안온놈이면 반환 안해야함
    }
    // syn 있으면 absolute_seqno를 seqno로 바꿔서 보내줘야함
    uint64_t abs_seqno = _reassembler.stream_out().bytes_written();  // 읽은데까지 반환
    if (_reassembler.stream_out().input_ended()) {
        abs_seqno += 1;  // FIN도 자리를 하나 먹기때문에
    }
    return wrap(abs_seqno + 1, _isn.value());  // 다시 더해서 보내야함 왜냐면 1빼서 가져왓으니까
};

size_t TCPReceiver::window_size() const { return _reassembler.stream_out().remaining_capacity(); }  // 남은 capacity전송
