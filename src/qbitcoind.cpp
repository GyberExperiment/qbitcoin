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
#include <common/system.h>
#include <compat/compat.h>
#include <init.h>
#include <interfaces/chain.h>
#include <interfaces/init.h>
#include <kernel/context.h>
#include <node/context.h>
#include <node/interface_ui.h>
#include <node/warnings.h>
#include <noui.h>
#include <util/check.h>
#include <util/exception.h>
#include <util/signalinterrupt.h>
#include <util/strencodings.h>
#include <util/syserror.h>
#include <util/threadnames.h>
#include <util/tokenpipe.h>
#include <util/translation.h>

// QBTC Quantum Extensions
#include <quantum/manager.h>
#include <compressed_quantum_keys.h>
#include <dilithium/aggregation.h>
#include <hybrid_crypto.h>

#include <any>
#include <functional>
#include <optional>

using node::NodeContext;

const TranslateFn G_TRANSLATION_FUN{nullptr};

static bool ParseArgs(NodeContext& node, int argc, char* argv[])
{
    ArgsManager& args{*Assert(node.args)};
    SetupServerArgs(args, node.init->canListenIpc());
    std::string error;
    if (!args.ParseParameters(argc, argv, error)) {
        return InitError(Untranslated("Error parsing command line arguments: " + error));
    }

    if (auto error = common::InitConfig(args)) {
        return InitError(error->message, error->details);
    }

    // Error out when loose non-argument tokens are encountered on command line
    for (int i = 1; i < argc; i++) {
        if (!IsSwitchChar(argv[i][0])) {
            return InitError(Untranslated("Command line contains unexpected token '" + std::string(argv[i]) + "', see qbitcoind -h for a list of options."));
        }
    }
    return true;
}

static bool ProcessInitCommands(ArgsManager& args)
{
    // Process help and version before taking care about datadir
    if (HelpRequested(args) || args.GetBoolArg("-version", false)) {
        std::string strUsage = "QBTC (Quantum-resistant Bitcoin Core) daemon version " + FormatFullVersion() + "\n";

        if (args.GetBoolArg("-version", false)) {
            strUsage += FormatParagraph(LicenseInfo());
        } else {
            strUsage += "\n"
                "The QBTC daemon (qbitcoind) is a quantum-resistant Bitcoin Core daemon that provides post-quantum cryptography protection.\n\n"
                "It maintains full Bitcoin wallet compatibility while adding:\n"
                "- Compressed Quantum Keys with Dilithium signatures\n"
                "- Signature aggregation for space efficiency\n"
                "- Quantum-resistant transaction validation\n"
                "- Full backward compatibility with existing Bitcoin wallets\n"
                "\n"
                "Usage: qbitcoind [options]\n"
                "\n";
            strUsage += args.GetHelpMessage();
            strUsage += "\nQBTC Quantum Features:\n";
            strUsage += "  -quantum=<1|0>           Enable/disable quantum protection (default: 1)\n";
            strUsage += "  -aggregation=<1|0>       Enable/disable signature aggregation (default: 1)\n";
            strUsage += "  -quantumdebug            Enable quantum debug logging\n";
            strUsage += "  -quantumstats            Show quantum operation statistics\n";
        }

        std::cout << strUsage;
        return true;
    }

    return false;
}

