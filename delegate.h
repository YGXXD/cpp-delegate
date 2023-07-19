#ifndef DELEGATE_H
#define DELEGATE_H

#include <memory>
#include <sstream>

#ifndef DECLARE_FUNCTION_DELEGATE
#define DECLARE_FUNCTION_DELEGATE(DelegateName, ReturnValueType, ...) typedef xxd::SingleDelegate<ReturnValueType, __VA_ARGS__> (DelegateName);
#define DECLARE_FUNCTION_DELEGATE_NO_PARAMETER(DelegateName, ReturnValueType) typedef xxd::SingleDelegate<ReturnValueType> (DelegateName);
#endif

#ifndef DECLARE_FUNCTION_MULTICAST_DELEGATE
#define DECLARE_FUNCTION_MULTICAST_DELEGATE(DelegateName, ...) typedef xxd::MultiDelegate<__VA_ARGS__> (DelegateName);
#define DECLARE_FUNCTION_MULTICAST_DELEGATE_NO_PARAMETER(DelegateName) typedef xxd::MultiDelegate<> (DelegateName);
#endif

namespace xxd
{

typedef struct{
    uint8_t tdlgt; // 代理类型
    uint32_t idlgt; // 代理id
    void* pdlgt; // 代理类
    void* bind; // 绑定地址
}DelegateHandle;

class DelegateInterface final
{
public:
    template<typename ReturnT, typename ...ArgsT>
    friend class SingleDelegate;

    template<typename ...ArgsT>
    friend class MultiDelegate;

private:
    template<typename ReturnT, typename ...ArgsT>
    struct IDelegate
    {
        virtual ReturnT operator() (ArgsT... args) = 0;
        virtual ~IDelegate() { };
    };

    //类成员函数模板
    template<typename ClassT, typename ReturnT, typename ...ArgsT>
    class ObjFuncDelegate : public IDelegate<ReturnT, ArgsT...>
    {
    public:
        typedef ReturnT(ClassT::* FunT) (ArgsT...);

        ObjFuncDelegate() = delete;
        explicit ObjFuncDelegate(ClassT* objPtr, const FunT& funPtr) :obj(objPtr), func(funPtr) { };

        virtual ReturnT operator() (ArgsT... args) override
        {
            return (obj->*func)(args...);
        }

        ClassT* obj;
        FunT func;
    };

    //非成员函数模板
    template<typename ReturnT, typename ...ArgsT>
    class FuncDelegate : public IDelegate<ReturnT, ArgsT...>
    {
    public:
        typedef ReturnT(*FunT) (ArgsT...);

        FuncDelegate() = delete;
        explicit FuncDelegate(FunT funPtr) :func(funPtr) { };

        virtual ReturnT operator() (ArgsT... args) override
        {
            return (*func)(args...);
        }

        FunT func;
    };
    
    //带有安全检测的类成员函数模板
    template<typename ClassT, typename ReturnT, typename ...ArgsT>
    class ObjFuncSafeDelegate : public DelegateInterface::IDelegate<ReturnT, ArgsT...>
    {
    public:
        typedef ReturnT(ClassT::* FunT) (ArgsT...);

        ObjFuncSafeDelegate() = delete;
        explicit ObjFuncSafeDelegate(const std::shared_ptr<ClassT>& objShared, const FunT& funPtr) :obj(objShared), func(funPtr) { };
        explicit ObjFuncSafeDelegate(const std::weak_ptr<ClassT>& objWeak, const FunT& funPtr) :obj(objWeak), func(funPtr) { };

        virtual ReturnT operator() (ArgsT... args) override
        {
            if(!obj.expired())
                return (obj.lock().get()->*func)(args...);
            return ReturnT();
        }
        
        std::weak_ptr<ClassT> obj;
        FunT func;
    };
    
    inline static std::string HandleToString(const DelegateHandle& handle)
    {
        std::stringstream ss;
        ss << handle.tdlgt << handle.idlgt << handle.pdlgt << handle.bind;
        return ss.str();
    };
};

template<typename ReturnT, typename ...ArgsT>
class SingleDelegate final
{
public:
    explicit SingleDelegate() = default;
    explicit SingleDelegate(typename DelegateInterface::FuncDelegate<ReturnT, ArgsT...>::FunT funPtr);
    
    template<typename ClassT>
    explicit SingleDelegate(ClassT* obj, const typename DelegateInterface::ObjFuncDelegate<ClassT, ReturnT, ArgsT...>::FunT& funPtr);

    template<typename ClassT>
    explicit SingleDelegate(std::shared_ptr<ClassT> objShared, const typename DelegateInterface::ObjFuncDelegate<ClassT, ReturnT, ArgsT...>::FunT& funPtr);
    
