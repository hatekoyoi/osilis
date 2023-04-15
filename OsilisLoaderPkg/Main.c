#include "elf.hpp"
#include "frame_buffer_config.hpp"
#include <Guid/FileInfo.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DiskIo2.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Uefi.h>

// メモリマップ構造体
struct MemoryMap {
    // メモリマップを格納するバッファサイズ
    UINTN buffer_size;
    // メモリマップを格納するバッファへのポインタ
    VOID* buffer;
    // メモリマップのサイズ
    UINTN map_size;
    // メモリマップのキー
    UINTN map_key;
    // メモリマップエントリーのサイズ
    UINTN descriptor_size;
    // メモリマップエントリーのバージョン
    UINT32 descriptor_version;
};

// UEFIからメモリマップを取得する
EFI_STATUS
GetMemoryMap(struct MemoryMap* map) {
    if (map->buffer == NULL) {
        return EFI_BUFFER_TOO_SMALL;
    }

    map->map_size = map->buffer_size;
    return gBS->GetMemoryMap(&map->map_size,
                             (EFI_MEMORY_DESCRIPTOR*)map->buffer,
                             &map->map_key,
                             &map->descriptor_size,
                             &map->descriptor_version);
}

const CHAR16*
GetMemoryTypeUnicode(EFI_MEMORY_TYPE type) {
    switch (type) {
        case EfiReservedMemoryType:
            return L"EfiReservedMemoryType";
        case EfiLoaderCode:
            return L"EfiLoaderCode";
        case EfiLoaderData:
            return L"EfiLoaderData";
        case EfiBootServicesCode:
            return L"EfiBootServicesCode";
        case EfiBootServicesData:
            return L"EfiBootServicesData";
        case EfiRuntimeServicesCode:
            return L"EfiRuntimeServicesCode";
        case EfiRuntimeServicesData:
            return L"EfiRuntimeServicesData";
        case EfiConventionalMemory:
            return L"EfiConventionalMemory";
        case EfiUnusableMemory:
            return L"EfiUnusableMemory";
        case EfiACPIReclaimMemory:
            return L"EfiACPIReclaimMemory";
        case EfiACPIMemoryNVS:
            return L"EfiACPIMemoryNVS";
        case EfiMemoryMappedIO:
            return L"EfiMemoryMappedIO";
        case EfiMemoryMappedIOPortSpace:
            return L"EfiMemoryMappedIOPortSpace";
        case EfiPalCode:
            return L"EfiPalCode";
        case EfiPersistentMemory:
            return L"EfiPersistentMemory";
        case EfiMaxMemoryType:
            return L"EfiMaxMemoryType";
        default:
            return L"InvalidMemoryType";
    }
}

// メモリマップをファイルに保存
EFI_STATUS
SaveMemoryMap(struct MemoryMap* map, EFI_FILE_PROTOCOL* file) {
    EFI_STATUS status;
    CHAR8 buf[256];
    UINTN len;

    CHAR8* header = "Index, Type, Type(name), PhysicalStart, NumberOfPages, Attribute\n";
    len = AsciiStrLen(header);
    status = file->Write(file, &len, header);
    if (EFI_ERROR(status)) {
        return status;
    }

    Print(L"map->buffer = %08lx, map->map_size = %08lx\n", map->buffer, map->map_size);

    EFI_PHYSICAL_ADDRESS iter;
    int i;
    for (iter = (EFI_PHYSICAL_ADDRESS)map->buffer, i = 0; iter < (EFI_PHYSICAL_ADDRESS)map->buffer + map->map_size;
         iter += map->descriptor_size, i++) {
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)iter;
        len = AsciiSPrint(buf,
                          sizeof(buf),
                          "%u, %x, %-ls, %08lx, %lx, %lx\n",
                          i,
                          desc->Type,
                          GetMemoryTypeUnicode(desc->Type),
                          desc->PhysicalStart,
                          desc->NumberOfPages,
                          desc->Attribute & 0xffffflu);

        status = file->Write(file, &len, buf);
        if (EFI_ERROR(status)) {
            return status;
        }
    }

    return EFI_SUCCESS;
}

