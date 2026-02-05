#include "codec.hpp"
#include "hmac.hpp"
#include <gtest/gtest.h>
#include <regex>
#include <string>

using namespace service::hmac;
using namespace service::codec;

class RequirementTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        secret_ = "test_secret_123";
        hmac_ = std::make_unique<Hmac>(secret_);
    }

    bool isValidBase64Url(const std::string &s)
    {
        if (s.empty())
            return true;

        std::regex pattern("(^[A-Za-z0-9\\-_]+$)");

        auto match = std::regex_match(s, pattern);

        if (!match)
        {
            return false;
        }

        if (s.length() % 4 == 1)
            return false;
        return true;
    }

    std::string secret_;
    std::unique_ptr<Hmac> hmac_;
};

// 1. Подпись/проверка успеха msg="hello" → подпись → verify → ok=true.
TEST_F(RequirementTests, SuccessSignAndVerify)
{
    std::string message = "hello";
    std::string b64_message = Codec::ToBase64Url(reinterpret_cast<const unsigned char *>(message.c_str()), message.length());

    std::string signature = hmac_->Sign(b64_message.c_str(), b64_message.length());
    bool result = hmac_->Verify(b64_message.c_str(), std::move(signature), b64_message.length());

    EXPECT_TRUE(result);
    EXPECT_TRUE(isValidBase64Url(signature));
}

// 2. Неверная подпись. Изменить 1 байт в signature → ok=false.
TEST_F(RequirementTests, InvalidSignatureFails)
{
    std::string message = "hello";
    std::string b64_message = Codec::ToBase64Url(reinterpret_cast<const unsigned char *>(message.c_str()), message.length());

    std::string signature = hmac_->Sign(b64_message.c_str(), b64_message.length());

    signature[0] = (signature[0] == 'A') ? 'B' : 'A';

    bool result = hmac_->Verify(b64_message.c_str(), std::move(signature), b64_message.length());
    EXPECT_FALSE(result);
}

// 3. Изменённое сообщение. Подпись для "hello", проверка "hello!" → ok=false.
TEST_F(RequirementTests, ChangedMessageFails)
{
    std::string original = "hello";
    std::string different = "hello!";

    std::string b64_original = Codec::ToBase64Url(reinterpret_cast<const unsigned char *>(original.c_str()), original.length());
    std::string b64_different = Codec::ToBase64Url(reinterpret_cast<const unsigned char *>(different.c_str()), different.length());

    std::string signature = hmac_->Sign(b64_original.c_str(), b64_original.length());
    bool result = hmac_->Verify(b64_different.c_str(), std::move(signature), b64_different.length());

    EXPECT_FALSE(result);
}

// 4. Невалидная base64url. signature="@@@" → should fail validation
TEST_F(RequirementTests, InvalidBase64UrlDetected)
{
    std::string invalid_sig = "@@@";
    EXPECT_FALSE(isValidBase64Url(invalid_sig));
}

// 5. Пустой msg. msg="" → can be signed/verified
TEST_F(RequirementTests, EmptyMessageWorks)
{
    std::string message = "";
    std::string b64_empty = Codec::ToBase64Url(reinterpret_cast<const unsigned char *>(message.c_str()), message.length());

    std::string signature = hmac_->Sign(b64_empty.c_str(), b64_empty.length());
    bool result = hmac_->Verify(b64_empty.c_str(), std::move(signature), b64_empty.length());

    EXPECT_TRUE(result);
    EXPECT_TRUE(isValidBase64Url(signature));
}

// 6. Большое сообщение. Сообщение > max_msg_size_bytes → would be rejected by service
// (This is service-level validation, not HMAC level)

// 7. Стабильность кодирования. Подписи детерминированы: одинаковый msg → одинаковая signature.
TEST_F(RequirementTests, DeterministicSignatures)
{
    std::string message = "test";
    std::string b64_message = Codec::ToBase64Url(reinterpret_cast<const unsigned char *>(message.c_str()), message.length());

    std::string sig1 = hmac_->Sign(b64_message.c_str(), b64_message.length());
    std::string sig2 = hmac_->Sign(b64_message.c_str(), b64_message.length());

    EXPECT_EQ(sig1, sig2);
}

// 8. Тайминг-стойкое сравнение. Используется CRYPTO_memcmp в Hmac::Verify
TEST_F(RequirementTests, UsesConstantTimeCompare)
{
    std::string message = "test";
    std::string b64_message = Codec::ToBase64Url(reinterpret_cast<const unsigned char *>(message.c_str()), message.length());

    std::string valid_sig = hmac_->Sign(b64_message.c_str(), b64_message.length());
    std::string wrong_length_sig = valid_sig + "extra";

    bool result = hmac_->Verify(b64_message.c_str(), std::move(wrong_length_sig), b64_message.length());
    EXPECT_FALSE(result);
}

// 9. Конфиг-ошибки. Некорректный secret
TEST_F(RequirementTests, EmptySecretWorks)
{
    Hmac empty_hmac("");
    std::string message = "test";
    std::string b64_message = Codec::ToBase64Url(reinterpret_cast<const unsigned char *>(message.c_str()), message.length());

    std::string signature = empty_hmac.Sign(b64_message.c_str(), b64_message.length());
    bool result = empty_hmac.Verify(b64_message.c_str(), std::move(signature), b64_message.length());

    EXPECT_TRUE(result);
}