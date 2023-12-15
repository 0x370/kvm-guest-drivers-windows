#pragma once
#include "ParaNdis-VirtQueue.h"
#include "ParaNdis-AbstractPath.h"

class CParaNdisRX : public CParaNdisTemplatePath<CVirtQueue>, public CNdisAllocatable < CParaNdisRX, 'XRHR' > {
public:
    CParaNdisRX();
    ~CParaNdisRX();

    bool Create(PPARANDIS_ADAPTER Context, UINT DeviceQueueIndex);

    BOOLEAN AddRxBufferToQueue(pRxNetDescriptor pBufferDescriptor);

    void PopulateQueue();

    void FreeRxDescriptorsFromList();

    void ReuseReceiveBuffer(pRxNetDescriptor pBuffersDescriptor)
    {
        TPassiveSpinLocker autoLock(m_Lock);

        ReuseReceiveBufferNoLock(pBuffersDescriptor);
    }

    VOID ProcessRxRing(CCHAR nCurrCpuReceiveQueue);

    BOOLEAN RestartQueue();

    void Shutdown()
    {
        TPassiveSpinLocker autoLock(m_Lock);

        m_VirtQueue.Shutdown();
        m_Reinsert = false;
    }

    void KickRXRing();

    PARANDIS_RECEIVE_QUEUE &UnclassifiedPacketsQueue() { return m_UnclassifiedPacketsQueue;  }

    LONG RegisterInflightAsyncAlloc(int val = 1)
    {
        return InterlockedAdd(&m_InflightAsyncAlloc, val);
    }

    LONG UnRegisterInflightAsyncAlloc()
    {
        return InterlockedDecrement(&m_InflightAsyncAlloc);
    }

private:
    /* list of Rx buffers available for data (under VIRTIO management) */
    LIST_ENTRY              m_NetReceiveBuffers;
    UINT                    m_NetNofReceiveBuffers;
    UINT                    m_MinReceiveBuffers;
    UINT                    m_MaxInflightAlloc;
    LONG volatile           m_InflightAsyncAlloc;

    UINT m_nReusedRxBuffersCounter, m_nReusedRxBuffersLimit = 0;

    bool m_Reinsert = true;

    PARANDIS_RECEIVE_QUEUE m_UnclassifiedPacketsQueue;

    void ReuseReceiveBufferNoLock(pRxNetDescriptor pBuffersDescriptor);
private:
    int PrepareReceiveBuffers();
    pRxNetDescriptor CreateRxDescriptorOnInit();
    VOID CreateRxDescriptorOnRuntime();
};

#ifdef PARANDIS_SUPPORT_RSS
VOID ParaNdis_ResetRxClassification(
    PARANDIS_ADAPTER *pContext);
#endif
