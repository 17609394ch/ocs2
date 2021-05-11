#include <gtest/gtest.h>
#include <ocs2_core/misc/ThreadPool.h>

using namespace ocs2;

TEST(testThreadPool, testCanExecuteTask) {
  ThreadPool pool(1);
  std::future<void> res;
  int answer = 0;

  res = pool.run([&answer](int) { answer = 42; });
  res.wait_for(std::chrono::seconds(1));

  EXPECT_EQ(answer, 42);
}

TEST(testThreadPool, testReturnType) {
  ThreadPool pool(1);
  std::future<int> res;

  res = pool.run([](int) -> int { return 42; });

  EXPECT_EQ(res.get(), 42);
}

TEST(testThreadPool, testPropagateException) {
  ThreadPool pool(1);
  std::function<void(int)> task;

  // send task to pool
  task = [](int) { throw std::string("exception"); };
  EXPECT_THROW(pool.run(task).get(), std::string);
}

TEST(testThreadPool, testCanExecuteMultipleTasks) {
  ThreadPool pool(2);
  std::function<void(int)> task;
  std::future<void> res1, res2;

  std::promise<void> barrier_promise;
  std::shared_future<void> barrier = barrier_promise.get_future();

  std::string data1, data2;

  res1 = pool.run([&data1, barrier](int) {
    data1 = "running";
    barrier.wait();
    data1 = "done";
  });
  res2 = pool.run([&data2, barrier](int) {
    data2 = "running";
    barrier.wait();
    data2 = "done";
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  // check that both threads are blocking
  EXPECT_EQ(data1, "running");
  EXPECT_EQ(data2, "running");

  // signal threads
  barrier_promise.set_value();

  // wait for pool to run tasks
  std::future_status status1 = res1.wait_for(std::chrono::seconds(1));
  ASSERT_EQ(status1, std::future_status::ready);

  std::future_status status2 = res2.wait_for(std::chrono::seconds(1));
  ASSERT_EQ(status2, std::future_status::ready);

  EXPECT_EQ(data1, "done");
  EXPECT_EQ(data2, "done");

  res1.get();
  res2.get();
}

TEST(testThreadPool, testRunMultiple) {
  ThreadPool pool(2);
  std::atomic_int counter;
  counter = 0;

  pool.runParallel([&](int) { counter++; }, 42);

  EXPECT_EQ(counter, 42);
}

TEST(testThreadPool, testNoThreads) {
  ThreadPool pool(0);

  auto fut1 = pool.run([&](int) -> std::string { return "runs on main thread"; });
  EXPECT_EQ(fut1.get(), "runs on main thread");
}
