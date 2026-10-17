#pragma once

#include "Core/CoreMinimal.h"
#include "Core/Debug/Debug.h"
#include "slang.h"
// slang.h first
#include "slang-com-helper.h"
#include "slang-com-ptr.h"

#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

/**
 * Slang Compilation API example:
 * https://docs.shader-slang.org/en/latest/compilation-api.html
 */

namespace Optim::Shaders
{

enum class EShaderCompileError
{
  None,
  FailedToLoadModule,
  FailedToLoadEntryPoint,
  FailedToComposeProgram,
  FailedToLinkProgram,
  FailedToGenerateTargetCode
};

// Helper function to check if a shader compile error is successful.
bool Success(EShaderCompileError error)
{
  return error == EShaderCompileError::None;
}

struct ShaderModule final
{
  Slang::ComPtr<slang::IModule>     shaderModule;
  Slang::ComPtr<slang::IEntryPoint> entryPointVertex;
  Slang::ComPtr<slang::IEntryPoint> entryPointFragment;

  // TODO:
  // Separate module loading from module linking and compilation.
  EShaderCompileError LoadFromModule(Slang::ComPtr<slang::ISession>& session, const char* moduleName)
  {
    Slang::ComPtr<slang::IBlob> diagnostics;

    // Load the module from the session. The module must be in the search path of the session.
    shaderModule = session->loadModule(moduleName, diagnostics.writeRef());
    if (!shaderModule) {
      Debug::PrintLn("Error: {}", (const char*)diagnostics->getBufferPointer());
      return EShaderCompileError::FailedToLoadModule;
    }

    // Load shader entry points:
    //  Name convention:
    //    VertexMain
    //    FragmentMain
    //
    // The vertex and fragment shader entry points must exist in the module
    // or else the shader is invalid.
    //
    // Could eventually use the reflection API to allow more flexible naming
    // of entry points, but for now this is a simple convention.

    Slang::ComPtr<slang::IEntryPoint> entryPointVertex;
    Slang::ComPtr<slang::IEntryPoint> entryPointFragment;

    shaderModule->findEntryPointByName("VertexMain", entryPointVertex.writeRef());
    shaderModule->findEntryPointByName("FragmentMain", entryPointFragment.writeRef());

    if (!entryPointVertex) {
      Debug::PrintLn("Failed to get entry point 'VertexMain'.");
      return EShaderCompileError::FailedToLoadEntryPoint;
    }
    if (!entryPointFragment) {
      Debug::PrintLn("Failed to get entry point 'FragmentMain'.");
      return EShaderCompileError::FailedToLoadEntryPoint;
    }

    // Validate dependencies
    Slang::ComPtr<slang::IComponentType> linkedProgram;

    auto result = shaderModule->link(linkedProgram.writeRef(), diagnostics.writeRef());

    if (SLANG_FAILED(result)) {
      // printf("[Slang Compiler] Error: %s\n", (const char*)diagnostics->getBufferPointer());
      Debug::PrintLn("[Slang Compiler] Error: {}", (const char*)diagnostics->getBufferPointer());
      return EShaderCompileError::FailedToLinkProgram;
    }

    // Compile to target code (SPIR-V)
    Slang::ComPtr<slang::IBlob> targetCode;
    result = linkedProgram->getTargetCode(0, targetCode.writeRef(), diagnostics.writeRef());
    if (SLANG_FAILED(result)) {
      Debug::PrintLn("[Slang Compiler] Error: {}", (const char*)diagnostics->getBufferPointer());
      return EShaderCompileError::FailedToGenerateTargetCode;
    }

    // Output binaries
    std::string   outputFileName = "Shaders/" + std::string(moduleName) + ".spv";
    std::ofstream outputFile(outputFileName, std::ios::binary);

    if (!outputFile) {
      Debug::PrintLn("[Slang Compiler] Failed to open output file: {}", outputFileName);
      return EShaderCompileError::FailedToGenerateTargetCode;
    }
    outputFile.write(reinterpret_cast<const char*>(targetCode->getBufferPointer()), targetCode->getBufferSize());
    outputFile.close();
    return EShaderCompileError::None;
  }

