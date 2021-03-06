#include "Layout/UITabLayout.h"
#include "Core/UIManager.h"


namespace DUILIB
{
	CTabLayoutUI::CTabLayoutUI() : m_iCurSel(-1)
	{
	}

	LPCTSTR CTabLayoutUI::GetClass() const
	{
		return DUI_CTR_TABLAYOUT;
	}

	LPVOID CTabLayoutUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, DUI_CTR_TABLAYOUT) == 0 ) return static_cast<CTabLayoutUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	BOOL CTabLayoutUI::Add(CControlUI* pControl)
	{
		BOOL ret = CContainerUI::Add(pControl);
		if( !ret ) return ret;

		if(m_iCurSel == -1 && pControl->IsVisible())
		{
			m_iCurSel = GetItemIndex(pControl);
		}
		else
		{
			pControl->SetVisible(FALSE);
		}

		return ret;
	}

	BOOL CTabLayoutUI::AddAt(CControlUI* pControl, int iIndex)
	{
		BOOL ret = CContainerUI::AddAt(pControl, iIndex);
		if( !ret ) return ret;

		if(m_iCurSel == -1 && pControl->IsVisible())
		{
			m_iCurSel = GetItemIndex(pControl);
		}
		else if( m_iCurSel != -1 && iIndex <= m_iCurSel )
		{
			m_iCurSel += 1;
		}
		else
		{
			pControl->SetVisible(FALSE);
		}

		return ret;
	}

	BOOL CTabLayoutUI::Remove(CControlUI* pControl, BOOL bDoNotDestroy)
	{
		if( pControl == NULL) return FALSE;

		int index = GetItemIndex(pControl);
		BOOL ret = CContainerUI::Remove(pControl, bDoNotDestroy);
		if( !ret ) return FALSE;

		if( m_iCurSel == index)
		{
			if( GetCount() > 0 )
			{
				m_iCurSel=0;
				GetItemAt(m_iCurSel)->SetVisible(TRUE);
			}
			else
				m_iCurSel=-1;
			NeedParentUpdate();
		}
		else if( m_iCurSel > index )
		{
			m_iCurSel -= 1;
		}

		return ret;
	}

	void CTabLayoutUI::RemoveAll()
	{
		m_iCurSel = -1;
		CContainerUI::RemoveAll();
		NeedParentUpdate();
	}

	int CTabLayoutUI::GetCurSel() const
	{
		return m_iCurSel;
	}

	BOOL CTabLayoutUI::SelectItem(int iIndex,  BOOL bTriggerEvent)
	{
		if( iIndex < 0 || iIndex >= m_items.GetSize() ) return FALSE;
		if( iIndex == m_iCurSel ) return TRUE;

		int iOldSel = m_iCurSel;
		m_iCurSel = iIndex;
		for( int it = 0; it < m_items.GetSize(); it++ )
		{
			if( it == iIndex ) {
				GetItemAt(it)->SetVisible(TRUE);
				GetItemAt(it)->SetFocus();
			}
			else GetItemAt(it)->SetVisible(FALSE);
		}
		NeedParentUpdate();

		if( m_pManager != NULL ) {
			m_pManager->SetNextTabControl();
			if (bTriggerEvent) m_pManager->SendNotify(this, DUI_MSGTYPE_TABSELECT, m_iCurSel, iOldSel);
		}
		return TRUE;
	}

	BOOL CTabLayoutUI::SelectItem(CControlUI* pControl, BOOL bTriggerEvent)
	{
		int iIndex=GetItemIndex(pControl);
		if (iIndex==-1)
			return FALSE;
		else
			return SelectItem(iIndex, bTriggerEvent);
	}

	void CTabLayoutUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcscmp(pstrName, _T("selectedid")) == 0 ) SelectItem(_ttoi(pstrValue));
		return CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	void CTabLayoutUI::SetPos(RECT rc, BOOL bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		rc = m_rcItem;

		// Adjust for inset
		rc.left += m_rcInset.left;
		rc.top += m_rcInset.top;
		rc.right -= m_rcInset.right;
		rc.bottom -= m_rcInset.bottom;

		for( int it = 0; it < m_items.GetSize(); it++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) {
				SetFloatPos(it);
				continue;
			}

			if( it != m_iCurSel ) continue;

			RECT rcPadding = pControl->GetPadding();
			rc.left += rcPadding.left;
			rc.top += rcPadding.top;
			rc.right -= rcPadding.right;
			rc.bottom -= rcPadding.bottom;

			SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };

			SIZE sz = pControl->EstimateSize(szAvailable);
			if( sz.cx == 0 ) {
				sz.cx = MAX(0, szAvailable.cx);
			}
			if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
			if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();

			if(sz.cy == 0) {
				sz.cy = MAX(0, szAvailable.cy);
			}
			if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
			if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();

			RECT rcCtrl = { rc.left, rc.top, rc.left + sz.cx, rc.top + sz.cy};
			pControl->SetPos(rcCtrl, FALSE);
		}
	}
}
