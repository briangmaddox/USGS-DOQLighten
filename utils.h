// $Id: utils.h,v 1.1.1.1 2002-02-19 19:07:48 bmaddox Exp $
// Brian Maddox - USGS MCMC SES - bmaddox@usgs.gov
// Last modified by $Author: bmaddox $ on $Date: 2002-02-19 19:07:48 $
// This header defines certain utility functions for this program. They have
// been put into this file because the main file was getting far too large
#ifndef DOQLIGHTEN_UTILS_H
#define DOQLIGHTEN_UTILS_H

#include "Image/DOQImageIFile.h"
#include <iostream.h>
#include <string.h>

// Calculate the stats we'll need for this
bool calcStats(DOQImageIFile* indoq, double& mean, double& stddev);

// Output a usage statement
void usage(char* myname);

// Output the header information
bool writeHeader(DOQImageIFile* doqin, char* filename);


#endif // #ifndef DOQLIGHTEN_UTILS_H
