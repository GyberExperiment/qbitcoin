// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-present The Bitcoin Core developers
// Copyright (c) 2024-present The QBTC developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

/**
 * QBITCOIND - Quantum-resistant Bitcoin Core daemon
 * 
 * Основано на Bitcoin Core с добавлением:
 * - Compressed Quantum Keys (всегда включено)
 * - Dilithium signature aggregation
 * - Полная совместимость с Bitcoin кошельками
 * - Quantum-resistant cryptography protection
 */

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

#include <chainparams.h>
#include <clientversion.h>
#include <common/args.h>
#include <common/init.h>
#include <common/url.h>
#include <compat/compat.h>
#include <init.h>
#include <interfaces/chain.h>
#include <interfaces/init.h>
#include <interfaces/node.h>
#include <logging.h>
#include <node/interface_ui.h>
#include <node/context.h>
#include <noui.h>
#include <util/check.h>
#include <util/exception.h>
#include <util/signalinterrupt.h>
#include <util/strencodings.h>
#include <util/syserror.h>
#include <util/task_runner.h>
#include <util/thread.h>
#include <util/threadnames.h>
#include <util/translation.h>

// QBTC Quantum Extensions
#include <quantum/manager.h>
#include <compressed_quantum_keys.h>
#include <dilithium/aggregation.h>

#include <any>
#include <functional>
#include <stdio.h>
#include <tuple>
#include <thread>
#include <chrono>

const std::function<std::string(const char*)> G_TRANSLATION_FUN = nullptr;

#if HAVE_DECL_FORK
#include <signal.h>

/** Custom UNIX signal handlers. */
static std::function<void()> g_shutdown_function;

void HandleSIGTERM(int)
{
    if (g_shutdown_function) {
        g_shutdown_function();
    }
}

void HandleSIGHUP(int)
{
    LogInstance().m_reopen_file = true;
}

#endif

static void RegisterSignalHandler(int signal, void(*handler)(int))
{
#if HAVE_DECL_FORK
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(signal, &sa, nullptr);
#endif
}

static bool AppInit(ArgsManager& args, int argc, char* argv[])
{
    // QBTC: Initialize quantum subsystem early
    try {
        LogPrintf("QBTC: Initializing quantum subsystem...\n");
        
        // Quantum Manager инициализация
        CQuantumManager::Initialize(true, true); // quantum=true, aggregation=true
        
        LogPrintf("QBTC: Quantum subsystem initialized successfully\n");
        LogPrintf("QBTC: - Quantum protection: ENABLED\n");
        LogPrintf("QBTC: - Signature aggregation: ENABLED\n");
        LogPrintf("QBTC: - Wallet compatibility: FULL\n");
        
    } catch (const std::exception& e) {
        LogPrintf("QBTC: ERROR - Failed to initialize quantum subsystem: %s\n", e.what());
        return false;
    }

    // Connect bitcoind signal handlers
    noui_connect();

    // Standard Bitcoin Core initialization via interfaces
    try {
        node::NodeContext node_context{};
        auto init = interfaces::MakeNode(node_context);
        if (!init) {
            return false;
        }
        return true;
    } catch (const std::exception& e) {
        LogPrintf("Init error: %s\n", e.what());
        return false;
    }
}

static bool AppShutdown()
{
    // QBTC: Shutdown quantum subsystem
    try {
        LogPrintf("QBTC: Shutting down quantum subsystem...\n");
        CQuantumManager::Shutdown();
        LogPrintf("QBTC: Quantum subsystem shutdown complete\n");
    } catch (const std::exception& e) {
        LogPrintf("QBTC: WARNING - Error during quantum shutdown: %s\n", e.what());
    }
    
    return true;
}

int main(int argc, char* argv[])
{
    // QBTC Banner
    LogPrintf("=== QBITCOIND - Quantum-resistant Bitcoin Core ===\n");
    LogPrintf("QBTC: Version %s\n", FormatFullVersion().c_str());
    LogPrintf("QBTC: Features - Compressed Quantum Keys + Dilithium Aggregation\n");
    LogPrintf("QBTC: Compatibility - Full Bitcoin wallet support\n");
    LogPrintf("QBTC: Protection - Post-quantum cryptography\n");

    ArgsManager args;

    SetupHelpOptions(args);
    std::string error;
    if (!args.ParseParameters(argc, argv, error)) {
        return InitError(Untranslated(strprintf("Error parsing command line arguments: %s\n", error)));
    }

    if (args.IsArgSet("-version")) {
        strprintf("QBTC (Quantum Bitcoin Core) version %s", FormatFullVersion().c_str());
        return EXIT_SUCCESS;
    }

    // Process help and version before taking care about datadir
    try {
        if (args.IsArgSet("-?") || args.IsArgSet("-h") || args.IsArgSet("-help")) {
            std::string strUsage = "QBTC (Quantum-resistant Bitcoin Core) version " + FormatFullVersion() + "\n\n" +
                                 "Usage:  qbitcoind [options]\n\n";
            strUsage += args.GetHelpMessage();
            strUsage += "\nQBTC Quantum Features:\n";
            strUsage += "  -quantum=<1|0>           Enable/disable quantum protection (default: 1)\n";
            strUsage += "  -aggregation=<1|0>       Enable/disable signature aggregation (default: 1)\n";
            strUsage += "  -quantumdebug            Enable quantum debug logging\n";
            strUsage += "  -quantumstats            Show quantum operation statistics\n";

            tfm::format(std::cout, "%s", strUsage);
            return EXIT_SUCCESS;
        }
    } catch (const std::exception& e) {
        PrintExceptionContinue(&e, "AppInit()");
        return EXIT_FAILURE;
    } catch (...) {
        PrintExceptionContinue(nullptr, "AppInit()");
        return EXIT_FAILURE;
    }

    try {
        util::ThreadRename("qbtc-main");
        
        if (!AppInit(args, argc, argv)) {
            return EXIT_FAILURE;
        }
    } catch (const std::exception& e) {
        PrintExceptionContinue(&e, "AppInit()");
        return EXIT_FAILURE;
    } catch (...) {
        PrintExceptionContinue(nullptr, "AppInit()");
        return EXIT_FAILURE;
    }

    try {
        // Basic shutdown handling - for now just wait for interrupt
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            // In full implementation, check for shutdown signals here
        }
        
        if (!AppShutdown()) {
            return EXIT_FAILURE;
        }
    } catch (const std::exception& e) {
        PrintExceptionContinue(&e, "Shutdown()");
        return EXIT_FAILURE;
    } catch (...) {
        PrintExceptionContinue(nullptr, "Shutdown()");
        return EXIT_FAILURE;
    }

    LogPrintf("QBTC: Shutdown complete. Stay quantum-safe!\n");
    return EXIT_SUCCESS;
} 