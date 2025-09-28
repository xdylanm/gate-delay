#include "adc_to_delay.h"

#include <fstream>
#include <iostream>

/*
    \brief Run the conversion from ADC value to delay in ticks, write to CSV for comparison
*/
int main() {

    std::ofstream csv_file("adc_to_delay_results.csv");
    csv_file << "adc_val,delay\n";
    for (uint16_t adc_val = 0; adc_val <= 1023; ++adc_val) {
        uint32_t delay = adc_to_delay(adc_val);
        csv_file << adc_val << "," << delay << "\n";
    }
    csv_file.close();
    std::cout << "Results written to adc_to_delay_results.csv\n";


    return 0;
}