// ファイルを開く準備
EFI_STATUS
OpenRootDir(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL** root) {
    EFI_STATUS status;
    EFI_LOADED_IMAGE_PROTOCOL* loaded_image;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;

    status = gBS->OpenProtocol(image_handle,
                               &gEfiLoadedImageProtocolGuid,
                               (VOID**)&loaded_image,
                               image_handle,
                               NULL,
                               EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (EFI_ERROR(status)) {
        return status;
    }

    gBS->OpenProtocol(loaded_image->DeviceHandle,
                      &gEfiSimpleFileSystemProtocolGuid,
                      (VOID**)&fs,
                      image_handle,
                      NULL,
                      EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (EFI_ERROR(status)) {
        return status;
    }

    return fs->OpenVolume(fs, root);
}

EFI_STATUS
OpenGOP(EFI_HANDLE image_handle, EFI_GRAPHICS_OUTPUT_PROTOCOL** gop) {
    EFI_STATUS status;
    // Graphics Output Protocol(GOP)のハンドルを取得
    UINTN num_gop_handles = 0;
    EFI_HANDLE* gop_handles = NULL;
    status = gBS->LocateHandleBuffer(ByProtocol, &gEfiGraphicsOutputProtocolGuid, NULL, &num_gop_handles, &gop_handles);
    if (EFI_ERROR(status)) {
        return status;
    }

    // GOPのハンドルを使用して、グラフィックス出力プロトコルを開く
    status = gBS->OpenProtocol(gop_handles[0],
                               &gEfiGraphicsOutputProtocolGuid,
                               (VOID**)gop,
                               image_handle,
                               NULL,
                               EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (EFI_ERROR(status)) {
        return status;
    }

    // バッファ開放
    FreePool(gop_handles);

    return EFI_SUCCESS;
}

// ピクセルフォーマットの値に応じて、Unicode文字列を返す
const CHAR16*
GetPixelFormatUnicode(EFI_GRAPHICS_PIXEL_FORMAT fmt) {
    switch (fmt) {
        case PixelRedGreenBlueReserved8BitPerColor:
            return L"PixelRedGreenBlueReserved8BitPerColor";
            break;
        case PixelBlueGreenRedReserved8BitPerColor:
            return L"PixelBlueGreenRedReserved8BitPerColor";
        case PixelBitMask:
            return L"PixelBitMask";
        case PixelBltOnly:
            return L"PixelBltOnly";
        case PixelFormatMax:
            return L"PixelFormatMax";
        default:
            return L"InvalidPixelFormat";
    }
}

void
Halt(void) {
    while (1) {
        __asm__("hlt");
    }
}

// ELFファイルのロードアドレスの範囲を取得
void
CalcLoadAddressRange(Elf64_Ehdr* ehdr, UINT64* first, UINT64* last) {
    // プログラムヘッダテーブルの先頭アドレスを計算
    Elf64_Phdr* phdr = (Elf64_Phdr*)((UINT64)ehdr + ehdr->e_phoff);

    // ロードアドレスを初期化する
    *first = MAX_UINT64;
    *last = 0;

    // プログラムヘッダテーブルをループしてロードアドレスの範囲を計算
    for (Elf64_Half i = 0; i < ehdr->e_phnum; ++i) {
        if (phdr[i].p_type != PT_LOAD) {
            continue;
        }
        // ロードアドレスの範囲を更新する
        *first = MIN(*first, phdr[i].p_vaddr);
        *last = MAX(*last, phdr[i].p_vaddr + phdr[i].p_memsz);
    }
}

// ELFファイルからロードセグメントをコピー
void
CopyLoadSegments(Elf64_Ehdr* ehdr) {
    // プログラムヘッダテーブルの先頭アドレスを計算
    Elf64_Phdr* phdr = (Elf64_Phdr*)((UINT64)ehdr + ehdr->e_phoff);

    // プログラムヘッダテーブルをループしてロードセグメントをコピー
    for (Elf64_Half i = 0; i < ehdr->e_phnum; ++i) {
        if (phdr[i].p_type != PT_LOAD) {
            continue;
        }

        // ファイル上のロードセグメントの先頭アドレスを計算
        UINT64 segm_in_file = (UINT64)ehdr + phdr[i].p_offset;

        // ロードセグメントをメモリ上にコピー
        CopyMem((VOID*)phdr[i].p_vaddr, (VOID*)segm_in_file, phdr[i].p_filesz);

        // ロードセグメントの残りの部分をゼロで埋める
        UINTN remain_bytes = phdr[i].p_memsz - phdr[i].p_filesz;
        SetMem((VOID*)(phdr[i].p_vaddr + phdr[i].p_filesz), remain_bytes, 0);
    }
}

EFI_STATUS EFIAPI
UefiMain(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table) {
    EFI_STATUS status;
    Print(L"Hello, Osilis World!\n");

    // メモリマップの初期化
    CHAR8 memmap_buf[4096 * 4];
    struct MemoryMap memmap = { sizeof(memmap_buf), memmap_buf, 0, 0, 0, 0 };

    // UEFIからメモリマップを取得する
    status = GetMemoryMap(&memmap);
    if (EFI_ERROR(status)) {
        Print(L"failed to get memory map: %r\n", status);
        Halt();
    }

    // ファイルを開く準備
    EFI_FILE_PROTOCOL* root_dir;
    status = OpenRootDir(image_handle, &root_dir);
    if (EFI_ERROR(status)) {
        Print(L"failed to open root directory: %r\n", status);
        Halt();
    }

    // "memmap"を開く
    EFI_FILE_PROTOCOL* memmap_file;
    status = root_dir->Open(
        root_dir, &memmap_file, L"\\memmap", EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
    if (EFI_ERROR(status)) {
        Print(L"failed to open file '\\memmap': %r\n", status);
        Print(L"Ignored.\n");
    } else {
        // メモリマップをファイルに保存
        status = SaveMemoryMap(&memmap, memmap_file);
        if (EFI_ERROR(status)) {
            Print(L"failed to save memory map: %r\n", status);
            Halt();
        }
        status = memmap_file->Close(memmap_file);
        if (EFI_ERROR(status)) {
            Print(L"failed to close memory map: %r\n", status);
            Halt();
        }
    }

    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
    status = OpenGOP(image_handle, &gop);
    if (EFI_ERROR(status)) {
        Print(L"failed to open GOP: %r\n", status);
        Halt();
    }

    // GOP モードの情報を表示する
    Print(L"Resolution: %ux%u, Pixel Format: %s, %u pixels/line\n",
          gop->Mode->Info->HorizontalResolution,
          gop->Mode->Info->VerticalResolution,
          GetPixelFormatUnicode(gop->Mode->Info->PixelFormat),
          gop->Mode->Info->PixelsPerScanLine);
    // フレームバッファの情報を表示する
    Print(L"Frame Buffer: 0x%0lx - 0x%0lx, Size: %lu bytes\n",
          gop->Mode->FrameBufferBase,
          gop->Mode->FrameBufferBase + gop->Mode->FrameBufferSize,
          gop->Mode->FrameBufferSize);

    // フレームバッファを白色で塗りつぶす
    UINT8* frame_buffer = (UINT8*)gop->Mode->FrameBufferBase;
    for (UINTN i = 0; i < gop->Mode->FrameBufferSize; i++) {
        frame_buffer[i] = 255;
    }

    // kernelを開く
    EFI_FILE_PROTOCOL* kernel_file;
    status = root_dir->Open(root_dir, &kernel_file, L"\\kernel.elf", EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) {
        Print(L"failed to open file '\\kernel.elf': %r\n", status);
        Halt();
    }

    // kernelのサイズを取得する
    UINTN file_info_size = sizeof(EFI_FILE_INFO) + sizeof(CHAR16) * 12;
    UINT8 file_info_buffer[file_info_size];
    status = kernel_file->GetInfo(kernel_file, &gEfiFileInfoGuid, &file_info_size, file_info_buffer);
    if (EFI_ERROR(status)) {
        Print(L"failed to get file information: %r\n", status);
        Halt();
    }
    EFI_FILE_INFO* file_info = (EFI_FILE_INFO*)file_info_buffer;
    UINTN kernel_file_size = file_info->FileSize;

    // kernelの読み込みとメモリへの配置
    VOID* kernel_buffer;
    status = gBS->AllocatePool(EfiLoaderData, kernel_file_size, &kernel_buffer);
    if (EFI_ERROR(status)) {
        Print(L"failed to allocate pool: %r\n", status);
        Halt();
    }
    status = kernel_file->Read(kernel_file, &kernel_file_size, kernel_buffer);
    if (EFI_ERROR(status)) {
        Print(L"error: %r", status);
        Halt();
    }

    // LoadされたKernelのメモリアドレス範囲を計算
    Elf64_Ehdr* kernel_ehdr = (Elf64_Ehdr*)kernel_buffer;
    UINT64 kernel_first_addr, kernel_last_addr;
    CalcLoadAddressRange(kernel_ehdr, &kernel_first_addr, &kernel_last_addr);

    // ページ数を計算してメモリを割り当てる
    UINTN num_pages = (kernel_last_addr - kernel_first_addr + 0xfff) / 0x1000;
    status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, num_pages, &kernel_first_addr);
    if (EFI_ERROR(status)) {
        Print(L"failed to allocate pages: %r\n", status);
        Halt();
    }

    // Kernelを割り当てたメモリにコピーする
    CopyLoadSegments(kernel_ehdr);
    // コピーしたKernelのメモリアドレス範囲を出力する
    Print(L"Kernel: 0x%0lx - 0x%0lx\n", kernel_first_addr, kernel_last_addr);

    // Kernelを割り当てたメモリ領域以外のメモリを解放する
    status = gBS->FreePool(kernel_buffer);
    if (EFI_ERROR(status)) {
        Print(L"failed to free pool: %r\n", status);
        Halt();
    }

    // メモリマップの取得とBoot終了
    status = gBS->ExitBootServices(image_handle, memmap.map_key);
    if (EFI_ERROR(status)) {
        // メモリマップの取得に失敗した場合はエラーを表示して停止
        status = GetMemoryMap(&memmap);
        if (EFI_ERROR(status)) {
            Print(L"failed to get memory map: %r\n", status);
            Halt();
        }
        // Bootの終了を再度試みる
        status = gBS->ExitBootServices(image_handle, memmap.map_key);
        if (EFI_ERROR(status)) {
            Print(L"Could not exit boot service: %r\n", status);
            Halt();
        }
    }

    // kernelにフレーム情報を渡す
    struct FrameBufferConfig config = { (UINT8*)gop->Mode->FrameBufferBase,
                                        gop->Mode->Info->PixelsPerScanLine,
                                        gop->Mode->Info->HorizontalResolution,
                                        gop->Mode->Info->VerticalResolution,
                                        0 };
    switch (gop->Mode->Info->PixelFormat) {
        case PixelRedGreenBlueReserved8BitPerColor:
            config.pixel_format = kPixelRGBResv8BitPerColor;
            break;
        case PixelBlueGreenRedReserved8BitPerColor:
            config.pixel_format = kPixelBGRResv8BitPerColor;
            break;
        default:
            Print(L"Unimplemented pixel format: %d\n", gop->Mode->Info->PixelFormat);
            Halt();
    }

    // kernelのエントリーポイントを実行する
    UINT64 entry_addr = *(UINT64*)(kernel_first_addr + 24);
    typedef void EntryPointType(const struct FrameBufferConfig*);
    EntryPointType* entry_point = (EntryPointType*)entry_addr;
    entry_point(&config);

    Print(L"All done\n");

    while (1)
        ;
    return EFI_SUCCESS;
}
