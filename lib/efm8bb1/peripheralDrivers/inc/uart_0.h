/******************************************************************************
 * Copyright (c) 2014 by Silicon Laboratories Inc. All rights reserved.
 *
 * http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
 *****************************************************************************/

#ifndef __UART_0_H__
#define __UART_0_H__


#include "efm8_config.h"
#include "SI_EFM8BB1_Register_Enums.h"

/**************************************************************************//**
 *@addtogroup uart_0 UART0 Driver
 *@{
 *
 *@brief Peripheral driver for uart0
 *
 * # Introduction #
 *
 * This module contains all the driver content for UART0
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
 * |USE                 |  295 |    6 |    0 |   0 |
 * |STDIO               |   20 |    0 |    0 |   0 |
 *
 * # Theory of Operation #
 *
 * The UART driver provides several levels of functionality. Higher level
 * functionality is more user friendly but also less broadly applicable.
 *
 * ### Buffered Api ###
 *
 * The driver provides high level functions for transferring a buffer of data.
 * This functionality is made available by setting EFM8PDL_UART0_USE_BUFFER
 * to 1.
 *
 * This functionality relies on the UART0 interrupt to function and the
 * library provides the ISR for that interrupt.
 *
 * For data transmission the user provides a data buffer and the length of that
 * buffer. The driver will then transmit the entire buffer before issuing an
 * end-of-transfer callback.
 *
 * In the following example we implement a ping-pong transmission.
 *
 * ~~~~~.c
 *
 * SI_SEGMENT_VARIABLE(bufferA[32], uint8_t, SI_SEG_XDATA);
 * SI_SEGMENT_VARIABLE(bufferB[32], uint8_t, SI_SEG_XDATA);
 * SI_VARIABLE_SEGMENT_POINTER(curBuffer, uint8_t, SI_SEG_XDATA);
 * bool targetA;
 *
 * // Some other code pushes data to curBuffer and handles
 * // kicking the transfer with an initial call to writeBuffer
 *
 * //when current transfer is complete send switch ping-pong
 * //  and send all data accumulated in the other buffer.
 * //  If no data is ready in other buffer we will write
 * void UART0_transmitCompleteCb()
 * {
 *   if(targetA)
 *   {
 *     //Don't transmit if no data is available
 *     if(curBuffer == bufferA)
 *     {
 *       //Some code to mark that the transfer is stalled
 *       return;
 *     }
 *
 *     // Send all data in A and start accumulating in B
 *     UART0_writeBuffer(bufferA, curBuffer - bufferA);
 *     curBuffer = bufferB;
 *     targetA = false;
 *   }
 *   else
 *   {
 *     //Don't transmit if no data is available
 *     if(curBuffer == bufferA)
 *     {
 *       //Some code to mark that the transfer is stalled
 *       return;
 *     }
 *
 *    //send all data in B and start accumulating in A
 *    UART0_writeBuffer(bufferA, curBuffer - bufferA);
 *    curBuffer = bufferA;
 *    targetA = true;
 *   }
 * }
 *
 * ~~~~~
 *
 * For reception the user provides a buffer and it's size. The library will
 * receive data into the buffer until it is full and then call the user
 * back. The user may query the number of bytes remaining in the buffer at
 * any time if they wish to process data before the buffer is full.
 *
 * This example provides ping-pong reception.
 *
 * ~~~~~.c
 *
 * SI_SEGMENT_VARIABLE(bufferA[32], uint8_t, SI_SEG_XDATA);
 * SI_SEGMENT_VARIABLE(bufferB[32], uint8_t, SI_SEG_XDATA);
 * SI_VARIABLE_SEGMENT_POINTER(curBuffer, uint8_t, SI_SEG_XDATA);
 * SI_VARIABLE_SEGMENT_POINTER(dataReady, uint8_t, SI_SEG_XDATA);
 * bool targetA;
 *
 * //Here the main loop handles stuff
 * void main()
 * {
 *   //other initialization
 *   UART0_readBuffer(bufferA, 32);
 *   targetA = true;
 *
 *   while(1)
 *   {
 *     if(dataReady != NULL)
 *     {
 *        //process data
 *        dataReady = NULL;
 *      }
 *   }
 * }
 *
 * // When the current buffer is full inform the main loop it's
 * //ready for processing and switch to the other buffer
 * void UART0_receiveCompleteCb()
 * {
 *   if(targetA)
 *   {
 *     datReady = bufferA;
 *     UART0_readBuffer(bufferB, 32);
 *     targetA = false;
 *   }
 *   else
 *   {
 *     datReady = bufferB;
 *     UART0_readBuffer(bufferA, 32);
 *     targetA = true;
 *   }
 * }
 *
 * ~~~~~
 *
 * ### STDIO Api ###
 *
 * One of the simplest use cases is using UART 0 to stdio data. The driver
 * provides a standard blocking implementation accessed by setting
 * EFM8PDL_UART0_USE_STDIO.
 *
 * When this is in use no other functions are available.
 *
 * When this is in use calls to printf will block until the entire string
 * has been transmitted on the uart.
 *
 * ### Runtime & Initialization API ###
 *
 * The final option is to use the runtime and initialization api to implement
 * a custom uart driver.
 *
 * The reference manual should be consulted for a full understanding of how
 * the block operates in order to use the Runtime api correctly.
 *
 * ### Hardware Configuration ###
 *
 * This Driver provides a basic level of configuration through the API. However
 * use of the Simplicity Hardware Configuration tool is highly recommended.
 *
 *****************************************************************************/

