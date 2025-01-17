#include "VmiHub.h"
#include "GlobalControl.h"
#include "os/linux/ActiveProcessesSupervisor.h"
#include "os/linux/SystemEventSupervisor.h"
#include "os/windows/ActiveProcessesSupervisor.h"
#include "os/windows/SystemEventSupervisor.h"

#include <csignal>
#include <memory>
#include <utility>

namespace
{
    int exitCode = 0;
    const std::string loggerName = std::filesystem::path(__FILE__).filename().stem();
}

VmiHub::VmiHub(std::shared_ptr<IConfigParser> configInterface,
               std::shared_ptr<ILibvmiInterface> vmiInterface,
               std::shared_ptr<ILogging> loggingLib,
               std::shared_ptr<IEventStream> eventStream,
               std::shared_ptr<IInterruptFactory> interruptFactory)
    : configInterface(std::move(configInterface)),
      vmiInterface(std::move(vmiInterface)),
      loggingLib(std::move(loggingLib)),
      logger(NEW_LOGGER(this->loggingLib)),
      eventStream(std::move(eventStream)),
      interruptFactory(std::move(interruptFactory))
{
}

void VmiHub::waitForEvents() const
{
    // TODO: only set postRunPluginAction to true after sample process is started
    GlobalControl::postRunPluginAction = true;
#ifdef TRACE_MODE
    auto loopStart = std::chrono::steady_clock::now();
#endif
    while (!GlobalControl::endVmi)
    {
        try
        {
#ifdef TRACE_MODE
            auto callStart = std::chrono::steady_clock::now();
            vmiInterface->waitForEvent();
            auto currentTime = std::chrono::steady_clock::now();
            auto callDuration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - callStart);
            auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - loopStart);
            logger->debug("Event loop call",
                          {logfield::create("durationMilliseconds", callDuration.count()),
                           logfield::create("totalElapsedTimeSeconds", elapsedTime.count())});
#else
            vmiInterface->waitForEvent();
#endif
        }
        catch (const std::exception& e)
        {
            logger->error("Error while waiting for events", {logfield::create("exception", e.what())});
            eventStream->sendErrorEvent(e.what());
            logger->info("Trying to get the VM state");

            exitCode = 1;
            GlobalControl::endVmi = true;
        }
    }
}

void VmiHub::performShutdownPluginAction() const
{
    vmiInterface->pauseVm();
    pluginSystem->passShutdownEventToRegisteredPlugins();
    vmiInterface->resumeVm();
}

void logReceivedSignal(int signal)
{
    if (signal > 0)
    {
        switch (signal)
        {
            case SIGINT:
            {
                GlobalControl::logger()->info("externalInterruptHandler: SIGINT received",
                                              {logfield::create("logger", loggerName)});
                break;
            }
            case SIGTERM:
            {
                GlobalControl::logger()->info("externalInterruptHandler: SIGTERM received",
                                              {logfield::create("logger", loggerName)});
                break;
            }
            default:
            {
                GlobalControl::logger()->error("Called for unhandled signal. This should never occur",
                                               {logfield::create("logger", loggerName)});
            }
        }
    }
}

void externalInterruptHandler(int signal)
{
    exitCode = 128 + signal;
    logReceivedSignal(signal);
    GlobalControl::endVmi = true;
}

void setupSignalHandling()
{
    struct sigaction sigactionStruct
    {
    };
    sigactionStruct.sa_handler = &externalInterruptHandler;
    auto status = sigaction(SIGINT, &sigactionStruct, nullptr);
    if (status != 0)
    {
        throw std::runtime_error("Unable to register SIGINT action handler.");
    }
    status = sigaction(SIGTERM, &sigactionStruct, nullptr);
    if (status != 0)
    {
        throw std::runtime_error("Unable to register SIGTERM action handler.");
    }
}

uint VmiHub::run(const std::unordered_map<std::string, std::vector<std::string>>& pluginArgs)
{
    vmiInterface->initializeVmi();
    std::shared_ptr<IActiveProcessesSupervisor> activeProcessesSupervisor;
    switch (vmiInterface->getOsType())
    {
        case VMI_OS_LINUX:
        {
            activeProcessesSupervisor =
                std::make_shared<Linux::ActiveProcessesSupervisor>(vmiInterface, loggingLib, eventStream);
            pluginSystem = std::make_shared<PluginSystem>(configInterface,
                                                          vmiInterface,
                                                          activeProcessesSupervisor,
                                                          std::make_shared<LegacyLogging>(configInterface),
                                                          loggingLib,
                                                          eventStream);
            systemEventSupervisor = std::make_shared<Linux::SystemEventSupervisor>(vmiInterface,
                                                                                   pluginSystem,
                                                                                   activeProcessesSupervisor,
                                                                                   configInterface,
                                                                                   interruptFactory,
                                                                                   loggingLib,
                                                                                   eventStream);
            break;
        }
        case VMI_OS_WINDOWS:
        {
#if defined(ARM64)
            throw new std::runtime_error("No support for Windows on ARM yet.");
#endif
            auto kernelObjectExtractor = std::make_shared<Windows::KernelAccess>(vmiInterface);
            activeProcessesSupervisor = std::make_shared<Windows::ActiveProcessesSupervisor>(
                vmiInterface, kernelObjectExtractor, loggingLib, eventStream);
            pluginSystem = std::make_shared<PluginSystem>(configInterface,
                                                          vmiInterface,
                                                          activeProcessesSupervisor,
                                                          std::make_shared<LegacyLogging>(configInterface),
                                                          loggingLib,
                                                          eventStream);
            systemEventSupervisor = std::make_shared<Windows::SystemEventSupervisor>(vmiInterface,
                                                                                     pluginSystem,
                                                                                     activeProcessesSupervisor,
                                                                                     configInterface,
                                                                                     interruptFactory,
                                                                                     loggingLib,
                                                                                     eventStream);
            break;
        }
        default:
        {
            throw std::runtime_error("Unknown operating system.");
        }
    }

    for (auto& plugin : configInterface->getPlugins())
    {
        pluginSystem->initializePlugin(plugin.first,
                                       plugin.second,
                                       pluginArgs.contains(plugin.first) ? pluginArgs.at(plugin.first)
                                                                         : std::vector<std::string>{plugin.first});
    }

    vmiInterface->pauseVm();
    systemEventSupervisor->initialize();
    vmiInterface->resumeVm();

    eventStream->sendReadyEvent();

    setupSignalHandling();
    waitForEvents();
    if (GlobalControl::postRunPluginAction)
    {
        performShutdownPluginAction();
    }
    systemEventSupervisor->teardown();
    return exitCode;
}
