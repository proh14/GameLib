// Copyright (C) 2024 Valeriano Alfonso Rodriguez (Kableado)

#include "Bucket.h"

#include "Util.h"

#include <stdlib.h>
#include <tgmath.h>

////////////////////////////////////////////////
// BBox
//

int BBox_Intersect(BBox bbox1, BBox bbox2) {
	if (bbox1->x2 >= bbox2->x1 && bbox1->x1 <= bbox2->x2 && bbox1->y2 >= bbox2->y1 && bbox1->y1 <= bbox2->y2) {
		return (1);
	}
	return (0);
}

////////////////////////////////////////////////
// Bucket
//

void Bucket_Init(Bucket bucket) { bucket->count = 0; }

void Bucket_AddItem(Bucket bucket, BBox item) {
	if (bucket->count >= Max_ItemsPerBucket) {
		Print("Bucket Overflow\n");
		return;
	}
	bucket->bbox[bucket->count] = item;
	bucket->count++;
}

void Bucket_AddItemUnique(Bucket bucket, BBox item) {
	for (int i = 0; i < bucket->count; i++) {
		if (bucket->bbox[i]->parent == item->parent) {
			return;
		}
	}
	Bucket_AddItem(bucket, item);
}

int Bucket_SearchItem(Bucket bucket, BBox item) {
	for (int i = 0; i < bucket->count; i++) {
		if (bucket->bbox[i]->parent == item->parent) {
			return i;
		}
	}
	return -1;
}

void Bucket_RemoveIndex(Bucket bucket, int index) {
	bucket->bbox[index] = bucket->bbox[bucket->count - 1];
	bucket->count--;
	bucket->bbox[bucket->count] = NULL;
}

void Bucket_RemoveItem(Bucket bucket, BBox item) {
	int index = Bucket_SearchItem(bucket, item);
	if (index < 0) {
		return;
	}
	Bucket_RemoveIndex(bucket, index);
}

////////////////////////////////////////////////
// BucketGrid
//

void BucketGrid_Init(BucketGrid bucketGrid, int width, int height, float bucketSize, float offsetX, float offsetY) {
	bucketGrid->width      = width;
	bucketGrid->height     = height;
	bucketGrid->bucketSize = bucketSize;
	bucketGrid->offsetX    = offsetX;
	bucketGrid->offsetY    = offsetY;
	bucketGrid->bucket     = (TBucket *)malloc(sizeof(TBucket) * bucketGrid->width * bucketGrid->height);
	for (int y = 0; y < bucketGrid->height; y++) {
		for (int x = 0; x < bucketGrid->width; x++) {
			Bucket_Init(&bucketGrid->bucket[x + y * width]);
		}
	}
}

void BucketGrid_DeInit(BucketGrid bucketGrid) { free(bucketGrid->bucket); }

int BucketGrid_AddItem(BucketGrid bucketGrid, BBox item) {
	const int x1Bucket = (int)floorf((item->x1 + bucketGrid->offsetX) / bucketGrid->bucketSize);
	const int x2Bucket = (int)ceilf((item->x2 + bucketGrid->offsetX) / bucketGrid->bucketSize);
	const int y1Bucket = (int)floorf((item->y1 + bucketGrid->offsetY) / bucketGrid->bucketSize);
	const int y2Bucket = (int)ceilf((item->y2 + bucketGrid->offsetY) / bucketGrid->bucketSize);

	int inserted = 0;
	for (int y = y1Bucket; y <= y2Bucket; y++) {
		for (int x = x1Bucket; x <= x2Bucket; x++) {
			if (y < 0 || y >= bucketGrid->height || x < 0 || x >= bucketGrid->width) {
				continue;
			}
			Bucket bucket = &bucketGrid->bucket[x + y * bucketGrid->width];
			Bucket_AddItem(bucket, item);
			inserted = 1;
		}
	}
	if (inserted) {
		item->changed = 0;
	}
	return inserted;
}

void BucketGrid_RemoveItem(BucketGrid bucketGrid, BBox item) {
	const int x1Bucket = (int)floorf((item->x1 + bucketGrid->offsetX) / bucketGrid->bucketSize);
	const int x2Bucket = (int)ceilf((item->x2 + bucketGrid->offsetX) / bucketGrid->bucketSize);
	const int y1Bucket = (int)floorf((item->y1 + bucketGrid->offsetY) / bucketGrid->bucketSize);
	const int y2Bucket = (int)ceilf((item->y2 + bucketGrid->offsetY) / bucketGrid->bucketSize);

	for (int y = y1Bucket; y <= y2Bucket; y++) {
		for (int x = x1Bucket; x <= x2Bucket; x++) {
			if (y < 0 || y >= bucketGrid->height || x < 0 || x >= bucketGrid->width) {
				continue;
			}
			Bucket bucket = &bucketGrid->bucket[x + y * bucketGrid->width];
			Bucket_RemoveItem(bucket, item);
		}
	}
}

void BucketGrid_Query(BucketGrid bucketGrid, BBox item, Bucket resultBucket) {
	const int x1Bucket = (int)floorf((item->x1 + bucketGrid->offsetX) / bucketGrid->bucketSize);
	const int x2Bucket = (int)ceilf((item->x2 + bucketGrid->offsetX) / bucketGrid->bucketSize);
	const int y1Bucket = (int)floorf((item->y1 + bucketGrid->offsetY) / bucketGrid->bucketSize);
	const int y2Bucket = (int)ceilf((item->y2 + bucketGrid->offsetY) / bucketGrid->bucketSize);

	Bucket_Init(resultBucket);

	for (int y = y1Bucket; y <= y2Bucket; y++) {
		for (int x = x1Bucket; x <= x2Bucket; x++) {
			if (y < 0 || y >= bucketGrid->height || x < 0 || x >= bucketGrid->width) {
				continue;
			}
			Bucket bucket = &bucketGrid->bucket[x + y * bucketGrid->width];
			for (int i = 0; i < bucket->count; i++) {
				if (BBox_Intersect(bucket->bbox[i], item)) {
					Bucket_AddItemUnique(resultBucket, bucket->bbox[i]);
				}
			}
		}
	}
}

int BucketGrid_ProcessChanged(BucketGrid bucketGrid) {
	TBucket bucketChanged;
	Bucket_Init(&bucketChanged);

	for (int y = 0; y < bucketGrid->height; y++) {
		for (int x = 0; x <= bucketGrid->width; x++) {
			Bucket bucket = &bucketGrid->bucket[x + y * bucketGrid->width];
			int i         = 0;
			while (i < bucket->count) {
				if (bucket->bbox[i]->changed) {
					if (bucketChanged.count == Max_ItemsPerBucket) {
						return 0;
					}
					Bucket_AddItemUnique(&bucketChanged, bucket->bbox[i]);
					Bucket_RemoveIndex(bucket, i);
				} else {
					i++;
				}
			}
		}
	}
	for (int i = 0; i < bucketChanged.count; i++) {
		BucketGrid_AddItem(bucketGrid, bucketChanged.bbox[i]);
	}
	return 1;
}

void BucketGrid_Clean(BucketGrid bucketGrid) {
	for (int y = 0; y < bucketGrid->height; y++) {
		for (int x = 0; x <= bucketGrid->width; x++) {
			Bucket bucket = &bucketGrid->bucket[x + y * bucketGrid->width];
			Bucket_Init(bucket);
		}
	}
}
