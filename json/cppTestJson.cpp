#include <iostream>
#include "rapidjson/document.h"

int main() {
    // JSON 数据
    const char* json = R"({"name": "ChatGPT", "age": 2, "skills": ["C++", "Python"]})";

    // 解析 JSON
    rapidjson::Document doc;
    doc.Parse(json);

    // 读取数据
    if (doc.HasMember("name") && doc["name"].IsString()) {
        std::cout << "Name: " << doc["name"].GetString() << std::endl;
    }

    if (doc.HasMember("age") && doc["age"].IsInt()) {
        std::cout << "Age: " << doc["age"].GetInt() << std::endl;
    }

    if (doc.HasMember("skills") && doc["skills"].IsArray()) {
        std::cout << "Skills: ";
        for (const auto& skill : doc["skills"].GetArray()) {
            std::cout << skill.GetString() << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}

