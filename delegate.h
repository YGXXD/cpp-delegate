#ifndef DELEGATE_H
#define DELEGATE_H

#include <memory>
#include <vector>

#ifndef DECLARE_FUNCTION_DELEGATE
#define DECLARE_FUNCTION_DELEGATE(DelegateName, ReturnValueType, ...) typedef std::FunDelegate<ReturnValueType, __VA_ARGS__> (DelegateName);
#define DECLARE_FUNCTION_DELEGATE_NO_PARAMETER(DelegateName, ReturnValueType) typedef std::FunDelegate<ReturnValueType> (DelegateName);
#endif

#ifndef DECLARE_FUNCTION_MULTICAST_DELEGATE
#define DECLARE_FUNCTION_MULTICAST_DELEGATE(DelegateName, ...) typedef std::MultiDelegate<__VA_ARGS__> (DelegateName);    
#define DECLARE_FUNCTION_MULTICAST_DELEGATE_NO_PARAMETER(DelegateName) typedef std::FunDelegate<void> (DelegateName);
#endif

namespace std
{
	class DelegateInterface final
	{
	public:
		template<typename ReturnT, typename ...ArgsT>
		friend class FunDelegate;

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
		class DynamicDelegate : public IDelegate<ReturnT, ArgsT...>
		{
		public:
			typedef ReturnT(ClassT::* FunT) (ArgsT...);

			DynamicDelegate() = delete;
			explicit DynamicDelegate(ClassT* objPtr, FunT funPtr) :obj(objPtr), func(funPtr) { };
			~DynamicDelegate() { };

			virtual ReturnT operator() (ArgsT... args) override
			{
				return (obj->*func)(args...);
			}

			ClassT* obj;
			FunT func;
		};

		//非成员函数模板
		template<typename ReturnT, typename ...ArgsT>
		class StaticDelegate : public IDelegate<ReturnT, ArgsT...>
		{
		public:
			typedef ReturnT(*FunT) (ArgsT...);

			StaticDelegate() = delete;
			explicit StaticDelegate(FunT funPtr) :func(funPtr) { };
			~StaticDelegate() { };

			virtual ReturnT operator() (ArgsT... args) override
			{
				return (*func)(args...);
			}

			FunT func;
		};
	};

	template<typename ReturnT, typename ...ArgsT>
	class FunDelegate final
	{
	public:
		explicit FunDelegate();
		explicit FunDelegate(typename DelegateInterface::StaticDelegate<ReturnT, ArgsT...>::FunT funPtr);

		template<typename ClassT>
		explicit FunDelegate(ClassT* obj, typename DelegateInterface::DynamicDelegate<ClassT, ReturnT, ArgsT...>::FunT funPtr);

		 ~FunDelegate();

		void Bind(typename DelegateInterface::StaticDelegate<ReturnT, ArgsT...>::FunT funPtr);

		template<typename ClassT>
		void Bind(ClassT* obj, typename DelegateInterface::DynamicDelegate<ClassT, ReturnT, ArgsT...>::FunT funPtr);

		ReturnT Invoke(ArgsT... args);
		ReturnT operator() (ArgsT... args);
		
		void Clear();
	private:
		std::unique_ptr<DelegateInterface::IDelegate<ReturnT, ArgsT...> > dlgtPtr;
	};

	template<typename ...ArgsT>
	class MultiDelegate final
	{
	public:
		MultiDelegate();
		~MultiDelegate();

		void AddFunc(typename DelegateInterface::StaticDelegate<void, ArgsT...>::FunT funPtr);

		template<typename ClassT>
		void AddFunc(ClassT* obj, typename DelegateInterface::DynamicDelegate<ClassT, void, ArgsT...>::FunT funPtr);

		void BroadCast(ArgsT... args);
		void operator() (ArgsT... args);
		
		bool RemoveFunc(typename DelegateInterface::StaticDelegate<void, ArgsT...>::FunT funPtr);

		template<typename ClassT>
		bool RemoveFunc(ClassT* obj, typename DelegateInterface::DynamicDelegate<ClassT, void, ArgsT...>::FunT funPtr);

		void Clear();
	private:
		std::vector<std::shared_ptr<DelegateInterface::IDelegate<void, ArgsT...> > > dlgtPtrArray;
	};
}

template<typename ReturnT, typename ...ArgsT>
inline std::FunDelegate<ReturnT, ArgsT...>::FunDelegate()
{

}

template<typename ReturnT, typename ...ArgsT>
inline std::FunDelegate<ReturnT, ArgsT...>::FunDelegate(typename DelegateInterface::StaticDelegate<ReturnT, ArgsT...>::FunT funPtr)
{
	Bind(funPtr);
}

template<typename ReturnT, typename ...ArgsT>
template<typename ClassT>
inline std::FunDelegate<ReturnT, ArgsT...>::FunDelegate(ClassT* obj, typename DelegateInterface::DynamicDelegate<ClassT, ReturnT, ArgsT...>::FunT funPtr)
{
	Bind(obj, funPtr);
}

