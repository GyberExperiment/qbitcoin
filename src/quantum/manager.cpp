#include <quantum/manager.h>
#include <logging.h>
#include <util/strencodings.h>
#include <util/translation.h>

// Static member initialization
std::unique_ptr<CQuantumManager> CQuantumManager::instance = nullptr;

CQuantumManager::CQuantumManager() 
    : quantum_enabled(true), aggregation_enabled(true) 
{
    LogPrintf("QBTC: Quantum protection enabled by default\n");
}

CQuantumManager& CQuantumManager::GetInstance()
{
    if (!instance) {
        instance.reset(new CQuantumManager());
    }
    return *instance;
}

void CQuantumManager::Initialize(bool enable_quantum, bool enable_aggregation)
{
    LogPrintf("CQuantumManager: Initializing quantum subsystem...\n");
    
    auto& manager = GetInstance();
    manager.quantum_enabled = enable_quantum;
    manager.aggregation_enabled = enable_aggregation;
    
    if (enable_aggregation) {
        try {
            manager.aggregator = std::make_unique<CDilithiumAggregator>();
            LogPrintf("CQuantumManager: Dilithium aggregator initialized\n");
        } catch (const std::exception& e) {
            LogPrintf("CQuantumManager: Warning - Failed to initialize aggregator: %s\n", e.what());
            manager.aggregation_enabled = false;
        }
    }
    
    LogPrintf("CQuantumManager: Initialization complete\n");
    LogPrintf("CQuantumManager: - Quantum protection: %s\n", enable_quantum ? "ENABLED" : "DISABLED");
    LogPrintf("CQuantumManager: - Signature aggregation: %s\n", manager.aggregation_enabled ? "ENABLED" : "DISABLED");
}

void CQuantumManager::Shutdown()
{
    LogPrintf("CQuantumManager: Shutting down quantum subsystem...\n");
    
    if (instance) {
        instance->aggregator.reset();
        instance.reset();
    }
    
    LogPrintf("CQuantumManager: Shutdown complete\n");
}

bool CQuantumManager::ProcessQuantumTransaction(const CTransaction& tx, std::string& error)
{
    if (!quantum_enabled) {
        error = "Quantum processing is disabled";
        return false;
    }
    
    // Basic validation for now
    if (tx.vin.empty()) {
        error = "Transaction has no inputs";
        return false;
    }
    
    if (tx.vout.empty()) {
        error = "Transaction has no outputs";
        return false;
    }
    
    return true;
}

bool CQuantumManager::ValidateQuantumWitness(const CQuantumWitness& witness, 
                                           const std::vector<CTxIn>& inputs,
                                           const uint256& tx_hash,
                                           std::string& error)
{
    if (!quantum_enabled) {
        error = "Quantum validation is disabled";
        return false;
    }
    
    if (witness.pubkeys.size() != inputs.size()) {
        error = strprintf("Witness pubkey count (%d) does not match input count (%d)", 
                         witness.pubkeys.size(), inputs.size());
        return false;
    }
    
    if (witness.address_hashes.size() != inputs.size()) {
        error = strprintf("Witness address count (%d) does not match input count (%d)", 
                         witness.address_hashes.size(), inputs.size());
        return false;
    }
    
    return true;
}

bool CQuantumManager::IsQuantumTransaction(const CTransaction& tx)
{
    // For now, consider all transactions as potentially quantum
    return quantum_enabled && !tx.vin.empty();
}

bool CQuantumManager::ExtractQuantumWitness(const CTransaction& tx, CQuantumWitness& witness)
{
    // Simplified extraction for now
    if (tx.vin.empty()) return false;
    
    // Check if first input has quantum witness data
    if (!tx.vin[0].scriptWitness.IsNull() && tx.vin[0].scriptWitness.stack.size() >= 2) {
        return witness.FromScriptWitness(tx.vin[0].scriptWitness);
    }
    
    return false;
}

bool CQuantumManager::IsQuantumAddress(const CScript& scriptPubKey)
{
    return QuantumAddressUtils::IsQuantumAddress(scriptPubKey);
}

bool CQuantumManager::ExtractQuantumAddressHash(const CScript& scriptPubKey, uint160& address_hash)
{
    return CQuantumTransactionValidator::ExtractAddressHash(scriptPubKey, address_hash);
}

CScript CQuantumManager::CreateQuantumScript(const uint160& address_hash)
{
    return QuantumAddressUtils::CreateP2WPKHScript(address_hash);
}

CScript CQuantumManager::CreateQuantumScript(const CQPubKey& pubkey)
{
    return QuantumAddressUtils::CreateP2WPKHScript(pubkey);
}

CQuantumKeyPair CQuantumManager::GenerateQuantumKeyPair()
{
    return CQuantumKeyPair::Generate();
}

bool CQuantumManager::CreateQuantumTransaction(const std::vector<COutPoint>& inputs,
                                             const std::vector<std::pair<uint160, CAmount>>& outputs,
                                             const std::vector<CQuantumKeyPair>& keypairs,
                                             CMutableTransaction& tx,
                                             std::string& error)
{
    CQuantumTransactionBuilder builder;
    
    // Add inputs with keypairs
    for (size_t i = 0; i < inputs.size() && i < keypairs.size(); i++) {
        if (!builder.AddInput(inputs[i], keypairs[i], 0)) {
            error = strprintf("Failed to add input %d", i);
            return false;
        }
    }
    
    // Add outputs
    for (const auto& output : outputs) {
        if (!builder.AddOutput(output.first, output.second)) {
            error = strprintf("Failed to add output");
            return false;
        }
    }
    
    return builder.BuildTransaction(tx, error);
}

CQuantumManager::QuantumStats CQuantumManager::GetStatistics() const
{
    // Return default stats for now
    return QuantumStats{
        .quantum_transactions_processed = 0,
        .aggregated_signatures_verified = 0,
        .quantum_addresses_created = 0,
        .average_aggregation_ratio = 0.0,
        .average_signature_size_reduction = 0.0
    };
}

void CQuantumManager::ResetStatistics()
{
    // No-op for now
}

void CQuantumManager::SetDebugMode(bool enable)
{
    // No-op for now
}

void CQuantumManager::LogQuantumOperation(const std::string& operation, const std::string& details)
{
    LogPrintf("CQuantumManager: %s - %s\n", operation, details);
} 