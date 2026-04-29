#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return {_sender.stream_in().remaining_capacity()}; } // 남은용량 반환

size_t TCPConnection::bytes_in_flight() const { return {_sender.bytes_in_flight()}; } // 보내는 중인데이터

size_t TCPConnection::unassembled_bytes() const { return {_receiver.unassembled_bytes()}; } // 

size_t TCPConnection::time_since_last_segment_received() const { return {_last_received_time}; } // 마지막으 받고 초기화되고 ticker가 돌아가고있음

void TCPConnection::segment_received(const TCPSegment &seg) { // 수신시

    // 그냥 데이터면 last_received_time초기화
    _last_received_time = 0; // 받았으니까 이거 초기화

    // rst 면 걍 거부
    if (seg.header().rst){ 
         _active = false;
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        return;
    }
    
    
    // ack(크기 > 0)면 잘받았다 ack보내주기
    if (seg.header().ack){
        auto window = seg.header().win;
        auto ackno = seg.header().ackno;
        _sender.ack_received(ackno,window); // 받았으니까 처리
        // if(seg.length_in_sequence_space() ==0){
        //     return; // 이건 데이터도없어서 다시 ack반환안해도됨
        // }
    } // synack일수도있으니까 ack먼저 처리

    // reciever 한테도 보내주기
    _receiver.segment_received(seg); 

    if (_receiver.stream_out().input_ended() && !_sender.stream_in().eof()) { // FIN을 받은경우는 꺼줌, 내가 FIN을 보낸경우에는 계속 true로 둠
        _linger_after_streams_finish = false; 
    }

    if (seg.header().syn && _sender.next_seqno_absolute()== 0) { // 처음 SYN에 대해서만 connect날리고 SYNACK는 패스시킴
        connect(); // 연결요청
        return;
    }

//     bool need_send = seg.length_in_sequence_space() > 0;

// // 리시버가 보기에 대답이 필요한 상황(Handshake 등) 체크
// if (_receiver.ackno().has_value() && (seg.length_in_sequence_space() == 0)
//     && seg.header().seqno == _receiver.ackno().value() - 1) {
//     need_send = true;
// }

// if (need_send) {
//     _sender.fill_window();
//     if (_sender.segments_out().empty()) {
//         _sender.send_empty_segment();
//     }
// }
// send_segment();

    // ack 보내주기 - 데이터가 있는거만
    if (seg.length_in_sequence_space()>0){
        _sender.fill_window(); // 데이터 보낼거 충전해둠
        if(_sender.stream_in().buffer_empty()){ // 그래도 보낼게 없다 즉 거절
            _sender.send_empty_segment(); // ackno전달용 pure ACK
        }
    }  

    // 0바이트 간보는거? 추가해야할지도

    send_segment();

}


size_t TCPConnection::write(const string &data) { // 발송시
    DUMMY_CODE(data);
    auto written_size = _sender.stream_in().write(data);// 데이터를 bytestream에 바로 씀

    _sender.fill_window(); // 해당 데이터로 세그먼트들을 제작

    send_segment(); // 세그먼트 전송

    return {written_size};
}

