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
    // 有纯虚函数是抽象类
    virtual std::string toString() = 0;
    virtual bool fromString(const std::string& val) = 0;
    virtual std::string getTypeName() const = 0;
protected:
    std::string m_name;  // 配置参数名称
    std::string m_description;  // 配置参数描述
};


//F from_type, T to_type
template<class F, class T>
class LexicalCast {
public:
    T operator() (const F& v) {
        return boost::lexical_cast<T>(v);
    }
};


// 进行特化，针对map,vector等复杂类型
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


// 针对复杂类型(如自定义类型)可以使用  序列化的方式进行字符串和类型间的转换
// FromStr  T operator() (const std::string&)
// ToStr std::string operator() (const T&)
// FromStr和ToStr的默认本版只支持简单类型转换，如自定义，vector的复杂类型的不行
template<class T, class FromStr = LexicalCast<std::string, T>
                , class ToStr = LexicalCast<T, std::string>>
class ConfigVar: public ConfigVarBase{
public:
    typedef std::shared_ptr<ConfigVar> ptr;
    typedef std::function<void(const T& old_value, const T& new_value)> on_change_cb;

    ConfigVar(const std::string& name, const T& default_value,
              const std::string& description = "")
            :ConfigVarBase(name, description)
            ,m_val(default_value) {}
    std::string toString() override {
        try 
        {
            // return boost::lexical_cast<std::string>(m_val);
            return ToStr()(m_val);
        } 
        catch(const std::exception& e) 
        {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::toString exception" 
            << e.what() << " convert " << typeid(m_val).name() << " to string.";
        }
        return "";
    }

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

    std::string getTypeName() const {
        return typeid(m_val).name();
    }

    const T getValue() const { return m_val; }
    void setValue(const T& v) {
        if(v == m_val) return;  // T 类型变量不一定有 operator==();
        for(auto& i : m_cbs) {
            i.second(m_val, v);
        }
        m_val = v;
    }

    void addListener(uint64_t key, on_change_cb cb) {
        m_cbs[key] = cb;
    }

    void delListener(uint64_t key) {
        auto it = m_cbs.find(key);
        if(it == m_cbs.end()) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "find key invaild, key=" << key;
            return;
        }
        m_cbs.erase(it);
    }

    on_change_cb getListener(uint64_t key) {
        auto it = m_cbs.find(key);
        return it == m_cbs.end() ? nullptr : it->second;
    }

    void clearListener() {
        m_cbs.clear();
    }
private:
    T m_val;
    // 变更回调函数组，方便标识和比较function，所以使用map
    std::map<uint64_t, on_change_cb> m_cbs;
};


// 下面的这个算是config管理员
class Config {
public:
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;
    // 找不到就创建
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
                                << " values:\n" << it->second->toString();
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

    static void LoadFromYaml(const YAML::Node& root);
    static ConfigVarBase::ptr LookupBase(const std::string& name);
private:
    static ConfigVarMap& GetDatas() {
        static ConfigVarMap s_datas;
        return s_datas;
    }
};

}

#endif