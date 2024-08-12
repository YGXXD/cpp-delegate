//  MIT License
//
//  Copyright (c) 2021-2022 有个小小杜
//
//  Created by 有个小小杜
//

#ifndef _YGXXD_DELEGATE_H_
#define _YGXXD_DELEGATE_H_

#include <vector>
#include <memory>
#include <unordered_map>

#define DECLARE_FUNCTION_DELEGATE(delegate_name, return_type, ...) typedef ::xxd::single_delegate<return_type(__VA_ARGS__)> (delegate_name);
#define DECLARE_FUNCTION_MULTICAST_DELEGATE(delegate_name, ...) typedef ::xxd::multi_delegate<__VA_ARGS__> (delegate_name);

namespace xxd 
{

class dlgt final
{
private:
    template<typename ReturnT, typename ...ArgsT>
    friend class single_delegate;

    template<typename ...ArgsT>
    friend class multi_delegate;

    // 代理接口
    template<typename ReturnT, typename ...ArgsT>
    struct idelegate;

    // 非成员函数委托模板
    template<typename ReturnT, typename ...ArgsT>
    class func_delegate;

    // 类成员函数委托模板
    template<class ClassT, typename ReturnT, typename ...ArgsT>
    class obj_func_delegate;

    // 带有安全检测的类成员函数委托模板
    template<class ClassT, typename ReturnT, typename ...ArgsT>
    class obj_func_safe_delegate;

    // 任意可调用对象委托模版
    template<class AnyFuncT, typename ReturnT, typename ...ArgsT>
    class any_func_delegate;

    template<typename ReturnT, typename ...ArgsT>
    struct idelegate<ReturnT(ArgsT...)>
    {
        virtual ReturnT operator() (ArgsT... args) noexcept = 0;
        virtual ~idelegate() { };
    };

    template<typename ReturnT, typename ...ArgsT>
    class func_delegate<ReturnT(ArgsT...)> : public idelegate<ReturnT(ArgsT...)>
    {
    public:
        typedef ReturnT(*func_type) (ArgsT...);

        func_delegate() = delete;
        explicit func_delegate(const func_type& fun) : func(fun) { };

        virtual ReturnT operator() (ArgsT... args) noexcept override
        {
            return (*func)(args...);
        }

        func_type func;
    };

    template<class ClassT, typename ReturnT, typename ...ArgsT>
    class obj_func_delegate<ClassT, ReturnT(ArgsT...)> : public idelegate<ReturnT(ArgsT...)>
    {
    public:
        typedef ReturnT(ClassT::* func_type) (ArgsT...);

        obj_func_delegate() = delete;
        explicit obj_func_delegate(ClassT* obj_ptr, const func_type& obj_fun) : obj(obj_ptr), func(obj_fun) { };

        virtual ReturnT operator() (ArgsT... args) noexcept override 
        {
            return (obj->*func)(args...);
        }

        ClassT* obj;
        func_type func;
    };
    
    template<class ClassT, typename ReturnT, typename ...ArgsT>
    class obj_func_safe_delegate<ClassT, ReturnT(ArgsT...)>  : public idelegate<ReturnT(ArgsT...)>
    {
    public:
        typedef ReturnT(ClassT::* func_type) (ArgsT...);

        obj_func_safe_delegate() = delete;
        explicit obj_func_safe_delegate(const std::shared_ptr<ClassT>& obj_shared, const func_type& obj_fun) : obj(obj_shared), func(obj_fun) { };

        virtual ReturnT operator() (ArgsT... args) noexcept override
        {
            if(!obj.expired())
                return (obj.lock().get()->*func)(args...);
            return ReturnT();
        }
        
        std::weak_ptr<ClassT> obj;
        func_type func;
    };
    
    template<class AnyFuncT, typename ReturnT, typename ...ArgsT>
    class any_func_delegate<AnyFuncT, ReturnT(ArgsT...)> : public idelegate<ReturnT(ArgsT...)>
    {
    public:
        any_func_delegate() = delete;
        explicit any_func_delegate(AnyFuncT&& fun) : func(fun) { };

        virtual ReturnT operator() (ArgsT... args) noexcept override
        {
            return func(args...);
        }

