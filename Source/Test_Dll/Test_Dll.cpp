#pragma comment(linker, "/ENTRY:DllMain")
#pragma comment(linker, "/SECTION:.text,ERW /MERGE:.rdata=.text /MERGE:.data=.text")
#pragma comment(linker, "/SECTION:.Amano,ERW /MERGE:.text=.Amano")

#pragma comment(linker, "/EXPORT:MyMessageBoxEx=XunYouCommonDLL2.MyMessageBoxEx")
#pragma comment(linker, "/EXPORT:isGameInclude=XunYouCommonDLL2.isGameInclude")
#pragma comment(linker, "/EXPORT:xunyou_login=XunYouCommonDLL2.xunyou_login")

#include "MyLibrary.cpp"
#include <Windns.h>

OVERLOAD_CPP_NEW_WITH_HEAP(MemoryAllocator::GetGlobalHeap())

TYPE_OF(NtQueryDirectoryFile)*      StubNtQueryDirectoryFile;
TYPE_OF(NtQueryAttributesFile)*     StubNtQueryAttributesFile;
TYPE_OF(NtQueryFullAttributesFile)* StubNtQueryFullAttributesFile;

NTSTATUS
NTAPI
TGitNtQueryDirectoryFile(
    HANDLE                  FileHandle,
    HANDLE                  Event  OPTIONAL,
    PIO_APC_ROUTINE         ApcRoutine  OPTIONAL,
    PVOID                   ApcContext  OPTIONAL,
    PIO_STATUS_BLOCK        IoStatusBlock,
    PVOID                   FileInformation,
    ULONG                   Length,
    FILE_INFORMATION_CLASS  FileInformationClass,
    BOOLEAN                 ReturnSingleEntry,
    PUNICODE_STRING         FileName  OPTIONAL,
    BOOLEAN                 RestartScan
)
{
    NTSTATUS Status;

    Status = StubNtQueryDirectoryFile(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, FileInformation, Length, FileInformationClass, ReturnSingleEntry, FileName, RestartScan);
    if (NT_SUCCESS(Status) && FileInformationClass == FileBothDirectoryInformation)
    {
        PFILE_BOTH_DIR_INFORMATION FileInfo;

        FileInfo = (PFILE_BOTH_DIR_INFORMATION)FileInformation;

        if (FileInfo != NULL)
        {
            CLEAR_FLAG(FileInfo->FileAttributes, FILE_ATTRIBUTE_REPARSE_POINT);
        }
    }

    return Status;
}

NTSTATUS
NTAPI
TGitNtQueryAttributesFile(
    POBJECT_ATTRIBUTES      ObjectAttributes,
    PFILE_BASIC_INFORMATION FileInformation
)
{
    NTSTATUS Status;

    Status = StubNtQueryAttributesFile(ObjectAttributes, FileInformation);

    if (NT_SUCCESS(Status) && FileInformation != NULL)
    {
        CLEAR_FLAG(FileInformation->FileAttributes, FILE_ATTRIBUTE_REPARSE_POINT);
    }

    return Status;
}

NTSTATUS
NTAPI
TGitNtQueryFullAttributesFile(
    POBJECT_ATTRIBUTES              ObjectAttributes,
    PFILE_NETWORK_OPEN_INFORMATION  FileInformation
)
{
    NTSTATUS Status;

    Status = StubNtQueryFullAttributesFile(ObjectAttributes, FileInformation);

    if (NT_SUCCESS(Status))
    {
        CLEAR_FLAG(FileInformation->FileAttributes, FILE_ATTRIBUTE_REPARSE_POINT);
    }

    return Status;
}

#if ML_AMD64

TYPE_OF(DnsQuery_W)*    StubDnsQuery_W;
TYPE_OF(DnsFree)*       StubDnsFree;

EXTC
DNS_STATUS
WINAPI
GitDnsQuery_W(
    PCWSTR          pszName,
    WORD            wType,
    DWORD           Options,
    PVOID           pExtra,
    PDNS_RECORD*   ppQueryResults,
    PVOID *         pReserved
)
{
    return StubDnsQuery_W(pszName, wType, Options, pExtra, ppQueryResults, pReserved);
}

EXTC
VOID
WINAPI
GitDnsFree(
    PVOID           pData,
    DNS_FREE_TYPE   FreeType
)
{
    StubDnsFree(pData, FreeType);
}

#endif

struct LolPacket
{
    BOOL THISCALL RecvPacket(ULONG unk, ULONG type1, ULONG type2, ULONG len, PBYTE buf);
};

TYPE_OF(&LolPacket::RecvPacket) StubRecvPacket;

