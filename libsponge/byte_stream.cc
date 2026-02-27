#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) { this->buffer_capacity = capacity; }

size_t ByteStream::write(const string &data) {
    size_t remain = this->remaining_capacity();
    if (remain >= data.length()) {
        for (size_t i = 0; i < data.length(); i++) {
            this->buffer.push_back(data[i]);
        }
        this->written_byte += data.length();
        return data.length();

    } else {
        for (size_t i = 0; i < remain; i++) {
            this->buffer.push_back(data[i]);
        }
        this->written_byte += remain;
        return remain;  // 근데 상수 안쓰면 중간에 바뀔수도있으려나?
    }
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    if (len > this->buffer_size()) {
        return {buffer.begin(), buffer.end()};  // 문자열반환
    }
    return {buffer.begin(), buffer.begin() + len};
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t size = buffer_size();
    if (len > size) {
        for (size_t i = 0; i < size; i++) {
            this->buffer.pop_front();
            this->read_byte += 1;
        }
    } else {
        for (size_t i = 0; i < len; i++) {
            this->buffer.pop_front();
            this->read_byte += 1;
        }
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {  // 버퍼를 읽음
                                                  //     size_t size = buffer_size();
                                                  //     if (len>size){
                                                  //         this->read_byte += size;
                                                  //     } else{this->read_byte += len;}
    string result = this->peek_output(len);       // read_byte 설정후 peek랑 pop함
    this->pop_output(len);

    return result;
}

void ByteStream::end_input() { this->end = true; }

bool ByteStream::input_ended() const { return {this->end}; }

size_t ByteStream::buffer_size() const { return {this->buffer.size()}; }

bool ByteStream::buffer_empty() const { return this->buffer.empty(); }

bool ByteStream::eof() const { return (this->buffer_size() == 0) && this->end; }

size_t ByteStream::bytes_written() const { return {this->written_byte}; }

size_t ByteStream::bytes_read() const { return {this->read_byte}; }

size_t ByteStream::remaining_capacity() const { return this->buffer_capacity - this->buffer_size(); }
