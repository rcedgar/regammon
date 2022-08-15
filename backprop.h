#ifndef backprop_h
#define backprop_h

class NNBackPropData
	{
public:
	unsigned m_LayerCount;

// One delta vector for each layer
	vector<vector<float> > m_Deltas;

 // Sum of deltas so far
	unsigned m_AccumCount;
	vector<vector<float> > m_AccumDeltas;

public:
	NNBackPropData()
		{
		}

	~NNBackPropData()
		{
		Clear();
		}

	void Clear()
		{
		m_Deltas.clear();
		m_AccumDeltas.clear();
		}

	void Alloc(const vector<unsigned> &Sizes)
		{
		Clear();

		m_LayerCount = SIZE(Sizes);
		assert(m_LayerCount > 1);
		m_Deltas.resize(LayerCount);
		m_AccumDeltas.resize(LayerCount);

	// First layer is input, special case
		m_Deltas[0] = 0;
		m_AccumDeltas[0] = 0;
		for (unsigned LayerIndex = 1; LayerIndex < m_LayerCount;
		  ++LayerIndex)
			{
			unsigned NodeCount = Sizes[LayerIndex];
			unsigned NodeCount1 = Sizes[LayerIndex-1];

			m_Deltas[LayerIndex] = new MyVec;
			m_AccumDeltas[LayerIndex] = new MyVec;

			m_Deltas[LayerIndex]->Alloc(NodeCount);
			m_AccumDeltas[LayerIndex]->Alloc(NodeCount);
			}
		}

	void Zero()
		{
		for (unsigned LayerIndex = 1; LayerIndex < m_LayerCount; ++LayerIndex)
			{
			m_Deltas[LayerIndex]->Zero();
			m_AccumDeltas[LayerIndex]->Zero();
			}
		}

	void ZeroAccum()
		{
		m_AccumCount = 0;
		for (unsigned LayerIndex = 1; LayerIndex < m_LayerCount; ++LayerIndex)
			m_AccumDeltas[LayerIndex]->Zero();
		}
	};

#endif // backprop_h
