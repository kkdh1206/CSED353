#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>

// Dummy implementation of a network interface
// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

// For Lab 5, please replace with a real implementation that passes the
// automated checks run by `make check_lab5`.

// You will need to add private members to the class declaration in `network_interface.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address) {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram,
                                     const Address &next_hop) {  // 로컬네트워크 기기 찾기위해 ARP(공유기 찾기등)
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();  // 일단 여기로 보내야하는듯 그럼 여기서 또 타고 이동함
    // EthernetAddress destination = dgram.header().dst;
    EthernetHeader header;
    EthernetFrame frame;
    EthernetFrame arp_frame;
    EthernetAddress dst;
    // 타입나누고 target ip주소 확인
    header.type = EthernetHeader::TYPE_IPv4;  // IPv4타입
    header.src = _ethernet_address;           // 헤더 완성

    frame.payload() = dgram.serialize();  // payload넣어줌

    if (_arp_storage.count(next_hop_ip)) {  // 이게 존재하면 이제 보냄
        dst = _arp_storage[next_hop_ip];    // 미리 저장된 MAC주소
        // ipv4로 frame에 싸서 거기로 보내줌 arp안보내도됨
        header.dst = dst;
        frame.header() = header;                            // 헤더넣어줌
        _frames_out.push(frame);                            // 전송
    } else {                                                // arp를 보내야함
        frame.header() = header;                            // 여기도 header 넣어주기
        _waiting_frames_out[next_hop_ip].push_back(frame);  // next_hop_ip 의 리스트에달아둠
        if (!_arp_resend[next_hop_ip]) {                    // true일때만 차단
            dst = ETHERNET_BROADCAST;                       // MAC몰라서 broadcast로 보냄
            ARPMessage arp_packet;
            arp_packet.opcode = ARPMessage::OPCODE_REQUEST;             // 요청보냄
            arp_packet.sender_ip_address = _ip_address.ipv4_numeric();  // 우리가 보냄
            arp_packet.sender_ethernet_address = _ethernet_address;
            arp_packet.target_ip_address = next_hop_ip;  // 상대 next hop에게 보냄
            // arp_packet.target_ethernet_address  // 이던 몰라서 못적음

            EthernetHeader arp_header;
            arp_header.type = EthernetHeader::TYPE_ARP;  // ARP타입
            arp_header.dst = dst;
            arp_header.src = _ethernet_address;  // 헤더 완성
            arp_frame.header() = arp_header;     // 헤더넣어줌

            arp_frame.payload() = arp_packet.serialize();  // payload넣어줌
            _frames_out.push(arp_frame);                   // 바로 보내줌
            _arp_send_time[next_hop_ip] = 0;               // 보냈으니까 초기화
            _arp_resend[next_hop_ip] = true;               // 보냈으니까 막아줌 -true가 막는거임
        }
    }

    DUMMY_CODE(dgram, next_hop, next_hop_ip);
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    DUMMY_CODE(frame);
    EthernetHeader header = frame.header();
    uint16_t type = header.type;
    if (header.dst == ETHERNET_BROADCAST || header.dst == _ethernet_address) {  // 나한테 온게 맞는경우

        if (type == EthernetHeader::TYPE_ARP) {  // MAC 확정용
            // ARP이므로 arp message로 파싱
            ARPMessage arp_message;
            if (arp_message.parse(frame.payload().concatenate()) == ParseResult::NoError) {  // 에러없이 파싱되면
                // arp_message에 데이터 채워짐
                if (arp_message.target_ip_address ==
                    _ip_address.ipv4_numeric()) {  // uint32_t로 비교 해서 우리에게 온게 맞는거면
                    // 캐시에 src저장, 시간 0으로 저장 일단 다함
                    _arp_storage[arp_message.sender_ip_address] = header.src;  // src MAC주소 저장하기
                    _arp_recv_time[arp_message.sender_ip_address] = 0;         // 시간 0

                    if (arp_message.opcode == ARPMessage::OPCODE_REPLY) {  // 대답이 온거면 밀렸던거 다시 보내주기
                        auto &waiting_list = _waiting_frames_out[arp_message.sender_ip_address];  // 참조로 가져옴!!!
                        // 복사로 가져오면 실제로 다 했는데도 삭제가안됨 그러면
                        // next_hop에서 응답해서 보내주면 그거 기반으로 기다리고있는 waiting list를 보내줘야함

                        for (auto it = waiting_list.begin(); it != waiting_list.end();) {
                            InternetDatagram datagram;
                            if (datagram.parse(it->payload().concatenate()) ==
                                ParseResult::NoError) {         // 페이로드 datagram까주고
                                it->header().dst = header.src;  // 도착한 MAC주소 넣어주기
                                _frames_out.push(*it);
                                it = waiting_list.erase(it);  // 삭제하고 이동 동시에됨 이러면
                            } else {
                                ++it;  // 아니면 다음칸이동
                            }
                        }
                    }
                    if (arp_message.opcode == ARPMessage::OPCODE_REQUEST) {  // 응답보내줘야함
                        ARPMessage arp_packet;
                        arp_packet.opcode = ARPMessage::OPCODE_REPLY;
                        arp_packet.sender_ip_address = _ip_address.ipv4_numeric();  // 우리가 보냄
                        arp_packet.sender_ethernet_address = _ethernet_address;
                        arp_packet.target_ip_address = arp_message.sender_ip_address;  // 받은쪽에게 보냄
                        arp_packet.target_ethernet_address = header.src;

                        EthernetFrame f;
                        EthernetHeader h;
                        h.type = EthernetHeader::TYPE_ARP;  // ARP타입
                        h.dst = header.src;                 // 발송이 온곳으로 답장
                        h.src = _ethernet_address;          // 헤더 완성
                        f.header() = h;
                        f.payload() = arp_packet.serialize();  // 직렬화
                        _frames_out.push(f);
                        _arp_send_time[arp_message.sender_ip_address] = 0;  // 보냈으니까 초기화
                    }
                }
            }
        } else if (type == EthernetHeader::TYPE_IPv4) {  // 이미 MAC주소 알고있어서 검증 끝남
            InternetDatagram datagram;
            if (datagram.parse(frame.payload()) == ParseResult::NoError) {  // 에러없이 파싱되면
                return datagram;                                            // 바로 데이터그램 반환
            }
        }
    }
    return {};  // 안되도 빈거 반환
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) {
    for (auto it = _arp_send_time.begin(); it != _arp_send_time.end();) {
        it->second += ms_since_last_tick;  // 시간 더해줌
        if (it->second >= 5 * 1000) {
            _arp_resend[it->first] = false;  // 재전송 허용
        }
        ++it;  // 다음칸이동
    }
    for (auto it = _arp_recv_time.begin(); it != _arp_recv_time.end();) {
        it->second += ms_since_last_tick;   // 시간 더해줌
        if (it->second >= 30 * 1000) {      // 30초넘으면
            _arp_storage.erase(it->first);  // storage꺼로 함
            it = _arp_recv_time.erase(it);  // 시간 삭제
            // 캐시에서도 삭제
        } else {
            ++it;  // 아니면 다음칸이동 삭제면 이동할필요없이 땡겨짐
        }
    }
    DUMMY_CODE(ms_since_last_tick);
}
