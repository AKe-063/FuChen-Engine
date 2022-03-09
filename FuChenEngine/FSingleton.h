#pragma once
template<typename T>

class FSingleton
{
public:
	static T& GetInstance()
	{
		static T instance;
		return instance;
	}

	FSingleton(T&&) = delete;
	FSingleton(const T&) = delete;
	void operator = (const T&) = delete;

protected:
	FSingleton() = default;
	virtual ~FSingleton() = default;
};