/******************************************************************************
 * Copyright (c) 2014 by Silicon Laboratories Inc. All rights reserved.
 *
 * http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
 *****************************************************************************/

#ifndef __PCA_0_H__
#define __PCA_0_H__


#include "efm8_config.h"
#include "SI_EFM8BB1_Register_Enums.h"

/**************************************************************************//**
 * @addtogroup pca_0 PCA0 Driver
 * @{
 *
 * @brief Peripheral driver for PCA0
 *
 * # Introduction #
 *
 * This module contains all the driver content for PCA0
 *
 * @warning The WDT0 module on this device interacts with the PCA.
 *
 * ### Memory Usage ###
 *
 * The table below shows the memory consumption of the library with various
 * options. The 'default' entry shows the consumption when most or all available
 * functions are called. Typical consumption is expected to be less than this
 * since there are normally many uncalled functions that will consume no
 * resources.
 *
 * @note It is possible for memory usage to exceed the listed values in rare cases
 *
 * | condition          | CODE | XRAM | IRAM | RAM |
 * |--------------------|------|------|------|-----|
 * |default             |  504 |    0 |    0 |   0 |
 *
 * # Theory of Operation #
 *
 * The Programable counter array provides 3 main catagories of functionality.
 *
 * - PWM
 * - Counter/capture
 * - Timer
 *
 * It consists of a single counter/timer and several channels which all operate
 * using the single counter.
 *
 * ### Timer operations ###
 *
 * The PCA can be used to provide basic timer operations. The timer period
 * is determined by the timer configuration. Each PCA channel may fire an
 * interrupt at an independent point in the period. For example We may set up
 * the timer to overflow (and provide an interrupt) every 100 cycles. We can
 * then configure on channel to provide an interrupt at 90 cycles and another
 * at 50 cycles. This would generate a 'half-way' and 'early-warning'
 * interrupts.
 *
 * ### PWM operations ###
 *
 * The PCA can be configured to output PWM waveforms (or square waveforms).
 * The period of the PWM is controlled by the time an thus the same for all
 * channels. The duty cycle and polarity is controlled by each channel
 * independently.
 *
 * A related mode is high frequency output mode where each PCA channel can be
 * configured to toggle it's output ever N ticks of the timer. This allows
 * each module to independently generate a digital frequency.
 *
 * ### Capture operations ###
 *
 * The state of the timer can be captured by a PCA channel based on several
 * triggers. The captured value can then be used for various operations.
 *
 * For example if channel 0 captures on the rising edge of a signal and
 * channel 0 captures on the falling edge of that same signal then the
 * high pulse with time can be calculated by the equation.
 *    time = (ch1_value - ch0_value) * period_of_timer
 *
 * Another mode of operation is to
 *
 * ### Watch Dog Timer ###
 *
 * While the Peripheral Driver Library defines a separate module for interacting with the
 * watch-dog timer the WDT is physically located in channel 4 of the PCA on this device.
 * Any changes to the counter in the WDT or PCA block will effect the other block. In
 * addition when the WDT is in used PCA channel 4 is unavailable.
 *
 * Runtime asserts have been placed in this module to help highlight cases where the
 * WDT and PCA interfere with one another. However it is not possible to catch all
 * issues and the user needs to be aware of this interaction.
 *
 * ### Hardware Configuration ###
 *
 * This driver provides basic facilities for configuring the PCA.
 * it is recommended that Simplicity Hardware Configurator be for more
 * advanced configuration as it provides more comprehensive validation
 * of user selections.
 *
 *****************************************************************************/

//Option macro documentation
/**************************************************************************//**
 * @addtogroup pca0_config Driver Configuration
 * @{
 *****************************************************************************/

/**************************************************************************//**
 * @def EFM8PDL_PCA0_USE_ISR
 * @brief Controls inclusion of PCA0 ISR and associated callbacks.
 *
 * When '1' the PCA0 ISR in the driver is used and callbacks are functional.
 *
 * Default setting is '1' and may be overridden by defining in 'efm8_config.h'.
 *
 *****************************************************************************/
/**  @} (end addtogroup pca0_config Driver Configuration) */

//Configuration defaults
#ifndef IS_DOXYGEN
  #define IS_DOXYGEN 0
#endif

#ifndef EFM8PDL_PCA0_USE_ISR
  #define EFM8PDL_PCA0_USE_ISR 1
#endif

// Runtime API
/**************************************************************************//**
 * @addtogroup pca0_runtime PCA0 Runtime API
 * @{
 *****************************************************************************/

