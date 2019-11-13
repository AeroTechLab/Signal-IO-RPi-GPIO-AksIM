/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//  Copyright (c) 2019 Leonardo Consoni <leonardjc@protonmail.com>             //
//                                                                             //
//  This file is part of Signal-IO-RPi-GPIO-AksIM.                             //
//                                                                             //
//  Signal-IO-RPi-GPIO-AksIM is free software: you can redistribute it and/or  //
//  modify it under the terms of the GNU Lesser General Public License as      //
//  published by the Free Software Foundation, either version 3 of the         //
//  License, or (at your option) any later version.                            //
//                                                                             //
//  Signal-IO-RPi-GPIO-AksIM is distributed in the hope that it will be        //
//  useful, but WITHOUT ANY WARRANTY; without even the implied warranty of     //
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               //
//  GNU Lesser General Public License for more details.                        //
//                                                                             //
//  You should have received a copy of the GNU Lesser General Public License   //
//  along with Signal-IO-RPi-GPIO-AksIM. If not, see                           //
//  <http://www.gnu.org/licenses/>.                                            //
//                                                                             //
///////////////////////////////////////////////////////////////////////////////// 

#include "interface/signal_io.h"

#include "bcm2835/bcm2835.h"

int32_t inputValues[ 2 ];

DECLARE_MODULE_INTERFACE( SIGNAL_IO_INTERFACE );

long int InitDevice( const char* configuration )
{  
  uint32_t speedHz = (uint32_t) strtoul( configuration, NULL, 0 );
  
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
  bcm2835_spi_setBitOrder( BCM2835_SPI_BIT_ORDER_MSBFIRST );      
  bcm2835_spi_setDataMode( BCM2835_SPI_MODE1 );                   
  bcm2835_spi_set_speed_hz( speedHz );

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
  // SPI transfer buffer is output (before) and input (after)
  uint8_t inputData[ 2 ];
  bcm2835_spi_transfern( (char*) inputData, 2 );
  uint16_t rawInputValue = ( ( inputData[ 0 ] << 8 ) & 0xFF00 ) + inputData[ 1 ];
  
  int32_t overflowsNumber = inputValues[ channel ] / UINT16_MAX;
  if( inputValues[ channel ] < 0 ) overflowsNumber--;
  int32_t newInputValue = overflowsNumber * UINT16_MAX + rawInputValue;
  if( ( inputValues[ channel ] - newInputValue ) > ( UINT16_MAX / 2 ) ) overflowsNumber++;
  if( ( inputValues[ channel ] - newInputValue ) < ( -UINT16_MAX / 2 ) ) overflowsNumber--;
  inputValues[ channel ] = overflowsNumber * UINT16_MAX + rawInputValue;
  
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
  return false;
}

bool AcquireOutputChannel( long int deviceID, unsigned int channel )
{
  return false;
}

void ReleaseOutputChannel( long int deviceID, unsigned int channel )
{
  return;
}
