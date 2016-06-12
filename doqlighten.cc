// $Id: doqlighten.cc,v 1.1.1.1 2002-02-19 19:07:48 bmaddox Exp $
// Brian Maddox - USGS MCMC - bmaddox@usgs.gov
// Last modified by $Author: bmaddox $ on $Date: 2002-02-19 19:07:48 $
// This program takes in a DOQ and does a Two Standard Deviation Contrast 
// Stretch on it so that the output GeoTIFF is brighter than the original
// native DOQ
   
#include "Image/DOQImageIFile.h"
#include "Image/GeoTIFFImageOFile.h"
#include "Image/RGBPallette.h"
#include "Image/RGBPixel.h"
#include "FrodoLib/SpinnerThingy.h"
#include "utils.h"
#include <iostream.h>
#include <string.h>
#include <string>
#include <strstream.h>
#include <time.h>
#include <stdio.h>
#include <fstream.h>
#include <getopt.h>
#include <math.h>

#define SOFTWARE "USGS DOQ to GeoTIFF contrast stretcher $Revision: 1.1.1.1 $"

int main (int argc, char* argv[])
{
  double rgbvalue = 0.0;     // RGB value for the stretch
  unsigned char color = 0;   // actual color to put into the colormap

  double mean = 0.0;               // Mean value of the DOQ data
  double stddev = 0.0;             // Standard deviation
  double difference = 0.0;         // To reduce the # of computations

  double min = 0.0;                // Calculated Minimum
  double max = 0.0;                //    ""      Maxmium
  int imin, imax = 0;              // integer version of above

  int myopt;  // for getopt
  bool usePrimaryDatum = true;   // whether or not to use the primary or
                                 // secondary datum 
  bool printHeader = false;      // self-evident

  SpinnerThingy thingy;
  DOQImageIFile* doqin;
  GeoTIFFImageOFile* geoout = NULL;
  RGBPixel* colors = NULL;
  unsigned char* lineu = NULL;
  char desc_string[100];
  char time_string[20];
  string* _tname = NULL;
  string* _tstate = NULL;
  string* _tquadrant = NULL;
  string* _tdate = NULL;
  char timebuff[12];
  int _tyear, _tmonth, _tday;
  int tDatYear = 0;
  string* _datum = NULL;
  string* footemp;
  long int doqlines, doqsamples;

  if (argc == 1) // if they just call it with no arguments
  {
    usage(argv[0]);
    exit(-1);
  }


  while ((myopt = getopt(argc, argv, "Vpsh?")) != -1)
    switch (myopt)
    {
    case 'V':
    {
      cout << argv[0] << ": $Revision: 1.1.1.1 $" << endl;
      exit(-1);
      break; // to make parser happy
    }
    case 'p':
    {
      usePrimaryDatum = true;
      break;
    }
    case 's':
    {
      usePrimaryDatum = false;
      break;
    }
    case 'h':
    {
      printHeader = true;
      break;
    }
    case '?':
    {
      usage(argv[0]);
      exit(-1);
      break;
    }
    default:
    {
      usage(argv[0]);
      exit(-1);
      break;
    }
    }

  // No command line args left?  Didn't call it correctly
  if (optind >= argc)
  {
    usage(argv[0]);
    exit(-1);
  }

  // This is a hack.  If you just send in a -, getopt doesn't process it like
  // you'd expect.  Basically, if the next argv element is NULL, the strcmp
  // would fail anyways.  BGM 1 June 1998
  if (argv[optind + 1] == NULL)
  {
    usage(argv[0]);
    exit(-1);
  }

  // added so users can't overwrite the input file... yep.   BGM 20 Nov 1997
  if (strcmp(argv[optind], argv[optind + 1]) == 0)
  {
    cerr << argv[0] << ": Input and output filenames do not differ. Exiting..."
	 << endl;
    exit(-1);
  }

  // Attempt to open files here
  // Open the DOQ and check to make sure everything is ok
  doqin = new DOQImageIFile(argv[optind++]);
  if (!doqin->good())
  {
    cerr << "Cannot open " << argv[optind - 1] << endl;
    delete doqin;
    exit(-1);
  }

  footemp = doqin->getBandOrganization(); // so we know how to handle later

  doqlines = doqin->getLines();
  doqsamples = doqin->getSamples();

  // Open the GeoTIFF and make sure everything is ok
  if (*footemp != "SINGLE FILE")
  {
    cerr << "Error: This program only works on Greyscale DOQ data. Exiting.."
	 << endl;
    delete doqin;
    exit(-1);
  }
  else
  {
    geoout = new GeoTIFFImageOFile(argv[optind], doqsamples, doqlines, 1,
				   doqin->getBitsPerPixel(), COMPRESSION_NONE,
				   PHOTOMETRIC_MINISBLACK);
  }

  if (!geoout->good())
  {
    cerr << "Cannot open " << argv[optind] << endl;
    delete doqin;
    delete geoout;
    return(-1);
  }

  // Change the Image lib's default resolution units
  geoout->setTag(TIFFTAG_RESOLUTIONUNIT, 
		 (unsigned short int)RESUNIT_CENTIMETER);

  // Output the header data to the text file if the user requested
  if (printHeader)
  {
    cout << "Writing header data... ";
    if (writeHeader(doqin, argv[optind]))
      cout << "Finished." << endl;
    else
      cout << "Failed!" << endl;
  }

  // Calculate the statistics for the DOQ
  calcStats(doqin, mean, stddev);
  min = mean - 2 * stddev;
  if (min < 0)
    min = 0;
  max = mean + 2 * stddev;
  if (max > 255)
    max = 255;
  difference = (256 / (max - min));
  
  // Set here to avoid multiple function calls
  imin = (int)rint(min);
  imax = (int)rint(max);

  // Go through and generate the pallette
  colors = new RGBPixel[256];
  for (int foocount = 0; foocount < imin; foocount++)
  {
    colors[foocount].setRed(0);
    colors[foocount].setGreen(0);
    colors[foocount].setBlue(0);
  }
  for (int foocount = imin; foocount < imax; foocount++)
  {
    if (rgbvalue > 255.0)
      color = 255;
    else if (rgbvalue < 0.0)
      color = 0;
    else
      color = (unsigned char)rint(rgbvalue);
    colors[foocount].setRed(color);
    colors[foocount].setGreen(color);
    colors[foocount].setBlue(color);
    rgbvalue = rgbvalue + difference;
  }
  for (int foocount = imax; foocount < 256; foocount++)
  {  
    colors[foocount].setRed(255);
    colors[foocount].setGreen(255);
    colors[foocount].setBlue(255);
  }

  if (usePrimaryDatum)
    _datum = doqin->getHorizontalDatum();
  else
    _datum = doqin->getSecondaryHorizontalDatum();

  if (!_datum)
  {
    cerr << "Datum not found.  Assuming NAD83" << endl;
    tDatYear = 1983;
  }
  else
  {
    if (*_datum == "NAD27")
      tDatYear = 1927;
    else if (*_datum == "NAD83")
      tDatYear = 1983;
    else 
    {
      cerr << "Unsupported datum of " << *_datum << ". Using NAD 83" << endl;
      tDatYear = 1983;
    }
  }
  
  // Get the date of the DOQ and encode it into a GeoTIFF date string
  _tdate = doqin->getProdDate();
  if (_tdate != NULL)
    strcpy(timebuff, _tdate->c_str()); 
  istrstream is(timebuff, 12);
  is >> _tyear >> _tmonth >> _tday;
  sprintf(time_string,"%4d:%02d:%02d %02d:%02d:%02d",
	  _tyear, _tmonth, _tday,
	  0, 0, 0);
 


  _tname = doqin->getQuadName();
  if (!_tname)
  {
    cerr << "No quad name.  Assuming name of NONAME DOQ" << endl;
    _tname = new string("NONAME DOQ");
  }

  _tstate = doqin->getState(1);
  _tquadrant = doqin->getQuadrant();
  if (_tquadrant == NULL) // Must be a full Quad
  {
    if (!_tstate)
      sprintf(desc_string, "USGS DOQ 1:24000 Quad of %s.", _tname->c_str());
    else
      sprintf(desc_string, "USGS DOQ 1:24000 Quad of %s,%s.", _tname->c_str(),
	      _tstate->c_str());
    geoout->setTag(TIFFTAG_XRESOLUTION, (float)240.0);
    geoout->setTag(TIFFTAG_YRESOLUTION, (float)240.0);
  }
  else // It's a quarter quad, assuming 12000 scale
  {
    if (!_tstate)
      sprintf(desc_string, "USGS DOQ 1:12000 %s Q-Quad of %s.",
	      _tquadrant->c_str(), _tname->c_str());
    else
      sprintf(desc_string, "USGS DOQ 1:12000 %s Q-Quad of %s,%s.",
	      _tquadrant->c_str(), _tname->c_str(), _tstate->c_str());
    geoout->setTag(TIFFTAG_XRESOLUTION, (float)120.0);
    geoout->setTag(TIFFTAG_YRESOLUTION, (float)120.0);
  }

  // Set the GeoTIFF and TIFF tags
  geoout->setTag(TIFFTAG_DATETIME,time_string);
  geoout->setTag(TIFFTAG_SOFTWARE,SOFTWARE);
  geoout->setTag(TIFFTAG_IMAGEDESCRIPTION,desc_string);
  if (usePrimaryDatum)
    geoout->setUSGS_UTM(doqin->getXOrigin(), doqin->getYOrigin(),
			doqin->getHorizontalResolution(),
			doqin->getHorizontalResolution(),
			doqin->getCoordinateZone(),
			tDatYear);
  else
    geoout->setUSGS_UTM(doqin->getSecondaryXOrigin(),
			doqin->getSecondaryYOrigin(),
			doqin->getHorizontalResolution(),
			doqin->getHorizontalResolution(),
			doqin->getCoordinateZone(), tDatYear);




  // Go do the actual conversion
  cout << "Converting " << doqlines << " scanlines to GeoTIFF..." << endl;
  for (int j=0; j < doqlines; j++)
  {
    if ((j % 128) == 0)
      thingy.update(j); // Update the status spinner
    
    lineu = (unsigned char *) doqin->getRawScanline(j);
    // Go through and change the pixel element to what's in the pallette
    for (int colcount = 0; colcount < doqin->getSamples(); colcount++)
      lineu[colcount] = colors[lineu[colcount]].getRed();
    geoout->putRawScanline(lineu, j);
    delete [] lineu;
  }

  thingy.done("Done.");
  
  delete doqin;
  delete geoout;
  delete _tname;
  delete _tquadrant;
  delete _tdate;
  delete _datum;
  delete footemp;
  delete [] colors;
  return 0;
}


