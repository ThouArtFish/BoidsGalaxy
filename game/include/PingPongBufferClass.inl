#pragma once

#include "PingPongBufferClass.hpp"

template<class T> PingPongBuffer<T>::PingPongBuffer(size_t size, TAGResourceManager::BufferAccess buffer_access) :
	buffer_a(size, 2, buffer_access), buffer_b(size, 2, buffer_access) 
{
	buffer_ptrs = { &buffer_a, &buffer_b };
}

template<class T> PingPongBuffer<T>::Buffer& PingPongBuffer<T>::getReadBuffer() {
	return *buffer_ptrs[read];
}

template<class T> PingPongBuffer<T>::Buffer& PingPongBuffer<T>::getWriteBuffer() {
	return *buffer_ptrs[write];
}

template<class T> void PingPongBuffer<T>::bindReadWrite(GLuint read_index, GLuint write_index) {
	if (is_bound) return;

	this->read_index = read_index;
	this->write_index = write_index;
	buffer_ptrs[read]->bindToShader(read_index, TAGResourceManager::ShaderBufferType::SHADER_STORAGE, 0);
	buffer_ptrs[write]->bindToShader(write_index, TAGResourceManager::ShaderBufferType::SHADER_STORAGE, 0);
	is_bound = true;
}

template<class T> void PingPongBuffer<T>::unbindReadWrite() {
	if (!is_bound) return;

	buffer_ptrs[read]->unbindFromShader(read_index, TAGResourceManager::ShaderBufferType::SHADER_STORAGE);
	buffer_ptrs[write]->unbindFromShader(write_index, TAGResourceManager::ShaderBufferType::SHADER_STORAGE);
	is_bound = false;
}

template<class T> void PingPongBuffer<T>::switchBuffers() {
	if (is_bound) return;

	const GLuint temp = read;
	read = write;
	write = temp;
}

