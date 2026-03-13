#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) { // 데이터 들어오는거 처리
    size_t lower = this.last_index; // 여기서부터 채워나가야함
    size_t upper = lower + this.capacity; // 이걸로 존재할수있는 영역 바운드
    string local_data = data;
    size_t local_index = index;
    // 내 앞 map 만 겹치는지 보면됨 왜냐면 서로 안겹치니까 그리고 내 바로 뒷 map과 겹치면 거기까지만 저장

    if (index >= upper || index + data.length() <= lower) {
        return; // 필요없는 범위라 버림
    }    


    if (index+local_data.length()>upper) // 튀어나오는거 먼저 자르자
    {
        local_data = local_data.substr(0, upper-index); // upper 전까지만 되야하니까
    }

    if (index < lower){
        local_data = local_data.substr(lower - index, local_data.length())
        local_index = lower;
    } 
    auto it = assembling_storage.lower_bound(local_index); // 이거보다 큰것 중 작은 걸 찾음
    if (it != assembling_storage.begin()) { //  만약 그게 map중 젤 작은게 아니라면 앞에 뭔가 있음 
        auto prev_it = std::prev(it); // 바로 앞 조각
        const string prev_string = prev_it->second;
        if (prev_it->first + prev_string.length() >= local_index + local_data.length()){ // 이미 다 중복되버린경우
            return; // 이러면 더이상 넣을거없어서 skip
        }
        if (prev_it->first + prev_string.length() > local_index){ // 기존 index보다 크면 잘라야함
            local_data = local_data.substr(prev_it->first + prev_string.length()-local_index,local_data.length());
            local_index = prev_it->first + prev_string.length();
        }
    }
    while (it != assembling_storage.end()){ // 마지막 까지 확인
        // map 에 넣어주기 이제부터 뒤에 겹치는건 다지움
        uint64_t next_start = it->first;
        uint64_t next_end = next_start + it->second.length();
        uint64_t my_end = local_index + local_data.length();

        if (my_end >= next_end){ // 포함되는경우 지우고 이동
            storage_bytes -= it->second.length();
            it = assembling_storage.erase(it);
        }
     
        else if (my_end > next_start) { // 걸치는 경우 전까지 자르고 종료
            local_data = local_data.substr(0, next_start - local_index);
            break; 
        } 
        else{
            break; // 둘다 아니면 할거 없으니 종료
        }
    }
    if (!local_data.empty()) { // data남아있을때
        assembling_storage[local_index] = local_data;
        storage_bytes += local_data.length(); // 넣어줄 바이트 수
}

    // last index인 데이터가 오면 읽음 처리해서 last index없데이트하고 storage_byte도 처리되야함

    // stitch 그니까 중복되는거 떼고 잘 붙이면서 넣고 storage_bytes 잘 업데이트 해야함
    // auto it = assembling_storage.lower_bound(last_index);
    while(!assembling_storage.empty() && last_index == assembling_storage.begin()->first){
       auto head = assembling_storage.begin(); // 제일 앞 잡음
        _output.write(head->second); 
        last_index += head->second.length(); // 지우기 전에 길이와 값을 다 계산
        storage_bytes -= head->second.length(); 
        
        assembling_storage.erase(head); // 마지막에 깔끔하게 삭제
    }

    // eof 처리해줘야함

    if (_is_eof_set && last_index == _eof_idx) {
        _output.end_input();
    }

    
}

size_t StreamReassembler::unassembled_bytes() const { return {this.storage_bytes}; }

bool StreamReassembler::empty() const { return {this.storage_bytes == 0}; } // 저장된거 없으면 참
