#ifndef PARTICLE_ID_HASH
#define PARTICLE_ID_HASH

//-
// ==========================================================================
// Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

/*

	Separate chained hash table for mapping particle ids to sample points in 
	an efficient manner.  Since particle ID's may not be contiguous, an array 
	may need to be arbitrarily large to index on particle ID.  The hash table
	overcomes this limitation by allowing multiple ID's to match a single hash
	key.

*/

#include <maya/MPointArray.h>

class ParticleIdHash
{
private:

	// Element class for the hash table
	class ParticleSample
	{
	public:
		ParticleSample(int inId, MPoint& inPosition, ParticleSample* inNext) : 
			id(inId), position(inPosition), next(inNext) {}
		int id;					// Particle id
		MPoint position;		// Particle position
		ParticleSample* next;	// Next pointer for hash table
	};

public:
	// Constructor / Destructor
	ParticleIdHash(int inSize) : size(inSize) {
		if (size <= 0)
		{
			size = 1;	// Cannot have hash tables with 0 or less elements
		}
		data = new ParticleSample*[size];
		for (int i = 0; i < size; i++)
		{
			data[i] = NULL;
		}
	}
	~ParticleIdHash() {
		for (int i = 0; i < size; i++)
		{
			ParticleSample* sample = data[i];
			ParticleSample* prev = NULL;
			while (sample != NULL)
			{
				prev = sample;
				sample = sample->next;
				delete prev;
			}
		}
		delete [] data;
	}

	// Add the point to the head of the list at its hash value
	void insert(int id, MPoint& pt) {
		ParticleSample* sample = new ParticleSample(id,pt,data[id%size]);
		data[id%size] = sample;
	}
	// Get the list of points for the given id
	MPointArray getPoints(int id) {
		MPointArray result;
		ParticleSample* sample = data[id%size];
		while (sample != NULL)
		{
			if (sample->id == id)
			{
				result.append(sample->position);
			}
			sample = sample->next;
		}
		return result;
	}

private:
	ParticleSample** data;
	int size;
};

#endif

