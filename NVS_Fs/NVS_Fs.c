/**
 * (c) Maschinenfabrik Bernard Krone GmbH, Spelle. All rights reserved.
 *
 * @file:    NVS_Fs.c
 * @date:    $LastChangedDate: 29.01.2024
 * @author:  $LastChangedBy: Garen Arabkirlian
 * @version: $Revision:
 */

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************/
/***************** Inclusions **************************/
/*******************************************************/

#include "NVS_Fs.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "esp_partition.h"
#include "esp_log.h"
#include <esp_log.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*******************************************************/
/***************** Local Macros ************************/
/*******************************************************/

/*******************************************************/
/***************** Local types *************************/
/*******************************************************/

/*******************************************************/
/***************** Local variables *********************/
/*******************************************************/
static char TAG[] = "RTC";


static const char *TAG = "";
static const char data[256] = "OfPpjO8GUqMu0nKpib34A5zATuD8NMLcDT8YekGABr7T7hg3XIsFpXWarFUxlkiOyx6Q9xB09xvhTKLc0dk937fjNuiyoA9cSNmxqsido9qsw2o85nJORBkoGnOsycYhZD5wvUdhLCcA1glv3MsgqloIcgN9lLbryzSnHJzPwHYSNN2rzsD7nVv6uKFklNAqFGKgklYPLdNp3eHSpI7M9ge6YtxgAhAdQZSz9KE3MATynjzLazf7YP2cI3MHYhqf";
static const char data2[128] = "PLRAzBrvzR5vMAGdwxUuEzesVHE1h4TG1avu43hbxCmmfOaJSIsRBmFMRFy4i7SeLC5PP12DZi0Bn5H326kZIvIukgtFY0wnv1OMes2fpbv29RirO2VA86DKDujq9Vpg";
static const char data3[64] = "837KinrNhnJJxTADg01XJiOKzCLRw6fXlt24WqlzQEIEDgRHBRIovNPkisJpCnXJ";

fileStruct fileArray[TOTAL_FILES];
const esp_partition_t *partition;
const esp_partition_t *meta;
const void *map_ptr;
const void *meta_ptr;
esp_partition_mmap_handle_t map_handle;
esp_partition_mmap_handle_t meta_handle;
size_t offset = 0;
int id = 0;
int32_t id32 = 0;
int intpoint = 0;
esp_err_t err;
nvs_handle_t my_handle;
fileStruct fileZero = {
		.size = (size_t)0,
};


/*******************************************************/
/************* Local function declarations *************/
/*******************************************************/

/*******************************************************/
/***************** Interface functions *****************/
/*******************************************************/

/**
 * @brief 		Main function for demo example.
 *
 *
 * @param[in]   none
 * @param[out]  none
 *
 * @return
 *     			none
 */