        typename std::decay<AnyFuncT>::type func;
    };
    
};

template<typename ReturnT, typename ...ArgsT>
class single_delegate;

template<typename ...ArgsT>
class multi_delegate;

struct delegate_handle
{
private:
    template<typename ...ArgsT>
    friend class multi_delegate;

    // 代理类型, 代理id, 代理类, 绑定地址
    inline delegate_handle(uint32_t tdlgt, uint32_t idlgt, void* pdlgt, void* pbind) : 
        t(tdlgt & 0xf), i(idlgt & 0xf), p(reinterpret_cast<uintptr_t>(pdlgt) & 0xf), b(reinterpret_cast<uintptr_t>(pbind) & 0xf) { }

    // Handle转化为字符串函数
    inline uint32_t to_key() const noexcept
    {
        return t | (i << 8) | (p << 16) | (b << 24);
    };

    uint8_t t;
    uint8_t i;
    uint8_t p;
    uint8_t b;
};

template<typename ReturnT, typename ...ArgsT>
class single_delegate<ReturnT(ArgsT...)> final
{
public:
    explicit single_delegate() = default;
    
    static inline single_delegate<ReturnT(ArgsT...)> create_function(const typename dlgt::func_delegate<ReturnT(ArgsT...)>::func_type& func) noexcept;
    
    template<class ClassT>
    static inline single_delegate<ReturnT(ArgsT...)> create_object(ClassT* obj, const typename dlgt::obj_func_delegate<ClassT, ReturnT(ArgsT...)>::func_type& obj_func) noexcept;
    
    template<class ClassT>
    static inline single_delegate<ReturnT(ArgsT...)> create_safe_obj(const std::shared_ptr<ClassT>& obj_shared, const typename dlgt::obj_func_delegate<ClassT, ReturnT(ArgsT...)>::func_type& obj_func) noexcept;
    
    template<class AnyFuncT>
    static inline single_delegate<ReturnT(ArgsT...)> create_any_func(AnyFuncT&& func) noexcept;
    
    // 绑定全局或静态函数
    inline void bind_function(const typename dlgt::func_delegate<ReturnT(ArgsT...)>::func_type& func) noexcept;

    // 绑定类成员函数
    template<class ClassT>
    inline void bind_object(ClassT* obj, const typename dlgt::obj_func_delegate<ClassT, ReturnT(ArgsT...)>::func_type& obj_func) noexcept;

    // 绑定安全的类成员函数
    template<class ClassT>
    inline void bind_safe_obj(const std::shared_ptr<ClassT>& obj_shared, const typename dlgt::obj_func_delegate<ClassT, ReturnT(ArgsT...)>::func_type& obj_func) noexcept;
    
    // 绑定任意可调用对象
    template<class AnyFuncT>
    inline typename std::enable_if<!std::is_same<typename std::decay<AnyFuncT>::type, single_delegate<ReturnT(ArgsT...)>>::value>::type bind_any_func(AnyFuncT&& func) noexcept;

    // 代理执行
    inline ReturnT invoke(ArgsT... args) noexcept;
    inline ReturnT invoke_if_bind(ArgsT... args) noexcept;
    inline ReturnT operator() (ArgsT... args) noexcept;
    
    // 解绑函数
    inline void unbind() noexcept;
    
private:
    std::shared_ptr<dlgt::idelegate<ReturnT(ArgsT...)>> dlgt_ptr;
};

template<typename ...ArgsT>
class multi_delegate final
{
public:
    explicit multi_delegate() = default;

    // 添加全局或静态函数
    inline delegate_handle add_function(const typename dlgt::func_delegate<void(ArgsT...)>::func_type& func) noexcept;

    // 添加类成员函数
    template<class ClassT>
    inline delegate_handle add_object(ClassT* obj, const typename dlgt::obj_func_delegate<ClassT, void(ArgsT...)>::func_type& obj_func) noexcept;
    
    // 添加安全的类成员函数
    template<class ClassT>
    inline delegate_handle add_safe_obj(const std::shared_ptr<ClassT>& obj_shared, const typename dlgt::obj_func_delegate<ClassT, void(ArgsT...)>::func_type& obj_func) noexcept;
    
