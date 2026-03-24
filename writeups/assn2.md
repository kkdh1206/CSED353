Assignment 2 Writeup
=============

My name: 김도형

My POVIS ID: kkdh1206

My student ID (numeric): 20210740

This assignment took me about [8] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPReceiver and wrap/unwrap routines:
[
    32비트 seqno가 4GB마다 돌아가는 문제를 해결하기 위해, 64비트 absolute_seqno로 변환하여 관리했습니다. unwrap 함수는 checkpoint를 기준으로 가장 가까운 64비트 숫자를 찾도록 설계되었습니다.
    
    unwrap 구현 시 현재 바퀴, 이전 바퀴, 다음 바퀴의 세 가지 후보(cand)를 계산하고 checkpoint와의 절대 거리가 가장 최소가 되는 값을 선택하도록 디자인했습니다.
    이때 세가지 후보는 정상적으로 syn에 seqno를 더한것, 1<<32 를 더한것, 1<<32 를 뺀것 이렇게 3가지로 나눠서 구현했습니다.

    TCPReceiver는 들어오는 seqno를 unwrap을 통해 abs_seqno로 변환한 뒤, SYN이 차지하는 1바이트를 고려하여 index로 변환(abs_seqno - 1)해 StreamReassembler에 전달합니다.
    초기에는 어차피 SYN이 이미 점유해서 0부터 시작하기에 1을 예외적으로 뺄 필요가 없습니다.

    ackno()는 조립이 완료된 bytes_written에 1을 더하고(SYN때문에 한칸씩 당겨져서 저장됬으므로)FIN 플래그의 존재 여부(FIN이 있으면 1을 더해줘야해서)를 더해 다시 wrap하여 송신자에게 보냅니다. window_size는 ByteStream의 남은 용량을 반환합니다.
]

Implementation Challenges:
[
    C++에서 buffer를 payload에받아서 데이터는 .copy 와 같은 함수를 써야한다는 사실을 몰랐어서 해맸습니다.
    SYN이 계속 패킷마다 있어서 계속 1을 더해줘서 누적이 되는줄 오해를 처음에 했어서 해맸었습니다. 
    추후, 올바른 개념을 이해하고 나서는 구현에 큰 어려움이 없었습니다.
]

Remaining Bugs:
[
    없습니다.
    이전에는 unwrap(UINT32_MAX, 0, 0) 상황에서 checkpoint 0과 가장 가까운 후보로 음수(-1)가 선택되어 uint64_t 언더플로우가 발생하는 버그가 있었습니다. 현재는 음수가 발생하는 후보를 원천 차단하도록 수정하여 해결하였습니다.
]


- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