template<typename ReturnT, typename ...ArgsT>
inline std::FunDelegate<ReturnT, ArgsT...>::~FunDelegate()
{
	Clear();
}

template<typename ReturnT, typename ...ArgsT>
inline void std::FunDelegate<ReturnT, ArgsT...>::Bind(typename DelegateInterface::StaticDelegate<ReturnT, ArgsT...>::FunT funPtr)
{
	Clear();
	dlgtPtr = std::make_unique<DelegateInterface::StaticDelegate<ReturnT, ArgsT...> >(funPtr);
}

template<typename ReturnT, typename ...ArgsT>
template<typename ClassT>
inline void std::FunDelegate<ReturnT, ArgsT...>::Bind(ClassT* obj, typename DelegateInterface::DynamicDelegate<ClassT, ReturnT, ArgsT...>::FunT funPtr)
{
	Clear();
	dlgtPtr = std::make_unique<DelegateInterface::DynamicDelegate<ClassT, ReturnT, ArgsT...> >(obj, funPtr);
}

template<typename ReturnT, typename ...ArgsT>
inline ReturnT std::FunDelegate<ReturnT, ArgsT...>::Invoke(ArgsT ...args)
{
	if(dlgtPtr.get())
		return (*dlgtPtr)(args...);
	
	return ReturnT();
}

template<typename ReturnT, typename ...ArgsT>
inline ReturnT std::FunDelegate<ReturnT, ArgsT...>::operator()(ArgsT ...args)
{
	if (dlgtPtr.get())
		return (*dlgtPtr)(args...);

	return ReturnT();
}

template<typename ReturnT, typename ...ArgsT>
inline void std::FunDelegate<ReturnT, ArgsT...>::Clear()
{
	dlgtPtr.reset();
}

template<typename ...ArgsT>
inline std::MultiDelegate<ArgsT...>::MultiDelegate()
{

}

template<typename ...ArgsT>
inline std::MultiDelegate<ArgsT...>::~MultiDelegate()
{
	Clear();
}

template<typename ...ArgsT>
inline void std::MultiDelegate<ArgsT...>::AddFunc(typename DelegateInterface::StaticDelegate<void, ArgsT...>::FunT funPtr)
{
	dlgtPtrArray.push_back(std::make_shared<DelegateInterface::StaticDelegate<void, ArgsT...> >(funPtr));
}

template<typename ...ArgsT>
template<typename ClassT>
inline void std::MultiDelegate<ArgsT...>::AddFunc(ClassT* obj, typename DelegateInterface::DynamicDelegate<ClassT, void, ArgsT...>::FunT funPtr)
{
	dlgtPtrArray.push_back(std::make_shared<DelegateInterface::DynamicDelegate<ClassT, void, ArgsT...> >(obj, funPtr));
}

template<typename ...ArgsT>
inline void std::MultiDelegate<ArgsT...>::BroadCast(ArgsT... args)
{
	for (auto it = dlgtPtrArray.begin(); it!= dlgtPtrArray.end(); it++)
	{
		(**it)(args...);
	}
}

template<typename ...ArgsT>
inline void std::MultiDelegate<ArgsT...>::operator()(ArgsT ...args)
{
	for (auto it = dlgtPtrArray.begin(); it != dlgtPtrArray.end(); it++)
	{
		(**it)(args...);
	}
}

template<typename ...ArgsT>
inline bool std::MultiDelegate<ArgsT...>::RemoveFunc(typename DelegateInterface::StaticDelegate<void, ArgsT...>::FunT funPtr)
{
	for (auto it = dlgtPtrArray.begin(); it != dlgtPtrArray.end(); it++)
	{
		DelegateInterface::IDelegate<void, ArgsT...>* dlgtPtr = (*it).get();
		auto flag = dynamic_cast<DelegateInterface::StaticDelegate<void, ArgsT...>*>(dlgtPtr);
		if (flag && flag->func == funPtr)
		{
			dlgtPtrArray.erase(it);
			return true;
		}
	}
	return false;
}

template<typename ...ArgsT>
template<typename ClassT>
inline bool std::MultiDelegate<ArgsT...>::RemoveFunc(ClassT* obj, typename DelegateInterface::DynamicDelegate<ClassT, void, ArgsT...>::FunT funPtr)
{
	for (auto it = dlgtPtrArray.begin(); it != dlgtPtrArray.end(); it++)
	{
		DelegateInterface::IDelegate<void, ArgsT...>* dlgtPtr = (*it).get();
		auto flag = dynamic_cast<DelegateInterface::DynamicDelegate<ClassT, void, ArgsT...>*>(dlgtPtr);
		if (flag && flag->func == funPtr && flag->obj == obj)
		{
			dlgtPtrArray.erase(it);
			return true;
		}
	}
	return false;
}

template<typename ...ArgsT>
inline void std::MultiDelegate<ArgsT...>::Clear()
{
	//引用计数为0时自动释放对象
	dlgtPtrArray.clear();
}

#endif