void app_main(void)
{

	Partition_init();
	openNVS();
	//meta_init();


	fileStruct file1 = NVS_writeFile(&data[0], script, sizeof(data), 1);
	fileStruct file2 = NVS_writeFile(&data2[0], hexFile, sizeof(data2), 2);
	fileStruct file3 = NVS_writeFile(&data3[0], binary, sizeof(data3), 3);
	fileStruct file4 = NVS_writeFile(&data[0], script, sizeof(data), 4);

	printf("\nBefore\n");
	NVS_readFile(file1);
	NVS_readFile(file2);
	NVS_readFile(file3);
	NVS_readFile(file4);

	retreiveFromFlash();

	fileStruct file5 = fileArray[1];
	printf("\nAfter\n");
	printf("file 5 id = %d\n", file5.id);
	printf("file 5 size = %d\n", (int)file5.size);
	printf("file 5 point = %d\n", (int)file5.point);
	printf("file 5 offset = %d\n", file5.offset);
	NVS_readFile(file5);

	file5 = deleteFile(file5);

	NVS_unmap();
	closeNVS();

}

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
void Partition_init(){
	// Find the partition map in the partition table
	partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "storage");
	assert(partition != NULL);

	// Prepare data to be read later using the mapped address
	ESP_ERROR_CHECK(esp_partition_erase_range(partition, 0, partition->size));

}

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
void closeNVS(){
	// Close
	nvs_close(my_handle);
}


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
void meta_init(){
	// Find the partition map in the partition table
	meta = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "meta");
	assert(meta != NULL);

	// Prepare data to be read later using the mapped address
	//ESP_ERROR_CHECK(esp_partition_erase_range(meta, 0, meta->size));
}

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
fileStruct NVS_writeFile(void* src, fileKey_t key, size_t size, int id){

	fileStruct newfile;
	size_t totalsize = 0;

	for(int j=id;j>0;j--){
		totalsize += fileArray[j].size;
	}



	newfile.key = key;
	newfile.size = size;
	newfile.id = id;
	newfile.offset = totalsize;
	newfile.valid = 1;

	esp_partition_write(partition, totalsize, src, size);

	ESP_LOGI(TAG, "Sample data to partition has been written");
	// Map the partition to data memory
	ESP_ERROR_CHECK(esp_partition_mmap(partition, 0, partition->size, ESP_PARTITION_MMAP_DATA, &map_ptr, &map_handle));
	ESP_LOGI(TAG, "Mapped partition to data memory address %p", map_ptr);

	newfile.point = map_ptr + totalsize; // to be fixed this should point to flash storage not RAM

	NVS_init_valPair();
	openNVS();
	//Save fileStruct in Flash
	switch(id){
	case 1:
		err = nvs_set_i32(my_handle, "FILE1_ID", newfile.id);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE1_KEY", newfile.key);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE1_SIZE", (int)newfile.size);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE1_OFFSET", newfile.offset);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE1_VALID", newfile.valid);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE1_POINT", (int)newfile.point);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		break;
	case 2:
		err = nvs_set_i32(my_handle, "FILE2_ID", newfile.id);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE2_KEY", newfile.key);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE2_SIZE", (int)newfile.size);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE2_OFFSET", newfile.offset);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE2_VALID", newfile.valid);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE2_POINT", (int)newfile.point);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");


		break;
	case 3:
		err = nvs_set_i32(my_handle, "FILE3_ID", newfile.id);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE3_KEY", newfile.key);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE3_SIZE", (int)newfile.size);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE3_OFFSET", newfile.offset);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE3_VALID", newfile.valid);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE3_POINT", (int)newfile.point);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		break;
	case 4:
		err = nvs_set_i32(my_handle, "FILE4_ID", newfile.id);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE4_KEY", newfile.key);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE4_SIZE", (int)newfile.size);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE4_OFFSET", newfile.offset);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE4_VALID", newfile.valid);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE4_POINT", (int)newfile.point);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		break;
	case 5:
		err = nvs_set_i32(my_handle, "FILE5_ID", newfile.id);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE5_KEY", newfile.key);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE5_SIZE", (int)newfile.size);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE5_OFFSET", newfile.offset);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE5_VALID", newfile.valid);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE5_POINT", (int)newfile.point);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		break;
	case 6:
		err = nvs_set_i32(my_handle, "FILE6_ID", newfile.id);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE6_KEY", newfile.key);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE6_SIZE", (int)newfile.size);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE6_OFFSET", newfile.offset);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE6_VALID", newfile.valid);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		err = nvs_set_i32(my_handle, "FILE6_POINT", (int)newfile.point);
		printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
		err = nvs_commit(my_handle);
		printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

		break;

	}


	// Store the fileStruct in the array
	fileArray[id] = newfile;

	return newfile;
}

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
void retreiveFromFlash(){

	// Read id
	printf("Reading id from NVS ... ");

	for(int i=1;i<6;i++){
		fileStruct file;

		const char* keyId[9];
		sprintf(keyId, "FILE%d_ID", i);

		err = nvs_get_i32(my_handle, keyId, &file.id);
		switch (err) {
		case ESP_OK:
			printf("Done\n");
			printf("id = %d\n", file.id);
			fileArray[i].id = file.id;
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			printf("The value is not initialized yet!\n");
			break;
		default :
			printf("Error (%s) reading!\n", esp_err_to_name(err));
		}


		const char* keySize[9];
		sprintf(keySize, "FILE%d_SIZE", i);
		int sizeInt = 0;

		err = nvs_get_i32(my_handle, keySize, &sizeInt);


		switch (err) {
		case ESP_OK:
			printf("Done\n");
			printf("size %d = %d\n",file.id, sizeInt);
			fileArray[i].size = sizeInt;
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			printf("The value is not initialized yet!\n");
			break;
		default :
			printf("Error (%s) reading!\n", esp_err_to_name(err));
		}

		const char* keyPoint[9];
		sprintf(keyPoint, "FILE%d_OFFSET", i);
		int pointer = 0;

		err = nvs_get_i32(my_handle, keyPoint, &offset);

		switch (err) {
		case ESP_OK:
			printf("Done\n");
			printf("offset %d = %d\n",file.id, offset);
			fileArray[i].offset = offset;
			fileArray[i].point = map_ptr + offset;
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			printf("The value is not initialized yet!\n");
			break;
		default :
			printf("Error (%s) reading!\n", esp_err_to_name(err));
		}

		const char* keyValid[9];
		sprintf(keyValid, "FILE%d_VALID", i);

		err = nvs_get_i32(my_handle, keyValid, &file.valid);


		switch (err) {
		case ESP_OK:
			printf("Done\n");
			printf("Valid %d = %d\n",file.id, file.valid);
			fileArray[i].valid = file.valid;
			break;
		case ESP_ERR_NVS_NOT_FOUND:
			printf("The value is not initialized yet!\n");
			break;
		default :
			printf("Error (%s) reading!\n", esp_err_to_name(err));
		}

		//fileArray[i] = file;

	}



}

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
void NVS_readFile(fileStruct file){
	// Print directly from memory

	for(int j=0;j<((int)(file.size));j++){
		printf("%c",*((char*)(file.point) + j));
	}

	printf("\n");
}

