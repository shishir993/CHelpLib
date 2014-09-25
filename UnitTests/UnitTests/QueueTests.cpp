
#include "Common.h"
#include "TestLinkedList.h"

#include "Queue.h"

static char* apszTest[] = { "string1", "string2", "string3", "string4", "string5" };
static int aiTest[] = { 1, 2, 3, 4, 50, 60, 700, 800, 1000, 2222 };

// Local functions
static HRESULT _TestStrings();
static HRESULT _TestIntegers();

HRESULT QueueRunTests()
{
    HRESULT hr = _TestStrings();
    if(FAILED(_TestIntegers()))
    {
        hr = E_FAIL;
    }
    return hr;
}

BOOL CompareStrings(PVOID pToFind, PVOID pCurVal)
{
    return strcmp((char*)pToFind, (char*)pCurVal) == 0;
}

static HRESULT _TestStrings()
{
    PCHL_QUEUE pq;
    HRESULT hr = S_OK;
    if(FAILED(CHL_DsCreateQ(&pq, CHL_VT_STRING, ARRAYSIZE(apszTest))))
    {
        printf("Failed to create string Queue\n");
        hr = E_FAIL;
        goto done;
    }
    
    // Insert all
    for(int i = 0; i < ARRAYSIZE(apszTest); ++i)
    {
        if(FAILED(pq->Insert(pq, apszTest[i], 
                    strlen(apszTest[i])+sizeof(apszTest[0][0]))))
        {
            printf("Failed to insert %d:%s\n", i, apszTest[i]);
            hr = E_FAIL;
        }
    }

    // Find head, middle and tail values
    if(FAILED(pq->Find(pq, (PVOID)apszTest[0], CompareStrings)))
    {
        printf("Failed to find %s\n", apszTest[0]);
        hr = E_FAIL;
    }
    else
    {
        printf("Found %s\n", apszTest[0]);
    }

    if(FAILED(pq->Find(pq, (PVOID)apszTest[2], CompareStrings)))
    {
        printf("Failed to find %s\n", apszTest[2]);
        hr = E_FAIL;
    }
    else
    {
        printf("Found %s\n", apszTest[2]);
    }

    if(FAILED(pq->Find(pq, (PVOID)apszTest[ARRAYSIZE(apszTest) - 1], CompareStrings)))
    {
        printf("Failed to find %s\n", apszTest[ARRAYSIZE(apszTest) - 1]);
        hr = E_FAIL;
    }
    else
    {
        printf("Found %s\n", apszTest[ARRAYSIZE(apszTest) - 1]);
    }

    char *psz = NULL;
    if(FAILED(pq->Peek(pq, (PVOID*)&psz, TRUE)))
    {
        printf("Failed to peek\n");
        hr = E_FAIL;
    }
    else
    {
        printf("Peeked into queue and found at head: [%s]\n", psz);
    }

    // Delete all and print them
    psz = NULL;
    int iFound = 0;
    while(SUCCEEDED(pq->Delete(pq, (PVOID*)&psz, TRUE)))
    {
        printf("Retrieved: [%s] ", psz);
        if(strcmp(apszTest[iFound++], psz) != 0)
        {
            printf("NO MATCH");
            hr = E_FAIL;
        }
        free(psz);
        printf("\n");
    }

    if(iFound != ARRAYSIZE(apszTest))
    {
        printf("Found only %d items out of %d inserted\n", iFound, ARRAYSIZE(apszTest));
        hr = E_FAIL;
    }

done:
    if(pq)
    {
        pq->Destroy(pq);
    }
    return hr;
}

static HRESULT _TestIntegers()
{
    PCHL_QUEUE pq;
    HRESULT hr = S_OK;
    if(FAILED(CHL_DsCreateQ(&pq, CHL_VT_INT32, ARRAYSIZE(aiTest))))
    {
        printf("Failed to create int Queue\n");
        hr = E_FAIL;
        goto done;
    }
    
    // Insert all
    for(int i = 0; i < ARRAYSIZE(aiTest); ++i)
    {
        if(FAILED(pq->Insert(pq, (PVOID)aiTest[i], sizeof aiTest[0])))
        {
            printf("Failed to insert %d:%d\n", i, aiTest[i]);
            hr = E_FAIL;
        }
    }

    // Delete all and print them
    int foundVal = NULL;
    int iFound = 0;
    while(SUCCEEDED(pq->Delete(pq, &foundVal, TRUE)))
    {
        printf("Retrieved: [%d] ", foundVal);
        if(aiTest[iFound++] != foundVal)
        {
            printf("NO MATCH");
            hr = E_FAIL;
        }
        printf("\n");
    }

    if(iFound != ARRAYSIZE(aiTest))
    {
        printf("Found only %d items out of %d inserted\n", iFound, ARRAYSIZE(aiTest));
        hr = E_FAIL;
    }

done:
    if(pq)
    {
        pq->Destroy(pq);
    }
    return hr;
}
