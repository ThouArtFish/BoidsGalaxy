#include "../include/QueryManagerClass.hpp"

QueryManager::QueryManager() {
	init();
}

QueryManager::~QueryManager() {
	clear();
}

void QueryManager::init() {
	glGenQueries(QUERY_COUNT, queries.data());
	valid.fill(false);
}

void QueryManager::clear() {
	glDeleteQueries(QUERY_COUNT, queries.data());
	valid.fill(false);
}

void QueryManager::begin() {
	glBeginQuery(GL_TIME_ELAPSED, queries[frame]);
}

void QueryManager::end() {
	glEndQuery(GL_TIME_ELAPSED);
	valid[frame] = true;
	if (++frame == QUERY_COUNT) frame = 0;
}

double QueryManager::poll() {
	size_t idx = (frame + 1) % QUERY_COUNT;
	if (!valid[idx]) return -1.0;
	GLint ready = 0;
	glGetQueryObjectiv(queries[idx], GL_QUERY_RESULT_AVAILABLE, &ready);
	if (!ready) return -1.0;
	GLuint64 ns = 0;
	glGetQueryObjectui64v(queries[idx], GL_QUERY_RESULT, &ns);
	valid[idx] = false;
	return (double)ns;
}