    template<typename ClassT>
    explicit SingleDelegate(std::weak_ptr<ClassT> objWeak, const typename DelegateInterface::ObjFuncDelegate<ClassT, ReturnT, ArgsT...>::FunT& funPtr);
    
    // 绑定全局或静态函数
    void BindFunction(typename DelegateInterface::FuncDelegate<ReturnT, ArgsT...>::FunT funPtr);

    // 绑定类成员函数
    template<typename ClassT>
    void BindObject(ClassT* obj, const typename DelegateInterface::ObjFuncDelegate<ClassT, ReturnT, ArgsT...>::FunT& funPtr);

    // 绑定安全的类成员函数
    template<typename ClassT>
    void BindSafeObj(const std::shared_ptr<ClassT>& objShared, const typename DelegateInterface::ObjFuncDelegate<ClassT, ReturnT, ArgsT...>::FunT& funPtr);
    
    template<typename ClassT>
    void BindSafeObj(const std::weak_ptr<ClassT>& objWeak, const typename DelegateInterface::ObjFuncDelegate<ClassT, ReturnT, ArgsT...>::FunT& funPtr);

    // 代理执行
    ReturnT Invoke(ArgsT... args);
    ReturnT operator() (ArgsT... args);
    
    // 解绑函数
    void UnBind();
private:
    std::unique_ptr<DelegateInterface::IDelegate<ReturnT, ArgsT...> > dlgtPtr;
};

template<typename ...ArgsT>
class MultiDelegate final
{
public:
    explicit MultiDelegate() = default;

    // 添加全局或静态函数
    DelegateHandle AddFunction(typename DelegateInterface::FuncDelegate<void, ArgsT...>::FunT funPtr);

    // 添加类成员函数
    template<typename ClassT>
    DelegateHandle AddObject(ClassT* obj, const typename DelegateInterface::ObjFuncDelegate<ClassT, void, ArgsT...>::FunT& funPtr);
    
    // 添加安全的类成员函数
    template<typename ClassT>
    DelegateHandle AddSafeObj(std::shared_ptr<ClassT> objShared, const typename DelegateInterface::ObjFuncDelegate<ClassT, void, ArgsT...>::FunT& funPtr);
    
    template<typename ClassT>
    DelegateHandle AddSafeObj(std::weak_ptr<ClassT> objWeak, const typename DelegateInterface::ObjFuncDelegate<ClassT, void, ArgsT...>::FunT& funPtr);

    // 多播代理执行
    void BroadCast(ArgsT... args);
    void operator() (ArgsT... args);
    
    // 移除函数
    bool Remove(const DelegateHandle& handle);
    bool Remove(typename DelegateInterface::FuncDelegate<void, ArgsT...>::FunT funPtr);

    template<typename ClassT>
    bool Remove(ClassT* obj, const typename DelegateInterface::ObjFuncDelegate<ClassT, void, ArgsT...>::FunT& funPtr);
    
    template<typename ClassT>
    bool Remove(const std::shared_ptr<ClassT>& objShared, const typename DelegateInterface::ObjFuncDelegate<ClassT, void, ArgsT...>::FunT& funPtr);
    
    template<typename ClassT>
    bool Remove(const std::weak_ptr<ClassT>& objWeak, const typename DelegateInterface::ObjFuncDelegate<ClassT, void, ArgsT...>::FunT& funPtr);
    
    // 清空代理
    void Clear();
private:
    
    uint32_t dlgtId;
    std::unordered_map<std::string, std::shared_ptr<DelegateInterface::IDelegate<void, ArgsT...> > > dlgtMap;
};

}

template<typename ReturnT, typename ...ArgsT>
inline xxd::SingleDelegate<ReturnT, ArgsT...>::SingleDelegate(typename DelegateInterface::FuncDelegate<ReturnT, ArgsT...>::FunT funPtr)
{
    BindFunction(funPtr);
}

template<typename ReturnT, typename ...ArgsT>
template<typename ClassT>
inline xxd::SingleDelegate<ReturnT, ArgsT...>::SingleDelegate(ClassT* obj, const typename DelegateInterface::ObjFuncDelegate<ClassT, ReturnT, ArgsT...>::FunT& funPtr)
{
    BindObject(obj, funPtr);
}

template<typename ReturnT, typename ...ArgsT>
template<typename ClassT>
inline xxd::SingleDelegate<ReturnT, ArgsT...>::SingleDelegate(std::shared_ptr<ClassT> objShared, const typename DelegateInterface::ObjFuncDelegate<ClassT, ReturnT, ArgsT...>::FunT& funPtr)
{
    BindSafeObj(objShared, funPtr);
}

