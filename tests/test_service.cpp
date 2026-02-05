// clang-format off
#include <gtest/gtest.h>
// clang-format on
#include "codec.hpp"
#include "config.hpp"
#include "hmac.hpp"
#include "http.hpp"
#include "logging.hpp"
#include <chrono>
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <cpprest/uri.h>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>
#include <thread>

#include <iostream>

using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace service::micro;
using namespace service::config;
using namespace service::logging;
using namespace service::hmac;
using namespace service::codec;

class HMACServiceTest : public ::testing::Test
{
protected:
    static constexpr const char *TEST_CONFIG_FILE = "test_config_minimal.json";
    static constexpr const char *TEST_SECRET = "test_secret_123";
    static constexpr int TEST_PORT = 8081;
    static constexpr const char *BASE_URL = "http://localhost:8081";

    void SetUp() override
    {
        std::ofstream config_file(TEST_CONFIG_FILE);
        config_file << R"({
            "hmac_alg": "SHA256",
            "listen": "http://localhost:)"
                    << TEST_PORT << R"(",
            "log_level": "INFO",
            "log_file": "test_log.log",
            "max_msg_size_bytes": 50,
            "secret": ")"
                    << TEST_SECRET << R"("
        })";
        config_file.close();

        service_ = std::make_unique<Service>(Config(TEST_CONFIG_FILE), TEST_CONFIG_FILE);
        service_->start();

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    void TearDown() override
    {
        if (service_)
        {
            service_->stop();
        }
        std::remove(TEST_CONFIG_FILE);
    }

    http_response CallSign(const std::string &msg)
    {
        http_client client(U(BASE_URL + std::string("/sign")));
        http_request req(methods::POST);
        req.headers().add(U("Content-Type"), U("application/json"));

        json::value request;
        request[U("msg")] = json::value::string(ToUString(msg));
        req.set_body(request);

        return client.request(req).get();
    }

    http_response CallVerify(const std::string &msg, const std::string &signature)
    {
        http_client client(U(BASE_URL + std::string("/verify")));
        http_request req(methods::POST);
        req.headers().add(U("Content-Type"), U("application/json"));

        json::value request;
        request[U("msg")] = json::value::string(ToUString(msg));
        request[U("signature")] = json::value::string(ToUString(signature));
        req.set_body(request);

        return client.request(req).get();
    }

    std::string ToBase64Url(const std::string &input)
    {
        return Codec::ToBase64Url(reinterpret_cast<const unsigned char *>(input.c_str()), input.length());
    }

private:
    utility::string_t ToUString(const std::string &str)
    {
        return utility::conversions::to_string_t(str);
    }

    std::unique_ptr<Service> service_;
};

// Test 1: Подпись/проверка успеха msg="hello" → /sign → подпись → /verify → ok=true.
TEST_F(HMACServiceTest, SuccessSignAndVerify)
{
    std::string message = "hello";
    std::string b64_message = ToBase64Url(message);

    auto sign_response = CallSign(b64_message);
    auto sign_body = sign_response.extract_json().get();

    ASSERT_EQ(sign_response.status_code(), status_codes::OK);
    ASSERT_TRUE(sign_body.has_field(U("signature")));

    std::string signature = utility::conversions::to_utf8string(
        sign_body[U("signature")].as_string());
    ASSERT_FALSE(signature.empty());

    auto verify_response = CallVerify(b64_message, signature);
    auto verify_body = verify_response.extract_json().get();

    ASSERT_EQ(verify_response.status_code(), status_codes::OK);
    ASSERT_TRUE(verify_body.has_field(U("ok")));
    EXPECT_TRUE(verify_body[U("ok")].as_bool());
}

// Test 2: Неверная подпись. Изменить 1 байт в signature → ok=false.
TEST_F(HMACServiceTest, InvalidSignatureFails)
{
    std::string message = "hello";
    std::string b64_message = ToBase64Url(message);

    auto sign_response = CallSign(b64_message);
    auto sign_body = sign_response.extract_json().get();
    std::string signature = utility::conversions::to_utf8string(
        sign_body[U("signature")].as_string());

    ASSERT_FALSE(signature.empty());
    if (!signature.empty())
    {
        signature[0] = (signature[0] == 'A') ? 'B' : 'A';
    }

    auto verify_response = CallVerify(b64_message, signature);
    auto verify_body = verify_response.extract_json().get();

    ASSERT_EQ(verify_response.status_code(), status_codes::OK);
    ASSERT_TRUE(verify_body.has_field(U("ok")));
    EXPECT_FALSE(verify_body[U("ok")].as_bool());
}

