/**
 * ТЕСТ РЕАЛЬНОЙ DILITHIUM АГРЕГАЦИИ В QBTC
 * 
 * Этот тест использует НАСТОЯЩУЮ интеграцию QBTC Dilithium
 * Без фальшивых research prototypes
 */

#include <iostream>
#include <vector>
#include <chrono>

#include "src/key.h"  // Using the main QBTC key types instead of qkey.h
#include "src/uint256.h"
#include "src/dilithium/aggregation.h"
#include "src/random.h"
#include "src/util/time.h"
#include "src/logging.h"

/**
 * Реальный production тест агрегации
 */
class QBTCProductionAggregationTest {
private:
    std::vector<CQKey> test_keys;
    std::vector<CQPubKey> test_pubkeys;
    std::vector<uint256> test_messages;
    std::vector<std::vector<unsigned char>> test_signatures;
    
public:
    bool InitializeTest(size_t key_count) {
        std::cout << "🔧 Инициализация production теста для " << key_count << " ключей..." << std::endl;
        
        test_keys.clear();
        test_pubkeys.clear();
        test_messages.clear();
        test_signatures.clear();
        
        test_keys.reserve(key_count);
        test_pubkeys.reserve(key_count);
        test_messages.reserve(key_count);
        test_signatures.reserve(key_count);
        
        // Генерация НАСТОЯЩИХ Dilithium ключей
        for (size_t i = 0; i < key_count; ++i) {
            CQKey key;
            key.MakeNewKey(true);
            
            if (!key.IsValid()) {
                std::cout << "❌ Ошибка генерации ключа " << i << std::endl;
                return false;
            }
            
            CQPubKey pubkey = key.GetPubKey();
            if (!pubkey.IsValid()) {
                std::cout << "❌ Ошибка извлечения публичного ключа " << i << std::endl;
                return false;
            }
            
            // Генерация сообщения
            uint256 message;
            GetStrongRandBytes({(unsigned char*)&message, sizeof(message)});
            
            // Создание подписи
            std::vector<unsigned char> signature;
            if (!key.Sign(message, signature)) {
                std::cout << "❌ Ошибка создания подписи " << i << std::endl;
                return false;
            }
            
            // Верификация подписи
            if (!pubkey.Verify(message, signature)) {
                std::cout << "❌ Ошибка верификации подписи " << i << std::endl;
                return false;
            }
            
            test_keys.push_back(std::move(key));
            test_pubkeys.push_back(pubkey);
            test_messages.push_back(message);
            test_signatures.push_back(std::move(signature));
        }
        
        std::cout << "✅ Инициализация завершена: " << test_keys.size() << " ключей готово" << std::endl;
        return true;
    }
    
    bool TestRealAggregation() {
        std::cout << "\n🚀 ТЕСТИРОВАНИЕ РЕАЛЬНОЙ DILITHIUM АГРЕГАЦИИ..." << std::endl;
        
        if (test_keys.empty()) {
            std::cout << "❌ Нет данных для тестирования. Запустите InitializeTest() сначала." << std::endl;
            return false;
        }
        
        CDilithiumAggregator aggregator;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Добавление подписей в агрегатор
        for (size_t i = 0; i < test_signatures.size(); ++i) {
            if (!aggregator.AddSignature(test_pubkeys[i], test_messages[i], test_signatures[i])) {
                std::cout << "❌ Ошибка добавления подписи " << i << " в агрегатор" << std::endl;
                return false;
            }
        }
        
        auto add_time = std::chrono::high_resolution_clock::now();
        
        // Выполнение агрегации
        CAggregatedSignature agg_sig = aggregator.Aggregate();
        
        auto agg_time = std::chrono::high_resolution_clock::now();
        
        if (!agg_sig.IsValid()) {
            std::cout << "❌ Агрегация не удалась" << std::endl;
            return false;
        }
        
        // Верификация агрегированной подписи
        bool verify_result = CDilithiumAggregator::VerifyAggregated(agg_sig);
        
        auto verify_time = std::chrono::high_resolution_clock::now();
        
        // Вычисление времени
        auto add_duration = std::chrono::duration_cast<std::chrono::microseconds>(add_time - start_time);
        auto agg_duration = std::chrono::duration_cast<std::chrono::microseconds>(agg_time - add_time);
        auto verify_duration = std::chrono::duration_cast<std::chrono::microseconds>(verify_time - agg_time);
        
        // Вычисление размеров
        size_t original_size = test_signatures.size() * DILITHIUM_SIGNATURE_SIZE;
        size_t aggregated_size = agg_sig.GetSerializeSize();
        double compression_ratio = (1.0 - double(aggregated_size) / original_size) * 100.0;
        
        std::cout << "\n📊 РЕЗУЛЬТАТЫ РЕАЛЬНОЙ АГРЕГАЦИИ:" << std::endl;
        std::cout << "Количество подписей: " << test_signatures.size() << std::endl;
        std::cout << "Время добавления: " << add_duration.count() / 1000.0 << " ms" << std::endl;
        std::cout << "Время агрегации: " << agg_duration.count() / 1000.0 << " ms" << std::endl;
        std::cout << "Время верификации: " << verify_duration.count() / 1000.0 << " ms" << std::endl;
        std::cout << "Исходный размер: " << original_size << " bytes" << std::endl;
        std::cout << "Агрегированный размер: " << aggregated_size << " bytes" << std::endl;
        std::cout << "🎯 РЕАЛЬНАЯ КОМПРЕССИЯ: " << compression_ratio << "%" << std::endl;
        std::cout << "Верификация: " << (verify_result ? "✅ УСПЕШНО" : "❌ ПРОВАЛЕНО") << std::endl;
        
        return verify_result;
    }
    
