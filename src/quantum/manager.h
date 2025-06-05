#ifndef BITCOIN_QUANTUM_MANAGER_H
#define BITCOIN_QUANTUM_MANAGER_H

#include <compressed_quantum_keys.h>
#include <dilithium/aggregation.h>
#include <primitives/transaction.h>
#include <script/interpreter.h>
#include <uint256.h>
#include <memory>

/**
 * QUANTUM MANAGER - Центральный координатор quantum функций в QBTC daemon
 * 
 * Обеспечивает:
 * - Прозрачную quantum защиту всех транзакций
 * - Полную совместимость с Bitcoin кошельками
 * - Автоматическую агрегацию Dilithium подписей
 * - Гибридную валидацию (ECDSA + Dilithium)
 */

class CQuantumManager {
private:
    static std::unique_ptr<CQuantumManager> instance;
    bool quantum_enabled;
    bool aggregation_enabled;
    std::unique_ptr<CDilithiumAggregator> aggregator;
    
    CQuantumManager();

public:
    static CQuantumManager& GetInstance();
    static void Initialize(bool enable_quantum = true, bool enable_aggregation = true);
    static void Shutdown();
    
    // === CONFIGURATION ===
    
    /** Проверка что quantum защита включена */
    bool IsQuantumEnabled() const { return quantum_enabled; }
    
    /** Проверка что агрегация включена */
    bool IsAggregationEnabled() const { return aggregation_enabled; }
    
    // === TRANSACTION PROCESSING ===
    
    /** Обработка transaction с quantum witness */
    bool ProcessQuantumTransaction(const CTransaction& tx, std::string& error);
    
    /** Валидация quantum witness */
    bool ValidateQuantumWitness(const CQuantumWitness& witness, 
                               const std::vector<CTxIn>& inputs,
                               const uint256& tx_hash,
                               std::string& error);
    
    /** Проверка является ли транзакция quantum-protected */
    bool IsQuantumTransaction(const CTransaction& tx);
    
    /** Извлечение quantum witness из transaction */
    bool ExtractQuantumWitness(const CTransaction& tx, CQuantumWitness& witness);
    
    // === SIGNATURE VALIDATION ===
    
    /** Гибридная валидация подписей (ECDSA + Dilithium) */
    bool ValidateHybridSignatures(const CTransaction& tx, 
                                 unsigned int input_index,
                                 const CScript& scriptPubKey,
                                 const CAmount& amount,
                                 std::string& error);
    
    /** Валидация агрегированной Dilithium подписи */
    bool ValidateAggregatedSignature(const CAggregatedSignature& agg_sig,
                                    const std::vector<CQPubKey>& pubkeys,
                                    const std::vector<uint256>& messages,
                                    std::string& error);
    
    // === ADDRESS MANAGEMENT ===
    
    /** Проверка является ли адрес quantum-protected */
    bool IsQuantumAddress(const CScript& scriptPubKey);
    
    /** Извлечение quantum address hash */
    bool ExtractQuantumAddressHash(const CScript& scriptPubKey, uint160& address_hash);
    
    /** Создание quantum-compatible scriptPubKey */
    CScript CreateQuantumScript(const uint160& address_hash);
    CScript CreateQuantumScript(const CQPubKey& pubkey);
    
    // === WALLET INTEGRATION ===
    
    /** Генерация quantum ключевой пары */
    CQuantumKeyPair GenerateQuantumKeyPair();
    
    /** Создание quantum транзакции */
    bool CreateQuantumTransaction(const std::vector<COutPoint>& inputs,
                                 const std::vector<std::pair<uint160, CAmount>>& outputs,
                                 const std::vector<CQuantumKeyPair>& keypairs,
                                 CMutableTransaction& tx,
                                 std::string& error);
    
    // === NETWORK COMPATIBILITY ===
    
    /** Конвертация quantum транзакции в Bitcoin-совместимый формат */
    bool ConvertToLegacyFormat(const CTransaction& quantum_tx, CTransaction& legacy_tx);
    
    /** Проверка совместимости с Bitcoin Core */
    bool IsLegacyCompatible(const CTransaction& tx);
    
    // === STATISTICS & MONITORING ===
    
    /** Статистика quantum операций */
    struct QuantumStats {
        uint64_t quantum_transactions_processed;
        uint64_t aggregated_signatures_verified;
        uint64_t quantum_addresses_created;
        double average_aggregation_ratio;
        double average_signature_size_reduction;
    };
    
    QuantumStats GetStatistics() const;
    void ResetStatistics();
    
    // === DEBUGGING & LOGGING ===
    
    /** Включение debug режима */
    void SetDebugMode(bool enable);
    
    /** Логирование quantum операций */
    void LogQuantumOperation(const std::string& operation, const std::string& details);
};

/**
 * QUANTUM SIGNATURE CHECKER - Расширение BaseSignatureChecker для quantum подписей
 */
class CQuantumSignatureChecker : public BaseSignatureChecker {
private:
    const CTransaction* tx;
    unsigned int input_index;
    const CAmount amount;
    
public:
    CQuantumSignatureChecker(const CTransaction* txToIn, unsigned int nInIn, const CAmount& amountIn)
        : tx(txToIn), input_index(nInIn), amount(amountIn) {}
    
    // Переопределяем для quantum поддержки
    bool CheckECDSASignature(const std::vector<unsigned char>& scriptSig, 
                            const std::vector<unsigned char>& vchPubKey, 
                            const CScript& scriptCode, 
                            SigVersion sigversion) const override;
    
    bool CheckDilithiumSignature(const std::vector<unsigned char>& signature, 
                                const std::vector<unsigned char>& vchPubKey, 
                                const CScript& scriptCode, 
                                SigVersion sigversion) const override;
    
    // Quantum-specific методы
    bool CheckQuantumWitness(const CQuantumWitness& witness, 
                           const uint256& tx_hash) const;
    
    bool CheckAggregatedSignature(const CAggregatedSignature& agg_sig,
                                 const std::vector<CQPubKey>& pubkeys,
                                 const std::vector<uint256>& messages) const;
};

/**
 * QUANTUM UTILITIES - Вспомогательные функции
 */
namespace QuantumUtils {
    /** Конвертация между quantum и legacy форматами */
    bool ConvertQuantumToLegacy(const CQuantumWitness& quantum, CScriptWitness& legacy);
    bool ConvertLegacyToQuantum(const CScriptWitness& legacy, CQuantumWitness& quantum);
    
    /** Проверка quantum совместимости */
    bool IsQuantumCompatible(const CScript& script);
    bool RequiresQuantumProcessing(const CTransaction& tx);
    
    /** Сжатие witness данных */
    size_t CalculateWitnessSavings(const CQuantumWitness& quantum_witness,
                                  const std::vector<CScriptWitness>& legacy_witnesses);
}

#endif // BITCOIN_QUANTUM_MANAGER_H 