/// PCA Channel Enumeration
typedef enum
{
  PCA0_CHAN0 = 0x0,         //!<  PCA Channel 0
  PCA0_CHAN1 = 0x1,         //!<  PCA Channel 1
  PCA0_CHAN2 = 0x2,         //!<  PCA Channel 2

#if IS_DOXYGEN
  PCA0_CHAN3 = -1,          //!< NOT SUPPORTED ON THIS DEVICE
  PCA0_CHAN4 = -1,          //!< NOT SUPPORTED ON THIS DEVICE
  PCA0_CHAN5 = -1,          //!< NOT SUPPORTED ON THIS DEVICE
#endif
} PCA0_Channel_t;

/**************************************************************************//**
 * @addtogroup pca0_if Interrupt Flag Enums
 * @{
 ******************************************************************************/
#define PCA0_OVERFLOW_IF  PCA0CN0_CF__BMASK    //!<  Counter overflow flag
#define PCA0_IOVERFLOW_IF PCA0PWM_COVF__BMASK //!<  Intermediate overflow flag
#define PCA0_CHAN0_IF     PCA0CN0_CCF0__BMASK  //!<  Channel 0
#define PCA0_CHAN1_IF     PCA0CN0_CCF1__BMASK  //!<  Channel 1
#define PCA0_CHAN2_IF     PCA0CN0_CCF2__BMASK  //!<  Channel 2

#if IS_DOXYGEN
  #define PCA0_CHAN3_IF -1 //!< @warning NOT SUPPORTED ON THIS DEVICE
  #define PCA0_CHAN4_IF -1 //!< @warning NOT SUPPORTED ON THIS DEVICE
  #define PCA0_CHAN5_IF -1 //!< @warning NOT SUPPORTED ON THIS DEVICE
#endif
/** @} (end addtogroup pca0_if Interrupt Flag Enums) */

/***************************************************************************//**
 * @brief
 * Return the value of the interrupt flags
 *
 * @return
 * The state of the flags. This value is the OR of all flags which are set.
 *
 * Valid enums can be fond in the Interrupt Flag Enums group.
 *
 ******************************************************************************/
uint8_t PCA0_getIntFlags();

/***************************************************************************//**
 * @brief
 * Clear the specified status flag
 *
 * @param flag:
 * Flag to clear. Multiple flags can be cleared by OR-ing the flags.
 *
 * Valid enums can be found in the Interrupt Flag Enums group.
 ******************************************************************************/
void PCA0_clearIntFlag(uint8_t flag);

/***************************************************************************//**
 * @brief
 * Enable or disable the PCA interrupts
 *
 * @param flag:
 * Interrupt to be enabled/disabled
 * @param enable:
 * New target status (true to enable)
 *
 * Multiple interrupts may be enabled/disabled at a time by or-ing their flags
 *
 * Valid enums can be found in the Interrupt Flag Enums group.
 *
 ******************************************************************************/
void PCA0_enableInt(uint8_t flag, bool enable);

/***************************************************************************//**
 * @brief
 * Read the channel capture/compare register
 *
 * @param channel:
 * Channel to read.
 *
 * @return
 * Current value of capture/compare register for the specified channel.
 *
 ******************************************************************************/
uint16_t PCA0_readChannel(PCA0_Channel_t channel);

/***************************************************************************//**
 * @brief
 * Write the channel capture.compare register
 *
 * @param channel:
 * Channel to read.
 * @param value:
 * Value to write to capture/compare register.
 *
 ******************************************************************************/
void PCA0_writeChannel(PCA0_Channel_t channel, uint16_t value);

/***************************************************************************//**
 * @brief
 * Read the PCA counter.
 *
 * @return
 * Current value of the PCA counter.
 ******************************************************************************/
uint16_t PCA0_readCounter();

/***************************************************************************//**
 * @brief
 * Write to the PCA counter.
 *
 * @param value:
 * Value to write to PCA counter.
 *
 ******************************************************************************/
void PCA0_writeCounter(uint16_t value);

/***************************************************************************//**
 * @brief
 * Start the PCA Counter.
 *
 ******************************************************************************/
void PCA0_run();

/***************************************************************************//**
 * @brief
 * Stop the PCA Counter.
 *
 ******************************************************************************/
void PCA0_halt();

/** @} (end addtogroup pca0_runtime PCA0 Runtime API) */

// Initialization API
/**************************************************************************//**
 * @addtogroup pca0_init PCA0 Initialization API
 * @{
 *****************************************************************************/