    bool TestSpaceSavings() {
        std::cout << "\n💾 ТЕСТИРОВАНИЕ ЭКОНОМИИ МЕСТА..." << std::endl;
        
        std::vector<size_t> test_sizes = {2, 5, 10, 25, 50, 100};
        
        std::cout << "Сигнатуры\tОригинал (KB)\tАгрегированный (KB)\tКомпрессия %" << std::endl;
        std::cout << "----------------------------------------------------------------" << std::endl;
        
        for (size_t size : test_sizes) {
            size_t original_kb = (size * DILITHIUM_SIGNATURE_SIZE) / 1024;
            size_t savings = CDilithiumAggregator::CalculateSpaceSavings(size);
            size_t agg_kb = ((size * DILITHIUM_SIGNATURE_SIZE) - savings) / 1024;
            double compression = (double(savings) / (size * DILITHIUM_SIGNATURE_SIZE)) * 100.0;
            
            std::cout << size << "\t\t" << original_kb << "\t\t" 
                      << agg_kb << "\t\t\t" << compression << "%" << std::endl;
        }
        
        return true;
    }
    
    bool TestBatchVerification() {
        std::cout << "\n⚡ ТЕСТИРОВАНИЕ BATCH ВЕРИФИКАЦИИ..." << std::endl;
        
        // Создание нескольких агрегированных подписей
        std::vector<CAggregatedSignature> batch_sigs;
        const size_t batch_count = 3;
        const size_t sigs_per_batch = 5;
        
        for (size_t batch = 0; batch < batch_count; ++batch) {
            if (!InitializeTest(sigs_per_batch)) {
                return false;
            }
            
            CDilithiumAggregator aggregator;
            
            for (size_t i = 0; i < test_signatures.size(); ++i) {
                if (!aggregator.AddSignature(test_pubkeys[i], test_messages[i], test_signatures[i])) {
                    return false;
                }
            }
            
            CAggregatedSignature agg_sig = aggregator.Aggregate();
            if (!agg_sig.IsValid()) {
                return false;
            }
            
            batch_sigs.push_back(agg_sig);
        }
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        bool batch_result = CDilithiumAggregator::VerifyBatch(batch_sigs);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        
        std::cout << "Batch верификация " << batch_count << " агрегированных подписей: " 
                  << (batch_result ? "✅ УСПЕШНО" : "❌ ПРОВАЛЕНО") << std::endl;
        std::cout << "Время batch верификации: " << duration.count() / 1000.0 << " ms" << std::endl;
        
        return batch_result;
    }
};

int main() {
    std::cout << "🔐 QBTC PRODUCTION DILITHIUM AGGREGATION TEST" << std::endl;
    std::cout << "=============================================" << std::endl;
    
    // Инициализация QBTC системы
    LogPrintf("QBTC Production Aggregation Test Started\n");
    
    QBTCProductionAggregationTest test;
    
    try {
        // Тест 1: Базовая агрегация
        std::cout << "\n=== ТЕСТ 1: БАЗОВАЯ АГРЕГАЦИЯ ===" << std::endl;
        if (!test.InitializeTest(10)) {
            std::cout << "❌ Ошибка инициализации теста" << std::endl;
            return 1;
        }
        
        if (!test.TestRealAggregation()) {
            std::cout << "❌ Ошибка базовой агрегации" << std::endl;
            return 1;
        }
        
        // Тест 2: Экономия места
        std::cout << "\n=== ТЕСТ 2: ЭКОНОМИЯ МЕСТА ===" << std::endl;
        if (!test.TestSpaceSavings()) {
            std::cout << "❌ Ошибка теста экономии места" << std::endl;
            return 1;
        }
        
        // Тест 3: Batch верификация
        std::cout << "\n=== ТЕСТ 3: BATCH ВЕРИФИКАЦИЯ ===" << std::endl;
        if (!test.TestBatchVerification()) {
            std::cout << "❌ Ошибка batch верификации" << std::endl;
            return 1;
        }
        
        std::cout << "\n🎉 ВСЕ ТЕСТЫ ПРОЙДЕНЫ УСПЕШНО!" << std::endl;
        std::cout << "QBTC Dilithium агрегация работает корректно в production среде!" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Исключение: " << e.what() << std::endl;
        return 1;
    }
} 