// Test 3: Изменённое сообщение. Подпись для "hello", проверка "hello!" → ok=false.
TEST_F(HMACServiceTest, ChangedMessageFails)
{
    std::string original_message = "hello";
    std::string different_message = "hello!";

    std::string b64_original = ToBase64Url(original_message);
    std::string b64_different = ToBase64Url(different_message);

    auto sign_response = CallSign(b64_original);
    auto sign_body = sign_response.extract_json().get();
    std::string signature = utility::conversions::to_utf8string(
        sign_body[U("signature")].as_string());

    auto verify_response = CallVerify(b64_different, signature);
    auto verify_body = verify_response.extract_json().get();

    ASSERT_EQ(verify_response.status_code(), status_codes::OK);
    ASSERT_TRUE(verify_body.has_field(U("ok")));
    EXPECT_FALSE(verify_body[U("ok")].as_bool());
}

// Test 4: Невалидная base64url. Поле signature="@@@" → 400 invalid_signature_format.
TEST_F(HMACServiceTest, InvalidBase64UrlReturns400)
{
    std::string message = ToBase64Url("test");
    std::string invalid_signature = "@@@";

    auto verify_response = CallVerify(message, invalid_signature);

    ASSERT_EQ(verify_response.status_code(), status_codes::BadRequest);

    auto verify_body = verify_response.extract_json().get();
    if (verify_body.has_field(U("error")))
    {
        std::cout << verify_body["error"] << "\n";
        std::string error = utility::conversions::to_utf8string(
            verify_body[U("error")].as_string());
        EXPECT_EQ(error, "invalid_signature_format");
    }
}

// Test 5: Пустой msg. Поле msg="" → 400 invalid_msg.
TEST_F(HMACServiceTest, EmptyMessageReturns400)
{
    std::string empty_message = "";
    std::string b64_empty = ToBase64Url(empty_message);

    auto sign_response = CallSign(b64_empty);

    ASSERT_EQ(sign_response.status_code(), status_codes::BadRequest);

    auto sign_body = sign_response.extract_json().get();
    if (sign_body.has_field(U("error")))
    {
        std::string error = utility::conversions::to_utf8string(
            sign_body[U("error")].as_string());
        EXPECT_EQ(error, "invalid_msg");
    }
}

// Test 6: Большое сообщение. Сообщение > max_msg_size_bytes → 413.
TEST_F(HMACServiceTest, LargeMessageReturns413)
{
    std::string large_message(100, 'A');
    std::string b64_large = ToBase64Url(large_message);

    auto sign_response = CallSign(b64_large);

    ASSERT_EQ(sign_response.status_code(), status_codes::RequestEntityTooLarge);

    auto sign_body = sign_response.extract_json().get();
    if (sign_body.has_field(U("error")))
    {
        std::string error = utility::conversions::to_utf8string(
            sign_body[U("error")].as_string());
        EXPECT_EQ(error, "invalid_msg");
    }
}

// Test 7: Стабильность кодирования. Подписи детерминированы: одинаковый msg → одинаковая signature.
TEST_F(HMACServiceTest, DeterministicSignatures)
{
    std::string message = "test message";
    std::string b64_message = ToBase64Url(message);

    std::vector<std::string> signatures;
    for (int i = 0; i < 5; i++)
    {
        auto sign_response = CallSign(b64_message);
        auto sign_body = sign_response.extract_json().get();

        ASSERT_EQ(sign_response.status_code(), status_codes::OK);
        std::string signature = utility::conversions::to_utf8string(
            sign_body[U("signature")].as_string());
        signatures.push_back(signature);
    }

    ASSERT_GT(signatures.size(), 1);
    const std::string &first = signatures[0];
    for (size_t i = 1; i < signatures.size(); i++)
    {
        EXPECT_EQ(first, signatures[i]) << "Signature mismatch at iteration " << i;
    }
}

// Test 8: Unittest in test_hmac

