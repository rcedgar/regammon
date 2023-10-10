#ifndef mymx_h
#define mymx_h

class MyMx
	{
public:
	unsigned m_RowCount;
	unsigned m_ColCount;
	float **m_Data;

public:
	MyMx()
		{
		m_RowCount = 0;
		m_ColCount = 0;
		m_Data = 0;
		}

	~MyMx()
		{
		Clear();
		}

public:
	void Clear()
		{
		if (m_Data == 0)
			return;

		for (unsigned Row = 0; Row < m_RowCount; ++Row)
			myfree(m_Data[Row]);
		myfree(m_Data);

		m_Data = 0;
		m_RowCount = 0;
		m_ColCount = 0;
		}

	void Alloc(unsigned RowCount, unsigned ColCount)
		{
		Clear();
		m_Data = myalloc(float *, RowCount);
		for (unsigned Row = 0; Row < RowCount; ++Row)
			m_Data[Row] = myalloc(float, ColCount);
		m_RowCount = RowCount;
		m_ColCount = ColCount;
		}

	void Zero()
		{
		for (unsigned Row = 0; Row < m_RowCount; ++Row)
			for (unsigned Col = 0; Col < m_ColCount; ++Col)
				m_Data[Row][Col] = float(0);
		}

	void AllocAndZero(unsigned RowCount, unsigned ColCount)
		{
		Alloc(RowCount, ColCount);
		Zero();
		}

	float Get(unsigned Row, unsigned Col) const
		{
		assert(Row < m_RowCount);
		assert(Col < m_ColCount);
		return m_Data[Row][Col];
		}

	void Set(unsigned Row, unsigned Col, float Value)
		{
		assert(Row < m_RowCount);
		assert(Col < m_ColCount);
		m_Data[Row][Col] = Value;
		}
	};

#endif // mymx_h