/// @brief Clock Selection Enum.
typedef enum
{
  PCA0_SYSCLK_DIV12 = PCA0MD_CPS__SYSCLK_DIV_12, //!< Select SystemClock/12
  PCA0_SYSCLK_DIV4 = PCA0MD_CPS__SYSCLK_DIV_4,   //!< Select SystemClock/4
  PCA0_TIMER0      = PCA0MD_CPS__T0_OVERFLOW,    //!< Select Timer0 Overflow
  PCA0_ECI         = PCA0MD_CPS__ECI,            //!< Select ECI falling edge
  PCA0_SYSCLK      = PCA0MD_CPS__SYSCLK,         //!< Select SystemClock
  PCA0_EXTOSC_DIV8 = PCA0MD_CPS__EXTOSC_DIV_8,   //!< Select ExternalOsc/8
  PCA0_LFOSC_DIV8  = PCA0MD_CPS__LFOSC_DIV_8,    //!< Select LowFrequencyOsc/8
} PCA0_Timebase_t;

/// @brief Channel mode enum.
typedef enum
{
  //xx10 000* (CPM) 0x20
  //0x*0 0xxx (PWM)
  PCA0_CAPTURE_POS_CEX   = PCA0CPM0_PWM16__8_BIT //!< Capture mode triggered by CEX rising
                           | PCA0CPM0_ECOM__DISABLED
                           | PCA0CPM0_CAPP__ENABLED
                           | PCA0CPM0_CAPN__DISABLED
                           | PCA0CPM0_MAT__DISABLED
                           | PCA0CPM0_TOG__DISABLED
                           | PCA0CPM0_PWM__DISABLED,

  //xx01 000* (CPM) 0x10
  //0x*0 0xxx (PWM)
  PCA0_CAPTURE_NEG_CEX   = PCA0CPM0_PWM16__8_BIT //!< Capture mode triggered by CEX falling
                           | PCA0CPM0_ECOM__DISABLED
                           | PCA0CPM0_CAPP__DISABLED
                           | PCA0CPM0_CAPN__ENABLED
                           | PCA0CPM0_MAT__DISABLED
                           | PCA0CPM0_TOG__DISABLED
                           | PCA0CPM0_PWM__DISABLED,

  //xx11 000* (CPM) 0x30
  //0x*0 0xxx (PWM)
  PCA0_CAPTUE_TOGGLE_CEX = PCA0CPM0_PWM16__8_BIT //!< Capture Mode triggered by CEX rising or falling
                           | PCA0CPM0_ECOM__DISABLED
                           | PCA0CPM0_CAPP__ENABLED
                           | PCA0CPM0_CAPN__ENABLED
                           | PCA0CPM0_MAT__DISABLED
                           | PCA0CPM0_TOG__DISABLED
                           | PCA0CPM0_PWM__DISABLED,

  //x100 100* (CPM) 0x48
  //0x*0 0xxx (PWM)
  PCA0_TIMER             = PCA0CPM0_PWM16__8_BIT //!< Timer mode
                           | PCA0CPM0_ECOM__ENABLED
                           | PCA0CPM0_CAPP__DISABLED
                           | PCA0CPM0_CAPN__DISABLED
                           | PCA0CPM0_MAT__ENABLED
                           | PCA0CPM0_TOG__DISABLED
                           | PCA0CPM0_PWM__DISABLED,

  //x100 110* (CPM) 0x4C
  //0x*0 0xxx (PWM)
  PCA0_HIGH_SPEED_OUT    = PCA0CPM0_PWM16__8_BIT //!< High speed output mode
                           | PCA0CPM0_ECOM__ENABLED
                           | PCA0CPM0_CAPP__DISABLED
                           | PCA0CPM0_CAPN__DISABLED
                           | PCA0CPM0_MAT__ENABLED
                           | PCA0CPM0_TOG__ENABLED
                           | PCA0CPM0_PWM__DISABLED,

  //x100 011* (CPM) 0x46
  //0x*0 0xxx (PWM)
  PCA0_FREQUENCY_OUT     = PCA0CPM0_PWM16__8_BIT //!< High frequency output mode
                           | PCA0CPM0_ECOM__ENABLED
                           | PCA0CPM0_CAPP__DISABLED
                           | PCA0CPM0_CAPN__DISABLED
                           | PCA0CPM0_MAT__DISABLED
                           | PCA0CPM0_TOG__ENABLED
                           | PCA0CPM0_PWM__ENABLED,

  //0111 101* (CPM) 0x4A
  //10*0 0000 (PWM) 0x80
  PCA0_PWM8              = PCA0CPM0_PWM16__8_BIT //!< 8-bit PWM (edge aligned)
                           | PCA0CPM0_ECOM__ENABLED
                           | 0x30 //Code for n-bit PWM
                           | PCA0PWM_CLSEL__8_BITS,

  PCA0_PWM8_CENTER       = PCA0CPM0_PWM16__8_BIT //!< 8-bit PWM (center aligned)
                           | PCA0CPM0_ECOM__ENABLED
                           | 0x30 //Code for n-bit PWM
                           | PCA0PWM_CLSEL__8_BITS
                           | 0x08, //center alignment

  //0111 101* (CPM) 0x4A
  //10*0 0001 (PWM) 0x81
  PCA0_PWM9              = PCA0CPM0_PWM16__8_BIT //!< 9-bit PWM (edge aligned)
                           | PCA0CPM0_ECOM__ENABLED
                           | 0x30 //Code for n-bit PWM
                           | PCA0PWM_CLSEL__9_BITS,

  PCA0_PWM9_CENTER       = PCA0CPM0_PWM16__8_BIT //!< 9-bit PWM (center aligned)
                           | PCA0CPM0_ECOM__ENABLED
                           | 0x30 //Code for n-bit PWM
                           | PCA0PWM_CLSEL__9_BITS
                           | 0x08, //center alignment

  //0111 101* (CPM) 0x4A
  //10*0 0010 (PWM) 0x82
  PCA0_PWM10             = PCA0CPM0_PWM16__8_BIT //!< 10-bit PWM (edge aligned)
                           | PCA0CPM0_ECOM__ENABLED
                           | 0x30 //Code for n-bit PWM
                           | PCA0PWM_CLSEL__10_BITS,

  PCA0_PWM10_CENTER      = PCA0CPM0_PWM16__8_BIT //!< 10-bit PWM (center aligned)
                           | PCA0CPM0_ECOM__ENABLED
                           | 0x30 //Code for n-bit PWM
                           | PCA0PWM_CLSEL__10_BITS
                           | 0x08, //center alignment

  //0111 101* (CPM) 0x4A
  //10*0 0011 (PWM) 0x83
  PCA0_PWM11             = PCA0CPM0_PWM16__8_BIT //!< 11-bit PWM (edge aligned)
                           | PCA0CPM0_ECOM__ENABLED
                           | 0x30 //Code for n-bit PWM
                           | PCA0PWM_CLSEL__11_BITS,

  PCA0_PWM11_CENTER      = PCA0CPM0_PWM16__8_BIT //!< 11-bit PWM (center aligned)
                           | PCA0CPM0_ECOM__ENABLED
                           | 0x30 //Code for n-bit PWM
                           | PCA0PWM_CLSEL__11_BITS
                           | 0x08, //center alignment

  //1111 101* (CPM) 0xCA
  //00*0 0xxx (PWM) 0x00
  PCA0_PWM16             = PCA0CPM0_PWM16__16_BIT //!< 16-bit PWM (edge aligned)
                           | PCA0CPM0_ECOM__ENABLED
                           | 0x30 //Code for n-bit PWM
                           | PCA0PWM_CLSEL__11_BITS,
                           //1111 101* (CPM) 0xCA
                           //00*0 0xxx (PWM) 0x00

  PCA0_PWM16_CENTER      = PCA0CPM0_PWM16__16_BIT //!< 16-bit PWM (center aligned)
                           | PCA0CPM0_ECOM__ENABLED
                           | 0x30 //Code for n-bit PWM
                           | PCA0PWM_CLSEL__11_BITS
                           | 0x08, //center alignment
} PCA0_ChannelMode_t;


