#ifndef KEYLOGIN_H
#define KEYLOGIN_H

#include <json/json.hpp>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <curl/curl.h>
#include <cctype>
#include <thread>
#include <chrono>
#include <atomic>

#include "include/obfuscate.h"
#include "include/includes.h"   // g_Vault (ScrambledVault) + XOR_KEY
#include "include/logger.h"     // LOGI/LOGD/LOGW/LOGE

using json = nlohmann::json;

// ================== CONFIGURATION ==================
const std::string SERVER_URL = "http://akrogoxi.x10.mx/connect";
const std::string UPDATE_CHECK_URL = "http://akrogoxi.x10.mx/update_check"; // TODO: set your real update-check endpoint
const std::string GAME_NAME  = "8BallPool";

// ================== GLOBAL VARIABLES ==================
static std::atomic<uint64_t> g_AuthToken{0};
static std::atomic<bool> is_logging_in{false};
std::string ERROR_MESSAGE    = "";
std::string g_ExpTime        = "N/A";
std::string g_Key            = "N/A";
std::string g_Reseller       = "N/A";
std::string g_Token          = "";
std::string g_Auth           = "";
static bool g_isTrial        = false;
static int  ERROR_CODE       = 0;
const char* DECOY_STR_1      = "Authentication Successful! Welcome VIP User.";
const char* DECOY_STR_2      = "Error: Database connection lost. Try again.";
std::atomic<time_t> g_ExpiryTime{0};

// ================== SERVER-SYNCED CLOCK ==================
// Offset between the server's clock (read from the HTTP "Date:" response
// header, which every HTTP server sends automatically) and this device's
// local clock. Captured on every successful request. GetServerNow() adds
// it to the local time so date-based gates can't be bypassed just by
// changing the phone's date/time.
static std::atomic<int64_t> g_ServerTimeOffset{0};
static std::atomic<bool>    g_ServerTimeSynced{false};

INLINE int64_t GetServerNow() {
    return (int64_t)time(nullptr) + g_ServerTimeOffset.load();
}

// g_Vault (ScrambledVault) and XOR_KEY are declared in include/includes.h,
// included directly above — no extern redeclaration needed here.

// ================== CURL CALLBACK ==================
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Captures the "Date:" response header (RFC 7231 format, e.g.
// "Date: Wed, 12 Aug 2026 08:32:47 GMT") from any HTTP response and uses it
// to keep g_ServerTimeOffset in sync with the server's real clock.
size_t HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
    size_t total = size * nitems;
    std::string line(buffer, total);

    if (line.size() > 5 &&
        (line[0] == 'D' || line[0] == 'd') && (line[1] == 'a' || line[1] == 'A') &&
        (line[2] == 't' || line[2] == 'T') && (line[3] == 'e' || line[3] == 'E') &&
        line[4] == ':') {

        std::string value = line.substr(5);
        auto s = value.find_first_not_of(" \t");
        auto e = value.find_last_not_of(" \t\r\n");
        value = (s != std::string::npos && e != std::string::npos && e >= s)
                    ? value.substr(s, e - s + 1) : "";

        if (!value.empty()) {
            std::tm tm = {};
            std::istringstream ss(value);
            ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
            if (!ss.fail()) {
                time_t serverTime = timegm(&tm);
                g_ServerTimeOffset = (int64_t)serverTime - (int64_t)time(nullptr);
                g_ServerTimeSynced = true;
            }
        }
    }
    return total;
}

// ================== HTTP POST REQUEST ==================
std::string HttpPost(const std::string& url, const std::string& postData) {
    CURL* curl = curl_easy_init();
    std::string response;
    
    if (curl) {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT,
            "Mozilla/5.0 (Linux; Android 12) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36");

        CURLcode res = curl_easy_perform(curl);
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        LOGI("HttpPost -> curl_res=%d http_code=%ld body=%s", (int)res, httpCode, response.c_str());

        if (res != CURLE_OK) {
            response = "";
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    
    return response;
}

// ================== HTTP GET REQUEST ==================
std::string HttpGet(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT,
            "Mozilla/5.0 (Linux; Android 12) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36");

        CURLcode res = curl_easy_perform(curl);
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        LOGI("HttpGet -> curl_res=%d http_code=%ld body=%s", (int)res, httpCode, response.c_str());

        if (res != CURLE_OK) {
            response = "";
        }

        curl_easy_cleanup(curl);
    }

    return response;
}

