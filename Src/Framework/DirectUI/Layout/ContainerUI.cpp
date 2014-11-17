#include "stdafx.h"
#include "ContainerUI.h"

UI_IMPLEMENT_DYNCREATE(CContainerUI);

CContainerUI::CContainerUI(void)
	: m_iChildPadding(0)
	, m_bAutoDestroy(true)
	, m_bDelayedDestroy(true)
	, m_bMouseChildEnabled(true)
	, m_bScrollProcess(false)
	, m_pVerticalScrollBar(NULL)
	, m_pHorizontalScrollBar(NULL)
{
}


CContainerUI::~CContainerUI(void)
{
	m_bDelayedDestroy = false;
	RemoveAll();
	if( m_pVerticalScrollBar )
		delete m_pVerticalScrollBar;
	if( m_pHorizontalScrollBar )
		delete m_pHorizontalScrollBar;
}

CControlUI* CContainerUI::GetItemAt(int iIndex) const
{
	if( iIndex < 0 || iIndex >= m_items.GetSize() )
		return NULL;
	return static_cast<CControlUI*>(m_items[iIndex]);
}

int CContainerUI::GetItemIndex(CControlUI* pControl) const
{
	for( int it = 0; it < m_items.GetSize(); it++ )
	{
		if( static_cast<CControlUI*>(m_items[it]) == pControl )
		{
			return it;
		}
	}

	return -1;
}

bool CContainerUI::SetItemIndex(CControlUI* pControl, int iIndex)
{
	for( int it = 0; it < m_items.GetSize(); it++ )
	{
		if( static_cast<CControlUI*>(m_items[it]) == pControl )
		{
			NeedUpdate();            
			m_items.Remove(it);
			return m_items.InsertAt(iIndex, pControl);
		}
	}

	return false;
}

int CContainerUI::GetCount() const
{
	return m_items.GetSize();
}

bool CContainerUI::Add(CControlUI* pControl)
{
	if( pControl == NULL)
		return false;

	if( m_pManager != NULL )
		m_pManager->InitControls(pControl, this);
	if( IsVisible() ) 
		NeedUpdate();
	else 
		pControl->SetInternVisible(false);
	return m_items.Add(pControl);   
}

bool CContainerUI::AddAt(CControlUI* pControl, int iIndex)
{
	if( pControl == NULL) return false;

	if( m_pManager != NULL )
		m_pManager->InitControls(pControl, this);
	if( IsVisible() )
		NeedUpdate();
	else
		pControl->SetInternVisible(false);
	return m_items.InsertAt(iIndex, pControl);
}

bool CContainerUI::Remove(CControlUI* pControl)
{
	if( pControl == NULL) return false;

	for( int it = 0; it < m_items.GetSize(); it++ )
	{
		if( static_cast<CControlUI*>(m_items[it]) == pControl )
		{
			NeedUpdate();
			if( m_bAutoDestroy )
			{
				if( m_bDelayedDestroy && m_pManager )
					m_pManager->AddDelayedCleanup(pControl);             
				else delete pControl;
			}
			return m_items.Remove(it);
		}
	}
	return false;
}

bool CContainerUI::RemoveAt(int iIndex)
{
	CControlUI* pControl = GetItemAt(iIndex);
	if (pControl != NULL)
	{
		return CContainerUI::Remove(pControl);
	}

	return false;
}

void CContainerUI::RemoveAll()
{
	for( int it = 0; m_bAutoDestroy && it < m_items.GetSize(); it++ )
	{
		if( m_bDelayedDestroy && m_pManager )
			m_pManager->AddDelayedCleanup(static_cast<CControlUI*>(m_items[it]));             
		else
			delete static_cast<CControlUI*>(m_items[it]);
	}
	m_items.Empty();
	NeedUpdate();
}

LPCTSTR CContainerUI::GetClass() const
{
	return _T("Container");
}

LPVOID CContainerUI::GetInterface(LPCTSTR lpszClass)
{

	if ( _tcscmp(lpszClass, _T("IContainer")) == 0 )
		return static_cast<IContainerUI*>(this);
	else if( _tcscmp(lpszClass, _T("Container")) == 0 )
		return this;
	else
		return CControlUI::GetInterface(lpszClass);
}