/// @brief Output Polarity enum.
typedef enum
{
  PCA0_NORMAL_POLARITY   = 0x0, //!< normal channel out
  PCA0_INVERT_POLARITY   = 0x1, //!< normal channel out
} PCA0_ChannelOutPolatiry_t;

/// @brief Idle state of PCA Counter
typedef enum
{
  PCA0_IDLE_RUN     = PCA0MD_CIDL__NORMAL,  //!< PCA runs when idle
  PCA0_IDLE_SUSPEND = PCA0MD_CIDL__SUSPEND, //!< PCA suspended when idle
} PCA0_IdleState_t;


/***************************************************************************//**
 * @brief Initialize the PCA
 *
 * @param timebase:
 * Timebase selection.
 * @param idleState:
 * Idle state selection.
 *
 ******************************************************************************/
void PCA0_init(PCA0_Timebase_t  timebase, PCA0_IdleState_t idleState);

/**************************************************************************//**
 * @brief
 * Initialize the PCA Channel.
 *
 * @param channel:
 * The channel to initialize.
 * @param mode:
 * The desired mode for this channel.
 * @param pol:
 * Desired output polarity for this channel.
 *
 * This function initialized the PCA channel including setting up
 * the shared PCA0PWM register.
 *
 * @warning 
 * All channels in 8-11 bit PWM mode must be in the same
 * mode. You can not use 8-bit PWM for one channel and 11-bit PWM
 * for another. The N-bit PWM mode will be set to whatever was
 * specified in the last call to PCA0_init Channel.
 *
 *****************************************************************************/