    // 添加任意可调用对象
    template<class AnyFuncT>
    inline typename std::enable_if<!std::is_same<typename std::decay<AnyFuncT>::type, multi_delegate<void(ArgsT...)>>::value, delegate_handle>::type add_any_func(AnyFuncT&& func) noexcept;
    
    // 多播代理执行
    void broad_cast(ArgsT... args) noexcept;
    inline void operator() (ArgsT... args) noexcept;
    
    // 根据代理句柄移除
    inline bool remove(const delegate_handle& handle) noexcept;
    
    // 移除全局或静态函数
    inline bool remove(const typename dlgt::func_delegate<void(ArgsT...)>::func_type& obj_func) noexcept;

    // 移除类成员函数
    template<class ClassT>
    inline bool remove(ClassT* obj, const typename dlgt::obj_func_delegate<ClassT, void(ArgsT...)>::func_type& obj_func) noexcept;
    
    // 移除安全类成员函数
    template<class ClassT>
    inline bool remove(const std::shared_ptr<ClassT>& obj_shared, const typename dlgt::obj_func_delegate<ClassT, void(ArgsT...)>::func_type& obj_func) noexcept;
    
    // 清空代理
    inline void Clear() noexcept;
private:
    // 遍历dlgt_map，判断为真才会被移除
    template<class Comp>
    bool remove(const Comp& rm_lambda) noexcept;
    
    uint32_t dlgt_id;
    std::unordered_map<uint32_t, std::shared_ptr<dlgt::idelegate<void(ArgsT...)>>> dlgt_map;
};

}

template<typename ReturnT, typename ...ArgsT>
inline xxd::single_delegate<ReturnT(ArgsT...)> xxd::single_delegate<ReturnT(ArgsT...)>::create_function(const typename dlgt::func_delegate<ReturnT(ArgsT...)>::func_type& func) noexcept
{
    single_delegate<ReturnT(ArgsT...)> dlgt;
    dlgt.bind_function(func);
    return dlgt;
}

template<typename ReturnT, typename ...ArgsT>
template<class ClassT>
inline xxd::single_delegate<ReturnT(ArgsT...)> xxd::single_delegate<ReturnT(ArgsT...)>::create_object(ClassT* obj, const typename dlgt::obj_func_delegate<ClassT, ReturnT(ArgsT...)>::func_type& obj_func) noexcept
{
    single_delegate<ReturnT(ArgsT...)> dlgt;
    dlgt.bind_object(obj, obj_func);
    return dlgt;
}

template<typename ReturnT, typename ...ArgsT>
template<class ClassT>
inline xxd::single_delegate<ReturnT(ArgsT...)> xxd::single_delegate<ReturnT(ArgsT...)>::create_safe_obj(const std::shared_ptr<ClassT>& obj_shared, const typename dlgt::obj_func_delegate<ClassT, ReturnT(ArgsT...)>::func_type& obj_func) noexcept
{
    single_delegate<ReturnT(ArgsT...)> dlgt;
    dlgt.bind_safe_obj(obj_shared, obj_func);
    return dlgt;
}

template<typename ReturnT, typename ...ArgsT>
template<class AnyFuncT>
inline xxd::single_delegate<ReturnT(ArgsT...)> xxd::single_delegate<ReturnT(ArgsT...)>::create_any_func(AnyFuncT&& func) noexcept
{
    single_delegate<ReturnT(ArgsT...)> dlgt;
    dlgt.bind_any_func(std::forward<AnyFuncT>(func));
    return dlgt;
}

template<typename ReturnT, typename ...ArgsT>
inline void xxd::single_delegate<ReturnT(ArgsT...)>::bind_function(const typename dlgt::func_delegate<ReturnT(ArgsT...)>::func_type& func) noexcept
{
    dlgt_ptr = std::make_shared<dlgt::func_delegate<ReturnT(ArgsT...)>>(func);
}

template<typename ReturnT, typename ...ArgsT>
template<class ClassT>
inline void xxd::single_delegate<ReturnT(ArgsT...)>::bind_object(ClassT* obj, const typename dlgt::obj_func_delegate<ClassT, ReturnT(ArgsT...)>::func_type& obj_func) noexcept
{
    dlgt_ptr = std::make_shared<dlgt::obj_func_delegate<ClassT, ReturnT(ArgsT...)>>(obj, obj_func);
}

