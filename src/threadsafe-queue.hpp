#pragma once

#include <mutex>
#include <optional>
#include <queue>

template <typename Msg>
class ThreadSafeQueue final {
   public:
	ThreadSafeQueue() = default;
	ThreadSafeQueue(const ThreadSafeQueue&) = delete;
	ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
	ThreadSafeQueue(ThreadSafeQueue&& other) {
		// No need to lock 'this' as it's a new object
		std::lock_guard<std::mutex> lock(other.m_mutex);
		m_queue = std::move(other.m_queue);
	}

	/**
	 * Get the number of messages in the queue.
	 */
	size_t size() const {
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_queue.size();
	}

	/**
	 * Push a message onto the queue.
	 */
	void push(const Msg& msg) {
		std::lock_guard<std::mutex> lock(m_mutex);
		m_queue.push(msg);
	}

	/**
	 * Construct a message in place and push it onto the queue.
	 */
	template <typename... Args>
	void emplace(Args&&... args) {
		std::lock_guard<std::mutex> lock(m_mutex);
		m_queue.emplace(std::forward<Args>(args)...);
	}

	/**
	 * Pop a message from the queue. If the queue is empty, returns
	 * std::nullopt.
	 */
	std::optional<Msg> pop() {
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_queue.empty()) {
			return std::nullopt;
		}
		Msg msg = m_queue.front();
		m_queue.pop();
		return msg;
	}

   private:
	std::queue<Msg> m_queue;
	std::mutex m_mutex;
};
