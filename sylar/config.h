// 配置模块

#ifndef __SYLAR_CONFIG_H__
#define __SYLAR_CONFIG_H__

#include <memory>
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <yaml-cpp/yaml.h>
#include "log.h"
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace sylar {

// 配置抽象类
class ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string& name, const std::string& description = "")
        :m_name(name)
        ,m_description(description) {
        std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
    }
    virtual ~ConfigVarBase() {}

    const std::string& getName() const { return m_name;}
    const std::string& getDescription() const { return m_description;}
    // 纯虚函数, 负责转配置类型为string类型
    virtual std::string toString() = 0;
    // 纯虚函数，负责从string类型转为配置类型
    virtual bool fromString(const std::string& val) = 0;
    virtual std::string getTypeName() const = 0;
protected:
    std::string m_name;         // 配置参数名称
    std::string m_description;  // 配置参数描述
};


//F from_type, T to_type，负责简单类型相互转换
template<class F, class T>
class LexicalCast {
public:
    T operator() (const F& v) {
        return boost::lexical_cast<T>(v);
    }
};


// 模板特化，针对string类型转为复杂类型vector<T>
template<class T>
class LexicalCast<std::string, std::vector<T>> {
public:
    // eg: v = "[1, 2, 3, 4]"
    std::vector<T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::vector<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};


