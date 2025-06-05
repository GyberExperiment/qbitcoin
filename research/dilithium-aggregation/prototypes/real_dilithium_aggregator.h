#ifndef REAL_DILITHIUM_AGGREGATOR_H
#define REAL_DILITHIUM_AGGREGATOR_H

#include "../../../src/qkey.h"
#include "../../../src/dilithium/api.h"
#include "../../../src/dilithium/aggregation.h"
#include "../../../src/uint256.h"
#include <vector>
#include <chrono>
#include <iostream>

/**
 * РЕАЛЬНАЯ DILITHIUM АГРЕГАЦИЯ
 * 
 * Использует настоящую QBTC интеграцию вместо поддельных данных
 * Интегрируется с существующим quantum-resistant кодом проекта
 */

namespace RealDilithiumAggregation {

/**
 * Структура для real performance benchmarks
 */
struct RealAggregationBenchmark {
    double keygen_time_ms;
    double signing_time_ms;
    double aggregation_time_ms;
    double verification_time_ms;
    size_t original_total_size;
    size_t aggregated_size;
    double compression_ratio;
    uint32_t signature_count;
    
    void CalculateMetrics() {
        if (original_total_size > 0) {
            compression_ratio = (1.0 - (double)aggregated_size / original_total_size) * 100.0;
        }
    }
    
    void Print() const;
};

/**
 * Реальная агрегация с настоящими Dilithium подписями
 */
class RealDilithiumAggregator {
private:
    std::vector<QKey> private_keys;
    std::vector<QPubKey> public_keys;
    std::vector<uint256> message_hashes;
    std::vector<std::vector<unsigned char>> real_signatures;
    
    CDilithiumAggregator core_aggregator;
    
public:
    RealDilithiumAggregator() = default;
    
    /**
     * Генерация НАСТОЯЩИХ Dilithium ключей и подписей
     */
    bool GenerateRealTestData(size_t count);
    
    /**
     * Создание НАСТОЯЩИХ подписей для тестирования
     */
    bool CreateRealSignatures();
    
    /**
     * НАСТОЯЩАЯ агрегация с реальным Dilithium
     */
    CAggregatedSignature AggregateReal();
    
    /**
     * Верификация агрегированной подписи
     */
    bool VerifyAggregated(const CAggregatedSignature& agg_sig);
    
    /**
     * Benchmark с реальными данными
     */
    RealAggregationBenchmark RunRealBenchmark(size_t signature_count);
    
    /**
     * Сравнение с фальшивой системой
     */
    void CompareWithFakeSystem();
    
    /**
     * Безопасность тестирование
     */
    bool TestSecurityProperties();
    
    /**
     * Rogue key attack тест
     */
    bool TestRogueKeyResistance();
    
    /**
     * Replay attack тест
     */
    bool TestReplayResistance();
    
    /**
     * Подлинность математики Dilithium
     */
    bool VerifyDilithiumMath();
    
    void Clear();
    size_t GetSignatureCount() const { return real_signatures.size(); }
};

/**
 * Integration с QBTC transaction system
 */
class QBTCTransactionAggregator {
private:
    std::vector<CMutableTransaction> test_transactions;
    
public:
    /**
     * Создание реальных QBTC транзакций с quantum подписями
     */
    bool CreateQuantumTransactions(size_t count);
    
    /**
     * Агрегация подписей в транзакциях
     */
    bool AggregateTransactionSignatures();
    
    /**
     * Тестирование в реальном blockchain контексте
     */
    bool TestBlockchainIntegration();
};

/**
 * Реальная математическая проверка
 */
namespace MathematicalValidation {
    /**
     * Проверка NTT operations
     */
    bool ValidateNTTOperations();
    
    /**
     * Проверка polynomial operations
     */
    bool ValidatePolynomialMath();
    
    /**
     * Проверка lattice properties
     */
    bool ValidateLatticeStructure();
    
    /**
     * Проверка модульной арифметики
     */
    bool ValidateModularArithmetic();
    
    /**
     * Проверка rejection sampling
     */
    bool ValidateRejectionSampling();
}

/**
 * Security analysis с реальными данными
 */
namespace SecurityValidation {
    /**
     * Проверка quantum resistance
     */
    bool ValidateQuantumResistance();
    
    /**
     * Проверка криптографической корректности
     */
    bool ValidateCryptographicSoundness();
    
    /**
     * Проверка aggregation binding
     */
    bool ValidateAggregationBinding();
    
    /**
     * Проверка на известные атаки
     */
    bool TestKnownAttacks();
}

} // namespace RealDilithiumAggregation

#endif // REAL_DILITHIUM_AGGREGATOR_H 