#include "../sylar/config.h"
#include "../sylar/log.h"
#include <iostream>
#include <yaml-cpp/yaml.h>

sylar::ConfigVar<int>::ptr g_int_value_config = 
    sylar::Config::Lookup("system.port", (int)8000, "system port");

// g_int_value_config是同名“system.port”， 但值类型不同，无法转换成功
sylar::ConfigVar<float>::ptr g_int_valuex_config = 
    sylar::Config::Lookup("system.port", (float)8000, "system port");

sylar::ConfigVar<float>::ptr g_float_value_config = 
    sylar::Config::Lookup("system.value", (float)0.21, "system value");

sylar::ConfigVar<std::vector<int>>::ptr g_vector_value_config = 
    sylar::Config::Lookup("system.int_vec", std::vector<int>{2, 3}, "system int vec");

sylar::ConfigVar<std::list<int>>::ptr g_list_value_config = 
    sylar::Config::Lookup("system.int_list", std::list<int>{1, 3}, "system int list");

sylar::ConfigVar<std::set<int>>::ptr g_set_value_config = 
    sylar::Config::Lookup("system.int_set", std::set<int>{1, 3}, "system int set");

sylar::ConfigVar<std::unordered_set<int>>::ptr g_uset_value_config = 
    sylar::Config::Lookup("system.int_uset", std::unordered_set<int>{1, 3}, "system int uset");

sylar::ConfigVar<std::map<std::string, int>>::ptr g_str_int_map_value_config = 
    sylar::Config::Lookup("system.str_int_map", std::map<std::string, int>{{"k", 2}}, "system str int map");

sylar::ConfigVar<std::unordered_map<std::string, int>>::ptr g_str_int_umap_value_config = 
    sylar::Config::Lookup("system.str_int_umap", std::unordered_map<std::string, int>{{"k", 2}}, "system str int umap");

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

// 读取配置文件
void test_yaml() {
    YAML::Node root = YAML::LoadFile("/home/xlzhang/Project/demo/sylar_demo/bin/conf/log.yml");
    print_yaml(root, 0);
}


void test_config() {
#define XX(g_var, name, prefix) \
    { \
        auto v = g_var->getValue(); \
        for(auto& i : v) { \
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name ": " << i; \
        } \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }

#define XX_M(g_var, name, prefix) \
    { \
        auto v = g_var->getValue(); \
        for(auto& i : v) { \
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name ": {" \
                << i.first << " - " << i.second << "}"; \
        } \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before: " << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before: " << g_float_value_config->getValue();
    XX(g_vector_value_config, int_vec, before)
    XX(g_list_value_config, int_list, before)
    XX(g_set_value_config, int_set, before)
    XX(g_uset_value_config, int_uset, before)
    XX_M(g_str_int_map_value_config, str_int_map, before)
    XX_M(g_str_int_umap_value_config, str_int_umap, before)

    YAML::Node root = YAML::LoadFile("/home/xlzhang/Project/demo/sylar_demo/bin/conf/log.yml");
    sylar::Config::LoadFromYaml(root);

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after: " << g_int_value_config->toString();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after: " << g_float_value_config->toString();
    XX(g_vector_value_config, int_vec, after)
    XX(g_list_value_config, int_list, after)
    XX(g_set_value_config, int_set, after)
    XX(g_uset_value_config, int_uset, after)
    XX_M(g_str_int_map_value_config, str_int_map, after)
    XX_M(g_str_int_umap_value_config, str_int_umap, after)
}


struct Person{
    std::string m_name="zxl";
    int m_age = 0;
    bool m_sex = 0;

    // 它自己的toString()只是方便我们测试
    std::string toString() const {
        std::stringstream ss;
        ss  << "{Person name=" << m_name
            << " age=" << m_age
            << " sex=" << m_sex
            << "}";
        return ss.str();
    }

    bool operator==(const Person& oth) const {
        return m_name == oth.m_name
        && m_age == oth.m_age
        && m_sex == oth.m_sex;
    }
};

namespace sylar{
// 下面是模板偏特化

template<>
class LexicalCast<std::string, Person> {
public:
    Person operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        Person p;
        p.m_name = node ["name"].as<std::string>();
        p.m_age = node["age"].as<int>();
        p.m_sex = node["sex"].as<bool>();
        return p;
    }
}; 


template<>
class LexicalCast<Person, std::string> {
public:
    std::string operator() (const Person& p) {
        YAML::Node node;
        node["name"] = p.m_name;
        node["age"] = p.m_age;
        node["sex"] = p.m_sex;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

}

sylar::ConfigVar<Person>::ptr g_person = 
    sylar::Config::Lookup("class.person", Person(), "class person");

sylar::ConfigVar<std::map<std::string, Person>>::ptr g_person_map = 
    sylar::Config::Lookup("class.map", std::map<std::string, Person>{{"class.person1", Person()}}, "class person map");

void test_class() {
#define XX_PM(g_var, prefix) \
    { \
        auto m = g_var->getValue(); \
        for(auto& i:m) { \
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << prefix << ": " << i.first << " - " << i.second.toString(); \
        } \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << prefix << ": size=" << m.size(); \
    }
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before: " << g_person->getValue().toString() << " - " << g_person->toString();
    XX_PM(g_person_map, "class.map before")
    g_person->addListener([](const Person& old_value, const Person& new_value){
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "old_value=" << old_value.toString()
            << " new_value=" << new_value.toString();
    });

    YAML::Node root = YAML::LoadFile("/home/xlzhang/Project/demo/sylar_demo/bin/conf/test.yml");
    sylar::Config::LoadFromYaml(root);

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after: " << g_person->getValue().toString() << " - " << g_person->toString();
    XX_PM(g_person_map, "class.map after")
}

void test_log() {
    sylar::Logger::ptr system_log = SYLAR_LOG_NAME("system");
    SYLAR_LOG_INFO(system_log) << "hello system" << std::endl;
    std::cout << sylar::LoggerMgr::GetInstance().toYamlString() << std::endl;
    YAML::Node root = YAML::LoadFile("/home/xlzhang/Project/demo/sylar_demo/bin/conf/log.yml");
    sylar::Config::LoadFromYaml(root);
    std::cout << "=================" << std::endl;
    std::cout << sylar::LoggerMgr::GetInstance().toYamlString() << std::endl;
    std::cout << "=================" << std::endl;
    SYLAR_LOG_INFO(system_log) << "hello system" << std::endl;
    system_log->setFormatter("%d - %m%n");
    SYLAR_LOG_INFO(system_log) << "hello system" << std::endl;
}


int main(int argc, char **argv) {
    // test_yaml();
    // test_log();
    test_class();
    // test_config();
    // sylar::Config::Visit([](sylar::ConfigVarBase::ptr var) {
    //     SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "name=" << var->getName()
    //         << " description=" << var->getDescription()
    //         << " typename=" << var->getTypeName()
    //         << " value=" << var->toString();
    // });
    return 0;
}