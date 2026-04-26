#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _retransmission_timeout{_initial_retransmission_timeout}
    , _stream(capacity) {
    // _retransmission_timeout = _initial_retransmission_timeout; // 초기화
}

uint64_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }  // 이걸 관리해서 더해주긴해야함

void TCPSender::fill_window() {
    // 일부로 dead락 방지용
    uint64_t win = (_window_size == 0) ? 1 : _window_size;

    // 이미 보낸거 제외하고 공간이있으면 계속 데이터 보냄
    while (win > _bytes_in_flight) {
        TCPSegment seg;

        // SYN: 시작할 때 켬
        if (_next_seqno == 0)
            seg.header().syn = true;
        seg.header().seqno = next_seqno();

        // 남은공간  계산
        uint64_t space = win - _bytes_in_flight - seg.length_in_sequence_space();
        // 남은공간, payload최대, 남은 데이터양 중에 작은거 고름
        size_t read_size = min({static_cast<size_t>(space), _stream.buffer_size(), TCPConfig::MAX_PAYLOAD_SIZE});
        // 이걸통해서 그냥 헤더에 신호만 보내는 케이스가 아닐때 데이터 보낼때만 데이터 싣어줌
        if (read_size > 0)
            seg.payload() = Buffer(_stream.read(read_size));

        // fin은 한번만 보내야하니까 - 이때까지 공간에 넣을수있으면 fin넣어줌
        if (!_fin_sent && _stream.eof() && (win > _bytes_in_flight + seg.length_in_sequence_space())) {
            seg.header().fin = true;
            _fin_sent = true;
        }

        // 데이터를 syn,data,fin 다해도 0이면 끝내야함 보낼거 없으니까 유령패킷 방지
        if (seg.length_in_sequence_space() == 0)
            break;

        // 발송
        _segments_out.push(seg);
        _outstanding_segments.push(seg);

        uint64_t len = seg.length_in_sequence_space();
        _bytes_in_flight += len;
        _next_seqno += len;

        //  타이머 켜기
        if (!_timer_active) {
            _timer_active = true;
            _time = 0;
        }

        // fin까지 보냈으면 끝
        if (seg.header().fin)
            break;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    uint64_t abs_seqno = unwrap(ackno, _isn, _next_seqno);  // _next_seqno기준으로 풀어도됨 근처일거니까
    if (abs_seqno > _next_seqno) {
        return;  // 말이안됨
    }
    bool check = false;
    while (!_outstanding_segments.empty() && unwrap(_outstanding_segments.front().header().seqno +
                                                        _outstanding_segments.front().length_in_sequence_space(),
                                                    _isn,
                                                    _next_seqno) <= abs_seqno) { // abs_seqno가 지금 온번호니까 그거보다 작은 outstanding은 이제 다온거니까 이제 삭제해둠
        // 끝번호랑 비교해서 끝자리까지 다 처리가 된건지 확인
        TCPSegment seg = _outstanding_segments.front();
        _outstanding_segments.pop();
        check = true;
        _bytes_in_flight -= seg.length_in_sequence_space();  // 도착됬으니까 이거에서 빼줌
    }
    if (check) {
        _retransmission_count = 0;  // ack받으면 다시 초기화해줌 - 이전에없던 새로인거가 확인되면
        _time = 0;                  // 시간도 초기화
        _retransmission_timeout = _initial_retransmission_timeout;  // RTO 초기화 해줘야함
    }
    if (_outstanding_segments.empty()) {
        _timer_active = false;  // 이제 보내고있는게 없으면 queue에서 빼버림
    }

    _window_size = window_size;

    fill_window();

    DUMMY_CODE(ackno, window_size);
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (!_timer_active) {
        return;  // 꺼져있으면 작동안함
    }
    _time += ms_since_last_tick;             // 시간 흐르는거 반영
    if (_retransmission_timeout <= _time) {  // 이러면 타임아웃임 - 등호포함해야함
        // 아 잠만 이거 온 윈도우 사이즈도 봐야하는데
        _retransmission_count += 1;
        if (_outstanding_segments.empty()) {
            return;  // 원래 존재하지않지만 왜냐면 timer가 안켜져서 혹시몰라서 근데 해둠
        }
        TCPSegment old = _outstanding_segments.front();  // 근데 비어있으면 어쩌지 - empty로 확인함
        _segments_out.push(old);
        if (_window_size > 0) { // zero window 상황에서는 retransmission 을 2배씩 늘려서 기다리지 않기위해 제외한상황만함
            _retransmission_timeout *= 2;
        }
        _time = 0;  // 시간초기화
    }

    DUMMY_CODE(ms_since_last_tick);
}

unsigned int TCPSender::consecutive_retransmissions() const { return _retransmission_count; }

void TCPSender::send_empty_segment() {  // ackno 대답용도 이제 데이터 보낼거 따로 없을때 이걸로 하는거임 - ackno는 나중에 달아줄거임 그리고 seqno는 empty기때문에 더이상 증가하지 않고 같은걸 계속보냄
    TCPSegment empty;
    empty.header().seqno =
        next_seqno();  // 보낼때 absoulte seqno로 보내는게아니라 seqno로 보낸다 이는 32비트로 더 경제적으로 보낼수있고
                       // 이렇게보내도 처음에 보내는쪽의 isn을 미리저장해두기 때문에 복구가 가능해서 가능하다.
    // 계산할 때는 64비트(절대), 전송할 때는 32비트(포장) 이라고한다
    _segments_out.push(empty);
}

// 양방향 통신이라 sender도 ackno를 보내줌 seqno는 당연히 데이터 보내주는거고