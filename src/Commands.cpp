#include "Commands.h"

const std::string EventType::C2D_COMMAND = "D_C2D_Command";
const std::string EventType::D2C_COMMAND = "D_D2C_Command";

const std::string CommandType::UNPROVISION{"D_Unprovision"};
const std::string CommandType::PROVISION{"D_Provision"};
const std::string CommandType::START{"D_Start"};
const std::string CommandType::STOP{"D_Stop"};
const std::string CommandType::START_PREVIEW{"D_StartPreview"};
const std::string CommandType::STOP_PREVIEW{"D_StopPreview"};
const std::string CommandType::GET_DEVICE_SETTINGS{"D_GetDeviceSettings"};
const std::string CommandType::SET_DEVICE_SETTINGS{"D_SetDeviceSettings"};
const std::string CommandType::DAT{"D_DAT"};
const std::string CommandType::PRV{"D_PRV"};
const std::string CommandType::UPDATE_DASHBOARD_CONTROLS{"D_UpdateDashboardControls"};

const std::string CommandStatus::SUCCESS{"D_Success"};
const std::string CommandStatus::FAILURE{"D_Failure"};
const std::string CommandStatus::PENDING{"D_Pending"};
