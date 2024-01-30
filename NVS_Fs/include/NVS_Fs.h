/**
 * (c) Maschinenfabrik Bernard Krone GmbH, Spelle. All rights reserved.
 *
 * @file:    NVS_Fs.h
 * @date:    $LastChangedDate: 29.01.2024
 * @author:  $LastChangedBy: Arabkirlian Garen
 * @version: $Revision:
 */
#ifndef NVS_Fs_H__
#define NVS_Fs_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************/
/***************** Inclusions **************************/
/*******************************************************/

#include "common_typedefs.h"
#include <stdbool.h>
#include <sys/time.h>


/*******************************************************/
/***************** Interface Macros ********************/
/*******************************************************/

#define NB_OF_CHUNKS 3 //12289 chunks for full 4MB
#define TOTAL_FILES 256


/*******************************************************/
/***************** Interface types *********************/
/*******************************************************/


typedef enum{
	hexFile,
	binary,
	script
}fileKey_t;



typedef struct {
	fileKey_t key;
	void* point;
	size_t size;
	int id;
	int offset;
	int valid;
} fileStruct;



/*******************************************************/
/***************** Interface functions *****************/
/*******************************************************/

/**
 * @brief 		Initializes the payload partition in the custom partition table.
 *
 *
 * @param[in]   none
 * @param[out]  none
 *
 * @return
 *     			none
 */
void Partition_init();

/**
 * @brief 		Initializes the payload partition in the custom partition table.
 *
 *
 * @param[in]   none
 * @param[out]  none
 *
 * @return
 *     			none
 */
fileStruct NVS_writeFile(void*, fileKey_t, size_t, int);

/**
 * @brief 		Reads data from specified fileStruct
 *
 *
 * @param[in]   fileStruct file
 * @param[out]  fileStruct file
 *
 * @return
 *     			fileStruct file in which we have written the data.
 */
void NVS_readFile(fileStruct);

/**
 * @brief 		unmap partition for NVS
 *
 *
 * @param[in]   none
 * @param[out]  none
 *
 * @return
 *     			none
 */
void NVS_unmap();

/**
 * @brief 		Initializes the meta data partition in the custom partition table.
 *
 *
 * @param[in]   none
 * @param[out]  none
 *
 * @return
 *     			none
 */
void meta_init();

/**
 * @brief 		Wrapper function to initialize NVS flash API from esp32 IDF
 *
 *
 * @param[in]   none
 * @param[out]  none
 *
 * @return
 *     			none
 */
void NVS_init_valPair();

/**
 * @brief 		Initializes meta partition as the nvs flash partition
 *
 *
 * @param[in]   none
 * @param[out]  none
 *
 * @return
 *     			none
 */
void openNVS();

/**
 * @brief 		Retreives all key-values from flash and stores them in the fileArray table as individual fileStructs in RAM.
 * 				Use case: Can be called after reboot to sync the file list.
 *
 *
 * @param[in]   none
 * @param[out]  none
 *
 * @return
 *     			none
 */
void retreiveFromFlash();

/**
 * @brief 		Wrapper function for nvs_close.
 *
 *
 * @param[in]   none
 * @param[out]  none
 *
 * @return
 *     			none
 */
void closeNVS();

/**
 * @brief 		Deletes a file by flagging it as invalid in its corresponding fileStruct.
 * 				Invalid files could be overwritten in case all files have been used.
 *
 *
 * @param[in]   fileStruct file
 * @param[out]  fileStruct file
 *
 * @return
 *     			fileStruct file with updated invalid flag.
 */
fileStruct deleteFile(fileStruct file);