BOOL THISCALL LolPacket::RecvPacket(ULONG unk, ULONG type1, ULONG type2, ULONG len, PBYTE buf)
{
    LOOP_ONCE
    {
        WCHAR file[MAX_NTPATH];
        NTSTATUS st;
        LARGE_INTEGER cnt;
        NtFileDisk bin;

        if (buf == NULL || len == 0)
            break;

        RtlQueryPerformanceCounter(&cnt);
        swprintf(file, L"C:\\lelog\\ts=%I64d_op=%X_len=%X_t1=%d_t2=%d", cnt.QuadPart, buf[0], len, type1, type2);

        st = bin.Create(file);
        FAIL_BREAK(st);

        bin.Write(buf, len);
    }

    return (this->*StubRecvPacket)(unk, type1, type2, len, buf);
}

VOID WriteBin(NtFileDisk &file, PVOID buf, ULONG_PTR len)
{
    SEH_TRY
    {
        file.Write(buf, len);
    }
    SEH_EXCEPT(1)
    {
    }
}

EXTC LRESULT NTAPI LbSendMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    NTSTATUS st;
    LRESULT Result;
    LARGE_INTEGER cnt;
    WCHAR file[MAX_NTPATH];

    Result = SendMessageW(hWnd, Msg, wParam, lParam);

    switch (Msg)
    {
        case 0x159C:
        {
            if (Result == 0x63)
                break;

                RtlQueryPerformanceCounter(&cnt);
                swprintf(file, L"C:\\lelog\\%I64d", cnt.QuadPart);

                NtFileDisk bin;

                st = bin.Create(file);
                FAIL_BREAK(st);

                WriteBin(bin, (PVOID)lParam, HIWORD(wParam));
        }
        break;
    }

    return Result;
}

HFONT NTAPI gCreateFontW(_In_ int cHeight, _In_ int cWidth, _In_ int cEscapement, _In_ int cOrientation, _In_ int cWeight, _In_ DWORD bItalic, _In_ DWORD bUnderline, _In_ DWORD bStrikeOut, _In_ DWORD iCharSet, _In_ DWORD iOutPrecision, _In_ DWORD iClipPrecision, _In_ DWORD iQuality, _In_ DWORD iPitchAndFamily, _In_opt_ LPCWSTR pszFaceName)
{
/*
    cHeight -= 28;
    if (cHeight <= 0)
        cHeight = 8;
*/
    return CreateFontW(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, L"����");
}

TYPE_OF(&NtDeviceIoControlFile) StubNtDeviceIoControlFile;

NTSTATUS
NTAPI
XyDeviceIoControlFile(
    HANDLE              FileHandle,
    HANDLE              Event,
    PIO_APC_ROUTINE     ApcRoutine,
    PVOID               ApcContext,
    PIO_STATUS_BLOCK    IoStatusBlock,
    ULONG               IoControlCode,
    PVOID               InputBuffer,
    ULONG               InputBufferLength,
    PVOID               OutputBuffer,
    ULONG               OutputBufferLength
)
{
    PSTR                        SerialNumber;
    NTSTATUS                    Status, Status2;
    IO_STATUS_BLOCK             IoStatus;
    FILE_FS_DEVICE_INFORMATION  FsDevice;
    PSENDCMDINPARAMS            SendCmdInParams;
    PSENDCMDOUTPARAMS           SendCmdOutParams;

    Status = StubNtDeviceIoControlFile(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, IoControlCode, InputBuffer, InputBufferLength, OutputBuffer, OutputBufferLength);
    FAIL_RETURN(Status);

    if (IoControlCode != SMART_RCV_DRIVE_DATA)
        return Status;

    SendCmdInParams = (PSENDCMDINPARAMS)InputBuffer;
    SendCmdOutParams = (PSENDCMDOUTPARAMS)OutputBuffer;

    if (SendCmdInParams == nullptr ||
        InputBufferLength < sizeof(*SendCmdInParams) - sizeof(SendCmdInParams->bBuffer)
       )
    {
        return Status;
    }

    if (SendCmdOutParams == nullptr ||
        OutputBufferLength < IDENTIFY_BUFFER_SIZE)
    {
        return Status;
    }

    if (SendCmdInParams->irDriveRegs.bFeaturesReg != 0)
        return Status;

    Status2 = ZwQueryVolumeInformationFile(FileHandle, &IoStatus, &FsDevice, sizeof(FsDevice), FileFsDeviceInformation);
    if (NT_FAILED(Status2))
        return Status;

    if (FsDevice.DeviceType != FILE_DEVICE_DISK)
        return Status;

    // typedef struct _IDSECTOR

    SerialNumber = (PSTR)PtrAdd(SendCmdOutParams->bBuffer, 0x14);

    for (ULONG_PTR Count = 20; Count != 0; --Count)
    {
        *SerialNumber++ = GetRandom32Range('A', 'Z');
        YieldProcessor();
    }

    return Status;
}

