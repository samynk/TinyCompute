#pragma once
#include<vector>

namespace GameOfLife
{
	using index = ptrdiff_t;

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

		void execute();
		

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
	};
}