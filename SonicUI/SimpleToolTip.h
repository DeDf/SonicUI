#pragma once

// ��װ�˱�׼windows��tooltip32
class CSimpleToolTip
{
public:
	CSimpleToolTip(void);
	~CSimpleToolTip(void);
	BOOL Create(HWND hParent);
	void Destory();
	void SetTipText(LPCTSTR lpszText, HWND hWnd, RECT& rt, DWORD dwCurrentHolder);

	// ����tip�Ŀ�ȣ����Ϊ0����û��ǿ�ƿ�ȣ�tip��������ʾ������������Ȼ�����ʾ
	void SetTipWidth(int nWidth);
	void ClearTip();
	DWORD GetCurrentHolder(){return m_dwCurrentHolder;}
protected:
	HWND m_hTipWnd;
	HWND m_hFakeParent;			// ��ǰΪ�ĸ��������
	DWORD m_dwCurrentHolder;	// ��ǰΪ�ĸ��������
};
