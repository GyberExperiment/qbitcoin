#ifndef BITCOIN_COMPRESSED_QUANTUM_KEYS_H
#define BITCOIN_COMPRESSED_QUANTUM_KEYS_H

// Полные includes для боевой реализации
#include <key_original.h>           // Для CKey (ECDSA)
#include <key.h>                   // Для CQKey (quantum)
#include <pubkey.h>
#include <hash.h>
#include <uint256.h>
#include <dilithium/aggregation.h>
#include <script/script.h>
#include <primitives/transaction.h>
#include <span.h>
#include <vector>
#include <memory>
#include <map>
#include <string>
#include <streams.h>
#include <serialize.h>
#include <logging.h>

/**
 * COMPRESSED QUANTUM KEYS IMPLEMENTATION
 * 
 * Концепция: Seed (32 байта) → ECDSA ключ + Dilithium ключ (связанные)
 * Адреса: bc1q... (Hash160 от Dilithium pubkey)
 * Агрегация: Dilithium подписи в witness
 */

// Forward declarations
class CKey;
class CQKey;  
class CQPubKey;

/** Пара связанных ECDSA и Dilithium ключей из одного seed */
class CQuantumKeyPair {
private:
    unsigned char seed_data[32];
    CKey ecdsa_key;                 // Обычный ECDSA ключ из key_original.h
    CQKey dilithium_key;            // Quantum ключ из key.h
    uint160 address_hash;
    bool is_valid;

public:
    CQuantumKeyPair();
    ~CQuantumKeyPair() = default;
    
    // Конструкторы копирования и перемещения
    CQuantumKeyPair(const CQuantumKeyPair& other);
    CQuantumKeyPair(CQuantumKeyPair&& other) noexcept;
    CQuantumKeyPair& operator=(const CQuantumKeyPair& other);
    CQuantumKeyPair& operator=(CQuantumKeyPair&& other) noexcept;
    
    /** Создание ключевой пары из seed */
    static CQuantumKeyPair FromSeed(const unsigned char* seed);
    static CQuantumKeyPair FromSeed(const uint256& seed);
    
    /** Генерация случайной ключевой пары */
    static CQuantumKeyPair Generate();
    
    /** Доступ к ключам */
    const CKey& GetECDSAKey() const { return ecdsa_key; }
    const CQKey& GetDilithiumKey() const { return dilithium_key; }
    CPubKey GetECDSAPubKey() const { return ecdsa_key.GetPubKey(); }
    CQPubKey GetDilithiumPubKey() const { return dilithium_key.GetPubKey(); }
    
    /** Получение адреса */
    uint160 GetAddressHash() const { return address_hash; }
    std::string GetAddress() const; // bc1q... формат
    
    /** Подписание */
    bool SignECDSA(const uint256& hash, std::vector<unsigned char>& signature) const;
    bool SignDilithium(const uint256& hash, std::vector<unsigned char>& signature) const;
    
    /** Верификация */
    bool VerifyECDSA(const uint256& hash, const std::vector<unsigned char>& signature) const;
    bool VerifyDilithium(const uint256& hash, const std::vector<unsigned char>& signature) const;
    
    /** Статус */
    bool IsValid() const { return is_valid; }
    
    /** Детерминистичная деривация (BIP32-style) */
    bool Derive(CQuantumKeyPair& child, unsigned int index) const;
    
    /** Сериализация */
    std::vector<unsigned char> Serialize() const;
    bool Deserialize(const std::vector<unsigned char>& data);
};

/** Агрегированный witness для quantum транзакций */
struct CQuantumWitness {
    CAggregatedSignature agg_signature;     // Агрегированная Dilithium подпись
    std::vector<CQPubKey> pubkeys;         // Dilithium публичные ключи
    std::vector<uint160> address_hashes;   // Соответствующие адреса
    uint32_t input_count;                  // Количество входов
    
    CQuantumWitness() : input_count(0) {}
    
    /** Сериализация в scriptWitness */
    CScriptWitness ToScriptWitness() const;
    
    /** Десериализация из scriptWitness */
    bool FromScriptWitness(const CScriptWitness& witness);
    
    /** Проверка валидности */
    bool IsValid() const;
    
    /** Размер witness данных */
    size_t GetSize() const;
    
    SERIALIZE_METHODS(CQuantumWitness, obj) {
        READWRITE(obj.agg_signature, obj.pubkeys, obj.address_hashes, obj.input_count);
    }
};