// Option macro documentation
/**************************************************************************//**
 * @addtogroup uart0_config Driver Configuration
 * @{
 *****************************************************************************/

/***************************************************************************//**
 * @def EFM8PDL_UART0_USE
 * @brief Controls inclusion of UART0 Peripheral Driver.
 *
 *
 * When '1' the UART0 driver is available.
 *
 * Default setting is '0' and may be overridden by defining in 'efm8_config.h'.
 *
 *****************************************************************************/

/**************************************************************************//**
 * @def EFM8PDL_UART0_USE_STDIO
 * @brief Controls the inclusion of putchar and setchar for use with printf/scanf.
 *
 * When '1' blocking implementations of putchar and set char are defined. This option
 * is intended to be use in place of all other options. If EFM8PDL_UART0_USE_STDIO
 * is '1' then EFM8PDL_UART0_USE should be 0 and the UART 0 peripheral driver should
 * not be called by the user directly accept for the initial setup.
 *
 * The putchar implementation provides an initialization function to prime the TX transfer
 * and configure the UART for receive and transmit. This function should be called immediately
 * after device configuration and before any printf or scanf calls.
 *
 * Default setting is '0' and may be overridden by defining in 'efm8_config.h'.
 *
 *****************************************************************************/

/**************************************************************************//**
 * @def EFM8PDL_UART0_USE_INIT
 * @brief Controls inclusion of UART0 Initialization API.
 *
 * When '1' the UART0 Initialization API is included in the driver.
 *
 * Default setting is '1' and may be overridden by defining in 'efm8_config.h'.
 *
 *****************************************************************************/

/**************************************************************************//**
 * @def EFM8PDL_UART0_USE_BUFFER
 * @brief Controls inclusion of UART0 Buffer Access API.
 *
 * When '1' the UART0 Buffered Access API is included in the driver.
 *
 * Default setting is '1' and may be overridden by defining in 'efm8_config.h'.
 *
 *****************************************************************************/

/**************************************************************************//**
 * @addtogroup uart0_config_buffered Buffered API Options
 * @{
 *****************************************************************************/

/**************************************************************************//**
 * @def EFM8PDL_UART0_RX_BUFTYPE
 * @brief Controls the type of pointer used for buffered receives.
 *
 * Sets the memory segment for the rx data buffer pointer when EFM8PDL_UART0_USE_BUFFER is '1'
 * valid values are:
 *
 * - SI_SEG_XDATA (default)
 * - SI_SEG_PDATA
 * - SI_SEG_IDATA
 * - SI_SEG_CODE
 * - SI_SEG_GENERIC
 *
 * @warning: Use of generic pointers will adversely effect the size and performance
 * of the buffering functions.
 *
 *****************************************************************************/

