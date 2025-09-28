
#include <iostream>
#include <cstdint>

uint8_t timer_lo_byte;
uint8_t timer_hi_byte;

void timer_overflow_handler() 
{
    timer_hi_byte += 1;
}

/*
    \brief A simple test bench to confirm overflow handling
*/
int main() 
{


    for (int i = 0; i < 1000; ++i ) {
        timer_lo_byte++;
        if (timer_lo_byte == 0) {
            timer_overflow_handler();
        }
    }

    // uint16_t timer_result = timer_hi_byte;
    // timer_result <<= 8;
    // timer_result |= timer_lo_byte;
    uint16_t timer_result = (static_cast<uint16_t>(timer_hi_byte) << 8) | timer_lo_byte;
    
    std::cout << "After 1000 steps: " << timer_result << std::endl;

    std::cout << "  hi: " << int(timer_hi_byte) << std::endl;
    std::cout << "  lo: " << int(timer_lo_byte) << std::endl;
    uint8_t large_stride = 255 - timer_lo_byte;
    large_stride += 3;
    uint8_t next_time = timer_lo_byte + large_stride;
    if (next_time < timer_lo_byte) {
        std::cout << "Overflow detected OK, next_time = " << int(next_time) << " (expected 2)" << std::endl;
    } else {
        std::cout << "ERROR: Overflow not detected" << std::endl;
    }


    return 0;
}