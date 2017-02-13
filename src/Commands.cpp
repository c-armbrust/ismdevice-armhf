#include "Commands.h"

const std::string EventType::C2D_COMMAND = "C2D_Command";
const std::string EventType::D2C_COMMAND = "D2C_Command";

const std::string CommandType::UNPROVISION{"Unprovision"};
const std::string CommandType::PROVISION{"Provision"};
const std::string CommandType::START{"Start"};
const std::string CommandType::STOP{"Stop"};
const std::string CommandType::START_PREVIEW{"StartPreview"};
const std::string CommandType::STOP_PREVIEW{"StopPreview"};
const std::string CommandType::GET_DEVICE_SETTINGS{"GetDeviceSettings"};
const std::string CommandType::SET_DEVICE_SETTINGS{"SetDeviceSettings"};
/* obsolete
 * const std::string CommandType::DAT{"D_DAT"};
 * const std::string CommandType::PRV{"D_PRV"};
*/
const std::string CommandType::CAPTURE_UPLOADED{"CaptureUploaded"};
const std::string CommandType::UPDATE_DASHBOARD_CONTROLS{"UpdateDashboardControls"};
const std::string CommandType::FIRMWARE_UPDATE_STATUS{"FirmwareUpdateStatus"};

const std::string CommandStatus::SUCCESS{"Success"};
const std::string CommandStatus::FAILURE{"Failure"};
const std::string CommandStatus::PENDING{"Pending"};
