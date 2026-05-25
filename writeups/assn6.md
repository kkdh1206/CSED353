Assignment 6 Writeup
=============

My name: 김도형

My POVIS ID: kkdh1206

My student ID (numeric): 20210740

This assignment took me about [3] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the Router:
[
    Router는 보내야하는 datagram의 목적지 IP 주소를 기반으로 next hop을 효율적으로 결정합니다.
    add_route는 라우팅 테이블에 새로운 경로를 추가하는 함수로, 입력받은 prefix, prefix_length, next_hop, interface_num을 구조체에 담아 내부 벡터인 route_entry_list에 순차적으로 저장합니다.
    route_one_datagram은 단일 데이터그램의 라우팅을 처리하고 패킷의 TTL을 확인하여 1 이하인 경우 즉시 버리고 아니면 TTL을 1 감소시킵니다. 
    TTL 통과를 했다면 알고리즘을 사용하여 prefix가 동일한것중 prefix길이가 가장 긴 경로로 결정합니다. 
    만약에 해당 entry에 next_hop이 존재하면 해당 next_hop 주소로, 존재하지 않으면 바로 최종 목적지 주소를 변환하여 해당 인터페이스의 send_datagram으로 전달합니다.
]

Implementation Challenges:
[
    기본 cand_entry를 비워두고 이거보다 길이가 더클때만 업데이트를 하게 코드를 짰었는데, 이로인해 길이가 0인 Prefix만 오는 경우에는 누락되버리는 문제가 생겨서
    부등호를 등호를 포함한 부등호로 바꾸어 해결했습니다.

    반복자를 통해서 for문을 돌고 원하는 candidate를 찾았을때 처음에는 deep copy로 구혆하렫ㄷ가 Address타입인 next_hop도 다 복사되면 비효율적이기에 참조를 통해서 해결했습니다.
]

Remaining Bugs:
[
    없습니다.
]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