// 模板特化，针对复杂类型vector<T>转为string类型
template<class F>
class LexicalCast<std::vector<F>, std::string> {
public:
    std::string operator() (const std::vector<F>& v) {
        YAML::Node node;
        for(auto& i: v) {
            node.push_back(YAML::Load(LexicalCast<F, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


// 模板特化，针对string类型转为复杂类型list<T>
template<class T>
class LexicalCast<std::string, std::list<T>> {
public:
    // eg: v = "[1, 2, 3, 4]"
    std::list<T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::list<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};


// 模板特化，针对复杂类型list<T>转为string类型
template<class F>
class LexicalCast<std::list<F>, std::string> {
public:
    std::string operator() (const std::list<F>& v) {
        YAML::Node node;
        for(auto& i: v) {
            node.push_back(YAML::Load(LexicalCast<F, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


// 模板特化，针对string类型转为复杂类型set<T>
template<class T>
class LexicalCast<std::string, std::set<T>> {
public:
    // eg: v = "{1, 2, 3, 4}"
    std::set<T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::set<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};


// 模板特化，针对复杂类型set<T>转为string类型
template<class F>
class LexicalCast<std::set<F>, std::string> {
public:
    std::string operator() (const std::set<F>& v) {
        YAML::Node node;
        for(auto& i: v) {
            node.push_back(YAML::Load(LexicalCast<F, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


// 模板特化，针对string类型转为复杂类型unordered_set<T>
template<class T>
class LexicalCast<std::string, std::unordered_set<T>> {
public:
    // eg: v = "{1, 2, 3, 4}"
    std::unordered_set<T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_set<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};


// 模板特化，针对复杂类型unorder_set<T>转为string类型
template<class F>
class LexicalCast<std::unordered_set<F>, std::string> {
public:
    std::string operator() (const std::unordered_set<F>& v) {
        YAML::Node node;
        for(auto& i: v) {
            node.push_back(YAML::Load(LexicalCast<F, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


// 模板特化，针对string类型转为复杂类型map<std::string, T>
template<class T>
class LexicalCast<std::string, std::map<std::string, T>> {
public:
    // eg: v = "{'id':1,'degree':'senior'}"
    std::map<std::string, T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::map<std::string, T> vec;
        std::stringstream ss;
        for(auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
        }
        return vec;
    }
}; 


// 模板特化，针对复杂类型map<std::string, F>转为string类型
template<class F>
class LexicalCast<std::map<std::string, F>, std::string> {
public:
    std::string operator() (const std::map<std::string, F>& v) {
        YAML::Node node;
        for(auto& i: v) {
            node[i.first] = YAML::Load(LexicalCast<F, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


// 模板特化，针对string类型转为复杂类型unordered_map<std::string, T>
template<class T>
class LexicalCast<std::string, std::unordered_map<std::string, T>> {
public:
    // eg: v = "{'id':1,'degree':'senior'}"
    std::unordered_map<std::string, T> operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_map<std::string, T> vec;
        std::stringstream ss;
        for(auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
        }
        return vec;
    }
}; 


// 模板特化，针对复杂类型unordered_map<std::string, F>转为string类型
template<class F>
class LexicalCast<std::unordered_map<std::string, F>, std::string> {
public:
    std::string operator() (const std::unordered_map<std::string, F>& v) {
        YAML::Node node;
        for(auto& i: v) {
            node[i.first] = YAML::Load(LexicalCast<F, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


/*
ConfigVar继承ConfigVarBase是模板类，负责包含管理未知类型为T的配置成员，继承非
模板重新类ConfigVarBase是为了后续通过多态方便管理ConfigVar
*/
template<class T, class FromStr = LexicalCast<std::string, T>
                , class ToStr = LexicalCast<T, std::string>>
class ConfigVar: public ConfigVarBase{
public:
    typedef std::shared_ptr<ConfigVar> ptr;
    typedef std::function<void(const T& old_value, const T& new_value)> on_change_cb;

    // ConfigVar构造，包含配置名称，配置描述，配置值
    ConfigVar(const std::string& name, const T& default_value,
              const std::string& description = "")
    :ConfigVarBase(name, description), m_val(default_value) {}
    
    /*
    使用之前定义的类型转换类LexicalCast将配置值转为string
    */
    std::string toString() override {
        try 
        {
            return ToStr()(m_val);
        } 
        catch(const std::exception& e) 
        {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::toString exception" 
            << e.what() << " convert " << typeid(m_val).name() << " to string.";
        }
        return "";
    }

    /*
    使用之前定义的类型转换类LexicalCast将string转换为配置值
    */
    bool fromString(const std::string& val) override {
        try
        {
            setValue(FromStr()(val));
            return true;
        }
        catch(const std::exception& e)
        {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::fromString exception" 
            << e.what() << " convert: string to " << typeid(m_val).name();
        }
        return false;
    }

    // 获取配置值的类型
    std::string getTypeName() const {
        return typeid(m_val).name();
    }

    // 获取配置值
    const T getValue() const { return m_val; }

    /* 传入类型为T的配置值v
    1. 如果v == ConfigVar的m_val，就不必重新设置（前提是T类型具备operator==()）
    2. 赋值新v给ConfigVar的m_val时，调用ConfigVar<T>的相关回调函数
    */
    void setValue(const T& v) {
        if(v == m_val) return;
        for(auto& i : m_cbs) {
            i.second(m_val, v);
        }
        m_val = v;
    }

    // 添加ConfigVar<T>的回调函数
    void addListener(uint64_t key, on_change_cb cb) {
        m_cbs[key] = cb;
    }

    // 删除ConfigVar<T>的回调函数
    void delListener(uint64_t key) {
        auto it = m_cbs.find(key);
        if(it == m_cbs.end()) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "find key invaild, key=" << key;
            return;
        }
        m_cbs.erase(it);
    }

    // 获取ConfigVar<T>的回调函数
    on_change_cb getListener(uint64_t key) {
        auto it = m_cbs.find(key);
        return it == m_cbs.end() ? nullptr : it->second;
    }

    // 清空ConfigVar<T>的回调函数
    void clearListener() {
        m_cbs.clear();
    }

private:
    T m_val;
    // 变更回调函数组，方便标识和比较function，所以使用map
    std::map<uint64_t, on_change_cb> m_cbs;
};


/* 
Config类是ConfigVar的管理类，内部具有模板成员以及 单例ConfigVarMap管理ConfigVar;
通过ConfigVarBase::ptr管理模板对象ConfigVar<T>
*/
class Config {
public:
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;

    /*
    模板成员函数，查询s_datas是否具备同名的ConfigVar，如果没有就重新创建
    */
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name, const T& default_value,
            const std::string& description = "") {
                std::string tmp_name = name;
                std::transform(name.begin(), name.end(), tmp_name.begin(), ::tolower);
                auto it = GetDatas().find(tmp_name);
                if(it != GetDatas().end()) {
                    auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
                    if(tmp) {
                        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Lookup name=" << name << "exists";
                        return tmp;
                    } else {
                        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name=" << name << " exists but type not "
                                << typeid(T).name() << " real_type=" << it->second->getTypeName()
                                << " values: " << it->second->toString();
                        return tmp;
                    }
                }

                if(name.find_first_not_of("abcdefghijkmlnopkrstuvwxyz._0123456789")
                        != std::string::npos) {
                    // name中有奇怪的字符
                    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name invaild " << name;
                    throw std::invalid_argument(name);
                }
                typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
                GetDatas()[name] = v;
                return v;
            }

    /*
    模板成员函数，赋值获取ConfigVar<T>::ptr, 没有就返回nullptr
    */
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
        std::string tmp_name = name;
        std::transform(name.begin(), name.end(), tmp_name.begin(), ::tolower);
        auto it = GetDatas().find(tmp_name);
        if (it == GetDatas().end()) {
            return nullptr;
        }
        // dynamic_pointer_cast即使失败了也是返回nullptr
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }

    static ConfigVarBase::ptr LookupBase(const std::string& name);

    static void LoadFromYaml(const YAML::Node& root);

private:

    // 获取ConfigVar的map容器s_datas，它是单例
    static ConfigVarMap& GetDatas() {
        static ConfigVarMap s_datas;
        return s_datas;
    }
};

}

#endif