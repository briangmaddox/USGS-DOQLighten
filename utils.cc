// $Id: utils.cc,v 1.1.1.1 2002-02-19 19:07:48 bmaddox Exp $
// Brian Maddox - USGS MCMC SES - bmaddox@usgs.gov
// Last modified by $Author: bmaddox $ on $Date: 2002-02-19 19:07:48 $
// This is the implementation file for various utilities

#include "utils.h"
#include <math.h>

// Go through the DOQ and calculate the mean, standard deviation, and other
// items necessary to do the contrast stretch
// Note: This calculates the mean of a scanline, then averages that with the 
// mean of the previous scanlines
bool calcStats(DOQImageIFile* indoq, double& mean, double& stddev)
{
  long double doqsamples = indoq->getSamples(); // to avoid multiple fn calls
  long double doqlines = indoq->getLines();     // " "
  long double scantotal = 0.0; // hold the total of the additions for the scanline
  long double diffsqr = 0.0;   // Square of the differences
  unsigned char* scanline = NULL; // pointer to the scanline

  cout << "Calculating mean .";
  cout.flush();
  for (long int count = 0; count < doqlines; count++)
  {
    if ( (count % 500 == 0) )
    {
      cout << ".";
      cout.flush();
    }
    scanline = (unsigned char*)indoq->getRawScanline(count);
    for (int colcount = 0; colcount < doqsamples; colcount++)
      scantotal = scantotal + (long double)scanline[colcount];
    delete [] scanline;
  }

  cout << "Done. " << endl;

  mean = scantotal / (long double)(doqsamples * doqlines);

  // Now we have our mean, calculate the square of the difference
  cout << "Calculating standard deviation .";
  cout.flush();
  for (long int count = 0; count < doqlines; count++)
  {
    if ( (count % 500) == 0)
    {
      cout << ".";
      cout.flush();
    }
    scanline = (unsigned char*)indoq->getRawScanline(count);
    for (long int colcount = 0; colcount < doqsamples; colcount++)
    {
      long double tvalue = (long double)scanline[colcount] - mean;
      diffsqr += ( tvalue * tvalue );
    }
    delete [] scanline;
  }

  // Now go find the square and return the mean
  stddev = sqrt(diffsqr / (doqsamples * doqlines));
  cout << "Done." << endl;

  return true;
}



// Output an error/usage message
void usage(char* myname)
{
  cerr << "Usage: " << myname << " [options] doq-file geotiff-file" << endl;
  cerr << "where options are:" << endl;
  cerr << "  -V     version information" << endl;
  cerr << "  -p     use primary datum information (default)" << endl;
  cerr << "  -s     use secondary datum information" << endl;
  cerr << "  -h     output header file (geotiff-file.hdr)" << endl;
  cerr << "  -?     display this information" << endl;
}