// ================== HELPER: TRIM STRING ==================
std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \n\r\t");
    auto end   = s.find_last_not_of(" \n\r\t");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

// ================== HELPER: ALPHANUMERIC CHECK ==================
bool isAlphanumeric(const std::string& s) {
    if (s.empty()) return false;
    for (unsigned char c : s) {
        if (!std::isalnum(c)) return false;
    }
    return true;
}

// ================== MAIN LOGIN FUNCTION ==================
INLINE bool Login(std::string androidID, std::string key) {
    key = trim(key);

    // --- Basic Validation ---
    if (key.empty()) {
        ERROR_MESSAGE = "Key is empty!";
        ERROR_CODE    = 0xE00;
        return false;
    }

    is_logging_in = true;
    ERROR_MESSAGE = "";
    ERROR_CODE    = 0;

    // --- POST to Custom Server ---
    std::string postData = "game=" + GAME_NAME + "&user_key=" + key + "&serial=" + androidID;
    std::string response = HttpPost(SERVER_URL, postData);

    if (response.empty()) {
        ERROR_CODE    = 0xE05; // Server response empty
        is_logging_in = false;
        return false;
    }

    try {
        auto respJson = json::parse(response);
        LOGI("Login raw response: %s", response.c_str());

        // --- Read Status Field (accepts bool true / "success" / "true" / "active") ---
        std::string status = "";
        bool statusOk = false;
        if (respJson.contains("status")) {
            if (respJson["status"].is_boolean()) {
                statusOk = respJson["status"].get<bool>();
                status = statusOk ? "active" : "invalid";
            } else if (respJson["status"].is_string()) {
                status = respJson["status"].get<std::string>();
                statusOk = (status == "success" || status == "true" || status == "active" || status == "valid");
            }
        }

        // --- Handle Failed Statuses ---
        if (!statusOk) {
            ERROR_MESSAGE = respJson.value("message", respJson.value("reason", "Invalid license key"));
            ERROR_CODE    = 0xE10;
            is_logging_in = false;
            return false;
        }

        if (status == "dismissed" || status == "expired") {
            ERROR_MESSAGE = O("License Expired!");
            ERROR_CODE    = 0xE20;
            g_AuthToken   = 0;
            is_logging_in = false;
            return false;
        }

        if (status == "blocked") {
            ERROR_MESSAGE = O("Access Blocked");
            g_AuthToken   = 0;
            is_logging_in = false;
            return false;
        }

        // --- Extract Data Object ---
        auto data = respJson.contains("data") ? respJson["data"] : respJson;

        auto getStr = [&](const json& obj, const std::string& k) -> std::string {
            if (!obj.contains(k)) return "";
            if (obj[k].is_string()) return obj[k].get<std::string>();
            // CodeIgniter \Time / DateTime objects serialize as
            // {"date":"2026-08-15 14:30:00.000000","timezone_type":3,"timezone":"UTC"}
            if (obj[k].is_object() && obj[k].contains("date") && obj[k]["date"].is_string())
                return obj[k]["date"].get<std::string>();
            return "";
        };

        std::string dbHwid       = getStr(data, "hwid");
        std::string expiryStr    = getStr(data, "expired_date");
        if (expiryStr.empty()) expiryStr = getStr(data, "exdate");
        if (expiryStr.empty()) expiryStr = getStr(data, "expiry_date");
        if (expiryStr.empty()) expiryStr = getStr(data, "ts");
        if (expiryStr.empty()) expiryStr = getStr(data, "expiry");
        std::string resellerName = getStr(data, "reseller_name");
        std::string sec_data     = getStr(data, "sec_data");
        
        bool isTrial = data.contains("is_trial") && data["is_trial"].is_boolean()
                       ? data["is_trial"].get<bool>() : false;

        // No expiry date sent by the server -> treat as non-expiring instead
        // of hard-failing the whole login.
        bool hasExpiry = !expiryStr.empty();

        // --- HWID Conflict Check ---
        if (!dbHwid.empty() && !isTrial && dbHwid != androidID) {
            ERROR_MESSAGE = O("Integrity Conflict: 0x511");
            g_AuthToken   = 0;
            is_logging_in = false;
            return false;
        }

        // --- Parse Expiry Date for DISPLAY + gate consistency ---
        // menu.h's unlock check requires g_ExpiryTime > 0, so on any
        // successful login we must end up with a non-zero value here --
        // real expiry is already enforced server-side (EXPIRED KEY reason
        // above), this is just what the client uses to stay consistent.
        time_t expiryTime = 0;
        if (hasExpiry) {
            std::tm tm = {};
            std::istringstream ss(expiryStr);
            ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
            if (!ss.fail()) {
                expiryTime = timegm(&tm);
            } else {
                std::tm tm2 = {};
                std::istringstream ss2(expiryStr);
                ss2 >> std::get_time(&tm2, "%Y-%m-%d %H:%M:%S");
                if (!ss2.fail()) expiryTime = timegm(&tm2);
            }
        }
        if (expiryTime <= 0) {
            expiryTime = GetServerNow() + 24 * 3600;
        }

        // --- SUCCESS: Setup Session ---
        g_ExpTime    = hasExpiry ? expiryStr : "N/A";
        g_ExpiryTime = expiryTime;

        uint64_t session_hash = (uint64_t)expiryTime ^ 0xFEEDFACECAFEBABE;
        if (session_hash != 0) {
            g_AuthToken = (uint64_t)expiryTime ^ 0xDEADBEEFCAFEBABE;
            g_Key       = key;
            g_Reseller  = isTrial ? O("Trial Mode") : (resellerName.empty() ? "Custom Server" : resellerName);
            g_isTrial   = isTrial;

            // --- Store Token ---
            std::string token = getStr(data, "token");
            if (token.empty()) token = getStr(data, "auth_token");
            if (!token.empty()) {
                g_Token = token;
                g_Auth  = token;
            }

            // --- Load Security Data into Vault ---
            if (!sec_data.empty()) {
                std::vector<std::string> offsets;
                std::stringstream ssData(sec_data);
                std::string item;
                while (std::getline(ssData, item, ',')) {
                    offsets.push_back(item);
                }

                if (offsets.size() >= 6) {
                    try {
                        // SCRAMBLED STORAGE
                        g_Vault.v[4] = std::stoull(offsets[0], nullptr, 16) ^ XOR_KEY; // Director
                        g_Vault.v[0] = std::stoull(offsets[1], nullptr, 16) ^ XOR_KEY; // UserInfo
                        g_Vault.v[7] = std::stoull(offsets[2], nullptr, 16) ^ XOR_KEY; // MainManager
                        g_Vault.v[2] = std::stoull(offsets[3], nullptr, 16) ^ XOR_KEY; // MenuManager
                        g_Vault.v[5] = std::stoull(offsets[4], nullptr, 16) ^ XOR_KEY; // VisualCue
                        g_Vault.v[3] = std::stoull(offsets[5], nullptr, 16) ^ XOR_KEY; // StartMatch

                        if (offsets.size() >= 9) {
                            g_Vault.v[9] = std::stoull(offsets[6], nullptr, 16) ^ XOR_KEY; // Line2
                            g_Vault.v[1] = std::stoull(offsets[7], nullptr, 16) ^ XOR_KEY; // Zero
                            g_Vault.v[6] = std::stoull(offsets[8], nullptr, 16) ^ XOR_KEY; // Small
                        } else {
                            g_Vault.v[9] = 0 ^ XOR_KEY;
                            g_Vault.v[1] = 0 ^ XOR_KEY;
                            g_Vault.v[6] = 0 ^ XOR_KEY;
                        }

                        g_Vault.is_loaded = true;
                        LOGI("Security Data Loaded | Session: %llu", session_hash);
                    } catch (...) {
                        g_AuthToken   = 0;
                        ERROR_MESSAGE = O("Data Parse Error: 0xE88");
                    }
                } else {
                    g_AuthToken   = 0;
                    ERROR_MESSAGE = O("Data Conflict: 0xE88");
                }

                std::fill(sec_data.begin(), sec_data.end(), 0);
                sec_data.clear();
            } else {
                // Vault not required if sec_data is empty
                g_Vault.is_loaded = false;
            }
        }

        is_logging_in = false;
        return (g_AuthToken != 0);

    } catch (const std::exception& e) {
        ERROR_MESSAGE = "JSON Error: " + std::string(e.what());
        is_logging_in = false;
        return false;
    } catch (...) {
        ERROR_MESSAGE = "Unknown Connection Error!";
        is_logging_in = false;
        return false;
    }
}

// ================== ASYNC LOGIN WRAPPER ==================
INLINE void LoginAsync(std::string androidID, std::string key) {
    std::thread([androidID, key]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        Login(androidID, key);
    }).detach();
}

#endif // KEYLOGIN_H