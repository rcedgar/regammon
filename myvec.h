#ifndef myvec_h
#define myvec_h

class MyVec
	{
public:
	unsigned m_Size;
	float *m_Data;

public:
	MyVec()
		{
		m_Size = 0;
		m_Data = 0;
		}

	~MyVec()
		{
		Clear();
		}

public:
	void Clear()
		{
		if (m_Data != 0)
			myfree(m_Data);
		m_Data = 0;
		m_Size = 0;
		}

	void Alloc(unsigned n)
		{
		if (m_Size == n)
			return;

		Clear();
		m_Data = myalloc(float, n);
		m_Size = n;
		}

	void Zero()
		{
		for (unsigned i = 0; i < m_Size; ++i)
			m_Data[i] = float(0);
		}

	void AllocAndZero(unsigned n)
		{
		Alloc(n);
		Zero();
		}

	float Get(unsigned i) const
		{
		assert(i < m_Size);
		return m_Data[i];
		}

	void Set(unsigned i, float Value)
		{
		assert(i < m_Size);
		m_Data[i] = Value;
		}

	void LogMe(const string &s) const
		{
		Log("%s", s.c_str());
		for (unsigned i = 0; i < m_Size; ++i)
			Log(" [%u]=%.3g", i, Get(i));
		Log("\n");
		}

	unsigned size() const { return m_Size; }
	float operator[] (unsigned Index) const { return Get(Index); }
	float &operator[] (unsigned Index) { return m_Data[Index]; }
	};

#endif // myvec_h