template<typename ReturnT, typename ...ArgsT>
template<typename ClassT>
inline xxd::SingleDelegate<ReturnT, ArgsT...>::SingleDelegate(std::weak_ptr<ClassT> objWeak, const typename DelegateInterface::ObjFuncDelegate<ClassT, ReturnT, ArgsT...>::FunT& funPtr)
{
    BindSafeObj(objWeak, funPtr);
}

template<typename ReturnT, typename ...ArgsT>
inline void xxd::SingleDelegate<ReturnT, ArgsT...>::BindFunction(typename DelegateInterface::FuncDelegate<ReturnT, ArgsT...>::FunT funPtr)
{
    UnBind()();
    dlgtPtr = std::make_unique<DelegateInterface::FuncDelegate<ReturnT, ArgsT...> >(funPtr);
}

template<typename ReturnT, typename ...ArgsT>
template<typename ClassT>
inline void xxd::SingleDelegate<ReturnT, ArgsT...>::BindObject(ClassT* obj, const typename DelegateInterface::ObjFuncDelegate<ClassT, ReturnT, ArgsT...>::FunT& funPtr)
{
    UnBind();
    dlgtPtr = std::make_unique<DelegateInterface::ObjFuncDelegate<ClassT, ReturnT, ArgsT...> >(obj, funPtr);
}

template<typename ReturnT, typename ...ArgsT>
template<typename ClassT>
inline void xxd::SingleDelegate<ReturnT, ArgsT...>::BindSafeObj(const std::shared_ptr<ClassT>& objShared, const typename DelegateInterface::ObjFuncDelegate<ClassT, ReturnT, ArgsT...>::FunT& funPtr)
{
    UnBind();
    dlgtPtr = std::make_unique<DelegateInterface::ObjFuncSafeDelegate<ClassT, ReturnT, ArgsT...> >(objShared, funPtr);
}

template<typename ReturnT, typename ...ArgsT>
template<typename ClassT>
inline void xxd::SingleDelegate<ReturnT, ArgsT...>::BindSafeObj(const std::weak_ptr<ClassT>& objWeak, const typename DelegateInterface::ObjFuncDelegate<ClassT, ReturnT, ArgsT...>::FunT& funPtr)
{
    UnBind();
    dlgtPtr = std::make_unique<DelegateInterface::ObjFuncSafeDelegate<ClassT, ReturnT, ArgsT...> >(objWeak, funPtr);
}

template<typename ReturnT, typename ...ArgsT>
inline ReturnT xxd::SingleDelegate<ReturnT, ArgsT...>::Invoke(ArgsT ...args)
{
    if(dlgtPtr.get())
        return (*dlgtPtr)(args...);
    
    return ReturnT();
}

template<typename ReturnT, typename ...ArgsT>
inline ReturnT xxd::SingleDelegate<ReturnT, ArgsT...>::operator()(ArgsT ...args)
{
    return Invoke(args...);
}

template<typename ReturnT, typename ...ArgsT>
inline void xxd::SingleDelegate<ReturnT, ArgsT...>::UnBind()
{
    dlgtPtr.reset();
}

template<typename ...ArgsT>
inline xxd::DelegateHandle xxd::MultiDelegate<ArgsT...>::AddFunction(typename DelegateInterface::FuncDelegate<void, ArgsT...>::FunT funPtr)
{
    DelegateHandle handle = { 0, dlgtId++, this, (void*)funPtr };
    dlgtMap[DelegateInterface::HandleToString(handle)] = std::make_shared<DelegateInterface::FuncDelegate<void, ArgsT...> >(funPtr);
    
    return handle;
}

template<typename ...ArgsT>
template<typename ClassT>
inline xxd::DelegateHandle xxd::MultiDelegate<ArgsT...>::AddObject(ClassT* obj, const typename DelegateInterface::ObjFuncDelegate<ClassT, void, ArgsT...>::FunT& funPtr)
{
    DelegateHandle handle = { 1, dlgtId++, this, (void*)obj };
    dlgtMap[DelegateInterface::HandleToString(handle)] = std::make_shared<DelegateInterface::ObjFuncDelegate<ClassT, void, ArgsT...> >(obj, funPtr);
    
    return handle;
}

