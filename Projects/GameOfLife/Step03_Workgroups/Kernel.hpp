#pragma once
#include<vector>

namespace GameOfLife
{
	using index = ptrdiff_t;

	struct uvec3 {
		uint32_t x;
		uint32_t y;
		uint32_t z;
	};

	inline uvec3 unflatten(uint32_t i, uvec3 dims) {
		// dims = global size (X * Y * Z total)
		uint32_t xy = dims.x * dims.y;
		uvec3 gid;
		gid.x = uint32_t(i % dims.x);
		gid.y = uint32_t((i / dims.x) % dims.y);
		gid.z = uint32_t(i / xy);
		return gid;
	}

	class Kernel
	{
	public:
		Kernel(size_t w, size_t h, size_t pad);

		index coordinateToIndex(index cx, index cy) const {
			return (cy + m_Pad) * m_PaddedWidth + (cx + m_Pad);
		}

		void set0(index cx, index cy) {
			m_State0[coordinateToIndex(cx, cy)] = 1;
		}

		uint32_t get1(index cx, index cy) const{
			return m_State1[coordinateToIndex(cx, cy)];
		}

		void dispatch();
		void dispatchWorkGroup();
		void main();

		size_t getWidth() const {
			return m_Width;
		}

		size_t getHeight() const {
			return m_Height;
		}

		void swapBuffers() {
			std::swap(m_State0, m_State1);
		}
	private:
		size_t m_Width;
		size_t m_PaddedWidth;
		size_t m_Height;
		size_t m_PaddedHeight;
		size_t m_Pad;
		std::vector<uint32_t> m_State0{};
		std::vector<uint32_t> m_State1{};

		uvec3 m_LocalSize{ 4,4,1 };
		static inline thread_local uvec3 workGroupID;
		static inline thread_local uvec3 globalInvocationID;
		static inline thread_local uvec3 localInvocationID;
	};
}