/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//               Copyright (C) 2011-2017 - The DESY CMS Group                  //
//                           All rights reserved                               //
//                                                                             //
//      The CMStkModLab source code is licensed under the GNU GPL v3.0.        //
//      You have the right to modify and/or redistribute this source code      //
//      under the terms specified in the license, which may be found online    //
//      at http://www.gnu.org/licenses or at License.txt.                      //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#ifdef USE_FAKEIO
#include "LeyboldGraphixThreeFake.h"
typedef LeyboldGraphixThreeFake LeyboldGraphixThree_t;
#else
#include "LeyboldGraphixThree.h"
typedef LeyboldGraphixThree LeyboldGraphixThree_t;
#endif

#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
  LeyboldGraphixThree_t leybold("/dev/ttyUSB0");

  std::cout << "version:                 " << leybold.GetVersion() << std::endl;
  std::cout << "serial number:           " << leybold.GetSerialNumber() << std::endl;
  std::cout << "item number:             " << leybold.GetItemNumber() << std::endl;

  leybold.SetDisplayUnit(VLeyboldGraphixThree::DisplayUnit_mbar);
  VLeyboldGraphixThree::DisplayUnit unit = leybold.GetDisplayUnit();
  std::cout << "display unit:            " << leybold.GetDisplayUnitName(unit) << std::endl << std::endl;

  std::cout << "number of channels:      " << leybold.GetNumberOfChannels() << std::endl << std::endl;

  leybold.SetSensorName(1, "SYS");
  leybold.SetSensorName(2, "P1");
  leybold.SetSensorName(3, "P2");

  leybold.SetSetPointChannelAssignment(1, VLeyboldGraphixThree::SetPointChannelOff);
  leybold.SetSetPointChannelAssignment(2, VLeyboldGraphixThree::SetPointChannelOff);
  leybold.SetSetPointChannelAssignment(3, VLeyboldGraphixThree::SetPointChannelOff);
  leybold.SetSetPointChannelAssignment(4, VLeyboldGraphixThree::SetPointChannelOff);
  leybold.SetSetPointChannelAssignment(5, VLeyboldGraphixThree::SetPointChannelOff);
  leybold.SetSetPointChannelAssignment(6, VLeyboldGraphixThree::SetPointChannelOff);

  for (int i=1;i<4;++i) {

    if (leybold.GetSensorDetectionMode(i)!=VLeyboldGraphixThree::SensorDetectionAuto) {
      leybold.SetSensorDetectionMode(i, VLeyboldGraphixThree::SensorDetectionAuto);
    }

    VLeyboldGraphixThree::SensorType type = leybold.GetSensorType(i);

    if (type==VLeyboldGraphixThree::SensorType_NOSEN) continue;

    if (type==VLeyboldGraphixThree::SensorType_TTRx) {
      leybold.SetSensorType(i, VLeyboldGraphixThree::SensorType_TTR91N);
    }

    // leybold.SetSensorType(i, VLeyboldGraphixThree::SensorType_TTRx);

    std::cout << "sensor " << i << std::endl;

    std::cout << "  detection mode:        " << leybold.GetSensorDetectionMode(i) << std::endl;
    std::cout << "  type:                  " << leybold.GetSensorTypeName(i) << std::endl;
    leybold.SetSensorName(i, std::string("SENSOR")+std::to_string(i));
    std::cout << "  name:                  " << leybold.GetSensorName(i) << std::endl;
    VLeyboldGraphixThree::SensorStatus status = leybold.GetSensorStatus(i);
    std::cout << "  status:                " << leybold.GetSensorStatusText(status) << std::endl;
    std::cout << "  pressure:              " << leybold.GetPressure(i) << std::endl;

    std::cout << std::endl;
  }

  /*

  for (int i=1;i<7;++i) {

    int setPointAssignment = leybold.GetSetPointChannelAssignment(i);

    if (setPointAssignment==0) continue;

    std::cout << "set point " << i << std::endl;

    leybold.SetSetPointOnPressure(i, 800);
    leybold.SetSetPointOffPressure(i, 850);

    std::cout << "  assignment:            " << leybold.GetSetPointChannelAssignment(i) << std::endl;
    std::cout << "  on pressure:           " << leybold.GetSetPointOnPressure(i) << std::endl;
    std::cout << "  off pressure:          " << leybold.GetSetPointOffPressure(i) << std::endl;
    std::cout << "  status:                " << leybold.GetSetPointStatus(i) << std::endl;

    std::cout << std::endl;
  }

  */

  return 0;
}
