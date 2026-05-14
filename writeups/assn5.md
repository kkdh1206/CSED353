Assignment 5 Writeup
=============

My name: 김도형

My POVIS ID: kkdh1206

My student ID (numeric): 20210740

This assignment took me about [18] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the NetworkInterface:
[
    NetworkInterface는 ip계층과 이더넷을 연결하며 ARP를 통해 ip 주소에 대응하는 MAC 주소를 확인합니다.
    
    send_datagram 은 송신할 데이터그램의 Next Hop IP가 ARP 캐시에 있는지 확인합니다. 
    존재하면 즉시 전송하고, 없으면 해당 IP를 키로 리스틀르 저장하는 _waiting_frames_out에 패킷을 저장한 뒤 ARP Request를 보냅니다.
    
    recv_frame은 수신된 프레임을 처리합니다. IPv4인 경우 데이터그램을 반환하고, ARP인 경우 필요한 정보를 저장합니다. 
    특히 REPLY 수신 시 해당 ip로 _waiting_frames_out의 리스트 패킷들에 상대 MAC주소를 채워 즉시 발송하며,
    REQUEST 수신 시에는 본인의 정보를 담아 REPLY를 보냅니다.
    
    tick()은 5초마다 ARP 재전송 제한을 관리하고 30초가 지난 캐시 항목을 삭제합니다.
]

Implementation Challenges:
[
    참조와 값 복사의 혼동 
    대기열에서 패킷 리스트를 꺼낼 때 참조(&)를 누락하여 복사본을 수정하는 실수가 있었습니다. 
    이로 인해 원본 맵 데이터가 삭제되지 않고 쌓이는 문제가 발생했으나, &를 사용해 원본에 접근하여 바로 삭제를 했습니다.
    
    데이터 파싱
    파싱 시 concatenate, serialize 누락으로 인한 예외가 발생했고 추가하여 해결했습니다.
]

Remaining Bugs:
[
    없습니다.
]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
