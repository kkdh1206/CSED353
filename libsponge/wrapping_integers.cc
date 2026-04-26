#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    DUMMY_CODE(n, isn);
    return WrappingInt32(isn.raw_value() +
                         static_cast<uint32_t>(n));  // 걍 n의 32비트 뒷쪽만 자르고 남긴후에 isn값더해줌
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    DUMMY_CODE(n, isn, checkpoint);
    // 후보1
    uint64_t cand1 = ((checkpoint >> 32) << 32) +
                     static_cast<uint64_t>(n.raw_value() - isn.raw_value());  // 아 msb음수는 아니겟지 뭐상관없을듯
    // 후보2
    uint64_t cand2 =
        (((checkpoint >> 32) << 32) + (1ull << 32)) + static_cast<uint64_t>(n.raw_value() - isn.raw_value());
    // 후보3
    uint64_t cand3 =
        (((checkpoint >> 32) << 32) - (1ull << 32)) + static_cast<uint64_t>(n.raw_value() - isn.raw_value());

    if (cand1 < checkpoint) {  // 오버플로우 방지용
        if (cand2 - checkpoint > checkpoint - cand1) {
            return cand1;
        } else {
            return cand2;
        }
    } else {
        if (cand1 - checkpoint >
            checkpoint - cand3) {  // 음수면안되니까 - 이러면 음수에서 오버플로떠버려서 음수개념없어서 문제생기는거 막음 - 지각한 케이스
            if (checkpoint >= cand3) {
                return cand3;
            } else {
                return cand1;
            }
        } else {
            return cand1;
        }
    }
}