/**************************************************************************//**
 * @def EFM8PDL_UART0_TX_BUFTYPE
 * @brief Controls the type of pointer used for buffered transmits.
 *
 * Sets the memory segment for the tx data buffer pointer when EFM8PDL_UART0_USE_BUFFER is '1'
 * valid values are:
 *
 * - SI_SEG_XDATA (default)
 * - SI_SEG_PDATA
 * - SI_SEG_IDATA
 * - SI_SEG_CODE
 * - SI_SEG_GENERIC
 *
 *****************************************************************************/

/** @} (end addtogroup uart0_config_buffered Buffered API Optionsn) */
/** @} (end addtogroup uart0_config Driver Configuration) */

// Option macro default values
#ifndef IS_DOXYGEN
  #define IS_DOXYGEN 0
#endif

#ifndef EFM8PDL_UART0_USE_STDIO
#define EFM8PDL_UART0_USE_STDIO 0
#endif
#ifndef EFM8PDL_UART0_USE_BUFFER
  #if (!EFM8PDL_UART0_USE_STDIO)
    #define EFM8PDL_UART0_USE_BUFFER 1 // buffer mode by default unless user has already selected one of the others
  #else
    #define EFM8PDL_UART0_USE_BUFFER 0 // buffer mode by default unless user has already selected one of the others
  #endif
#endif
#ifndef EFM8PDL_UART0_TX_BUFTYPE
#define EFM8PDL_UART0_TX_BUFTYPE SI_SEG_XDATA
#endif
#ifndef EFM8PDL_UART0_RX_BUFTYPE
#define EFM8PDL_UART0_RX_BUFTYPE SI_SEG_XDATA
#endif

// Runtime API
/**************************************************************************//**
 * @addtogroup uart0_runtime UART0 Runtime API
 * @{
 *****************************************************************************/

/**************************************************************************//**
 * @addtogroup uart0_if Interrupt Flag Enums
 * @{
 *****************************************************************************/
#define UART0_TX_IF SCON0_TI__BMASK /**< UART0 TX Interrupt */
#define UART0_RX_IF SCON0_RI__BMASK /**< UART0 RX Interrupt  */
/** @} (end addtogroup uart0_if Interrupt Flag Enums) */

/***************************************************************************//**
 * @brief
 * Return the value of the specified interrupt flag.
 *
 * @param flag:
 * Flag to check. If OR'd together will return >0 if either flag is set.
 *
 * @return
 * The state of the flags. This value is the OR of all flags which are set.
 *
 * ~~~~~.c
 * if(UART0_getIntFlags() & UART_TX_IF)
 * {
 *   //do something
 * }
 *
 * uint8_t value = UART0_getIntFlags();
 * if(value)
 * {
 *   //do some stuff that needs to be done for RX and TX
 *   if (value & UART_RX_IF)
 *   {
 *     //Do stuff that only needs to be done for RX
 *   }
 * }
 * ~~~~~
 *
 * Valid flags can be found in the Interrupt Flag Enums group.
 *
 *****************************************************************************/
uint8_t UART0_getIntFlags();

/***************************************************************************//**
 * @brief
 * Clear the specified interrupt flag.
 *
 * @param flag:
 * Flag to clear. Multiple flags can be cleared by OR-ing the flags.
 *
 * Valid flags can be found in the Interrupt Flag Enums group.
 *
 ******************************************************************************/
void UART0_clearIntFlag(uint8_t flag);

/***************************************************************************//**
 * @brief
 * Sets the TX complete interrupt flag.
 *
 * It is common to operate the UART in polling mode where the procedure for
 * transmitting a byte is to block till TX is complete and then clear the flag
 * and write to SBUF. For these cases it is necessary to manually set the TX bit
 * to initialize the UART.
 *
 ******************************************************************************/
