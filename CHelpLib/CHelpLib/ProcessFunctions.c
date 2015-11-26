
// ProcessFunctions.cpp
// Contains functions that provide help Process related tasks
// Shishir Bhat (http://www.shishirbhat.com)
// History
//      03/25/14 Initial version
//      09/12/14 Naming convention modifications
//

#include "InternalDefines.h"
#include "ProcessFunctions.h"

// GetProcNameFromID()
// Given a process ID, returns the name of the executable
// 
HRESULT CHL_PsGetProcNameFromID(_In_ DWORD pid, _Inout_z_ WCHAR *pwsProcName, _In_ DWORD dwBufSize)
{
    HANDLE hProcess = NULL;
    HMODULE ahModules[512];
    DWORD dwNeeded = 0;
    HRESULT hr = S_OK;

    ASSERT(dwBufSize > 5);    // 3(extension) + 1(.) + 1(imagename atleast one char)

    if(pid == 0)
    {
        return E_INVALIDARG;
    }

    if((hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, pid)) == NULL)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if(!EnumProcessModules(hProcess, ahModules, sizeof(ahModules), &dwNeeded))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

    //logdbg("#Modules = %d", dwNeeded/sizeof(HMODULE));
    if(!GetModuleBaseName(hProcess, ahModules[0], pwsProcName, dwBufSize))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

done:
    CloseHandle(hProcess);
    return hr;

}// GetProcNameFromID()

// Given handle to a map view of a file, return pointer to the IMAGE_NT_HEADERS structure
//
HRESULT CHL_PsGetNtHeaders(_In_ HANDLE hMapView, _Out_ PIMAGE_NT_HEADERS *ppstNtHeaders)
{
    PIMAGE_DOS_HEADER pDOSHeader = NULL;

    HRESULT hr = S_OK;
    if(!ISVALID_HANDLE(hMapView) || !ppstNtHeaders)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        pDOSHeader = (PIMAGE_DOS_HEADER)hMapView;
    
        // verify "MZ" in the DOS header
	    if(pDOSHeader->e_magic == IMAGE_DOS_SIGNATURE)
	    {
            *ppstNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hMapView + pDOSHeader->e_lfanew);
	    }
        else
        {
            hr = E_UNEXPECTED;
        }
    }
    return hr;
}

// Get a byte pointer to the start of code in a executable file(memory mapped)
//
HRESULT CHL_PsGetPtrToCode(
    _In_ DWORD dwFileBase, 
    _In_ PIMAGE_NT_HEADERS pNTHeaders, 
    _Out_ PDWORD pdwCodePtr, 
    _Out_ PDWORD pdwSizeOfData,
    _Out_ PDWORD pdwCodeSecVirtAddr)
{

	PIMAGE_SECTION_HEADER pImgSecHeader = NULL;
	DWORD dwSecChars = 0;

    HRESULT hr = S_OK;
    if(!pNTHeaders || !pdwCodePtr)
    {
        hr = E_INVALIDARG;
        goto done;
    }

	// Get the .text section's header using AddressOfEntryPoint as the RVA
    hr = CHL_PsGetEnclosingSectionHeader(
            pNTHeaders->OptionalHeader.AddressOfEntryPoint,
			pNTHeaders, 
            &pImgSecHeader);
	if(FAILED(hr))
	{
		goto done;
	}

	// test if the retrieved section contains code and is executable
    dwSecChars = pImgSecHeader->Characteristics;
	if( ! ((dwSecChars & IMAGE_SCN_CNT_CODE) && (dwSecChars & IMAGE_SCN_MEM_EXECUTE)) )
	{
		hr = E_UNEXPECTED;
		goto done;
	}

	*pdwCodePtr = dwFileBase + pImgSecHeader->PointerToRawData;
	
	// From "Microsoft Portable Executable and Common Object File Format Specification"
	// todo: If SizeOfRawData is less than VirtualSize, the remainder of the section is zero-filled.
	// done: 081812
    if(pdwSizeOfData)
    {
        if(pImgSecHeader->SizeOfRawData < pImgSecHeader->Misc.VirtualSize)
        {
		    *pdwSizeOfData = pImgSecHeader->SizeOfRawData;
        }
	    else
        {
		    *pdwSizeOfData = pImgSecHeader->Misc.VirtualSize;
        }
    }
	
    if(pdwCodeSecVirtAddr)
    {
        *pdwCodeSecVirtAddr = pNTHeaders->OptionalHeader.ImageBase + pImgSecHeader->VirtualAddress;
    }

done:
	return hr;
}


// CHL_PsGetEnclosingSectionHeader()
// ** From Matt Pietrek's PEDUMP.exe **
// Given an RVA, look up the section header that encloses it and return a
// pointer to its IMAGE_SECTION_HEADER.
//
// Args:
//		
//			
// RetVal:
//	
//
HRESULT CHL_PsGetEnclosingSectionHeader(_In_ DWORD rva, _In_ PIMAGE_NT_HEADERS pNTHeader, _Out_ PIMAGE_SECTION_HEADER *ppstSecHeader)
{
    int index;
    PIMAGE_SECTION_HEADER pSection = NULL;
    
    HRESULT hr = S_OK;
    if(!pNTHeader || !ppstSecHeader)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        pSection = IMAGE_FIRST_SECTION(pNTHeader);
        hr = E_NOT_SET; // Start with this, set to S_OK if found
        for ( index = 0; index < pNTHeader->FileHeader.NumberOfSections; ++index, pSection++ )
        {
            // Is the RVA within this section?
            if ( (rva >= pSection->VirtualAddress) && 
                 (rva < (pSection->VirtualAddress + pSection->Misc.VirtualSize)))
            {
                *ppstSecHeader = pSection;
                hr = S_OK;
                break;
            }
        }
    }
    return hr;
}
