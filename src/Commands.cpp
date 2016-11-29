#include "Commands.h"

const std::string EventType::C2D_COMMAND = "Y_C2D_Command";
const std::string EventType::D2C_COMMAND = "Y_D2C_Command";

const std::string CommandType::UNPROVISION{"Y_Unprovision"};
const std::string CommandType::PROVISION{"Y_Provision"};
const std::string CommandType::START{"Y_Start"};
const std::string CommandType::STOP{"Y_Stop"};
const std::string CommandType::START_PREVIEW{"Y_StartPreview"};
const std::string CommandType::STOP_PREVIEW{"Y_StopPreview"};
const std::string CommandType::GET_DEVICE_SETTINGS{"Y_GetDeviceSettings"};
const std::string CommandType::SET_DEVICE_SETTINGS{"Y_SetDeviceSettings"};
/* obsolete
 * const std::string CommandType::DAT{"D_DAT"};
 * const std::string CommandType::PRV{"D_PRV"};
*/
const std::string CommandType::CAPTURE_UPLOADED{"Y_CaptureUploaded"};
const std::string CommandType::UPDATE_DASHBOARD_CONTROLS{"Y_UpdateDashboardControls"};

const std::string CommandStatus::SUCCESS{"Y_Success"};
const std::string CommandStatus::FAILURE{"Y_Failure"};
const std::string CommandStatus::PENDING{"Y_Pending"};
