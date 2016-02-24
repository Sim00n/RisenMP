#pragma once
#include <Windows.h>
#include <assert.h>
#include <new>

#ifdef DEBUG_BUILD
#	define DEBUG_M(msg, ...) printf(msg, __VA_ARGS__)
#else
#	define DEBUG_M(msg, ...) do {} while (true)
#endif

struct Vec3
{
	float x = 0.0f, y = 0.0f, z = 0.0f;
	Vec3() { this->x = 0.0f; this->y = 0.0f; this->z = 0.0f; }
	Vec3(float x, float y, float z) { this->x = x; this->y = y; this->z = z; }
};

struct bCMatrix
{
	float matrix[4][4];

	void Identity(void)
	{
		for (int x = 0; x < 4; ++x) {
			for (int y = 0; y < 4; ++y) {
				matrix[x][y] = 0.0f;
			}
		}

		matrix[0][0] = matrix[1][1] = matrix[2][2] = matrix[3][3] = 1.0f;
	}

	void Transform(const Vec3 &pos)
	{
		matrix[3][0] = pos.x;
		matrix[3][1] = pos.y;
		matrix[3][2] = pos.z;
	}
};

struct bCQuaternion {
	float x, y, z, theta;
};

/**
* @brief Implementation of bCString class based on Genome engine implementation.
*
* This class is located in SharedBase.dll
*/
class bCString
{
private:
	/// Structure holding string data.
	struct bCStringData
	{
		unsigned m_size; // Amount of the stored characters (without null terminator)
		unsigned m_refs; // Reference count of this data. (Probably: bCAtomic<unsigned> m_refs)

						 /// Constructor.
		bCStringData() : m_size(0), m_refs(0) {}
	};

	/// The string buffer.
	char *m_buffer;
public:
	/// Explict constructor creating string from standard char array.
	explicit bCString(const char *const buffer)
	{
		assert(buffer);

		const size_t bufferSize = strlen(buffer);

		char *const dataBuffer = new char[8 + bufferSize + 1];

		bCStringData *const data = new (dataBuffer) bCStringData;

		// As we are running only in one thread for now we can simply set
		// reference count here to 1 without need of doing atomic operation.
		data->m_refs = 1;

		data->m_size = static_cast<unsigned>(bufferSize);

		m_buffer = static_cast<char *>(dataBuffer + 8);

		memcpy(m_buffer, buffer, bufferSize);
		m_buffer[bufferSize] = 0;
	}

	/**
	* @name Set of the methods that are removed for now as we do not handle them correctly.
	*
	* @todo: Implement this set of methods.
	*/
	//@{
	bCString(void) = delete;
	bCString(const bCString&) = delete;
	bCString(bCString&&) = delete;

	void operator=(const char *) = delete;
	void operator=(const bCString&) = delete;
	void operator=(bCString&&) = delete;
	//@}

	/// Destructor.
	~bCString()
	{
		if (!m_buffer) {
			return;
		}

		char *const data = (m_buffer - 8);
		if (data) {
			delete[]data;
		}
		m_buffer = nullptr;
	}
};