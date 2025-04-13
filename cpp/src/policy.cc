#include "policy.h"

using namespace std;

struct CompareTask {
  bool operator()(const Event::Task& a, const Event::Task& b) {
    if (a.priority != b.priority) {
      return a.priority == Event::Task::Priority::kLow;
    }
    return a.deadline > b.deadline;
  }
};

priority_queue<Event::Task, vector<Event::Task>, CompareTask> cpuTodo;
priority_queue<Event::Task, vector<Event::Task>, CompareTask> ioTodo;

int nowTime = -1;

void processTaskArrival(const Event& event) {
  cpuTodo.push(event.task);
}

void processTaskCompletion(int taskId) {
  removeTaskFromQueue(cpuTodo, taskId);
  removeTaskFromQueue(ioTodo, taskId);
}

void processIoRequest(const Event& event) {
  ioTodo.push(event.task);
  removeTaskFromQueue(cpuTodo, event.task.taskId);
}

void processIoEnd(const Event& event) {
  cpuTodo.push(event.task);
  removeTaskFromQueue(ioTodo, event.task.taskId);
}

void removeTaskFromQueue(priority_queue<Event::Task, vector<Event::Task>, CompareTask>& queue, int taskId) {
  vector<Event::Task> temp;
  while (!queue.empty()) {
    if (queue.top().taskId != taskId) {
      temp.push_back(queue.top());
    }
    queue.pop();
  }
  for (const auto& task : temp) {
    queue.push(task);
  }
}

Action policy(const std::vector<Event>& events, int currentCpu, int currentIo) {
  Action result;
  for (const auto& event : events) {
    switch (event.type) {
      case Event::Type::kTimer:
        nowTime = event.time;
        break;
      case Event::Type::kTaskArrival:
        processTaskArrival(event);
        break;
      case Event::Type::kTaskFinish:
        processTaskCompletion(event.task.taskId);
        break;
      case Event::Type::kIoRequest:
        processIoRequest(event);
        break;
      case Event::Type::kIoEnd:
        processIoEnd(event);
        break;
    }
  }

  if (!cpuTodo.empty() && cpuTodo.top().deadline <= nowTime) {
    result.cpuTask = cpuTodo.top().taskId;
    cpuTodo.pop();
  } else if (!cpuTodo.empty()) {
    result.cpuTask = cpuTodo.top().taskId;
    cpuTodo.pop();
  }

  if (!ioTodo.empty() && ioTodo.top().deadline <= nowTime) {
    result.ioTask = ioTodo.top().taskId;
    ioTodo.pop();
  } else if (!ioTodo.empty()) {
    result.ioTask = ioTodo.top().taskId;
    ioTodo.pop();
  }

  return result;
}