template<typename ...ArgsT>
template<typename ClassT>
inline xxd::DelegateHandle xxd::MultiDelegate<ArgsT...>::AddSafeObj(std::shared_ptr<ClassT> objShared, const typename DelegateInterface::ObjFuncDelegate<ClassT, void, ArgsT...>::FunT& funPtr)
{
    DelegateHandle handle = { 2, dlgtId++, this, (void*)objShared.get() };
    dlgtMap[DelegateInterface::HandleToString(handle)] = std::make_shared<DelegateInterface::ObjFuncSafeDelegate<ClassT, void, ArgsT...> >(objShared, funPtr);
    
    return handle;
}

template<typename ...ArgsT>
template<typename ClassT>
inline xxd::DelegateHandle xxd::MultiDelegate<ArgsT...>::AddSafeObj(std::weak_ptr<ClassT> objWeak, const typename DelegateInterface::ObjFuncDelegate<ClassT, void, ArgsT...>::FunT& funPtr)
{
    DelegateHandle handle = { 2, dlgtId++, this, (void*)objWeak.lock().get() };
    dlgtMap[DelegateInterface::HandleToString(handle)] = std::make_shared<DelegateInterface::ObjFuncSafeDelegate<ClassT, void, ArgsT...> >(objWeak, funPtr);
    
    return handle;
}
template<typename ...ArgsT>
inline void xxd::MultiDelegate<ArgsT...>::BroadCast(ArgsT ...args)
{
    for (const auto& it : dlgtMap)
    {
        (*(it.second))(args...);
    }
}

template<typename ...ArgsT>
inline void xxd::MultiDelegate<ArgsT...>::operator()(ArgsT ...args)
{
    BroadCast(args...);
}

template<typename ...ArgsT>
inline bool xxd::MultiDelegate<ArgsT...>::Remove(const xxd::DelegateHandle& handle)
{
    std::string key = DelegateInterface::HandleToString(handle);
    if(dlgtMap.count(key))
    {
        dlgtMap.erase(key);
        return true;
    }
    return false;
}

template<typename ...ArgsT>
inline bool xxd::MultiDelegate<ArgsT...>::Remove(typename DelegateInterface::FuncDelegate<void, ArgsT...>::FunT funPtr)
{
    for (auto it = dlgtMap.begin(); it != dlgtMap.end(); it++)
    {
        DelegateInterface::IDelegate<void, ArgsT...>* dlgtPtr = (*it).second.get();
        auto flag = dynamic_cast<DelegateInterface::FuncDelegate<void, ArgsT...>*>(dlgtPtr);
        if (flag && flag->func == funPtr)
        {
            dlgtMap.erase(it);
            return true;
        }
    }
    return false;
}

template<typename ...ArgsT>
template<typename ClassT>
inline bool xxd::MultiDelegate<ArgsT...>::Remove(ClassT* obj, const typename DelegateInterface::ObjFuncDelegate<ClassT, void, ArgsT...>::FunT& funPtr)
{
    for (auto it = dlgtMap.begin(); it != dlgtMap.end(); it++)
    {
        DelegateInterface::IDelegate<void, ArgsT...>* dlgtPtr = (*it).second.get();
        auto flag = dynamic_cast<DelegateInterface::ObjFuncDelegate<ClassT, void, ArgsT...>*>(dlgtPtr);
        if (flag && flag->func == funPtr && flag->obj == obj)
        {
            dlgtMap.erase(it);
            return true;
        }
    }
    return false;
}

template<typename ...ArgsT>
template<typename ClassT>
inline bool xxd::MultiDelegate<ArgsT...>::Remove(const std::shared_ptr<ClassT>& objShared, const typename DelegateInterface::ObjFuncDelegate<ClassT, void, ArgsT...>::FunT& funPtr)
{
    for (auto it = dlgtMap.begin(); it != dlgtMap.end(); it++)
    {
        DelegateInterface::IDelegate<void, ArgsT...>* dlgtPtr = (*it).second.get();
        auto flag = dynamic_cast<DelegateInterface::ObjFuncSafeDelegate<ClassT, void, ArgsT...>*>(dlgtPtr);
        if (flag && flag->func == funPtr && flag->obj.lock() == objShared)
        {
            dlgtMap.erase(it);
            return true;
        }
    }
    return false;
}

template<typename ...ArgsT>
template<typename ClassT>
inline bool xxd::MultiDelegate<ArgsT...>::Remove(const std::weak_ptr<ClassT>& objWeak, const typename DelegateInterface::ObjFuncDelegate<ClassT, void, ArgsT...>::FunT& funPtr)
{
    auto objShared = objWeak.lock();
    if(objShared.get())
    {
        return Remove(objShared, funPtr);
    }
    return false;
}

template<typename ...ArgsT>
inline void xxd::MultiDelegate<ArgsT...>::Clear()
{
    //引用计数为0时自动释放对象
    dlgtMap.clear();
}

#endif