void CContainerUI::SetManager(CWindowUI* pManager, CControlUI* pParent)
{
	for( int it = 0; it < m_items.GetSize(); it++ )
	{
		static_cast<CControlUI*>(m_items[it])->SetManager(pManager, this);
	}

	if( m_pVerticalScrollBar != NULL )
		m_pVerticalScrollBar->SetManager(pManager, this);
	if( m_pHorizontalScrollBar != NULL )
		m_pHorizontalScrollBar->SetManager(pManager, this);
	CControlUI::SetManager(pManager, pParent);
}

void CContainerUI::Render(IUIRender* pRender,LPCRECT pRcPaint)
{
	CDuiRect rcTemp;
	// ˢ�������Ƿ���ؼ������ص�
	if ( !rcTemp.IntersectRect(pRcPaint, &m_rcItem))
		return;

	// �ص����������ƾ���
	CRenderClip clip;
	CRenderClip::GenerateClip(pRender->GetPaintDC(), rcTemp, clip);
	// �ص����Ȼ����ֿؼ�
	CControlUI::Render(pRender, pRcPaint);

	if( m_items.GetSize() > 0 )
	{   // �����ӿؼ�����Ҫ�ݹ����ͼ
		
		// �ӿؼ������м����ڱ߿�
		CDuiRect rcContent = m_rcItem;
		rcContent.left += m_rcInset.left;
		rcContent.top += m_rcInset.top;
		rcContent.right -= m_rcInset.right;
		rcContent.bottom -= m_rcInset.bottom;

		// �����Ҫ��ʾ������������������������Ŀ��߶�
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() )
			rcContent.right -= m_pVerticalScrollBar->GetFixedWidth();
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() )
			rcContent.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

		// �������Ƿ��ڻ�ͼ������
		if( !rcTemp.IntersectRect(pRcPaint, &rcContent) )
		{   //����������ˢ�������ص�
			
			for( int it = 0; it < m_items.GetSize(); it++ )
			{
				// �ݹ��ӿؼ�������״̬�Ϳؼ��������ͼ�����ص��ģ�����
				CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
				if( !pControl->IsVisible() )
					continue;
				if( !rcTemp.IntersectRect( pRcPaint, &pControl->GetRect()) )
					continue;

				// �ӿؼ��Ǹ�������
				if( pControl ->IsFloat() )
				{
					// ��鸡���ؼ��Ƿ񳬳��˲��ֿؼ�����
					if( !rcTemp.IntersectRect(&m_rcItem, &pControl->GetRect()) )
						continue;
					pControl->Render(pRender, pRcPaint);
				}
			}
		}
		else
		{   // ��������ˢ�������ص�

			// Ϊ�ص����ִ�����������
			CRenderClip childClip;
			CRenderClip::GenerateClip(pRender->GetPaintDC(), rcTemp, childClip);
			for( int it = 0; it < m_items.GetSize(); it++ )
			{   // �ݹ��ӿؼ�������״̬�Ϳؼ��������ͼ�����ص��ģ�����
				CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
				if( !pControl->IsVisible() )
					continue;
				if( !rcTemp.IntersectRect(pRcPaint, &pControl->GetRect()) )
					continue;

				// �ӿؼ��Ǹ�������
				if( pControl ->IsFloat() )
				{
					// ��鸡���ؼ��Ƿ񳬳��˲��ֿؼ�����
					if( !rcTemp.IntersectRect(&m_rcItem, &pControl->GetRect()) )
						continue;

					// ���ƿռ����ص�������ˢ��
					CRenderClip::UseOldClipBegin(pRender->GetPaintDC(), childClip);
					pControl->Render(pRender, pRcPaint);
					CRenderClip::UseOldClipEnd(pRender->GetPaintDC(), childClip);
				}
				else
				{
					// ���Ǹ����ؼ�����󲻴��ڸ����ڣ��ؼ�������ˢ�����ص��ͻ�ͼ
					if( !rcTemp.IntersectRect( &rcContent, &pControl->GetRect()) )
						continue;
					pControl->Render(pRender, pRcPaint);
				}
			}
		}
	}

	// ��󻭹�����
	if( m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible() )
	{
		if( rcTemp.IntersectRect( pRcPaint, &m_pVerticalScrollBar->GetRect()) )
		{
			m_pVerticalScrollBar->Render(pRender, pRcPaint);
		}
	}

	if( m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible() )
	{
		if( rcTemp.IntersectRect( pRcPaint, &m_pHorizontalScrollBar->GetRect()) )
		{
			m_pHorizontalScrollBar->Render(pRender, pRcPaint);
		}
	}
}

