//-----------------------------------------------------------------------------
#ifndef _BitFlag_h_
#define _BitFlag_h_
//-----------------------------------------------------------------------------
//for example
//enum eDisableReasonForShortcut
//{
//	DisableReasonForShortcut_1 = 0x00000001,
//	DisableReasonForShortcut_2 = 0x00000002,
//	DisableReasonForShortcut_3 = 0x00000004,
//	DisableReasonForShortcut_4 = 0x00000008,
//	DisableReasonForShortcut_5 = 0x00000010,
//	DisableReasonForShortcut_6 = 0x00000020,
//};
//-----------------------------------------------------------------------------
class BitFlag
{
public:
	BitFlag();

	void AddFlag(unsigned int theFlag);
	void RemoveFlag(unsigned int theFlag);
	void Clear();

	bool IsZero() const;
	bool IsFlagExist(unsigned int theFlag) const;

	void SetValue(unsigned int uiValue);
	unsigned int GetValue() const;

private:
	unsigned int m_uiValue;
};
//-----------------------------------------------------------------------------
inline BitFlag::BitFlag()
{
	m_uiValue = 0;
}
//-----------------------------------------------------------------------------
inline void BitFlag::AddFlag(unsigned int theFlag)
{
	m_uiValue = m_uiValue | theFlag;
}
//-----------------------------------------------------------------------------
inline void BitFlag::RemoveFlag(unsigned int theFlag)
{
	m_uiValue = m_uiValue & (~theFlag);
}
//-----------------------------------------------------------------------------
inline void BitFlag::Clear()
{
	m_uiValue = 0;
}
//-----------------------------------------------------------------------------
inline bool BitFlag::IsZero() const
{
	return (m_uiValue == 0);
}
//-----------------------------------------------------------------------------
inline bool BitFlag::IsFlagExist(unsigned int theFlag) const
{
	return (m_uiValue & theFlag) != 0;
}
//-----------------------------------------------------------------------------
inline void BitFlag::SetValue(unsigned int uiValue)
{
	m_uiValue = uiValue;
}
//-----------------------------------------------------------------------------
inline unsigned int BitFlag::GetValue() const
{
	return m_uiValue;
}
//-----------------------------------------------------------------------------
#endif //_SoBitFlag_h_
//-----------------------------------------------------------------------------
