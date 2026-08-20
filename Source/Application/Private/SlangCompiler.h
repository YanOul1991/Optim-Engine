#pragma once

#include "OptimEngine/Core/CoreMinimal.h"
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

struct ShaderModule final
{
  Slang::ComPtr<slang::IModule>     shaderModule;
  Slang::ComPtr<slang::IEntryPoint> entryPointVertex;
  Slang::ComPtr<slang::IEntryPoint> entryPointFragment;

  /**
   * Try to load a slang module.
   */
  bool TryLoadModule(const Slang::ComPtr<slang::ISession>& session, const char* moduleName)
  {
    Slang::ComPtr<slang::IBlob> diagnostics;
    shaderModule = session->loadModule(moduleName, diagnostics.writeRef());
    if (!shaderModule) {
      printf("Error: %s\n", (const char*)diagnostics->getBufferPointer());
      return false;
    }
    return true;
  }

  /**
   * Try to load entry points from a shader module.
   *
   * Looks for already defined entry points name: `VertexMain` & `FragmentMain`.
   * Fails if any of those is missing.
   *
   * The module must mark entry points with the `[shader("<shader_stage>")]` attribute.
   */
  bool TryLoadEntryPoints()
  {
    shaderModule->findEntryPointByName("VertexMain", entryPointVertex.writeRef());
    shaderModule->findEntryPointByName("FragmentMain", entryPointFragment.writeRef());

    if (!entryPointVertex) {
      printf("Failed to get entry point 'VertexMain'.\n");
      return false;
    }
    if (!entryPointFragment) {
      printf("Failed to get entry point 'FragmentMain'.\n");
      return false;
    }
    return true;
  }
};

/**
 * Helper class for the slang compiler.
 */
class SlangCompilerInterface final
{
 public:
  inline bool InitalizeGlobalSession()
  {
    // Create Global session
    slang::createGlobalSession(globalSession.writeRef());

    return globalSession != nullptr;
  }

  inline void VerifyTargetSupport()
  {
    if (!globalSession) {
      printf("Error: Global Session is not initialized.\n");
    }

    // Creation of session description~

    slang::SessionDesc sessionDesc = {};

    slang::TargetDesc targetDesc = {};
    targetDesc.format            = SlangCompileTarget::SLANG_SPIRV;
    targetDesc.profile           = globalSession->findProfile("spriv_1_5");

    sessionDesc.targets     = &targetDesc;
    sessionDesc.targetCount = 1;

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
    Slang::ComPtr<slang::ISession> session;
    globalSession->createSession(sessionDesc, session.writeRef());

    // Load modules
    //  - Use `ISession::loadModule()` to load from source file (<some_module>.slang)
    //
    //  - Use `ISession::loadModleFromSourceString()` to load from
    //    code in memory. (eg. vector<char>, string)
    //
    //  Slang documentation basic example:
    //  https://docs.shader-slang.org/en/latest/compilation-api.html#load-modules

    Slang::ComPtr<slang::IModule> shaderModule;
    {
      Slang::ComPtr<slang::IBlob> diagnostics;

      const char* moduleName = "sample";
      shaderModule           = session->loadModule(moduleName, diagnostics.writeRef());

      if (!shaderModule) {
        printf("Error: %s\n", (const char*)diagnostics->getBufferPointer());
        return;
      }
    }

    // Get Entry Points by name
    Slang::ComPtr<slang::IEntryPoint> entryPointVertex;
    Slang::ComPtr<slang::IEntryPoint> entryPointFragment;

    shaderModule->findEntryPointByName("VertexMain", entryPointVertex.writeRef());
    shaderModule->findEntryPointByName("FragmentMain", entryPointFragment.writeRef());

    if (!entryPointVertex) {
      printf("Failed to get entry point 'VertexMain'.\n");
      return;
    }
    else if (!entryPointFragment) {
      printf("Failed to get entry point 'FragmentMain'.\n");
      return;
    }

    // COMPOSE MODULES AND ENTRY POINTS

    std::array<slang::IComponentType*, 3> componentTypes = {
      shaderModule,
      entryPointVertex,
      entryPointFragment
    };

    /***
     * If using a multiple entry points shader like for spirv it it possible to
     * ommi the composed program step and immediatly go to the link step.
     *
     * And call the `link` function of the shader module directly instead.
     *
     * https://docs.shader-slang.org/en/latest/compilation-api.html#get-target-kernel-code
     */
    // Slang::ComPtr<slang::IComponentType> composedProgram;
    // {
    //   Slang::ComPtr<slang::IBlob> diagnostics;
    //   Slang::Result               result = session->createCompositeComponentType(
    //     componentTypes.data(),
    //     componentTypes.size(),
    //     composedProgram.writeRef(),
    //     diagnostics.writeRef());
    //   printf("[Slang Compiler] Composed program creation status: %s\n", (SLANG_FAILED(result) ? "FAILURE" : "SUCCESS"));
    //   if (SLANG_FAILED(result)) {
    //     printf("[Slang Compiler] Error: %s\n", (const char*)diagnostics->getBufferPointer());
    //     return;
    //   }
    // }

    // Validate dependencies
    Slang::ComPtr<slang::IComponentType> linkedProgram;
    {
      Slang::ComPtr<slang::IBlob> diagnostics;
      auto                        result = shaderModule->link(linkedProgram.writeRef(), diagnostics.writeRef());

      printf("[Slang Compiler] Link Status: %s\n", (SLANG_FAILED(result) ? "FAILURE" : "SUCCESS"));
      if (SLANG_FAILED(result)) {
        printf("[Slang Compiler] Error: %s\n", (const char*)diagnostics->getBufferPointer());
        return;
      }
    }

    // Create Target Code for SPIRV
    Slang::ComPtr<slang::IBlob> sprivCode;
    {
      Slang::ComPtr<slang::IBlob> diagnostics;

      auto result = linkedProgram->getTargetCode(0, sprivCode.writeRef(), diagnostics.writeRef());

      printf("[Slang Compiler] SPRIV Kernel Code Generation Status: %s\n", (SLANG_FAILED(result) ? "FAILURE" : "SUCCESS"));
      if (SLANG_FAILED(result)) {
        printf("[Slang Compiler] Error: %s\n", (const char*)diagnostics->getBufferPointer());
        return;
      }
    }

    std::ofstream outFile("Shaders/sample.spv", std::ios::out | std::ios::binary);
    if (!outFile) {
      std::cerr << "[Slang Compiler] Error: could not create the file.\n";
      return;
    }

    outFile.write(reinterpret_cast<const char*>(sprivCode->getBufferPointer()), sprivCode->getBufferSize());
    outFile.close();

    std::cout << "[Slang Compiler] SPRIV Shader successfully created.\n";
  }

 private:
  Slang::ComPtr<slang::IGlobalSession> globalSession;
};
} // namespace Optim::Shaders