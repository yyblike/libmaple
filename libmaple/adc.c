/******************************************************************************
 * The MIT License
 *
 * Copyright (c) 2010 Perry Hung.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *****************************************************************************/

/**
 * @brief Analog to digital converter routines
 */

#include "libmaple.h"
#include "rcc.h"
#include "adc.h"

/* The ADC input clock is generated from PCLK2/APB2 divided by a prescaler
 * and it must not exceed 14MHz.
 *
 * ADC1 and ADC2 are clocked by APB2
 *
 * 1) Power on by setting ADON in ADC_CR2
 * Conversion starts when ADON is set for a second time after some
 * time t > t_stab.
 *
 * Up to 16 selected conversion must be selected in ADC_SQRx
 *
 * Single conversion mode:
 * Set the ADON bit in the ADC_CR2 register
 * Once the conversion is complete:
 *  Converted data is stored in ADC_DR
 *  EOC flag is set
 *  Interrupt is generated if EOCIE is set
 *
 * Calibration:
 * Calibration is started by setting the CAL bit in the ADC_CR2 register.
 * Once calibration is over, the CAL bit is reset by hardware and normal
 * conversion can be performed. Calibrate at power-on.
 *
 * ALIGN in ADC_CR2 selects the alignment of data
 *
 * IMPORTANT: maximum external impedance must be below 0.4kOhms for 1.5
 * sample conversion time.
 *
 * At 55.5 cycles/sample, the external input impedance < 50kOhms*/

void set_adc_smprx(adc_smp_rate smp_rate);

void adc_init(adc_smp_rate smp_rate) {
    rcc_set_prescaler(RCC_PRESCALER_ADC, RCC_ADCPRE_PCLK_DIV_6);
    rcc_clk_enable(RCC_ADC1);
    rcc_reset_dev(RCC_ADC1);

    ADC_CR1  = 0;
    /* Software triggers conversions */
    ADC_CR2  = CR2_EXTSEL_SWSTART | CR2_EXTTRIG;
    ADC_SQR1 = 0;

    /* Set the sample conversion time.  See note above for impedance
       requirements. */
    adc_set_sample_rate(smp_rate);

    /* Enable the ADC */
    CR2_ADON_BIT = 1;

    /* Reset the calibration registers and then perform a reset */
    CR2_RSTCAL_BIT = 1;
    while(CR2_RSTCAL_BIT)
        ;

    CR2_CAL_BIT = 1;
    while(CR2_CAL_BIT)
        ;
}


void adc_disable(void) {
    CR2_ADON_BIT = 0;
}

/* Turn the given sample rate into values for ADC_SMPRx.  (Don't
 * precompute in order to avoid wasting space).
 *
 * Don't call this during conversion!
 */
void adc_set_sample_rate(adc_smp_rate smp_rate) {
    uint32 adc_smpr1_val = 0, adc_smpr2_val = 0;
    int i;
    for (i = 0; i < 10; i++) {
        if (i < 8) {
            /* ADC_SMPR1 determines sample time for channels [10,17] */
            adc_smpr1_val |= smp_rate << (i * 3);
        }
        /* ADC_SMPR2 determines sample time for channels [0,9] */
        adc_smpr2_val |= smp_rate << (i * 3);
    }
    ADC_SMPR1 = adc_smpr1_val;
    ADC_SMPR2 = adc_smpr2_val;
}
