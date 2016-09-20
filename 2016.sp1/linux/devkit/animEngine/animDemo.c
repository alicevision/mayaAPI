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

#include <animEngine.h>

int
main (int argc, char *argv[])
{
	EtChannel *channelList;
	EtChannel *channel;
	EtInt	numCurves;
	EtInt	i;
	EtKey *	key;
	EtTime	time;

	/* make sure we have been given the name of a .anim file */
	if (argc < 2) {
		fprintf (stderr, "##### Usage: %s .anim file\n", argv[0]);
		exit (-1);
	}

	/* read in the list of channels */
	channelList = engineAnimReadCurves (argv[1], &numCurves);
	if (channelList == kEngineNULL) {
		fprintf (stderr, "##### Unable to parse %s\n", argv[1]);
		exit (-1);
	}

	/* print the list of keys for each chanel */
	channel = channelList;
	while (channel != kEngineNULL) {
		if (channel->curve != kEngineNULL) {
			printf ("%s {\n", channel->channel);	/* channel name */
			for (i = 0; i < channel->curve->numKeys; i++) {
				key = &(channel->curve->keyList[i]);
				printf ("  %g %g %g %g %g %g\n",
					key->time, key->value,
					key->inTanX, key->inTanY,
					key->outTanX, key->outTanY
				);
			}
			printf ("}\n");
		}
		channel = channel->next;
	}

	/* evaluate the channels for 180 frames */
	for (time = 0; time <= 180.0; time++) {
		channel = channelList;
		while (channel != kEngineNULL) {
			if (channel->curve != kEngineNULL) {
				/* Note: evaluation is in seconds (hence the / 24.0) */
				printf ("%s %g %g\n",
					channel->channel,
					time / 24.0,
					engineAnimEvaluate (channel->curve, time / 24.0)
				);
			}
			channel = channel->next;
		}
	}

	/* free up the channel list */
	engineAnimFreeChannelList (channelList);

	return (0);
}
