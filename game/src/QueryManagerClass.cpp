#include "../include/QueryManagerClass.hpp"

QueryManager::QueryManager() {
	init();
}

QueryManager::~QueryManager() {
	clear();
}

void QueryManager::init() {
	std::array<GLuint, QUERY_COUNT> query_ids;
	glGenQueries(QUERY_COUNT, query_ids.data());
	for (size_t i = 0; i < QUERY_COUNT; i++) {
		queries[i].id = query_ids[i];
		queries[i].valid = false;
	}
}

void QueryManager::clear() {
	std::array<GLuint, QUERY_COUNT> query_ids;
	for (size_t i = 0; i < QUERY_COUNT; i++) {
		queries[i].valid = false;
		query_ids[i] = queries[i].id;
	}
	glDeleteQueries(QUERY_COUNT, query_ids.data());
}

void QueryManager::begin(size_t tag) {
	queries[frame].tag = tag;
	glBeginQuery(GL_TIME_ELAPSED, queries[frame].id);
}

void QueryManager::end() {
	glEndQuery(GL_TIME_ELAPSED);
	queries[frame++].valid = true;
	if (frame == QUERY_COUNT) frame = 0;
}

QueryManager::Result QueryManager::poll() {
	const size_t idx = check_frame++ % QUERY_COUNT;
	if (check_frame == QUERY_COUNT) check_frame = 0;
	const size_t tag = queries[idx].tag;
	if (!queries[idx].valid) return { tag, -1.0 };
	GLint ready = 0;
	glGetQueryObjectiv(queries[idx].id, GL_QUERY_RESULT_AVAILABLE, &ready);
	if (!ready) return { tag, -1.0 };
	GLuint64 ns = 0;
	glGetQueryObjectui64v(queries[idx].id, GL_QUERY_RESULT, &ns);
	queries[idx].valid = false;
	return { tag, static_cast<double>(ns) };
}