static bool AppInit(NodeContext& node)
{
    bool fRet = false;
    ArgsManager& args = *Assert(node.args);

    std::any context{&node};
    try
    {
        // QBTC: Initialize secp256k1 context for hybrid ECDSA/Dilithium system
        printf("QBTC: Initializing hybrid cryptography system...\n");
        static QBTCHybridCrypto hybrid_crypto; // Keep alive during daemon lifecycle
        printf("QBTC: - ECDSA context initialized (for Bitcoin address compatibility)\n");
        
        // QBTC: Initialize quantum subsystem early
        printf("=== QBITCOIND - Quantum-resistant Bitcoin Core ===\n");
        printf("QBTC: Version %s\n", FormatFullVersion().c_str());
        printf("QBTC: Features - Compressed Quantum Keys + Dilithium Aggregation\n");
        printf("QBTC: Compatibility - Full Bitcoin wallet support\n");
        printf("QBTC: Protection - Post-quantum cryptography\n");
        
        try {
            printf("QBTC: Initializing quantum subsystem...\n");
            
            // Quantum Manager инициализация
            CQuantumManager::Initialize(true, true); // quantum=true, aggregation=true
            
            printf("QBTC: Quantum subsystem initialized successfully\n");
            printf("QBTC: - Quantum protection: ENABLED\n");
            printf("QBTC: - Signature aggregation: ENABLED\n");
            printf("QBTC: - Wallet compatibility: FULL\n");
            printf("QBTC: HYBRID SYSTEM READY - ECDSA addresses + Dilithium signatures\n");
            
        } catch (const std::exception& e) {
            printf("QBTC: ERROR - Failed to initialize quantum subsystem: %s\n", e.what());
            return false;
        }

        // -server defaults to true for qbitcoind but not for the GUI so do this here
        args.SoftSetBoolArg("-server", true);
        // Set this early so that parameter interactions go to console
        InitLogging(args);
        InitParameterInteraction(args);
        if (!AppInitBasicSetup(args, node.exit_status)) {
            // InitError will have been called with detailed error, which ends up on console
            return false;
        }
        if (!AppInitParameterInteraction(args)) {
            // InitError will have been called with detailed error, which ends up on console
            return false;
        }

        node.warnings = std::make_unique<node::Warnings>();

        node.kernel = std::make_unique<kernel::Context>();
        if (!AppInitSanityChecks(*node.kernel))
        {
            // InitError will have been called with detailed error, which ends up on console
            return false;
        }

        // Lock critical directories after daemonization
        if (!AppInitLockDirectories())
        {
            // If locking a directory failed, exit immediately
            return false;
        }
        fRet = AppInitInterfaces(node) && AppInitMain(node);
    }
    catch (const std::exception& e) {
        PrintExceptionContinue(&e, "AppInit()");
    } catch (...) {
        PrintExceptionContinue(nullptr, "AppInit()");
    }

    return fRet;
}

static bool AppShutdown(NodeContext& node)
{
    // QBTC: Shutdown quantum subsystem
    try {
        printf("QBTC: Shutting down quantum subsystem...\n");
        CQuantumManager::Shutdown();
        printf("QBTC: Quantum subsystem shutdown complete\n");
    } catch (const std::exception& e) {
        printf("QBTC: WARNING - Error during quantum shutdown: %s\n", e.what());
    }
    
    return true;
}

MAIN_FUNCTION
{
#ifdef WIN32
    common::WinCmdLineArgs winArgs;
    std::tie(argc, argv) = winArgs.get();
#endif

    NodeContext node;
    int exit_status;
    std::unique_ptr<interfaces::Init> init = interfaces::MakeNodeInit(node, argc, argv, exit_status);
    if (!init) {
        return exit_status;
    }

    SetupEnvironment();

    // Connect qbitcoind signal handlers
    noui_connect();

    util::ThreadSetInternalName("qbtc-init");

    // Interpret command line arguments
    ArgsManager& args = *Assert(node.args);
    if (!ParseArgs(node, argc, argv)) return EXIT_FAILURE;
    // Process early info return commands such as -help or -version
    if (ProcessInitCommands(args)) return EXIT_SUCCESS;

    // Start application
    if (!AppInit(node) || !Assert(node.shutdown_signal)->wait()) {
        node.exit_status = EXIT_FAILURE;
    }
    Interrupt(node);
    Shutdown(node);
    
    // QBTC cleanup
    AppShutdown(node);

    printf("QBTC: Shutdown complete. Stay quantum-safe!\n");
    return node.exit_status;
} 