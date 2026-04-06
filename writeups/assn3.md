Assignment 3 Writeup
=============

My name: 김도형

My POVIS ID: kkdh1206

My student ID (numeric): 20210740

This assignment took me about [20] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPSender:
[
    fill_window()는 가용 윈도우 크기(_window_size)와 현재 날아가고 있는 데이터(_bytes_in_flight)를 비교하는 while (win > _bytes_in_flight) 루프를 기반으로 설계했습니다. 수신자의 윈도우가 0일 때는 데드락 방지를 위해 윈도우 크기를 1로 간주하여 Zero-Window Probing을 수행하도록 처리했습니다.
    
    패킷을 생성할 때 SYN과 FIN 플래그가 각각 1바이트의 시퀀스 공간을 차지한다는 점을 고려했습니다. 윈도우의 남은 공간, 스트림 버퍼의 보낼 데이터 남은 크기, MAX_PAYLOAD_SIZE 중 최솟값을 계산하여 페이로드를 채우도록 구현했습니다. FIN 플래그의 1번만 전송하기위해 _fin_sent 상태 변수를 도입해 관리했습니다.

    또한, 크기가 0인 유령패킷이 생기는 것을 방지하기위해 크기가 0이면 break로 탈출시켰습니다.
    
    ack_received()는 들어오는 ackno를 unwrap하여 abs_ackno로 변환한 뒤, outstanding_segment에서 이 ackno보다 작은 경우(끝위치가)는 다 제거해줬습니다. 제거가 되면 즉, 데이터가 도착이 된 의미가있는 신호라면 타이머를 초기화하고 더이상 기다리는 outstanding segment가 없으면 타이머를 꺼줬습니다.
    
    tick() 함수는 시간의 흐름을 추적하며, RTO를 초과하면 큐의 가장 오래된 패킷을 재전송합니다. 이때 혼잡 제어를 위해 윈도우 크기가 0보다 클 때만 2배로 RTO가 커지게 해줬고, RTO = 0일때는 의미가 없고 계속 호출될것이라 제외했습니다.
]

Implementation Challenges:
[
    가장 크게 헤맸던 부분은 fill_window()에서 남은 윈도우 공간(gap)을 수동으로 뺄셈하며 제어하려다 발생한 uint64_t underflow 문제였습니다. 패킷 크기를 빼는 과정에서 음수가 발생하면 엄청 큰수로 변환되어 무한 루프(Subprocess killed)가 터지는 현상을 겪었고, 이를 win > _bytes_in_flight로 매번 실시간 한계치를 비교하는 구조로 변경하여 해결했습니다.
    
    또한, 스트림에 보낼 데이터가 없을 때 SYN, FIN을 보낼때 제약을 잘 걸지않으면 FIN이 계속 무한히 붙여지는 문제가 생겨 조건을 걸어야했습니다.

]

Remaining Bugs:
[
    없습니다. 
    무한 루프 버그(처음에는 gap으로 제어하려다보니 uint64가 음수가 없기때문에 오작동하면 underflow가 되버리는 문제가 있었습니다.), 유령 패킷(0바이트 패킷) 생성 버그, 윈도우 수축 시의 프로토콜 위반 버그 등은 모두 예외 처리와 로직 구조 개선을 통해 원천 차단하여 해결하였습니다.
]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