BOOL Initialize(PVOID BaseAddress)
{
    LdrAddRefDll(LDR_ADDREF_DLL_PIN, BaseAddress);
    LdrDisableThreadCalloutsForDll(BaseAddress);

    MEMORY_FUNCTION_PATCH f[] = 
    {
        INLINE_HOOK_JUMP(ZwDeviceIoControlFile, XyDeviceIoControlFile, StubNtDeviceIoControlFile),
    };

    Nt_PatchMemory(0, 0, f, countof(f));

    return TRUE;
}

BOOL CDECL UnInitialize(PVOID BaseAddress)
{
    ml::MlUnInitialize();

    return FALSE;
}

BOOL WINAPI DllMain(PVOID BaseAddress, ULONG Reason, PVOID Reserved)
{
    switch (Reason)
    {
        case DLL_PROCESS_ATTACH:
            return Initialize(BaseAddress) || UnInitialize(BaseAddress);

        case DLL_PROCESS_DETACH:
            UnInitialize(BaseAddress);
            break;
    }

    return TRUE;
}

#if 0

#pragma comment(linker, "/EXPORT:Direct3DCreate9=_Direct3DCreate9@4")
#pragma comment(linker, "/EXPORT:D3DPERF_SetMarker=_D3DPERF_SetMarker@8")
#pragma comment(linker, "/EXPORT:D3DPERF_BeginEvent=_D3DPERF_BeginEvent@8")
#pragma comment(linker, "/EXPORT:D3DPERF_EndEvent=_D3DPERF_EndEvent@0")

#include <d3d9.h>

PVOID GetD3DRoutine(PCSTR Name)
{
    ULONG           Length;
    NTSTATUS        Status;
    PVOID           hModule;
    WCHAR           szPath[MAX_NTPATH];
    UNICODE_STRING  DllPath;

    static WCHAR D3d9Dll[] = L"d3d9.dll";

    Length = Nt_GetSystemDirectory(szPath, countof(szPath));

    CopyStruct(szPath + Length, D3d9Dll, sizeof(D3d9Dll));
    DllPath.Buffer = szPath;
    DllPath.Length = (USHORT)((Length + CONST_STRLEN(D3d9Dll)) * sizeof(WCHAR));
    DllPath.MaximumLength = DllPath.Length;

    Status = LdrLoadDll(NULL, 0, &DllPath, &hModule);
    if (!NT_SUCCESS(Status))
        return NULL;

    LdrAddRefDll(LDR_ADDREF_DLL_PIN, hModule);

    return Nt_GetProcAddress(hModule, Name);
}

EXTC IDirect3D9* STDCALL Direct3DCreate9(UINT SDKVersion)
{
    static IDirect3D9* (STDCALL *pfDirect3DCreate9)(UINT SDKVersion);

    if (pfDirect3DCreate9 == NULL)
    {
        *(PVOID *)&pfDirect3DCreate9 = GetD3DRoutine("Direct3DCreate9");
        if (pfDirect3DCreate9 == NULL)
            return NULL;

        Initialize(NULL);
    }

    return pfDirect3DCreate9(SDKVersion);
}

EXTC VOID WINAPI D3DPERF_SetMarker(D3DCOLOR Color, PCWSTR Name)
{
    static VOID (STDCALL *pfD3DPERF_SetMarker)(D3DCOLOR Color, PCWSTR Name);

    if (pfD3DPERF_SetMarker == NULL)
    {
        *(PVOID *)&pfD3DPERF_SetMarker = GetD3DRoutine("D3DPERF_SetMarker");
    }

    pfD3DPERF_SetMarker(Color, Name);
}

EXTC int WINAPI D3DPERF_BeginEvent( D3DCOLOR col, LPCWSTR wszName )
{
    static TYPE_OF(D3DPERF_BeginEvent)* BeginEvent;

    if (BeginEvent == NULL)
    {
        *(PVOID *)&BeginEvent = GetD3DRoutine("D3DPERF_BeginEvent");
    }

    return BeginEvent(col, wszName);
}

EXTC int WINAPI D3DPERF_EndEvent( void )
{
    static TYPE_OF(D3DPERF_EndEvent)* EndEvent;

    if (EndEvent == NULL)
    {
        *(PVOID *)&EndEvent = GetD3DRoutine("D3DPERF_EndEvent");
    }

    return EndEvent();
}

#endif