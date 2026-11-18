#pragma once

#include <glad/glad.h>
#include <array>

class QueryManager {
public:
	struct Query {
		GLuint id = 0, tag = 0;
		bool valid = false;
	};
	struct Result {
		size_t tag = 0;
		double time_in_ns = 0.0;
	};

	QueryManager();
	~QueryManager();
	void init();
	void clear();
	void begin(size_t tag = 0);
	void end();
	Result poll();
private:
	static constexpr GLuint QUERY_COUNT = 10;

	size_t frame = 0, check_frame = 0;
	std::array<Query, QUERY_COUNT> queries{};
};