void PCA0_initChannel(PCA0_Channel_t channel,
                      PCA0_ChannelMode_t  mode,
                      PCA0_ChannelOutPolatiry_t pol
                      );
/***************************************************************************//**
 * @brief
 *   Restore the PCA to it's uninitialized (reset) state.
 *
 * This function restores the entire PCA INCLUDING ALL CHANNELS to default values.
 ******************************************************************************/
void PCA0_reset();

/***************************************************************************//**
 * @brief
 * Restore the PCA Channel to it's uninitialized (reset) state.
 *
 * @param channel:
 * Channel to reset;
 *
 * This function restores only the specified channels to the reset state.
 ******************************************************************************/
void PCA0_resetChannel(PCA0_Channel_t channel);

/** @} (end addtogroup pca0_init PCA0 Initialization API) */

//=========================================================
// ISR API
//=========================================================
#if (EFM8PDL_PCA0_USE_ISR == 1) || IS_DOXYGEN
/**************************************************************************//**
 * @def void PCA0_ISR()
 * @brief PCA Interrupt handler (not a callback).
 *
 * This ISR is provided by the library when EFM8PDL_PCA0_USE_ISR = "1".
 *
 *****************************************************************************/
#endif //EFM8PDL_PCA0_USE_ISR

// Callbacks
/**************************************************************************//**
 * @addtogroup pca0_callback User Callbacks
 * @{
 *****************************************************************************/
#if (EFM8PDL_PCA0_USE_ISR == 1) || IS_DOXYGEN

/***************************************************************************//**
 * @addtogroup pca0_callbacks_isr ISR API
 * @{
 *
 * These callbacks will be called by the library when
 * EFM8PDL_PCA0_USE_ISR. If the ISR Api is disabled
 * the callbacks do not need to be provided by the user.
 *
 *****************************************************************************/

/***************************************************************************//**
 * @brief
 * Callback for overflow of PCA.
 *
 * This function is defined by the user and called by the peripheral driver the PCA counter overflows.
 *
 * @warning
 * This function is called from an ISR and should be as short as possible.
 *
 ******************************************************************************/
extern void PCA0_overflowCb();

/***************************************************************************//**
 * @brief
 * Callback for intermediate overflow of PCA.
 *
 * This function is defined by the user and called by the peripheral driver the
 * PCA counter half way point is reached as defined by the N-bit PWM settings.
 *
 * @warning
 * This function is called from an ISR and should be as short as possible
 *
 ******************************************************************************/
extern void PCA0_intermediateOverflowCb();

/***************************************************************************//**
 * @brief
 * Callback for channel 0 events.
 *
 * This function is defined by the user and called when there is a capture
 * or compare event on channel 0
 *
 * @warning
 * This function is called from an ISR and should be as short as possible.
 *
 ******************************************************************************/
extern void PCA0_channel0EventCb();

/***************************************************************************//**
 * @brief
 * Callback for channel 1 events.
 *
 * This function is defined by the user and called when there is a capture
 * or compare event on channel 1
 *
 * @warning
 * This function is called from an ISR and should be as short as possible.
 *
 ******************************************************************************/
extern void PCA0_channel1EventCb();

/***************************************************************************//**
 * @brief
 * Callback for channel 2 events.
 *
 * This function is defined by the user and called when there is a capture
 * or compare event on channel 2
 *
 * @warning
 * This function is called from an ISR and should be as short as possible.
 *
 ******************************************************************************/
extern void PCA0_channel2EventCb();

/**************************************************************************//**
 * @def void PCA0_channel3EventCb()
 * @warning NOT SUPPORTED ON THIS DEVICE
 *****************************************************************************/

/**************************************************************************//**
 * @def void PCA0_channel4EventCb()
 * @warning NOT SUPPORTED ON THIS DEVICE
 *****************************************************************************/

/**************************************************************************//**
 * @def void PCA0_channel5EventCb()
 * @warning NOT SUPPORTED ON THIS DEVICE
 *****************************************************************************/

#endif //EFM8PDL_PCA0_USE_ISR
/** @} (end addtogroup pca0_callbacks_isr ISR API) */
/** @} (end addtogroup pca0_callback User Callbacks) */
/** @} (end addtogroup PCA0 Driver) */
#endif //__PCA_0_H__
