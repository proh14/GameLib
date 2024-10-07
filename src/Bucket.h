// Copyright (C) 2024 Valeriano Alfonso Rodriguez (Kableado)

#ifndef Bucket_H
#define Bucket_H

////////////////////////////////////////////////
// BBox
//
typedef struct TBBox TBBox, *BBox;
struct TBBox {
	float x1, x2;
	float y1, y2;
	void *parent;
	int changed;
};

int BBox_Intersect(BBox bbox1, BBox bbox2);

////////////////////////////////////////////////
// Bucket
//
#define Max_ItemsPerBucket 200
typedef struct TBucket TBucket, *Bucket;
struct TBucket {
	BBox bbox[Max_ItemsPerBucket];
	int count;
};

void Bucket_Init(Bucket bucket);

void Bucket_AddItem(Bucket bucket, BBox item);

void Bucket_AddItemUnique(Bucket bucket, BBox item);

int Bucket_SearchItem(Bucket bucket, BBox item);

void Bucket_RemoveIndex(Bucket bucket, int index);

void Bucket_RemoveItem(Bucket bucket, BBox item);

////////////////////////////////////////////////
// BucketGrid
//
typedef struct TBucketGrid TBucketGrid, *BucketGrid;
struct TBucketGrid {
	TBucket *bucket;
	int width;
	int height;
	float bucketSize;
	float offsetX;
	float offsetY;
};

void BucketGrid_Init(BucketGrid bucketGrid, int width, int height, float bucketSize, float offsetX, float offsetY);

void BucketGrid_DeInit(BucketGrid bucketGrid);

int BucketGrid_AddItem(BucketGrid bucketGrid, BBox item);

void BucketGrid_RemoveItem(BucketGrid bucketGrid, BBox item);

void BucketGrid_Query(BucketGrid bucketGrid, BBox item, Bucket resultBucket);

int BucketGrid_ProcessChanged(BucketGrid bucketGrid);

void BucketGrid_Clean(BucketGrid bucketGrid);

#endif
