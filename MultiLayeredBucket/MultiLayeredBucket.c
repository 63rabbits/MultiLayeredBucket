#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include "MultiLayeredBucket.h"

//////////////////////////////////////////////////
MLB_t *createMLBucket(int numOfLayers, int beginValue, int endValue) {
    if (numOfLayers <= 0) return NULL;
    
    MLB_t *B = malloc(sizeof(MLB_t));
    if (B == NULL) return NULL;
    
    B->layerNo = numOfLayers - 1;
    B->beginValue = beginValue;
    B->range = endValue - beginValue + 1;
    B->minPos = 0;
    B->beginOfminPos = B->beginValue;
    B->num = 0;
    B->lengthOfArray = (int)ceil(pow(B->range, 1.0/numOfLayers));
    B->rangeOfCell = (int)ceil((double)B->range / B->lengthOfArray);
    
    B->array = malloc(B->lengthOfArray * sizeof(void *));
    if (B->array == NULL) {
        free(B);
        return NULL;
    }
    for (int i=0; i<B->lengthOfArray; i++) {
        B->array[i] = createDLList();
    }
    
    B->next = NULL;
    if (numOfLayers > 1) {
        B->next = createMLBucket(numOfLayers - 1, beginValue, beginValue + B->rangeOfCell - 1);
        if (B->next == NULL) {
            free(B->next->array);
            free(B->next);
            return NULL;
        }
    }
    
    return B;
}

void destroyMLBucket(MLB_t *B, MLB_OPTION_e option) {
    if (B->next != NULL) {
        destroyMLBucket(B->next, option);
    }
    
    for (int i= 0; i < B->lengthOfArray; i++) {
        // * Avoid memory leaks. *
        // Since elements registered in the list point to other areas,
        // they are deleted individually.
        DLL_t *list = B->array[i];
        while (!isEmptyDLList(list)) {
            MLBE_t *bucketElement = pullDLList(list);
            if ((option == MLB_OPTION_WITH_ELEMENT) &&
                (bucketElement->element != NULL)) {
                free(bucketElement->element);
            }
            free(bucketElement);
        }
        destroyDLList(list, DLL_OPTION_NONE);
    }
    free(B->array);
    free(B);
}

bool insertIntoMLBucket(MLB_t *B, int value, void *element) {
    // Block illegal parameters.
    if (B == NULL) return false;
    MLBE_t *bucketElement = createBucketElement(value, element);
    bool check = insertBucketElementIntoMLBucket(B, bucketElement);
    if (!check) {
        destroyBucketElement(bucketElement);
        return false;
    }
    return true;
}

bool insertBucketElementIntoMLBucket(MLB_t *B, MLBE_t *element) {
    int value = element->value;
    if ((value < B->beginValue) ||
        (value >= (B->beginValue + B->range))) {
        return false;
    }
    
    if (B->num <= 0) {
        changeLowerLayers(B, 0);
    }
    
    bool result = false;
    int index = (B->minPos + (int)floor((double)(value - B->beginOfminPos) / B->rangeOfCell)) % B->lengthOfArray;
    if ((index == B->minPos) && (B->next != NULL)) {
        result = insertBucketElementIntoMLBucket(B->next, element);
    }
    else {  // for lowest layer.
        result = insertAtTailOnDLList(B->array[index], element);
    }
    if (result) {
        B->num++;
        if (value < B->beginOfminPos) {
            changeLowerLayers(B, index);
        }
    }
    return result;
}

void *pullMinOnMLBucket(MLB_t *B) {
    // Block illegal parameters.
    if (B == NULL) return NULL;
    
    if (B->next != NULL) {
        while (B->next->num > 0) {
            void *element = pullMinOnMLBucket(B->next);
            if (element != NULL) {
                B->num--;
                if (B->next->num <= 0) {
                    int next = findNextOnTheLayer(B);
                    changeLowerLayers(B, next);
                }
                return element;
            }
            int next = findNextOnTheLayer(B);
            changeLowerLayers(B, next);
        }
        return NULL;
    }
    
    // for lowest layer.
    while (B->num > 0) {
        DLL_t *list = B->array[B->minPos];
        if (!isEmptyDLList(list)) {
            MLBE_t *bucketElement = pullHeadOnDLList(list);
            B->num--;
            if (isEmptyDLList(list)) {
                B->minPos = findNextOnTheLayer(B);
                B->beginOfminPos = B->beginValue + B->minPos * B->rangeOfCell;
            }
            void *element = bucketElement->element;
            destroyBucketElement(bucketElement);
            return element;
        }
        B->minPos = findNextOnTheLayer(B);
        B->beginOfminPos = B->beginValue + B->minPos * B->rangeOfCell;
    }
    return NULL;
}