template<typename ReturnT, typename ...ArgsT>
template<class ClassT>
inline void xxd::single_delegate<ReturnT(ArgsT...)>::bind_safe_obj(const std::shared_ptr<ClassT>& obj_shared, const typename dlgt::obj_func_delegate<ClassT, ReturnT(ArgsT...)>::func_type& obj_func) noexcept
{
    dlgt_ptr = std::make_shared<dlgt::obj_func_safe_delegate<ClassT, ReturnT(ArgsT...)>>(obj_shared, obj_func);
}

template<typename ReturnT, typename ...ArgsT>
template<class AnyFuncT>
inline typename std::enable_if<!std::is_same<typename std::decay<AnyFuncT>::type, xxd::single_delegate<ReturnT(ArgsT...)>>::value>::type xxd::single_delegate<ReturnT(ArgsT...)>::bind_any_func(AnyFuncT&& func) noexcept
{
    dlgt_ptr = std::make_shared<dlgt::any_func_delegate<AnyFuncT, ReturnT(ArgsT...)>>(std::forward<AnyFuncT>(func));
}

template<typename ReturnT, typename ...ArgsT>
inline ReturnT xxd::single_delegate<ReturnT(ArgsT...)>::invoke(ArgsT ...args) noexcept
{
    return (*dlgt_ptr)(args...);
}

template<typename ReturnT, typename ...ArgsT>
inline ReturnT xxd::single_delegate<ReturnT(ArgsT...)>::invoke_if_bind(ArgsT ...args) noexcept
{
    return dlgt_ptr.get() ? (*dlgt_ptr)(args...) : ReturnT();
}

template<typename ReturnT, typename ...ArgsT>
inline ReturnT xxd::single_delegate<ReturnT(ArgsT...)>::operator()(ArgsT ...args) noexcept
{
    return invoke(args...);
}

template<typename ReturnT, typename ...ArgsT>
inline void xxd::single_delegate<ReturnT(ArgsT...)>::unbind() noexcept
{
    dlgt_ptr.reset();
}

template<typename ...ArgsT>
inline xxd::delegate_handle xxd::multi_delegate<ArgsT...>::add_function(const typename dlgt::func_delegate<void(ArgsT...)>::func_type& func) noexcept
{
    delegate_handle handle(0, dlgt_id++, this, reinterpret_cast<void*>(func));
    dlgt_map[handle.to_key()] = std::make_shared<dlgt::func_delegate<void(ArgsT...)>>(func);
    
    return handle;
}

template<typename ...ArgsT>
template<class ClassT>
inline xxd::delegate_handle xxd::multi_delegate<ArgsT...>::add_object(ClassT* obj, const typename dlgt::obj_func_delegate<ClassT, void(ArgsT...)>::func_type& obj_func) noexcept
{
    delegate_handle handle(0x1, dlgt_id++, this, reinterpret_cast<void*>(obj));
    dlgt_map[handle.to_key()] = std::make_shared<dlgt::obj_func_delegate<ClassT, void(ArgsT...)>>(obj, obj_func);
    
    return handle;
}

template<typename ...ArgsT>
template<class ClassT>
inline xxd::delegate_handle xxd::multi_delegate<ArgsT...>::add_safe_obj(const std::shared_ptr<ClassT>& obj_shared, const typename dlgt::obj_func_delegate<ClassT, void(ArgsT...)>::func_type& obj_func) noexcept
{
    delegate_handle handle(0x2, dlgt_id++, this, reinterpret_cast<void*>(obj_shared.get()));
    dlgt_map[handle.to_key()] = std::make_shared<dlgt::obj_func_safe_delegate<ClassT, void(ArgsT...)>>(obj_shared, obj_func);
    
    return handle;
}

template<typename ...ArgsT>
template<class AnyFuncT>
inline typename std::enable_if<!std::is_same<typename std::decay<AnyFuncT>::type, xxd::multi_delegate<void(ArgsT...)>>::value, xxd::delegate_handle>::type xxd::multi_delegate<ArgsT...>::add_any_func(AnyFuncT&& func) noexcept
{
    delegate_handle handle(0x4, dlgt_id++, this, 0);
    dlgt_map[handle.to_key()] = std::make_shared<dlgt::any_func_delegate<AnyFuncT, void(ArgsT...)>>(std::forward<AnyFuncT>(func));
    
    return handle;
}

