#include <bits/stdc++.h>

int word_count = 0;
std::mutex word_count_mutex;

std::queue<std::string> request_queue;
std::mutex queue_mutex;

void count_words_in_chunk(const std::string& chunk) {
  std::cout << chunk << '\n';
  auto prev_was_word = true;
  for (auto&& i : chunk) {
    if (std::isspace(i) && prev_was_word) {
      {
        std::lock_guard<std::mutex> lock(word_count_mutex);
        ++word_count;
      }
      prev_was_word = false;
    }
    else {
      if (!prev_was_word) {
        prev_was_word = true;
      }
    }
  }
}

void worker_thread() {
  while (true) {
    std::string chunk;

    {
      std::lock_guard<std::mutex> lock(queue_mutex);

      if (request_queue.empty()) {
        continue;
      }

      chunk = request_queue.front();

      if (chunk == "") {
        break;
      }

      request_queue.pop();
    }

    count_words_in_chunk(chunk);
  }
}

int main() {
  const unsigned int num_threads = std::thread::hardware_concurrency();

  // start worker threads
  std::vector<std::thread> threads;
  for (unsigned int i = 0; i < num_threads; ++i) {
    threads.emplace_back(worker_thread);
  }

  // open file stream
  std::ifstream file("./test.txt");
  if (!file.is_open()) {
    std::cerr << "Failed to open file." << std::endl;
    return 1;
  }

  std::string buffer(1024, 0);

  // read file in chunks
  while (!file.eof()) {
    file.read(buffer.data(), buffer.size());
    std::streamsize s = file.gcount();

    {
      std::lock_guard<std::mutex> lock(queue_mutex);
      request_queue.push(buffer);
    }
  }

  // signal worker threads to finish
  for (unsigned int i = 0; i < num_threads; ++i) {
    request_queue.push("");
  }

  // wait for worker threads to finish
  for (auto& thread : threads) {
    thread.join();
  }

  // print result
  std::cout << "Word count: " << word_count << std::endl;

  return 0;
}