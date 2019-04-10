#ifndef MultiLayeredBucket_h
#define MultiLayeredBucket_h

#include <stdbool.h>
#include "DualyLinkedList.h"

//////////////////////////////////////////////////
typedef enum MLBOption {
    MLB_OPTION_NONE,
    MLB_OPTION_WITH_ELEMENT
} MLB_OPTION_e;

typedef struct BucketElement {
    int value;
    void *element;
} MLBE_t;

typedef struct BucketLayer {
    int layerNo;                // Top <- 0,1,2 ...,n-1 -> Bottom
    int beginValue;
    int range;                  // end value - begin value + 1
    
    int minPos;                 // index of cell including min. value.
    int beginOfminPos;          // begin value of cell including min. value.
    int num;                    // number of elements including lower layer.
    
    int lengthOfArray;
    int rangeOfCell;
    DLL_t **array;              // array of buckets.
    
    struct BucketLayer *next;
} MLB_t;

//////////////////////////////////////////////////
MLB_t *createMLBucket(int numOfLayers, int beginValue, int endValue);
void destroyMLBucket(MLB_t *B, MLB_OPTION_e option);
bool insertIntoMLBucket(MLB_t *B, int value, void *element);
bool insertBucketElementIntoMLBucket(MLB_t *B, MLBE_t *element);
void *pullMinOnMLBucket(MLB_t *B);
int findNextOnTheLayer(MLB_t *B);
void changeLowerLayers(MLB_t *B, int index);
void convertLowerLayersToList(MLB_t *B, DLL_t *list);
void convertListToLowerLayers(MLB_t *B, int index);
bool isEmptyMLBucket(MLB_t *B);
void printMLBucketStructure(MLB_t *B);
MLBE_t *createBucketElement(int value, void *elemet);
bool destroyBucketElement(MLBE_t *element);

#endif
