#pragma once
#include <functional>
#include <thread>
#include <chrono>

using TaskType = std::function<void(float)>;

class FTaskManager
{
public:
	FTaskManager();
	virtual ~FTaskManager();
	
	std::unordered_map<std::string, TaskType> tasks;

	void Register(const std::string& name, TaskType task);
	void Unregister(const std::string& name);
	void Notify(const std::string& name, const float& value);

	static std::unique_ptr<FTaskManager>& GetFTaskManager();

protected:
	static std::unique_ptr<FTaskManager> fTaskManager;

private:

};