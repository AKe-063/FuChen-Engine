#include "stdafx.h"
#include "FTaskManager.h"

std::unique_ptr<FTaskManager> FTaskManager::fTaskManager(new FTaskManager());

FTaskManager::FTaskManager()
{

}

FTaskManager::~FTaskManager()
{

}

void FTaskManager::Register(const std::string& name, TaskType task)
{
	tasks.insert({ name, task });
}

void FTaskManager::Unregister(const std::string& name)
{
	tasks.erase(name);
}

void FTaskManager::Notify(const std::string& name, const float& value)
{
	if (tasks.find(name) == tasks.end())
	{
		auto& task = tasks.find(name)->second;
		task(value);
	}
}

std::unique_ptr<FTaskManager>& FTaskManager::GetFTaskManager()
{
	return fTaskManager;
}
