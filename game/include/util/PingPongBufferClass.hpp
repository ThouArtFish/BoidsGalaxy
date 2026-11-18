#pragma once

#include <ResourceManagerClass.hpp> 

template<class T> class PingPongBuffer {
public:
	using Buffer = TAGResourceManager::UnmappedBuffer<T>;

	PingPongBuffer(size_t size, TAGResourceManager::BufferAccess buffer_access);

	Buffer& getReadBuffer();
	Buffer& getWriteBuffer();
	void bindReadWrite(GLuint read_index, GLuint write_index);
	void unbindReadWrite();
	void switchBuffers();
private:
	bool is_bound = false;
	size_t write = 0, read = 1;
	GLuint write_index = 0, read_index = 0;
	Buffer buffer_a, buffer_b;
	std::array<Buffer*, 2> buffer_ptrs;
};

#include "PingPongBufferClass.inl"