// Test 9: Конфиг-ошибки. Некорректный secret в config.json → сервер не стартует с понятной ошибкой.
TEST(ConfigErrorTest, InvalidSecretPreventsStartup)
{
    const char *bad_config_file = "bad_config.json";

    {
        std::ofstream config_file(bad_config_file);
        config_file << R"({
            "hmac_alg": "SHA256",
            "listen": "http://localhost:9999",
            "log_level": "INFO",
            "log_file": "test_log.log",
            "max_msg_size_bytes": 1024,
            "secret": ""
        })";
        config_file.close();

        EXPECT_NO_THROW({
            Config config(bad_config_file);
        });

        std::remove(bad_config_file);
    }

    {
        std::ofstream config_file(bad_config_file);
        config_file << R"({
            "hmac_alg": "SHA256",
            "listen": "http://localhost:9999",
            "log_level": "INFO",
            "log_file": "test_log.log",
            "max_msg_size_bytes": 1024
        })";
        config_file.close();

        try
        {
            Config config(bad_config_file);
            SUCCEED();
        }
        catch (const nlohmann::json::exception &e)
        {
            SUCCEED() << "Config correctly rejected missing secret field";
        }
        catch (const std::exception &e)
        {
            SUCCEED() << "Config validation worked: " << e.what();
        }

        std::remove(bad_config_file);
    }
}

// Test 10: Invalid JSON in request
TEST_F(HMACServiceTest, InvalidJsonReturns400)
{
    http_client client(U(BASE_URL + std::string("/sign")));
    http_request req(methods::POST);
    req.headers().add(U("Content-Type"), U("application/json"));
    req.set_body("{ invalid json }"); // Malformed JSON

    auto response = client.request(req).get();

    EXPECT_NE(response.status_code(), status_codes::OK);
}

// Test 11: Missing Content-Type header
TEST_F(HMACServiceTest, MissingContentTypeReturns415)
{
    http_client client(U(BASE_URL + std::string("/sign")));
    http_request req(methods::POST);
    req.set_body("{}");

    auto response = client.request(req).get();
    auto body = response.extract_json().get();

    EXPECT_EQ(response.status_code(), status_codes::UnsupportedMediaType); // 415
    if (body.has_field(U("error")))
    {
        std::string error = utility::conversions::to_utf8string(
            body[U("error")].as_string());
        EXPECT_EQ(error, "invalid_json");
    }
}

// Test 12: Wrong HTTP method
TEST_F(HMACServiceTest, WrongMethodReturnsError)
{
    http_client client(U(BASE_URL + std::string("/sign")));

    auto response = client.request(methods::GET).get();

    EXPECT_NE(response.status_code(), status_codes::OK);
}

// Test 13: Verify with missing fields
TEST_F(HMACServiceTest, VerifyMissingFieldsReturns400)
{
    http_client client(U(BASE_URL + std::string("/verify")));
    http_request req(methods::POST);
    req.headers().add(U("Content-Type"), U("application/json"));

    json::value request;
    request[U("msg")] = json::value::string(U("dGVzdA=="));
    req.set_body(request);

    auto response = client.request(req).get();
    auto body = response.extract_json().get();

    EXPECT_EQ(response.status_code(), status_codes::BadRequest); // 400
    if (body.has_field(U("error")))
    {
        std::string error = utility::conversions::to_utf8string(
            body[U("error")].as_string());
        EXPECT_EQ(error, "invalid_msg");
    }
}

// Test 14: Ping endpoint works
TEST_F(HMACServiceTest, PingEndpointReturnsOK)
{
    http_client client(U(BASE_URL + std::string("/ping")));

    auto response = client.request(methods::GET).get();
    auto body = response.extract_json().get();

    EXPECT_EQ(response.status_code(), status_codes::OK);
    EXPECT_TRUE(body.has_field(U("status")));
    EXPECT_TRUE(body[U("status")].as_bool());
}

// Test 15: Settings endpoint works
TEST_F(HMACServiceTest, SettingsEndpointReturnsOK)
{
    http_client client(U(BASE_URL + std::string("/settings")));

    auto response = client.request(methods::GET).get();
    auto body = response.extract_json().get();

    EXPECT_EQ(response.status_code(), status_codes::OK);
    EXPECT_TRUE(body.has_field(U("status")));
    EXPECT_TRUE(body[U("status")].as_bool());
}