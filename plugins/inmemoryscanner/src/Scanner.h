#pragma once

#include "Config.h"
#include "Dumping.h"
#include "OutputXML.h"
#include "Semaphore.h"
#include "YaraInterface.h"
#include <condition_variable>
#include <memory>
#include <vmicore/plugins/PluginInterface.h>
#include <yara/limits.h> // NOLINT(modernize-deprecated-headers)

class Scanner
{
  public:
    Scanner(const Plugin::PluginInterface* pluginInterface,
            std::shared_ptr<IConfig> configuration,
            std::unique_ptr<YaraInterface> yaraEngine,
            std::unique_ptr<IDumping> dumping);

    static std::unique_ptr<std::string> getFilenameFromPath(const std::string& path);

    void scanProcess(std::shared_ptr<const ActiveProcessInformation> processInformation);

    void scanAllProcesses();

    void saveOutput();

  private:
    const Plugin::PluginInterface* pluginInterface;
    std::shared_ptr<IConfig> configuration;
    std::unique_ptr<YaraInterface> yaraEngine;
    OutputXML outputXml{};
    std::unique_ptr<IDumping> dumping;
    std::filesystem::path inMemoryResultsTextFile;
    Semaphore<std::mutex, std::condition_variable> semaphore =
        Semaphore<std::mutex, std::condition_variable>(YR_MAX_THREADS);

    bool shouldRegionBeScanned(const MemoryRegion& memoryRegionDescriptor);

    void scanMemoryRegion(pid_t pid, const std::string& processName, const MemoryRegion& memoryRegionDescriptor);

    void logInMemoryResultToTextFile(const std::string& processName,
                                     pid_t pid,
                                     Plugin::virtual_address_t baseAddress,
                                     const std::vector<Rule>& results);
};
