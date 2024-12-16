#include <iostream>
#include <vector>
#include <random>
#include <thread>
#include <chrono>
#include <exception>

#include "thread_pool.h"


int job(int id) {
  std::random_device r;
  std::mt19937 gen(r());
  std::uniform_int_distribution<int> dist(5, 10);

  auto sleep_time = dist(gen);
  std::cout << "Thread id = " << std::this_thread::get_id() << " is handling "
    << "job id = " << id << ". The job is going to sleep for " << sleep_time
    << "s." << std::endl;

  std::this_thread::sleep_for(std::chrono::seconds(sleep_time));

  std::cout << "Thread id = " << std::this_thread::get_id() << " finished "
    << "job id = " << id << "." << std::endl;

  return id;
}

int main(int argc, char* argv[]) {
  auto pool = ThreadPool<int>(4);

  std::vector<std::future<int>> futures {};

  for (int i = 0; i < 8; ++i) {
    futures.emplace_back(pool.submit(&job, i));
  }

  pool.wait();

  // Check the expected return from the futures
  for (int i = 0; i < futures.size(); ++i) {
    auto& future = futures[i];
    try {
      std::cout << "Job id = " << i << ", result = " << future.get() << ".\n";
    } catch (const std::exception& e) {
      std::cerr << "Job id = " << i << " threw an exception: " << e.what()
        << "\n";
    } catch (...) {
      std::cout << "Job id = " << i << " threw an unknown exception!\n";
    }
  }
  std::cout << std::endl;

  return 0;
}
