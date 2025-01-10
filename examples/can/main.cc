

#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <condition_variable>
#include <cstring>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

// Task class definition
class Task {
 public:
  Task(std::function<void()> func, std::chrono::system_clock::time_point tp) : func_(func), tp_(tp) {}
  std::function<void()> func_;
  std::chrono::system_clock::time_point tp_;
};

// Task comparison for priority queue
struct TaskCmp {
  bool operator()(const Task& lhs, const Task& rhs) const { return lhs.tp_ > rhs.tp_; }
};

class TaskScheduler {
 public:
  TaskScheduler(const std::string& ifname) : ifname_(ifname), running_(false) { init_can_socket(); }

  ~TaskScheduler() { close(can_socket_); }

  void start() {
    running_ = true;
    worker_thread_ = std::thread(&TaskScheduler::run, this);
  }

  void stop() {
    {
      std::unique_lock<std::mutex> lock(task_mutex_);
      running_ = false;
      cv_.notify_all();
    }
    worker_thread_.join();
  }

  void schedule(std::function<void()> func, std::chrono::milliseconds delay) {
    std::unique_lock<std::mutex> lock(task_mutex_);
    tasks_.emplace(func, std::chrono::system_clock::now() + delay);
    cv_.notify_all();
  }

 private:
  void init_can_socket() {
    can_socket_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (can_socket_ < 0) {
      perror("Socket creation failed");
      exit(1);
    }

    struct ifreq ifr;
    strcpy(ifr.ifr_name, ifname_.c_str());
    ioctl(can_socket_, SIOCGIFINDEX, &ifr);

    struct sockaddr_can addr;
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(can_socket_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
      perror("Socket binding failed");
      close(can_socket_);
      exit(1);
    }
  }

  void run() {
    while (running_) {
      Task task = wait_for_next_task();
      task.func_();
    }
  }

  Task wait_for_next_task() {
    std::unique_lock<std::mutex> lock(task_mutex_);
    while (tasks_.empty() || tasks_.top().tp_ > std::chrono::system_clock::now()) {
      if (tasks_.empty()) {
        cv_.wait(lock);
      } else {
        cv_.wait_until(lock, tasks_.top().tp_);
      }
    }
    Task task = tasks_.top();
    tasks_.pop();
    return task;
  }

  std::string ifname_;
  int can_socket_;
  bool running_;
  std::priority_queue<Task, std::vector<Task>, TaskCmp> tasks_;
  std::mutex task_mutex_;
  std::condition_variable cv_;
  std::thread worker_thread_;
};

void send_can_message(int can_socket, const struct can_frame& frame) {
  int nbytes = write(can_socket, &frame, sizeof(struct can_frame));
  if (nbytes != sizeof(struct can_frame)) {
    std::cerr << "Error sending CAN message" << std::endl;
  }
}

void receive_can_message(int can_socket, struct can_frame& frame) {
  int nbytes = read(can_socket, &frame, sizeof(struct can_frame));
  if (nbytes != sizeof(struct can_frame)) {
    std::cerr << "Error receiving CAN message" << std::endl;
  }
}

int main() {
  std::string ifname = "can0";  // Change this to wer desired CAN interface name
  TaskScheduler scheduler(ifname);

  scheduler.start();

  scheduler.schedule(
      [&scheduler]() {
        // Send CAN message periodically
        struct can_frame frame;
        frame.can_id = 0x123;
        frame.can_dlc = 8;
        for (int i = 0; i < 8; i++) {
          frame.data[i] = i;
        }
        send_can_message(scheduler.can_socket_, frame);
      },
      std::chrono::milliseconds(1000));

  scheduler.schedule(
      [&scheduler]() {
        // Receive CAN messages
        struct can_frame received_frame;
        receive_can_message(scheduler.can_socket_, received_frame);
      },
      std::chrono::milliseconds(10));

  // Add more tasks for heartbeat messages or other periodic sends if necessary

  // Wait for user input to stop the scheduler
  std::cin.get();
  scheduler.stop();

  return 0;
}