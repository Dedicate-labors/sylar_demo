#include "config.h"
#include<map>
#include<list>
#include<utility>

namespace sylar {
    // 这里的LookupBase和Lookup含义是不一样的，后者是侧重创造方面，前者才更偏向查找含义
    ConfigVarBase::ptr Config::LookupBase(const std::string& name) {
        RWMutexType::ReadLock lock(GetMutex()); 
        auto it = GetDatas().find(name);
        return it == GetDatas().end() ? nullptr:it->second;
    }

    /*
    A:
        a:10
        c:str
    output的pair:("A.a", 10)，注意这个10是node
    prefix真就是个前缀

    ListAllMember的作用是进行平铺yaml的内容为一个map
    */
    static void ListAllMember(const std::string& prefix, const YAML::Node& node,
                              std::list<std::pair<std::string, const YAML::Node>>& output) {
        // prefix不符合定义
        if(prefix.find_first_not_of("abcdefghijkmlnopkrstuvwxyz._0123456789") 
                != std::string::npos) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "config invail name: " << prefix << " : " << node;
            return;
        }
        output.push_back(std::make_pair(prefix, node));

        // 注意：数组是没法A.a这样的，所以只用判断map
        if(node.IsMap()) {
            for(auto it = node.begin(); it != node.end(); ++it) {
                ListAllMember(prefix.empty() ? it->first.as<std::string>():prefix + "." + it->first.as<std::string>(), it->second, output);
            }
        }
    }

    void Config::LoadFromYaml(const YAML::Node& root) {
        std::list<std::pair<std::string, const YAML::Node>> all_nodes;
        ListAllMember("", root, all_nodes);

        for(auto& i : all_nodes) {
            std::string key = i.first;
            if(key.empty()) {
                continue;
            }
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            ConfigVarBase::ptr var = LookupBase(key);

            // 存在就更新旧值
            if(var) {
                if(i.second.IsScalar()) {
                    var->fromString(i.second.Scalar());
                } else {
                    std::stringstream ss;
                    ss << i.second;
                    var->fromString(ss.str());
                }
            }
        }
    }

    void Config::Visit(std::function<void(ConfigVarBase::ptr)> cb) {
        RWMutexType::ReadLock lock(GetMutex());
        ConfigVarMap& m = GetDatas();
        for(auto it = m.begin(); it != m.end(); ++it) {
            cb(it->second);
        }
    }
}