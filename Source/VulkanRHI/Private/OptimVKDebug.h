#pragma once

#include "VulkanMinimal.h"

namespace Optim::VK {
  
/**
 * @brief
 * DebugCallback function parameters
 *
 * From Vulkan Tutorial:
 * https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/02_Validation_layers.html#_message_callback
 *
 * @param severity
 * Specifies the severity of the message.
 *
 * Type `enum class vk::DebugUtilsMessageSeverityFlagBitsEXT` with
 * the following values:
 *
 *  - `eVerbose`: Diagnostic message from Vulkan components like the loader, layers and drivers.
 *  - `eInfo`   : Informational message like the creation of a resource.
 *  - `eWarning`: Message about behavior that is not necessarily an error, but very likely a bug in your application.
 *  - `eError`  : Message about behavior that is invalid and may cause crashes.
 *
 * @param messageType
 * Type `vk::DebugUtilsMessageTypeFlagsEXT` with
 * the following values:
 *
 *  - `eGeneral`      : Some event has happened that is unrelated to the specification or performance.
 *  - `eValidation`   : Something has happened that violates the specification or indicates a possible mistake.
 *  - `ePerformance`  : Potential non-optimal use of Vulkan.
 *
 * @param pCallbackData
 * A pointer to a `vk::DebugUtilsMessengerCallbackDataEXT` struct
 * containing details about the message.
 *
 * Most notable data members:
 *  - `pMessage`    : The debug message as a null-terminated string.
 *  - `pObjects`    : Array of Vulkan object handles related to the message.
 *  - `objectCount` : Number of objects in the array.
 *
 * @param pUserData
 * pointer specified during the setup of the callback and allows you to pass your own data to it.
 */
VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT      severity,
                                               vk::DebugUtilsMessageTypeFlagsEXT             messageType,
                                               const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                               void*                                         pUserData) {

  std::cerr << "Validation layer: type " << to_string(messageType) << "msg: " << pCallbackData->pMessage << '\n';

  return vk::False;
}

} // namespace Optim::VK
