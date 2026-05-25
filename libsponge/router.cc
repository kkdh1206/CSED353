#include "router.hh"

#include <iostream>

using namespace std;

// Dummy implementation of an IP router

// Given an incoming Internet datagram, the router decides
// (1) which interface to send it out on, and
// (2) what next hop address to send it to.

// For Lab 6, please replace with a real implementation that passes the
// automated checks run by `make check_lab6`.

// You will need to add private members to the class declaration in `router.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

//! \param[in] route_prefix The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
//! \param[in] prefix_length For this route to be applicable, how many high-order (most-significant) bits of the route_prefix will need to match the corresponding bits of the datagram's destination address?
//! \param[in] next_hop The IP address of the next hop. Will be empty if the network is directly attached to the router (in which case, the next hop address should be the datagram's final destination).
//! \param[in] interface_num The index of the interface to send the datagram out on.
void Router::add_route(const uint32_t route_prefix,
                       const uint8_t prefix_length,
                       const optional<Address> next_hop,
                       const size_t interface_num) {
    cerr << "DEBUG: adding route " << Address::from_ipv4_numeric(route_prefix).ip() << "/" << int(prefix_length)
         << " => " << (next_hop.has_value() ? next_hop->ip() : "(direct)") << " on interface " << interface_num << "\n";

    DUMMY_CODE(route_prefix, prefix_length, next_hop, interface_num);
    // Your code here.
    // 걍 저장만하면됨
    RouteEntry entry;
    entry.route_prefix = route_prefix;
    entry.prefix_length = prefix_length;
    if (next_hop) {  // 있으면
        entry.next_hop = next_hop;
    }
    entry.interface_num = interface_num;
    route_entry_list.push_back(entry);  // entry 에 추가
}

//! \param[in] dgram The datagram to be routed
void Router::route_one_datagram(InternetDatagram &dgram) {
    DUMMY_CODE(dgram);
    // dgram.header().ttl--; // 먼저 감소 - 아 그럼 빼는 연산 쓸데없이 될지도 근데 원칙은 이게맞을지도 - 아 아님 이러면
    // -1 되면 망함
    if (dgram.header().ttl <= 1) {  // 1부터는 걍 빼면 0되서 패스
        return;
    }
    dgram.header().ttl--;
    // auto candidate = route_entry_list.first().next_hop; // 일단 여기 보낸다 생각
    uint8_t candidate_prefix_length = 0;
    RouteEntry *cand_entry = nullptr;

    for (auto it = route_entry_list.begin(); it != route_entry_list.end(); it++) {
        uint32_t mask = (it->prefix_length == 0)
                            ? 0
                            : (0xFFFFFFFF << (32 - it->prefix_length));  // 앞쪽만 비교할거니까 앞쪽을 만들어줌
        if ((dgram.header().dst & mask) == (mask & it->route_prefix)) {  // 만약 prefix같으면 후보 도전
            if (candidate_prefix_length <= it->prefix_length) {
                candidate_prefix_length = it->prefix_length;  // candidate 수정
                cand_entry = &(*it);  // 참조포인터 - 이거로 해야 Address 저장된 next_hop가 deepcopy안된다고 함
            }
        }
    }

    if (cand_entry) {
        if (cand_entry->next_hop) {  // next_hop존재시
            _interfaces[cand_entry->interface_num].send_datagram(
                dgram, cand_entry->next_hop.value());  // value로 해야 optional에서 값이꺼내짐
        } else {                                       // next_hop 없으면 바로보내기
            _interfaces[cand_entry->interface_num].send_datagram(
                dgram,
                Address::from_ipv4_numeric(
                    dgram.header().dst));  // route_prefix는 앞에만 같아서 안됨 그리고 dst변환도해줘야하고
        }
    }

    // Your code here.
}

void Router::route() {
    // Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
    for (auto &interface : _interfaces) {
        auto &queue = interface.datagrams_out();
        while (not queue.empty()) {
            route_one_datagram(queue.front());  // 오 한개씩 쪼개서 짜놓아져있네
            queue.pop();
        }
    }
}