void NVS_FileArray_toString(){ // Does not work

	printf("\nFile Array table: \n");
	for(int j=0;j<((int)sizeof(fileArray));j++){
		printf("%d",*((int*)(fileArray) + j));
	}

	printf("\n");
}


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
void NVS_unmap(){
	// Unmap mapped memory
	esp_partition_munmap(map_handle);
	ESP_LOGI(TAG, "Unmapped partition from data memory");

	ESP_LOGI(TAG, "Example end");
}




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
void NVS_init_valPair(){
	// Initialize NVS
	err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		// NVS partition was truncated and needs to be erased
		// Retry nvs_flash_init
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
	}
	ESP_ERROR_CHECK( err );

}

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
void openNVS(){
	// Open
	printf("\n");
	printf("Opening Non-Volatile Storage (NVS) handle... ");

	//err = nvs_open("storage", NVS_READWRITE, &my_handle);
	err = nvs_flash_init_partition("meta_files");
	if (err != ESP_OK) {
		printf("Error (%s) nvs_flash_init_partition failed!\n", esp_err_to_name(err));
	}
	err = nvs_open_from_partition("meta_files","storage", NVS_READWRITE, &my_handle);
	if (err != ESP_OK) {
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	}
	printf("Done\n");
}


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
fileStruct deleteFile(fileStruct file){

	//Set valid member to zero in RAM
	file.valid = 0;
	fileArray[file.id].valid = 0;

	//Set valid key to zero in flash
	const char* keyvalid[9];
	sprintf(keyvalid, "FILE%d_VALID", file.id);
	err = nvs_set_i32(my_handle, keyvalid, file.valid);
	printf((err != ESP_OK) ? "Failed!\n" : "Done writing to flash\n");
	err = nvs_commit(my_handle);
	printf((err != ESP_OK) ? "Failed!\n" : "Done committing to flash\n");

	printf("File %d is now invalid", file.id);
	return file;
}



#ifdef __cplusplus
} /* end of extern "C" */
#endif
