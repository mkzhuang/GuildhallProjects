#pragma once
#include <atomic>
#include <thread>
#include <deque>
#include <mutex>
#include <vector>

enum class JobState
{
	INVALID = - 1,

	QUEUEING,
	EXECUTING,
	COMPLETED,
	RETRIVED
};

class Job
{
	friend class JobWorkerThread;
	
public:
	Job(uint8_t jobType, int jobIndex = -1);
	virtual ~Job() {}

private:
	virtual void Execute() = 0;
	virtual void OnFinished() = 0;

public:
	void SetJobIndex(int jobIndex);
	int GetJobIndex() const;
	void SetJobState(JobState jobState);
	JobState GetJobState() const;

private:
	uint8_t m_jobType = 0;
	std::atomic<int> m_jobIndex = 0;
	std::atomic<JobState> m_state = JobState::INVALID;
};

class JobSystem;

class JobWorkerThread
{
public:
	JobWorkerThread(JobSystem* jobSystem, int workerThreadID);
	~JobWorkerThread();

	void JobWorkerMain();
	void SetAllowedJobTypes(uint8_t jobMask);
	void AddAllowedJobTypes(uint8_t jobMask);
	void Quit();

public:
	JobSystem* m_jobSystem = nullptr;
	std::atomic<bool> m_isQuitting = false;
	int m_workerThreadID = -1;
	std::thread* m_thread = nullptr;
	std::atomic<uint8_t> m_jobMask = 0b00000000;
};


struct JobSystemConfig
{
	int m_numberWorkerThreads = 0;
};


class JobSystem
{
public:
	JobSystem(JobSystemConfig const& config);
	~JobSystem();
	void Startup();
	void BeginFrame();
	void EndFrame();
	void ShutDown();

	void SetJobTypeForWorker(int workerThreadID, uint8_t jobType);
	void QueueJob(Job* jobToExecute);
	Job* SendJobToExecute();
	void MoveJobToCompletedList(Job* completedJob);
	Job* RetrieveCompletedJob();
	void ClearAllJobs();

private:
	JobSystemConfig m_config;

	std::vector<JobWorkerThread*> m_workerThreads;

	std::deque<Job*> m_queuedJobs;
	std::mutex m_queuedJobsMutex;

	std::vector<Job*> m_executingJobs;
	std::mutex m_executingJobsMutex;

	std::deque<Job*> m_completedJobs;
	std::mutex m_completedJobsMutex;

	std::atomic<int> m_numberWorkingThread = 0;
};


