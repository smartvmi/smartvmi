#pragma once

#include "../src/Config.h"
#include <gmock/gmock.h>

class MockConfig : public IConfig
{
  public:
    MOCK_METHOD(void, parseConfiguration, (const Plugin::IPluginConfig&), (override));
    MOCK_METHOD(std::filesystem::path, getSignatureFile, (), (const, override));
    MOCK_METHOD(std::filesystem::path, getOutputPath, (), (const, override));
    MOCK_METHOD(bool, isProcessIgnored, (const std::string& processName), (const, override));
    MOCK_METHOD(bool, isScanAllRegionsActivated, (), (const, override));
    MOCK_METHOD(bool, isDumpingMemoryActivated, (), (const, override));
    MOCK_METHOD(uint64_t, getMaximumScanSize, (), (const, override));
    MOCK_METHOD(void, overrideDumpMemoryFlag, (bool value), (override));
};