template<typename ...ArgsT>
inline void xxd::multi_delegate<ArgsT...>::broad_cast(ArgsT ...args) noexcept
{
    for (const auto& it : dlgt_map)
    {
        (*(it.second))(args...);
    }
}

template<typename ...ArgsT>
inline void xxd::multi_delegate<ArgsT...>::operator()(ArgsT ...args) noexcept
{
    broad_cast(args...);
}

template<typename ...ArgsT>
inline bool xxd::multi_delegate<ArgsT...>::remove(const xxd::delegate_handle& handle) noexcept
{
    uint32_t key = handle.to_key();
    return dlgt_map.erase(key);
}

template<typename ...ArgsT>
inline bool xxd::multi_delegate<ArgsT...>::remove(const typename dlgt::func_delegate<void(ArgsT...)>::func_type& func) noexcept
{
    auto rm_lambda = [&](const typename std::unordered_map<uint32_t, std::shared_ptr<dlgt::idelegate<void(ArgsT...)>>>::iterator& it)->bool
    {
        dlgt::idelegate<void(ArgsT...)>* dlgt_ptr = (*it).second.get();
        auto flag = dynamic_cast<dlgt::func_delegate<void(ArgsT...)>*>(dlgt_ptr);
        return flag && flag->func == func;
    };
    return remove(rm_lambda);
}

template<typename ...ArgsT>
template<class ClassT>
inline bool xxd::multi_delegate<ArgsT...>::remove(ClassT* obj, const typename dlgt::obj_func_delegate<ClassT, void(ArgsT...)>::func_type& obj_func) noexcept
{
    auto rm_lambda = [&](const typename std::unordered_map<uint32_t, std::shared_ptr<dlgt::idelegate<void(ArgsT...)>>>::iterator& it)->bool
    {
        dlgt::idelegate<void(ArgsT...)>* dlgt_ptr = (*it).second.get();
        auto flag = dynamic_cast<dlgt::obj_func_delegate<ClassT, void(ArgsT...)>*>(dlgt_ptr);
        return flag && flag->func == obj_func && flag->obj == obj;
    };
    return remove(rm_lambda);
}

template<typename ...ArgsT>
template<class ClassT>
inline bool xxd::multi_delegate<ArgsT...>::remove(const std::shared_ptr<ClassT>& obj_shared, const typename dlgt::obj_func_delegate<ClassT, void(ArgsT...)>::func_type& obj_func) noexcept
{
    auto rm_lambda = [&](const typename std::unordered_map<uint32_t, std::shared_ptr<dlgt::idelegate<void(ArgsT...)>>>::iterator& it)->bool
    {
        dlgt::idelegate<void(ArgsT...)>* dlgt_ptr = (*it).second.get();
        auto flag = dynamic_cast<dlgt::obj_func_safe_delegate<ClassT, void(ArgsT...)>*>(dlgt_ptr);
        return flag && flag->func == obj_func && flag->obj.lock() == obj_shared;
    };
    return remove(rm_lambda);
}

template<typename ...ArgsT>
inline void xxd::multi_delegate<ArgsT...>::Clear() noexcept
{
    //引用计数为0时自动释放对象
    dlgt_map.clear();
}

template<typename ...ArgsT>
template<class Comp>
bool xxd::multi_delegate<ArgsT...>::remove(const Comp& rm_lambda) noexcept
{
    std::vector<typename std::unordered_map<uint32_t, std::shared_ptr<dlgt::idelegate<void(ArgsT...)>>>::iterator> delete_items;
    
    for (auto it = dlgt_map.begin(); it != dlgt_map.end(); it++)
    {
        if (rm_lambda(it))
        {
            delete_items.push_back(it);
        }
    }
    
    bool rm_success = delete_items.size();
    if(rm_success)
    {
        for(const auto& dlgt_it : delete_items)
            dlgt_map.erase(dlgt_it);
    }
    return rm_success;
}

#endif