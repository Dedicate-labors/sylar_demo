// 配置模块

#ifndef __SYLAR_CONFIG_H__
#define __SYLAR_CONFIG_H__

#include<memory>
#include<string>
#include<sstream>
#include<boost/lexical_cast.hpp>
#include<algorithm>
#include<yaml-cpp/yaml.h>
#include "log.h"

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
private:
    std::string m_name;  // 配置参数名称
    std::string m_description;  // 配置参数描述
};

template<class T>
class ConfigVar: public ConfigVarBase{
public:
    typedef std::shared_ptr<ConfigVar> ptr;

    ConfigVar(const T& default_value, const std::string& name, 
              const std::string& description = "")
            :ConfigVarBase(name, description)
            ,m_val(default_value) {}
    std::string toString() override {
        try 
        {
            return boost::lexical_cast<std::string>(m_val);
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
           m_val = boost::lexical_cast<T>(val);
        }
        catch(const std::exception& e)
        {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::fromString exception" 
            << e.what() << " convert: string to " << typeid(m_val).name();
        }
        return false;
    }

    std::string getTypeName() const {
        return "";
    }

    const T getValue() const { return m_val; }
    void setValue(const T& v) { m_val = v; }
private:
    T m_val;
};


// 下面的这个算是config管理员
class Config {
public:
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;
    // 找不到就创建
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const T& default_value,const std::string& name, 
            const std::string& description = "") {
                auto tmp = Lookup<T>(name);
                if(tmp) {
                    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Lookup name=" << name << "exists";
                    return tmp;
                }
                if(name.find_first_not_of("abcdefghijkmlnopkrstuvwxyz._0123456789")
                        != std::string::npos) {
                    // name中有奇怪的字符
                    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name invaild " << name;
                    throw std::invalid_argument(name);
                }
                typename ConfigVar<T>::ptr v(new ConfigVar<T>(default_value, name, description));
                m_datas[name] = v;
                return v;
            }
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
        std::string tmp_name = name;
        std::transform(name.begin(), name.end(), tmp_name.begin(), ::tolower);
        auto it = m_datas.find(tmp_name);
        if (it == m_datas.end()) {
            return nullptr;
        }
        // dynamic_pointer_cast即使失败了也是返回nullptr
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }

    static void LoadFromYaml(const YAML::Node& root);
    static ConfigVarBase::ptr LookupBase(const std::string& name);
private:
    static ConfigVarMap m_datas;
};

}

#endif