int findNextOnTheLayer(MLB_t *B) {
    for (int i=1; i<B->lengthOfArray; i++) {
        int index = (B->minPos + i) % B->lengthOfArray;
        if (!isEmptyDLList(B->array[index])) {
            return index;
        }
    }
    return B->minPos;
}

void changeLowerLayers(MLB_t *B, int index) {
    // Block illegal parameters.
    if (index == B->minPos) return;                     // already converted.
    if ((index < 0) || (index >= B->lengthOfArray)) return; // illegal index.
    
    if (B->next != NULL) {
        convertLowerLayersToList(B->next, B->array[B->minPos]);
        convertListToLowerLayers(B, index);
        return;
    }
    
    // for lowest layer.
    B->minPos = index;
    B->beginOfminPos = B->beginValue + index * B->rangeOfCell;
}

void convertLowerLayersToList(MLB_t *B, DLL_t *list) {
    // Block illegal parameters.
    if (B->num <= 0) return;
    
    if (B->next != NULL) {
        convertLowerLayersToList(B->next, list);
    }
    
    // for lowest layer.
    for (int i=0; i < B->lengthOfArray; i++) {
        if (B->array[i] != NULL) {
            list = margeDLList(list, B->array[i]);
        }
    }
    B->num = 0;
    B->minPos = 0;
}

bool isEmptyMLBucket(MLB_t *B) {
    if (B->num <= 0) return true;
    return false;
}

void convertListToLowerLayers(MLB_t *B, int index) {
    if (index == B->minPos) return;     // already converted.
    
    DLL_t *list = B->array[index];
    B->minPos = index;
    B->beginOfminPos = B->beginValue + index * B->rangeOfCell;
    
    // initialize lower layeres.
    MLB_t *upperB = B;
    MLB_t *lowerB = upperB->next;
    while (lowerB != NULL) {
        lowerB->beginValue = upperB->beginOfminPos;
        lowerB->minPos = 0;
        lowerB->beginOfminPos = lowerB->beginValue;
        lowerB->num = 0;
        upperB = lowerB;
        lowerB = upperB->next;
    }
    
    // insert elements of list into lower layeres.
    while (!isEmptyDLList(list)) {
        MLBE_t *bucketElement = pullHeadOnDLList(list);
        insertBucketElementIntoMLBucket(B->next, bucketElement);
    }
}

void printMLBucketStructure(MLB_t *B) {
    printf("\n");
    printf("*** debug ***\n");
    
    MLB_t *b = B;
    while (b != NULL) {
        printf("layerNo = %d.\n", b->layerNo);
        printf("beginValue = %d.\n", b->beginValue);
        printf("range = %d.\n", b->range);
        printf("minPos = %d.\n", b->minPos);
        printf("beginOfminPos = %d.\n", b->beginOfminPos);
        printf("num = %d.\n", b->num);
        printf("lengthOfArray = %d.\n", b->lengthOfArray);
        printf("rangeOfCell = %d.\n", b->rangeOfCell);
        
        for (int i=0; i<b->lengthOfArray; i++) {
            printf("*** array[%d] = ", i);
            DLL_t *list = b->array[i];
            if (list != NULL) {
                if ((b->next != NULL) && (i == b->minPos)) {
                    printf("link to lower layer.");
                }
                else {
                    DLLC_t *cell = list->head;
                    while (cell != NULL) {
                        printf("%d, ", ((MLBE_t *)cell->element)->value);
                        cell = cell->next;
                    }
                }
            }
            printf("\n");
        }
        b = b->next;
    }
}

MLBE_t *createBucketElement(int value, void *elemet) {
    // Block illegal parameters.
    if (elemet == NULL) return NULL;
    
    MLBE_t *bucketElement = malloc(sizeof(MLBE_t));
    if (bucketElement == NULL) return NULL;
    
    bucketElement->value = value;
    bucketElement->element = elemet;
    
    return bucketElement;
}

bool destroyBucketElement(MLBE_t *element) {
    // Block illegal parameters.
    if (element == NULL) return false;
    free(element);
    return true;
}
