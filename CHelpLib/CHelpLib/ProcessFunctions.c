
// ProcessFunctions.cpp
// Contains functions that provide help Process related tasks
// Shishir K Prasad (http://www.shishirprasad.net)
// History
//      03/25/14 Initial version
//

#include "ProcessFunctions.h"

// GetProcNameFromID()
// Given a process ID, returns the name of the executable
// 
BOOL fChlPsGetProcNameFromID(DWORD pid, WCHAR *pwsProcName, DWORD dwBufSize)
{
    HANDLE hProcess = NULL;
    HMODULE ahModules[512];
    DWORD dwNeeded = 0;

    ASSERT(dwBufSize > 5);    // 3(extension) + 1(.) + 1(imagename atleast one char)

    if(pid == 0)
    {
        return FALSE;
    }

    if((hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, pid)) == NULL)
    {
        return FALSE;
    }

    if(!EnumProcessModules(hProcess, ahModules, sizeof(ahModules), &dwNeeded))
    {
        goto error_return;
    }

    //logdbg("#Modules = %d", dwNeeded/sizeof(HMODULE));
    if(!GetModuleBaseName(hProcess, ahModules[0], pwsProcName, dwBufSize))
    {
        goto error_return;
    }

    CloseHandle(hProcess);
    return TRUE;

error_return:
    CloseHandle(hProcess);
    return FALSE;

}// GetProcNameFromID()

// Given handle to a map view of a file, return pointer to the IMAGE_NT_HEADERS structure
//
BOOL fChlPsGetNtHeaders(HANDLE hMapView, __out PIMAGE_NT_HEADERS *ppstNtHeaders)
{
    PIMAGE_DOS_HEADER pDOSHeader = NULL;

    // Argument validation
    if(!ISVALID_HANDLE(hMapView) || !ppstNtHeaders)
    {
        SetLastError(ERROR_BAD_ARGUMENTS);
        return FALSE;
    }

    pDOSHeader = (PIMAGE_DOS_HEADER)hMapView;
    
    // verify "MZ" in the DOS header
	if( ! (pDOSHeader->e_magic == IMAGE_DOS_SIGNATURE) )
	{
		SetLastError(CHLE_PROC_DOSHEADER);
        return FALSE;
	}

    *ppstNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hMapView + pDOSHeader->e_lfanew);
    return TRUE;
}

// Get a byte pointer to the start of code in a executable file(memory mapped)
//
BOOL fChlPsGetPtrToCode(
    DWORD dwFileBase, 
    PIMAGE_NT_HEADERS pNTHeaders, 
    __out PDWORD pdwCodePtr, 
    __out PDWORD pdwSizeOfData,
    __out PDWORD pdwCodeSecVirtAddr)
{

	PIMAGE_SECTION_HEADER pImgSecHeader = NULL;
	DWORD dwSecChars = 0;

    // Argument validation
    if(!pNTHeaders || !pdwCodePtr)
    {
        SetLastError(ERROR_BAD_ARGUMENTS);
        return FALSE;
    }

	// Get the .text section's header using AddressOfEntryPoint as the RVA
	if( !fChlPsGetEnclosingSectionHeader(
            pNTHeaders->OptionalHeader.AddressOfEntryPoint,
			pNTHeaders, 
            &pImgSecHeader))
	{
		SetLastError(CHLE_PROC_TEXTSECHDR);
		return FALSE;
	}

	dwSecChars = pImgSecHeader->Characteristics;

	// test if the retrieved section contains code and is executable
	if( ! ((dwSecChars & IMAGE_SCN_CNT_CODE) && (dwSecChars & IMAGE_SCN_MEM_EXECUTE)) )
	{
		SetLastError(CHLE_PROC_NOEXEC);
		return FALSE;
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

	return TRUE;

}


/* GetEnclosingSectionHeader()
 * ** From Matt Pietrek's PEDUMP.exe **
 * Given an RVA, look up the section header that encloses it and return a
 * pointer to its IMAGE_SECTION_HEADER.
 *
 * Args:
 *		
 *			
 * RetVal:
 *	
 */
BOOL fChlPsGetEnclosingSectionHeader(DWORD rva, PIMAGE_NT_HEADERS pNTHeader, __out PIMAGE_SECTION_HEADER *ppstSecHeader)
{
    int index;
    PIMAGE_SECTION_HEADER pSection = NULL;
    
    if(!pNTHeader || !ppstSecHeader)
    {
        SetLastError(ERROR_BAD_ARGUMENTS);
        return FALSE;
    }

    pSection = IMAGE_FIRST_SECTION(pNTHeader);
    
    for ( index = 0; index < pNTHeader->FileHeader.NumberOfSections; ++index, pSection++ )
    {
        // Is the RVA within this section?
        if ( (rva >= pSection->VirtualAddress) && 
             (rva < (pSection->VirtualAddress + pSection->Misc.VirtualSize)))
        {
            *ppstSecHeader = pSection;
            return TRUE;
        }
    }
    
    return FALSE;
}