void CContainerUI::SetRect(LPCRECT rc)
{
	CControlUI::SetRect(rc);
	if( m_items.IsEmpty() )
		return;

	// ��������������
	CDuiRect rcTemp(rc);
	rcTemp.left += m_rcInset.left;
	rcTemp.top += m_rcInset.top;
	rcTemp.right -= m_rcInset.right;
	rcTemp.bottom -= m_rcInset.bottom;

	for( int it = 0; it < m_items.GetSize(); it++ )
	{   // �ݹ��ӿؼ��������꣬���ص�����
		CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
		if( !pControl->IsVisible() )
			continue;
		if( pControl->IsFloat() )
		{
			SetFloatPos(it);
		}
		else
		{
			pControl->SetRect(&rcTemp); // ���з�float�ӿؼ��Ŵ������ͻ���
		}
	}
}

void CContainerUI::SetFloatPos(int iIndex)
{
	// ��ΪCControlUI::SetPos��float�Ĳ���Ӱ�죬���ﲻ�ܶ�float������ӹ�������Ӱ��
	if( iIndex < 0 || iIndex >= m_items.GetSize() )
		return;

	CControlUI* pControl = static_cast<CControlUI*>(m_items[iIndex]);

	if( !pControl->IsVisible() )
		return;
	if( !pControl->IsFloat() )
		return;

	SIZE szXY = pControl->GetFixedXY();
	SIZE sz = {pControl->GetFixedWidth(), pControl->GetFixedHeight()};
	RECT rcCtrl = { 0 };
	if( szXY.cx >= 0 )
	{
		rcCtrl.left = m_rcItem.left + szXY.cx;
		rcCtrl.right = m_rcItem.left + szXY.cx + sz.cx;
	}
	else
	{
		rcCtrl.left = m_rcItem.right + szXY.cx - sz.cx;
		rcCtrl.right = m_rcItem.right + szXY.cx;
	}
	if( szXY.cy >= 0 )
	{
		rcCtrl.top = m_rcItem.top + szXY.cy;
		rcCtrl.bottom = m_rcItem.top + szXY.cy + sz.cy;
	}
	else
	{
		rcCtrl.top = m_rcItem.bottom + szXY.cy - sz.cy;
		rcCtrl.bottom = m_rcItem.bottom + szXY.cy;
	}
	//if( pControl->IsRelativePos() )
	//{
	//	TRelativePosUI tRelativePos = pControl->GetRelativePos();
	//	SIZE szParent = {m_rcItem.right-m_rcItem.left,m_rcItem.bottom-m_rcItem.top};
	//	if(tRelativePos.szParent.cx != 0)
	//	{
	//		int nIncrementX = szParent.cx-tRelativePos.szParent.cx;
	//		int nIncrementY = szParent.cy-tRelativePos.szParent.cy;
	//		rcCtrl.left += (nIncrementX*tRelativePos.nMoveXPercent/100);
	//		rcCtrl.top += (nIncrementY*tRelativePos.nMoveYPercent/100);
	//		rcCtrl.right = rcCtrl.left+sz.cx+(nIncrementX*tRelativePos.nZoomXPercent/100);
	//		rcCtrl.bottom = rcCtrl.top+sz.cy+(nIncrementY*tRelativePos.nZoomYPercent/100);
	//	}
	//	pControl->SetRelativeParentSize(szParent);
	//}
	pControl->SetRect(&rcCtrl);
}