void UART0_initTxPolling();

/***************************************************************************//**
 * @brief
 * Write 8 bits to the UART to be transmitted.
 *
 * @param value:
 * Data to be transmitted.
 *
 * If the UART already has data pending transmission it will be overwritten.
 *
 ******************************************************************************/
void UART0_write(uint8_t value);

/***************************************************************************//**
 * @brief
 * Read the last received byte from UART.
 *
 * @return
 * The most recent byte read by the UART.
 *
 ******************************************************************************/
uint8_t UART0_read(void);

/***************************************************************************//**
 * @brief
 * Write the a byte to the UART with an extra bit.
 *
 * @param value:
 * Data to transmit.
 *
 * Data[9] should contain the value of the extra bit.
 *
 ******************************************************************************/
void UART0_writeWithExtraBit(uint16_t value);

/***************************************************************************//**
 * @brief
 * Read a byte from the UART with an extra bit.
 *
 * @return
 * The last byte received with data[9] set to the value of the extra bit.
 *
 ******************************************************************************/
uint16_t UART0_readWithExtraBit(void);
/** @} (end addtogroup uart0_runtime UART0 Runtime API) */

// Initialization API
/***************************************************************************//**
 * @addtogroup uart0_init UART0 Initialization API
 * @{
 ******************************************************************************/

/// UART transfer width enums.
typedef enum
{
  UART0_WIDTH_8 = SCON0_SMODE__8_BIT, //!< UART in 8-bit mode.
  UART0_WIDTH_9 = SCON0_SMODE__9_BIT, //!< UART in 9-bit mode.
} UART0_Width_t;

/// UART Multiprocessor support enums.
typedef enum
{
  UART0_MULTIPROC_DISABLE = SCON0_MCE__MULTI_DISABLED, //!< UART Multiprocessor communication Disabled.
  UART0_MULTIPROC_ENABLE  = SCON0_MCE__MULTI_ENABLED,  //!< UART Multiprocessor communication Enabled.
} UART0_Multiproc_t;

/// UART RX support enums
typedef enum
{
  UART0_RX_ENABLE  = SCON0_REN__RECEIVE_ENABLED,   //!< UART Receive Enabled.
  UART0_RX_DISABLE = SCON0_REN__RECEIVE_DISABLED,  //!< UART Receive Disabled.
} UART0_RxEnable_t;

/***************************************************************************//**
 * @brief
 * Initialize the UART
 *
 * @param rxen:
 * Receive enable status.
 * @param width:
 * Data word width.
 * @param mce:
 * Multiprocessor mode status.
 *
 ******************************************************************************/
void UART0_init(UART0_RxEnable_t rxen, UART0_Width_t width, UART0_Multiproc_t mce);

/***************************************************************************//**
 * @brief
 * Restore the UART to it's uninitialized (reset) state.
 *
 ******************************************************************************/
void UART0_reset();

/** @} (end uart0_init UART0 Initialization API) */

// Buffer API
/**************************************************************************//**
 * @addtogroup uart0_buffer UART0 Buffer Access API
 * @{
 *****************************************************************************/
#if (EFM8PDL_UART0_USE_BUFFER == 1) || IS_DOXYGEN

/***************************************************************************//**
 * @brief
 * Transmit a buffer of data via UART.
 *
 * @param[in] buffer:
 * Pointer to buffer of data to be transmitted.
 * @param length:
 * Number of bytes in transfer to be transmitted.
 *
 * Buffer transfers support only 8-bit wide transfers.
 *
 ******************************************************************************/
void UART0_writeBuffer(SI_VARIABLE_SEGMENT_POINTER(buffer,
                                                uint8_t,
                                                EFM8PDL_UART0_TX_BUFTYPE),
                                                uint8_t length);

/***************************************************************************//**
 * @brief
 * Receive a buffer of data via UART.
 *
 * @param[out] buffer:
 * Pointer to buffer of data to be transmitted.
 * @param length:
 * Number of bytes in transfer to be transmitted.
 *
 * Buffered transfers support only 8-bit words.
 *
 ******************************************************************************/
