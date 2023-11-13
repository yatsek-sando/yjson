#include <string>
#include <map>
#include <vector>
#include <variant>
#include <optional>
#include <iostream>
#include <sstream>

class YJson;

using YJsonVar = std::variant<
    bool,
    int,
    double,
    std::string,
    std::vector<YJson>,
    std::map<std::string, YJson>
>;

class YJson {
public:
    YJson() {}
    YJson(bool value) : value_(value) {}
    YJson(int value) : value_(value) {}
    YJson(double value) : value_(value) {}
    YJson(const std::string& value) : value_(value) {}
    YJson(const char* value) : value_(std::string(value)) {}
    YJson(const std::vector<YJson>& value) : value_(value) {}
    YJson(const std::map<std::string, YJson>& value) : value_(value) {}

    bool has_value() const { return value_.has_value(); }
    void reset() { value_.reset(); }

    std::string dump() const {
        if (!value_) {
            return "null";
        }
        return dumpValue(*value_);
    }
    template <typename Visitor>
    auto visit(Visitor&& visitor) {
        if (value_.has_value()) {
            return std::visit(std::forward<Visitor>(visitor), *value_);
        }
        throw std::runtime_error("No value present in YJson.");
    }

    YJson& operator[](const std::string& key) {
        if (!value_ || !std::holds_alternative<std::map<std::string, YJson>>(*value_)) {
            value_ = std::map<std::string, YJson>{};
        }
        return std::get<std::map<std::string, YJson>>(*value_)[key];
    }

    YJson& operator[](size_t index) {
        if (!value_ || !std::holds_alternative<std::vector<YJson>>(*value_)) {
            value_ = std::vector<YJson>{};
        }
        auto& vec = std::get<std::vector<YJson>>(*value_);
        if (index >= vec.size()) {
            vec.resize(index + 1);
        }
        return vec[index];
    }
    YJson& operator=(std::initializer_list<YJson> init_list) {
        value_ = std::vector<YJson>(init_list.begin(), init_list.end());
        return *this;
    }


private:
    std::optional<YJsonVar> value_;

    static std::string dumpValue(const YJsonVar& value) {
        std::ostringstream oss;
        std::visit([&oss](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, bool>) {
                oss << (arg ? "true" : "false");
            } else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, double>) {
                oss << arg;
            } else if constexpr (std::is_same_v<T, std::string>) {
                oss << "\"" << arg << "\"";
            } else if constexpr (std::is_same_v<T, std::vector<YJson>>) {
                oss << "[";
                for (auto it = arg.begin(); it != arg.end(); ++it) {
                    if (it != arg.begin()) {
                        oss << ", ";
                    }
                    oss << it->dump();
                }
                oss << "]";
            } else if constexpr (std::is_same_v<T, std::map<std::string, YJson>>) {
                oss << "{";
                for (auto it = arg.begin(); it != arg.end();) {
                    oss << "\"" << it->first << "\": " << it->second.dump();
                    if (++it != arg.end()) {
                        oss << ", ";
                    }
                }
                oss << "}";
            }
        }, value);
        return oss.str();
    }
};


int main() {
    using JsonList = std::vector<YJson>;
    using JsonObj = std::map<std::string,YJson>;
    YJson jsonInt(42);
    std::cout << "jsonInt:\n" << jsonInt.dump() << std::endl;

    YJson jsonString("Hello, World!");
    std::cout << "jsonString:\n" << jsonString.dump() << std::endl;

    YJson jsonArray(std::vector<YJson>{jsonInt, jsonString});
    std::cout << "jsonArray:\n" << jsonArray.dump() << std::endl;

    YJson jsonObj;

    jsonObj["array"][0] = 1;
    jsonObj["array"][1] = 2;
    jsonObj["array"][2] = 3;
    jsonObj["array"][3] = "text";
    jsonObj["array"][6] = JsonObj{{"a",2},{"b",3}};

    jsonObj["data"]["key1"] = "value1";
    jsonObj["data"]["key2"] = false;

    std::cout << jsonObj.dump() << std::endl;

    return 0;
}
