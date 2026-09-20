#pragma once
#include "aip-cpp-sdk-4.16.7/speech.h"
#include "logger.hpp"

class ASRClient
{
public:
    ASRClient(const std::string &app_id, const std::string &api_key, const std::string &secret_key)
        :_client(app_id, api_key, secret_key){}
    std::string recognize(const std::string &speech_data)
    {
        std::map<std::string, std::string> options;
        options["dev_pid"] = "1537";
        Json::Value result = _client.recognize(speech_data, "pcm", 16000, options);
        if (result["err_no"].asInt() != 0) 
        {
            LOG_ERROR("语音识别失败: {}", result["err_msg"].asString());
            return std::string();
        }
        return result["result"][0].asString();
    }
private:
    aip::Speech _client;
};