void UART0_readBuffer(SI_VARIABLE_SEGMENT_POINTER(buffer,
                                               uint8_t,
                                               EFM8PDL_UART0_RX_BUFTYPE),
                                               uint8_t length);

/***************************************************************************//**
 * @brief
 * Abort current buffer transmission.
 *
 * Data already moved into the UART will finish transmission. No more
 * data will be pulled out of the TX buffer.
 *
 ******************************************************************************/
void UART0_abortWrite();

/***************************************************************************//**
 * @brief
 * Abort current buffer reception.
 *
 * No more data will be written to the RX buffer.
 *
 ******************************************************************************/
void UART0_abortRead();

/***************************************************************************//**
 * @brief
 * Return the number of bytes remaining in the TX buffer.
 *
 * @return
 * number of btyes remaining in TX buffer. 0 if no transfer is in progress.
 *
 * @returns 0 if transfer is not in progress.
 ******************************************************************************/
uint8_t UART0_txBytesRemaining();

/***************************************************************************//**
 * @brief
 * Return the number of bytes remaining in the RX buffer.
 *
 * @return
 * number of btyes remaining in RX buffer. 0 if no transfer is in progress.
 *
 ******************************************************************************/
uint8_t UART0_rxBytesRemaining();
/** @} (end uart0_buffer UART0 Buffer Access API) */

/**************************************************************************//**
 * @def void UART0_ISR()
 * @brief UART0 Interrupt handler.
 *
 * This callback is implemented inside the driver if EFM8PDL_UART0_USE_BUFFER is set
 * otherwise the user must implement the ISR.
 *
 *****************************************************************************/
#endif // EFM8PDL_UART0_USE_BUFFER

// Callbacks
/**************************************************************************//**
 * @addtogroup uart0_callbacks User Callbacks
 * @{
 *****************************************************************************/

/**************************************************************************//**
 *@addtogroup uart0_callbacks_buffer Buffer Access API
 *@{
 *
 * These callbacks will be called by the library when
 * EFM8PDL_UART0_USE_BUFFER. If the Buffered Access API is disabled
 * the callbacks do not need to be provided by the user.
 *
 *****************************************************************************/
#if (EFM8PDL_UART0_USE_BUFFER == 1) || IS_DOXYGEN

/***************************************************************************//**
 * @brief
 * Callback for reception of byte.
 *
 * This function is called when all expected bytes have been received.
 *
 * @warning
 * This function is called from an ISR and should be as short as possible.
 *
 ******************************************************************************/
void UART0_receiveCompleteCb();

/***************************************************************************//**
 * @brief
 * Callback for transmission of a byte.
 *
 * This function is called when all bytes in the buffer have been transferred.
 *
 * @warning
 * This function is called from an ISR and should be as short as possible.
 *
 ******************************************************************************/
void UART0_transmitCompleteCb();

#endif //EFM8PDL_UART0_USE_BUFFER
/** @} (end uart0_callbacks_buffer Buffer Access API) */
/** @} (end uart0_callbacks User Callbacks) */


// STIDO API
/**************************************************************************//**
 * @addtogroup uart0_stdio UART0 STDIO API
 * @{
 *
 * This API is intended to be used in place of all other uart driver
 * API's and will assume control of the uart.
 *
 * @warning
 * This implementation is blocking and may hang the MCU under certain
 * conditions.
 *
 ******************************************************************************/
#if (EFM8PDL_UART0_USE_STDIO == 1) || IS_DOXYGEN

/***************************************************************************//**
 * @brief
 * Initializes uart for STDIO operation.
 *
 * This function sets up the uart for use by printf/scanif. It must be called
 * once durring device initialization **before** using STDIO.
 *
 ******************************************************************************/
void UART0_initStdio();

#endif //EFM8PDL_UART0_USE_STDIO
/** @} (end uart0_stdio UART0 STDIO API) */
/** @} (end uart_0 UART0 Driver) */
#endif //__UART_0_H__
