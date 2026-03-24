Assignment 2 Writeup
=============

My name: 김도형

My POVIS ID: kkdh1206

My student ID (numeric): 20210740

This assignment took me about [n] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPReceiver and wrap/unwrap routines:
[]

Implementation Challenges:
[]

Remaining Bugs:
[
unwrap(WrappingInt32(UINT32_MAX), WrappingInt32(0), 0)` should have been `static_cast<uint64_t>(UINT32_MAX)`, but the former is
        18446744073709551615
and the latter is
        4294967295 (difference of 4294967296)
 (at line 25)


]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