bool TCPConnection::active() const {
    if(!_active){
        return false;
    }

   if (_receiver.stream_out().input_ended() &&  // 비정상종료에서 정상종료 되면 안되기 때문에 이안에 넣어줌
        _sender.stream_in().eof() && 
        _sender.bytes_in_flight() == 0) { // 만약 reciever, sender모두 종료에 보내는것도 없다면 종료상황임
        
            if (!_linger_after_streams_finish ||  // FIN을 내가 받고 답장을 한 상황이거나
                _last_received_time >= 10 * _cfg.rt_timeout) { // 보냈는데 시간 많이 지났으면 _active false로 해줌
                return false;
            }
        }
    return true;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) { DUMMY_CODE(ms_since_last_tick); 
    
    _last_received_time += ms_since_last_tick; // 시간흐르도록
    _sender.tick(ms_since_last_tick); // 여기도 시간흐르도록

    if (!active()) {
        _active = false;
        return;  // 꺼져있으면 작동안함
    }

    if (_sender.consecutive_retransmissions() <= TCPConfig::MAX_RETX_ATTEMPTS){

        
        _sender.fill_window(); // zero probing 보낼수도
        send_segment(); // 재전송 혹은 실패 seg를 전송


    }
    else{
        _active = false;
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();

        TCPSegment seg;
        _sender.send_empty_segment(); // empty하나 쌓아둠
        auto& queue = _sender.segments_out(); 
        seg = queue.back(); // 젤뒤에 추가됬을거임
        seg.header().rst = true; // 오류 flag 설정
        auto ackno = _receiver.ackno(); // 사실 이거 ackno win이 rst에서는 필요없을거같긴한데 일단 보내줌
        auto win = _receiver.window_size();
        seg.header().win = win;
        if (ackno.has_value()){
            seg.header().ackno = ackno.value(); // syn 일때는 ackno없음 그냥 isn값만 seqno에 실어서 보냄, 이제 이걸 받은쪽이 isn설정됬으니가 ackno응답이 가능해서 실어서줌
            // optional 타입이라 값을 받으려면 .value()로 까야함
            seg.header().ack = true;
        }
        _segments_out.push(seg);
        queue.pop(); // empty제거
    }
    
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input(); // 종료를 위해 이걸 호출
    _sender.fill_window(); // fin을 채워서 segment만들어둠
    send_segment(); // 세그먼트 전송
}

void TCPConnection::connect() { // 연결하는 함수 3hand shake 의 syn보내는 첫부분만 담당
    _active = true; // 켜주고
    if (_sender.next_seqno_absolute() > 0) { // 방어로직 - 이미 syn을 보냈으면 1이상이라서 더이상 connect호출해도 무시해야함
        return;
    }
    _sender.fill_window();
    send_segment();
}

void TCPConnection::send_segment() { // queue에 쌓인걸 보내는함수
    auto& queue = _sender.segments_out(); // auto로 하면 타입 신경안써도 됨 그리고 & 붙여서 참조로 가져와야 pop이 먹힘 복사안해서
    while(!queue.empty()){ //while(queue){ 아 파이썬아니라서 이거 안됨 // queue가 존재하면
        TCPSegment seg;
        // size_t win;
        seg = queue.front();
        queue.pop();
        // 지금은 seqno만 있고 ackno랑 win을 알아야하는데 이건 Sender말고 Reciever에서 알고있음

        auto win = _receiver.window_size();
        auto ackno = _receiver.ackno(); // auto는 뒤에 바로 결과 값이 있어야 사용가능
        seg.header().win = win;
        if (ackno.has_value()){ 
            seg.header().ackno = ackno.value(); // syn 일때는 ackno없음 그냥 isn값만 seqno에 실어서 보냄, 이제 이걸 받은쪽이 isn설정됬으니가 ackno응답이 가능해서 실어서줌
            // optional 타입이라 값을 받으려면 .value()로 까야함
            seg.header().ack = true; // ackno가 있으면 무조건 대답이니까 
        }
        
        _segments_out.push(seg);
        
    }
}

TCPConnection::~TCPConnection() { // 소멸자
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
            // RST를 전송해주고 더이상 안보내니까 _active를 false로 두고 stream_in, stream_out에 에러보내줌

            TCPSegment seg;
            _sender.send_empty_segment(); // empty하나 쌓아둠
            auto& queue = _sender.segments_out(); 
            seg = queue.back(); // 젤뒤에 추가됬을거임
            seg.header().rst = true; // 오류 flag 설정
            auto ackno = _receiver.ackno(); // auto는 뒤에 바로 결과 값이 있어야 사용가능
            auto win = _receiver.window_size();
            seg.header().win = win;
            if (ackno.has_value()){
                seg.header().ackno = ackno.value(); // syn 일때는 ackno없음 그냥 isn값만 seqno에 실어서 보냄, 이제 이걸 받은쪽이 isn설정됬으니가 ackno응답이 가능해서 실어서줌
                // optional 타입이라 값을 받으려면 .value()로 까야함
                seg.header().ack = true;
            }

            _segments_out.push(seg);
            queue.pop(); // empty제거

            _active = false;
            _sender.stream_in().set_error();
            _receiver.stream_out().set_error();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
