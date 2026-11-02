#pragma once

#include <glad/glad.h>
#include <array>

class QueryManager {
public:
	QueryManager();
	~QueryManager();
	void init();
	void clear();
	void begin();
	void end();
	double poll();
private:
	static constexpr GLuint QUERY_COUNT = 10;

	size_t frame = 0;
	std::array<GLuint, QUERY_COUNT> queries{};
	std::array<bool, QUERY_COUNT> valid{};
};