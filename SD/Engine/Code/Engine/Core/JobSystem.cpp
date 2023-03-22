#include "Engine/Core/JobSystem.hpp"

Job::Job(uint8_t jobType, int jobIndex)
	: m_jobType(jobType)
	, m_jobIndex(jobIndex)
{
}


void Job::SetJobIndex(int jobIndex)
{
	m_jobIndex = jobIndex;
}


int Job::GetJobIndex() const
{
	return m_jobIndex;
}


void Job::SetJobState(JobState jobState)
{
	m_state = jobState;
}


JobState Job::GetJobState() const
{
	return m_state;
}


JobWorkerThread::JobWorkerThread(JobSystem* jobSystem, int workerThreadID)
	: m_jobSystem(jobSystem)
	, m_workerThreadID(workerThreadID)
{
	m_thread = new std::thread(&JobWorkerThread::JobWorkerMain, this);
}


JobWorkerThread::~JobWorkerThread()
{
}


void JobWorkerThread::JobWorkerMain()
{
	while (!m_isQuitting)
	{
		Job* jobToExecute = m_jobSystem->SendJobToExecute();
		if (jobToExecute != nullptr && (jobToExecute->m_jobType & m_jobMask) != 0)
		{
			jobToExecute->Execute();
			m_jobSystem->MoveJobToCompletedList(jobToExecute);
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}
}


void JobWorkerThread::SetAllowedJobTypes(uint8_t jobMask)
{
	m_jobMask = jobMask;
}


void JobWorkerThread::AddAllowedJobTypes(uint8_t jobMask)
{
	m_jobMask |= jobMask;
}


void JobWorkerThread::Quit()
{
	m_isQuitting = true;
}


JobSystem::JobSystem(JobSystemConfig const& config)
	: m_config(config)
{
}


JobSystem::~JobSystem()
{

}


void JobSystem::Startup()
{
	m_workerThreads.reserve(m_config.m_numberWorkerThreads);
	for (int workerIndex = 0; workerIndex < m_config.m_numberWorkerThreads; workerIndex++)
	{
		JobWorkerThread* newWorkerThread = new JobWorkerThread(this, workerIndex);
		m_workerThreads.push_back(newWorkerThread);
	}
}


void JobSystem::BeginFrame()
{
	
}


void JobSystem::EndFrame()
{

}


void JobSystem::ShutDown()
{
	for (int workerIndex = 0; workerIndex < m_config.m_numberWorkerThreads; workerIndex++)
	{
		JobWorkerThread* workerThread = m_workerThreads[workerIndex];
		workerThread->Quit();
	}

	ClearAllJobs();
}


void JobSystem::SetJobTypeForWorker(int workerThreadID, uint8_t jobType)
{
	m_workerThreads[workerThreadID]->SetAllowedJobTypes(jobType);
}


void JobSystem::QueueJob(Job* jobToExecute)
{
	m_queuedJobsMutex.lock();
	jobToExecute->SetJobState(JobState::QUEUEING);
	m_queuedJobs.push_back(jobToExecute);
	m_queuedJobsMutex.unlock();
}


Job* JobSystem::SendJobToExecute()
{
	m_queuedJobsMutex.lock();
	Job* newJob = nullptr;
	if (!m_queuedJobs.empty())
	{
		newJob = m_queuedJobs.front();
		newJob->SetJobState(JobState::EXECUTING);
		m_queuedJobs.pop_front();
	}
	m_queuedJobsMutex.unlock();

	if (!newJob)
	{
		return newJob;
	}

	m_numberWorkingThread++;

	m_executingJobsMutex.lock();
	for (int index = 0; index < (int)m_executingJobs.size(); index++)
	{
		Job*& currentJob = m_executingJobs[index];
		if (!currentJob)
		{
			currentJob = newJob;
			newJob->SetJobIndex(index);
			m_executingJobsMutex.unlock();
			return currentJob;
		}
	}
	
	m_executingJobs.push_back(newJob);
	newJob->SetJobIndex((int)m_executingJobs.size() - 1);
	m_executingJobsMutex.unlock();
	return newJob;
}


void JobSystem::MoveJobToCompletedList(Job* completedJob)
{
	m_executingJobsMutex.lock();
	int jobIndex = completedJob->GetJobIndex();
	m_executingJobs[jobIndex] = nullptr;
	completedJob->SetJobState(JobState::COMPLETED);
	m_executingJobsMutex.unlock();

	m_numberWorkingThread--;

	m_completedJobsMutex.lock();
	m_completedJobs.push_back(completedJob);
	m_completedJobsMutex.unlock();
}


Job* JobSystem::RetrieveCompletedJob()
{
	m_completedJobsMutex.lock();
	Job* completedJob = nullptr;
	if (!m_completedJobs.empty())
	{
		completedJob = m_completedJobs.front();
		completedJob->SetJobState(JobState::RETRIVED);
		m_completedJobs.pop_front();
	}
	m_completedJobsMutex.unlock();
	return completedJob;
}


void JobSystem::ClearAllJobs()
{
	m_queuedJobsMutex.lock();
	while (!m_queuedJobs.empty())
	{
		Job* job = m_queuedJobs.front();
		m_queuedJobs.pop_front();
		delete job;
	}
	m_queuedJobsMutex.unlock();

	while (m_numberWorkingThread != 0)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}

	m_completedJobsMutex.lock();
	while (!m_completedJobs.empty())
	{
		Job* job = m_completedJobs.front();
		m_completedJobs.pop_front();
		delete job;
	}
	m_completedJobsMutex.unlock();
}


