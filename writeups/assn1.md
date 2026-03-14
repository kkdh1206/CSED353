Assignment 1 Writeup
=============

My name: 김도형

My POVIS ID: kkdh1206

My student ID (numeric): 20210740

This assignment took me about [8] hours to do (including the time on studying, designing, and writing the code).

Program Structure and Design of the StreamReassembler:
[
  핵심 자료구조: 순서가 뒤섞인 세그먼트를 효율적으로 처리하기 위해 `std::map`을 
  선택하여 리스트의 인덱스 이동 오버헤드를 방지했습니다. (빈칸오버헤드도 방지가능)

  윈도우 경계 자르기: 허용 가능한 윈도우 범위를 벗어나는 데이터는 즉시 초반에 잘라냅니다.
 
  중복 데이터 처리: `map`에 저장된 조각들과 겹치는지 확인합니다. 틈새에 끼워 
  넣는 대신 겹치는 부분을 잘라내거나 완전히 포함된 조각을 삭제한뒤 앞에 input data를 넣어 대체 했습니다. 
  이때 문자열 앞부분이 잘리면 논리적 시작 `index`도 그만큼 밀어주어 동기화했습니다.
  
  순차적 배출: 삽입 후, `map`의 가장 앞 조각이 대기 중인 순서와 일치하면 
  `ByteStream`에 연속해서 기록하고 `map`에서 삭제합니다.

  EOF는 미리 종료 전에 올수도있기 때문에, 저장이 된다면 미리 기록해두고 last_index가 마지막에 닿으면 종료합니다.
]

Implementation Challenges:
[
  Stitching 정책: 중복 처리가 까다로웠으나, 덮어쓰기/자르기 위주로 
  로직을 설계하여 단순화했습니다. (겹치면 뒤에거면 삭제하고 새로 덮어쓰도록)
  
  EOF 처리: 윈도우 밖으로 잘려 나간 꼬리 부분에 달린 가짜 EOF를 걸러내고, 
  정확한 종료 시점을 기록하는 예외 처리가 까다로웠습니다. 특히, 빈문자열에 EOF가 엣지케이스였습니다.
  
  Capacity 잘못된 이해: 초기엔 `ByteStream`으로 데이터를 `write`하면 
  즉시 Reassembler 공간이 빈다고 착각했습니다. 이후 `capacity`가 전체 공유 
  메모리이며, 응용 프로그램이 `read`를 해야만 윈도우(`upper`)가 전진한다는 
  것을 깨닫고 로직을 수정했습니다.
]

Remaining Bugs:
[
    남은 버그는 없습니다. 이전에는 길이가 0이고 EOF 플래그만 있는 세그먼트가 
  입구컷 조건문에 걸려 무시되는 엣지 케이스가 있었으나, 부등호 조건을 
  `<`로 수정하여 빈 문자열 형태의 EOF도 정상적으로 처리하도록 해결했습니다.
]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
