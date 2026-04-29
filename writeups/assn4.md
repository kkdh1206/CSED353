Assignment 4 Writeup
=============

My name: 김도형

My POVIS ID: kkdh1206

My student ID (numeric): 20210740

This assignment took me about [21] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Your benchmark results (without reordering, with reordering): [1.60, 1.67]

Program Structure and Design of the TCPConnection:
[
    TCPConnection은 TCPSender와 TCPReceiver를 결합하여 적재 적소에 _sender, _reciever 를 호출해줍니다.
    연결성을 먼저 _connected로 확인해주고 연결이 안됬을때는 SYN 패킷 이외에는 받지않습니다.
    segment_received()에서는 수신된 패킷의 RST 플래그를 가장 먼저 검사하여 비정상 종료를 처리했습니다.
    그리고 ACK데이터라면 _sender에 다음 데이터를 준비시키고 _receiver에도 데이터를 넘겨줍니다.
    그리고 나서 FIN을 받은 경우라면 linger 변수를 false로 꺼서 종료시 기다리는로직을 패스합니다.
    SYN이면 connect를 호출하고 데이터가 있는 경우 해당 데이터를 fill_window로 데이터를 너허서 보내줍니다.

    active함수는 현재 이 connection이 유지가 되는지를 기준으로 판단하게 하였고,
    tick은 sender의 tick을 호출해주고 시간을 재는용으로 사용했습니다.
    또한, MAX_RETX_ATTEMPTS를 안넘었다면 fill_window를 통해 zero probing을 해줬습니다.
]

Implementation Challenges:
[
    active를 초기에 정의를 {false}로 해뒀는데 까먹고 진행하다가 많은 테스트에서 실패를 했었습니다.
    코드 상에서 보이는 문제가 아니라서 찾는데 시간이 많이걸렸습니다.

    가장 큰 난관은 연결되지 않은 상태에서도 tick()이 호출되어서 fill_window가 계속 호출되어 문제가 생겼습니다. 이를 _sender.next_seqno_absolute() > 0 가드를 통해 통신이 시작된 경우에만 발송하도록 제한하여 해결했습니다. 

    마지막으로 ACK 생성 조건에서 데이터의 유무(buffer_empty())와 패킷의 대기 유무(segments_out().empty())를 혼동하여 문제가 생겼으나 해결했습니다.
]

Remaining Bugs:
[
    없습니다. 
    keep-alive를 제대로 케이스 처리하지않아서 테스트중 한개가 랜덤으로 통과를 했지만, 수정한 이후 안정적으로 통과가 되었습니다.
]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