  [[nodiscard]]
  inline bool IsValid() const
  {
    return shaderModule != nullptr &&
           entryPointVertex != nullptr &&
           entryPointFragment != nullptr;
  }

  /**
   * Try to load a slang module.
   */
  // [[deprecated]]
  // bool TryLoadModule(const Slang::ComPtr<slang::ISession>& session, const char* moduleName)
  // {
  //   Slang::ComPtr<slang::IBlob> diagnostics;
  //   shaderModule = session->loadModule(moduleName, diagnostics.writeRef());
  //   if (!shaderModule) {
  //     printf("Error: %s\n", (const char*)diagnostics->getBufferPointer());
  //     return false;
  //   }
  //   return true;
  // }

  /**
   * Try to load entry points from a shader module.
   *
   * Looks for already defined entry points name: `VertexMain` & `FragmentMain`.
   * Fails if any of those is missing.
   *
   * The module must mark entry points with the `[shader("<shader_stage>")]` attribute.
   */
  // [[deprecated]]
  // bool TryLoadEntryPoints()
  // {
  //   shaderModule->findEntryPointByName("VertexMain", entryPointVertex.writeRef());
  //   shaderModule->findEntryPointByName("FragmentMain", entryPointFragment.writeRef());
  //   if (!entryPointVertex) {
  //     printf("Failed to get entry point 'VertexMain'.\n");
  //     return false;
  //   }
  //   if (!entryPointFragment) {
  //     printf("Failed to get entry point 'FragmentMain'.\n");
  //     return false;
  //   }
  //   return true;
  // }
};

/**
 * Helper class for the slang compiler.
 */
class SlangSession final
{
 public:
  inline bool InitGlobal()
  {
    // Create Global session
    slang::createGlobalSession(globalSession.writeRef());
    return globalSession != nullptr;
  }

  inline void CreateSession()
  {
    if (!globalSession) {
      Debug::PrintLn("[Slang Compiler] Global session is not initialized.");
      return;
    }

    // Session and target description
    slang::SessionDesc sessionDesc = {};
    slang::TargetDesc  targetDesc  = {};
    targetDesc.format              = SlangCompileTarget::SLANG_SPIRV;
    targetDesc.profile             = globalSession->findProfile("spriv_1_5");
    sessionDesc.targets            = &targetDesc;
    sessionDesc.targetCount        = 1;

    slang::PreprocessorMacroDesc preprocessorMacroDesc[] = {
      {  "BIAS_VALUE",  "1138" },
      { "OTHER_MACRO", "float" }
    };
    // Preprocessor defines
    sessionDesc.preprocessorMacros     = preprocessorMacroDesc;
    sessionDesc.preprocessorMacroCount = std::size(preprocessorMacroDesc);

    // Compiler Options
    std::array<slang::CompilerOptionEntry, 1> options = {
      { slang::CompilerOptionName::EmitSpirvDirectly,
       { slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr } }
    };
    sessionDesc.compilerOptionEntries    = options.data();
    sessionDesc.compilerOptionEntryCount = options.size();

    // Location where slang modules live. For now only in the Shaders directory
    const char* searchPaths[] = {
      "Shaders"
    };

    sessionDesc.searchPaths     = searchPaths;
    sessionDesc.searchPathCount = std::size(searchPaths);

    // Create Comilation Session
    globalSession->createSession(sessionDesc, session.writeRef());
  }

  Slang::ComPtr<slang::ISession>& GetGlobalSession() noexcept { return session; }

 private:
  Slang::ComPtr<slang::IGlobalSession> globalSession;
  Slang::ComPtr<slang::ISession>       session;
};
} // namespace Optim::Shaders