// output the header file
bool writeHeader(DOQImageIFile* doqin, char* filename)
{
  string* temp = NULL;
  char* newname = new char[strlen(filename) + 5];
  strcpy(newname, filename);
  strcat(newname, ".hdr");
  ofstream outfile(newname);
  if (!outfile.good())
  {
    cerr << "Error opening " << newname << " for output!  File not written"
	 << endl;
    outfile.close();
    delete [] newname;
    return false;
  }

  outfile.setf(ios::fixed);
  outfile.precision(5);

  outfile << "FILENAME: " << filename << endl;

  temp = doqin->getQuadName();

  if (temp != NULL)
  {
    outfile << "QUADRANGLE_NAME: " << *temp << endl;  delete temp;
  }
  else
    outfile << "QUADRANGLE_NAME: UNDEFINED" << endl;

  temp = doqin->getQuadrant();
  if (temp != NULL)
  {
    outfile << "QUADRANT: " << *temp << endl; delete temp;
  }
  else
    outfile << "QUADRANT: UNDEFINED" << endl;

  outfile << "WEST_LONGITUDE: " << doqin->getWestHour() << " "
	  << doqin->getWestMinute() << " " << doqin->getWestSecond() << endl;
  outfile << "EAST_LONGITUDE: " << doqin->getEastHour() << " "
	  << doqin->getEastMinute() << " " << doqin->getEastSecond() << endl;
  outfile << "NORTH_LATITUDE: " << doqin->getNorthHour() << " "
	  << doqin->getNorthMinute() << " " << doqin->getNorthSecond() << endl;
  outfile << "SOUTH_LATITUDE: " << doqin->getSouthHour() << " "
	  << doqin->getSouthMinute() << " " << doqin->getSouthSecond() << endl;

  temp = doqin->getProdDate();
  if (temp != NULL)
  {
    outfile << "PRODUCTION_DATE: " << *temp << endl; delete temp;
  }
  else
    outfile << "PRODUCTION_DATE: UNDEFINED" << endl;

  temp = doqin->getRasterOrder();
  if (temp != NULL)
  {
    outfile << "RASTER_ORDER: " << *temp << endl; delete temp;
  }
  else
    outfile << "RASTER_ORDER: UNDEFINED" << endl;

  temp = doqin->getBandOrganization();
  if (temp != NULL)
  {
    outfile << "BAND_ORGANIZATION: " << *temp << endl; delete temp;
  }
  else
    outfile << "BAND_ORGANIZATION: UNDEFINED" << endl;

  for (int bcount = 0; bcount <= 2; bcount++)
  {
    temp = doqin->getBandContent(bcount);
    if (temp != NULL)
    {
      outfile << "BAND_CONTENT: " << *temp << endl; delete temp;
    }
    else
      outfile << "BAND_CONTENT: UNDEFINED" << endl;
  }

  outfile << "BITS_PER_PIXEL: " << doqin->getBitsPerPixel() << endl;
  outfile << "SAMPLES_AND_LINES: " << doqin->getSamples() << " "
	  << doqin->getLines() << endl;

  temp = doqin->getHorizontalDatum();
  if (temp != NULL)
  {
    outfile << "HORIZONTAL_DATUM: " << *temp << endl; delete temp;
  }
  else
    outfile << "HORIZONTAL_DATUM: UNDEFINED" << endl;

  temp = doqin->getHorizontalCoordinateSystem();
  if (temp != NULL)
  {
    outfile << "HORIZONTAL_COORDINATE_SYSTEM: " << *temp << endl; delete temp;
  }
  else
    outfile << "HORIZONTAL_COORDINATE_SYSTEM: UNDEFINED" << endl;

  outfile << "COORDINATE_ZONE: " << doqin->getCoordinateZone() << endl;

  temp = doqin->getHorizontalUnits();
  if (temp != NULL)
  {
    outfile << "HORIZONTAL_UNITS: " << *temp << endl; delete temp;
  }
  else
    outfile << "HORIZONTAL_UNITS: UNDEFINED" << endl;

  outfile << "HORIZONTAL_RESOLUTION: " << doqin->getHorizontalResolution()
          << endl;


  temp = doqin->getSecondaryHorizontalDatum();
  if (temp != NULL)
  {
    outfile << "SECONDARY_HORIZONTAL_DATUM: " << *temp << endl; delete temp;
  }
  else
    outfile << "SECONDARY_HORIZONTAL_DATUM: UNDEFINED" << endl;

  outfile << "XY_ORIGIN: " << doqin->getXOrigin() << " "
	  << doqin->getYOrigin() << endl;
  outfile << "SECONDARY_XY_ORIGIN: " << doqin->getSecondaryXOrigin() << " "
	  << doqin->getSecondaryYOrigin() << endl;

  int numNations = doqin->getNumNations();
  for (int foocount = 1; foocount <= numNations; foocount++)
  {
    temp = doqin->getNation(foocount);
    if (temp != NULL)
    {
      outfile << "NATION: " << *temp << endl; delete temp;
    }
    else
      outfile << "NATION: UNDEFINED" << endl;
  }

  int numStates = doqin->getNumStates();
  for (int foocount = 1; foocount <= numStates; foocount++)
  {
    temp = doqin->getState(foocount);
    if (temp != NULL)
    {
      outfile << "STATE: " << *temp << endl; delete temp;
    }
    else
      outfile << "STATE: UNDEFINED" << endl;
  }

  outfile << "NW_QUAD_CORNER_XY: " << doqin->getNWQuadX() << " "
	  << doqin->getNWQuadY() << endl;
  outfile << "NE_QUAD_CORNER_XY: " << doqin->getNEQuadX() << " "
	  << doqin->getNEQuadY() << endl;
  outfile << "SE_QUAD_CORNERY_XY: " << doqin->getSEQuadX() << " "
	  << doqin->getSEQuadY() << endl;
  outfile << "SW_QUAD_CORNER_XY: " << doqin->getSWQuadX() << " "
	  << doqin->getSWQuadY() << endl;
  outfile << "SECONDARY_NW_QUAD_XY: " << doqin->getSecNWQuadX() << " "
	  << doqin->getSecNWQuadY() << endl;
  outfile << "SECONDARY_NE_QUAD_XY: " << doqin->getSecNEQuadX() << " "
	  << doqin->getSecNEQuadY() << endl;
  outfile << "SECONDARY_SE_QUAD_XY: " << doqin->getSecSEQuadX() << " "
	  << doqin->getSecSEQuadY() << endl;
  outfile << "SECONDARY_SW_QUAD_XY: " << doqin->getSecSWQuadX() << " "
	  << doqin->getSecSWQuadY() << endl;
  outfile << "RMSE_XY: " << doqin->getRMSE() << endl;

  temp = doqin->getImageSource();
  if (temp != NULL)
  {
    outfile << "IMAGE_SOURCE: " << *temp << endl; delete temp;
  }
  else 
    outfile << "IMAGE_SOURCE: UNDEFINED" << endl;

  int numSources = doqin->getNumImageSources();
  for (int foocount = 1; foocount <= numSources; foocount++)
  {
    temp = doqin->getSourceImageID(foocount);
    if (temp != NULL)
    {
      outfile << "SOURCE_IMAGE_ID: " << *temp << endl; delete temp;
    }
    else
      outfile << "SOURCE_IMAGE_ID: UNDEFINED" << endl;
    temp = doqin->getSourceImageDate(foocount);
    if (temp != NULL)
    {
      outfile << "SOURCE_IMAGE_DATE: " << *temp << endl; delete temp;
    }
    else
      outfile << "SOURCE_IMAGE_DATE: UNDEFINED" << endl;
  }
  
  temp = doqin->getSourceDEMDate();
  if (temp != NULL)
  {
    outfile << "SOURCE_DEM_DATE: " << *temp << endl; delete temp;
  }
  else
    outfile << "SOURCE_DEM_DATE: UNDEFINED" << endl;

  temp = doqin->getAgency();
  if (temp != NULL)
  {
    outfile << "AGENCY: " << *temp << endl; delete temp;
  }
  else
    outfile << "AGENCY: UNDEFINED" << endl;

  temp = doqin->getProducer();
  if (temp != NULL)
  {
    outfile << "PRODUCER: " << *temp << endl; delete temp;
  }
  else
    outfile << "PRODUCER: UNDEFINED" << endl;

  temp = doqin->getProductionSystem();
  if (temp != NULL)
  {
    outfile << "PRODUCTION_SYSTEM: " << *temp << endl; delete temp;
  }
  else
    outfile << "PRODUCTION_SYSTEM: UNDEFINED" << endl;

  temp = doqin->getCompression();
  if (temp != NULL)
  {
    outfile << "COMPRESSION: " << *temp << endl; delete temp;
  }
  else
    outfile << "COMPRESSION: UNDEFINED" << endl;

  temp = doqin->getStandardVersion();
  if (temp != NULL)
  {
    outfile << "STANDARD_VERSION: " << *temp << endl; delete temp;
  }
  else
    outfile << "STANDARD_VERSION: UNDEFINED" << endl;

  temp = doqin->getMetaDataDate();
  if (temp != NULL)
  {
    outfile << "METADATA_DATE: " << *temp << endl; delete temp;
  }
  else
    outfile << "METADATA_DATE: UNDEFINED" << endl;

  outfile << "DATA_FILE_SIZE: " << doqin->getDataSize() << endl;
  outfile << "BYTE_COUNT: " << doqin->getByteCount() << endl;
  
  outfile.close();
  delete [] newname;
  return true;
}
