////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Copyright (c) 2019 Leonardo Consoni <leonardjc@protonmail.com>            //
//                                                                            //
//  This file is part of Signal-IO-RPi-SPI.                                   //
//                                                                            //
//  Signal-IO-RPi-SPI is free software: you can redistribute it and/or modify //
//  it under the terms of the GNU Lesser General Public License as published  //
//  by the Free Software Foundation, either version 3 of the License, or      //
//  (at your option) any later version.                                       //
//                                                                            //
//  Signal-IO-RPi-SPI is distributed in the hope that it will be useful,      //
//  but WITHOUT ANY WARRANTY; without even the implied warranty of            //
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              //
//  GNU Lesser General Public License for more details.                       //
//                                                                            //
//  You should have received a copy of the GNU Lesser General Public License  //
//  along with Signal-IO-RPi-SPI. If not, see <http://www.gnu.org/licenses/>. //
//                                                                            //
//////////////////////////////////////////////////////////////////////////////// 

#include "interface/signal_io.h"

#include "bcm2835/bcm2835.h"

#define INT16_MAX 65536

int32_t inputValues[ 2 ];
uint16_t outputData[ 2 ];

DECLARE_MODULE_INTERFACE( SIGNAL_IO_INTERFACE );

long int InitDevice( const char* deviceName )
{  
  if( !bcm2835_init() )
  {
    fprintf( stderr, "bcm2835_init failed. Are you running as root??\n" );
    return SIGNAL_IO_DEVICE_INVALID_ID;
  }
  if( !bcm2835_spi_begin() )
  {
    fprintf( stderr, "bcm2835_spi_begin failed. Are you running as root??\n" );
    return SIGNAL_IO_DEVICE_INVALID_ID;
  }
  bcm2835_spi_setBitOrder( BCM2835_SPI_BIT_ORDER_MSBFIRST );      // The default
  bcm2835_spi_setDataMode( BCM2835_SPI_MODE0 );                   // The default
  bcm2835_spi_setClockDivider( BCM2835_SPI_CLOCK_DIVIDER_512 );

  bcm2835_spi_setChipSelectPolarity( BCM2835_SPI_CS0, LOW );      // the default
  bcm2835_spi_setChipSelectPolarity( BCM2835_SPI_CS1, LOW );      // the default

  return 0;
}

void EndDevice( long int deviceID )
{
  bcm2835_spi_end();
  bcm2835_close();
  
  return;
}

size_t GetMaxInputSamplesNumber( long int deviceID )
{
  return 1;
}

size_t Read( long int deviceID, unsigned int channel, double* ref_value )
{
  *ref_value = 0.0;
  
  if( channel > 2 ) return 0;
  
  bcm2835_spi_chipSelect( ( channel == 0 ) ? BCM2835_SPI_CS0 : BCM2835_SPI_CS1 );  
  uint16_t data = outputData[ channel ];
  bcm2835_spi_transfern( (char*) &data, 2 );
  
  int32_t overflowsNumber = inputValues[ channel ] / INT16_MAX;
  if( inputValues[ channel ] < 0 ) overflowsNumber--;
  int32_t newInputValue = overflowsNumber * INT16_MAX + (int32_t) data;
  if( ( inputValues[ channel ] - newInputValue ) > ( INT16_MAX / 2 ) ) overflowsNumber++;
  if( ( inputValues[ channel ] - newInputValue ) < ( -INT16_MAX / 2 ) ) overflowsNumber--;
  inputValues[ channel ] = overflowsNumber * INT16_MAX + (int32_t) data;
  
  *ref_value = (double) inputValues[ channel ];

  return 1;
}

bool HasError( long int deviceID )
{
  return false;
}

void Reset( long int deviceID )
{
  return;
}

bool CheckInputChannel( long int deviceID, unsigned int channel )
{
  if( channel > 2 ) return false;
  
  return true;
}

bool Write( long int deviceID, unsigned int channel, double value )
{
  /*if( channel > 2 )*/ return false;
  
  //bcm2835_spi_chipSelect( ( channel == 0 ) ? BCM2835_SPI_CS0 : BCM2835_SPI_CS1 );  
  //outputData[ channel ] = (uint16_t) value;
  //bcm2835_spi_transfern( (char*) &(outputData[ channel ]), 2 );
  //outputData[ channel ] = (uint16_t) value;
  
  //return true;
}

bool AcquireOutputChannel( long int deviceID, unsigned int channel )
{
  /*if( channel > 2 )*/ return false;
  
  //return true;
}

void ReleaseOutputChannel( long int deviceID, unsigned int channel )
{
  return;
}