/** Билдер агрегированных транзакций */
class CQuantumTransactionBuilder {
private:
    std::vector<CTxIn> inputs;
    std::vector<CTxOut> outputs;
    std::vector<CQuantumKeyPair> input_keypairs;
    std::vector<CAmount> input_amounts;
    
public:
    CQuantumTransactionBuilder() = default;
    
    /** Добавление входа с quantum ключом */
    bool AddInput(const COutPoint& outpoint, 
                  const CQuantumKeyPair& keypair,
                  CAmount amount,
                  const CScript& scriptPubKey = CScript());
    
    /** Добавление выхода */
    bool AddOutput(const CScript& scriptPubKey, CAmount amount);
    bool AddOutput(const uint160& address_hash, CAmount amount); // P2WPKH
    
    /** Построение транзакции с агрегацией */
    bool BuildTransaction(CMutableTransaction& tx, std::string& error) const;
    
    /** Подписание с агрегацией */
    bool SignTransaction(CMutableTransaction& tx, std::string& error) const;
    
    /** Очистка билдера */
    void Clear();
    
    /** Информация */
    size_t GetInputCount() const { return inputs.size(); }
    size_t GetOutputCount() const { return outputs.size(); }
    CAmount GetTotalInputAmount() const;
    CAmount GetTotalOutputAmount() const;
};

/** Валидатор quantum транзакций */
class CQuantumTransactionValidator {
public:
    /** Проверка одиночной quantum транзакции */
    static bool ValidateTransaction(const CTransaction& tx, std::string& error);
    
    /** Проверка агрегированной транзакции */
    static bool ValidateAggregatedTransaction(const CTransaction& tx, std::string& error);
    
    /** Проверка quantum witness */
    static bool ValidateQuantumWitness(const CQuantumWitness& witness, 
                                      const std::vector<CTxIn>& inputs,
                                      const uint256& tx_hash,
                                      std::string& error);
    
    /** Извлечение адреса из scriptPubKey */
    static bool ExtractAddressHash(const CScript& scriptPubKey, uint160& address_hash);
    
    /** Проверка соответствия адреса и ключа */
    static bool VerifyAddressKeyMatch(const uint160& address_hash, const CQPubKey& pubkey);
};

/** Утилиты для работы с адресами */
namespace QuantumAddressUtils {
    /** Создание P2WPKH scriptPubKey из quantum ключа */
    CScript CreateP2WPKHScript(const CQPubKey& pubkey);
    CScript CreateP2WPKHScript(const uint160& address_hash);
    
    /** Создание bech32 адреса */
    std::string CreateBech32Address(const uint160& address_hash, const std::string& hrp = "bc");
    
    /** Парсинг bech32 адреса */
    bool ParseBech32Address(const std::string& address, uint160& address_hash, std::string& hrp);
    
    /** Конвертация между форматами */
    uint160 PubKeyToAddressHash(const CQPubKey& pubkey);
    bool IsQuantumAddress(const CScript& scriptPubKey);
}

/** Менеджер quantum кошелька */
class CQuantumWalletManager {
private:
    std::vector<CQuantumKeyPair> keypairs;
    std::map<uint160, size_t> address_to_index;
    
public:
    /** Генерация новой ключевой пары */
    CQuantumKeyPair GenerateNewKeyPair();
    
    /** Добавление существующей ключевой пары */
    bool AddKeyPair(const CQuantumKeyPair& keypair);
    
    /** Поиск ключевой пары по адресу */
    const CQuantumKeyPair* GetKeyPairByAddress(const uint160& address_hash) const;
    
    /** Получение всех адресов */
    std::vector<uint160> GetAllAddresses() const;
    
    /** Создание транзакции */
    bool CreateTransaction(const std::vector<COutPoint>& inputs,
                          const std::vector<std::pair<uint160, CAmount>>& outputs,
                          CMutableTransaction& tx,
                          std::string& error);
    
    /** Подписание транзакции */
    bool SignTransaction(CMutableTransaction& tx, std::string& error);
    
    /** Сохранение/загрузка кошелька */
    bool SaveToFile(const std::string& filename) const;
    bool LoadFromFile(const std::string& filename);
    
    /** Статистика */
    size_t GetKeyPairCount() const { return keypairs.size(); }
    void Clear() { keypairs.clear(); address_to_index.clear(); }
};

#endif // BITCOIN_COMPRESSED_QUANTUM_KEYS_H 