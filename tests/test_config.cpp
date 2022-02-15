#include "../sylar/config.h"
#include "../sylar/log.h"
#include <yaml-cpp/yaml.h>

sylar::ConfigVar<int>::ptr g_int_value_config = 
    sylar::Config::Lookup((int)8000, "system.port", "system.port");

sylar::ConfigVar<float>::ptr g_float_value_config = 
    sylar::Config::Lookup((float)0.21, "system.value", "system.value");

void print_yaml(const YAML::Node& node, int level) {
    if(node.IsScalar()) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level*4, ' ') << node.Scalar() << " - " << node.Type() << " - " << level;
    } else if(node.IsNull()) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level*4, ' ') << "NULL - " << node.Type() << " - " << level;
    } else if(node.IsMap()) {
        for(auto it = node.begin(); it != node.end(); ++it) {
            // YAML::Node node;node.as<string>()把值变成string, 也能直接打印node
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level*4, ' ') << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    } else if (node.IsSequence()) {
        for(size_t i = 0; i < node.size(); ++i) {
            // YAML::Node node;node.as<string>()把值变成string, 也能直接打印node
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level*4, ' ') << i << " - " << node[i].Tag() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

void test_yaml() {
    YAML::Node root = YAML::LoadFile("/home/xlzhang/Project/demo/sylar_demo/bin/conf/log.yml");
    print_yaml(root, 0);
}


void test_config() {
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before: " << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before: " << g_float_value_config->getValue();
    YAML::Node root = YAML::LoadFile("/home/xlzhang/Project/demo/sylar_demo/bin/conf/log.yml");
    sylar::Config::LoadFromYaml(root);
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after: " << g_int_value_config->toString();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after: " << g_float_value_config->toString();
}

int main(int argc, char **argv) {
    // test_yaml();
    test_config();
    return 0;
}