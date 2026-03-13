Assignment 1 Writeup
=============

My name: 김도형

My POVIS ID: kkdh1206

My student ID (numeric): 20210740

This assignment took me about [n] hours to do (including the time on studying, designing, and writing the code).

Program Structure and Design of the StreamReassembler:
[
    바로 그때그때 접근이 되야하니까 리스트나 map을 쓰는게 좋은데 리스트는 오버헤드 클것 같아서 map으로 자료구조를 설정함
    capacity크기 만큼의 map을 만들고 채워넣으면서 있다가 읽게되도 map은 list와 달리 인덱스 당기지 않고 걍삭제만 해도됨

    처음 capacity만큼 잘라주고 양끝을 그리고
    앞부분에 map이 존재하면 그만큼 자르고
    뒷부분부터 이제 Map 저장해나가면서 겹치는건 걍 다지워버리고 애매하게 겹칠때 처리하면서 멈춤
    이때 자르고 나서 index도 옮겨줘야 에러가 없음

    

]

Implementation Challenges:
[]

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
