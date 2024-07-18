// Fill out your copyright notice in the Description page of Project Settings.


#include "Util/MyUtil.h"
#include "IImageWrapperModule.h"
#include <algorithm> // for std::min_element
#include <initializer_list> // for std::initializer_list

// 添加Windows平台头文件
#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include <Windows.h>
#include <shobjidl.h> 
#include <commdlg.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

#include <Developer/GraphColor/Private/appconst.h>
#include <Shellapi.h>





MyUtil::MyUtil()
{
}

MyUtil::~MyUtil()
{
}

bool MyUtil::LoadImageToTexture(FString& ImgPath, TObjectPtr<UTexture2D>& Texture2D)
{
    if (!FPaths::FileExists(ImgPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Img Not Exist"));
        return false;
    }

    // 将图像文件加载为字节数组
    TArray<uint8> FileData;
    if (!FFileHelper::LoadFileToArray(FileData, *ImgPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Img Loading Failed"));
        return false;
    }

    // 使用图像包装器模块创建图像包装器
    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
    EImageFormat ImageFormat = ImageWrapperModule.GetImageFormatFromExtension(*ImgPath);
    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
    if (!ImageWrapper.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Img Loading Failed."));
        return false;
    }

    //解码 图片 到 FImage对象， 注意有些图片虽然是png后缀，但是确实JPG 格式，例如微信截图
    //此时会导致报错
    if (!ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num()))
    {
        //注意 解码失败 4个字后面要跟任意字符串，否则会报错，可能和UE内部宏的机制有关
        UE_LOG(LogTemp, Error, TEXT("Img获取原始数据失败!!! 路径: %s"), *ImgPath);
        return false;
    }

    // 获取解码后的原始图像数据
    TArray<uint8> RawData;
    if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData))
    {
        //注意 解码失败 4个字后面要跟任意字符串，否则会报错，可能和UE内部宏的机制有关
        UE_LOG(LogTemp, Error, TEXT("Img获取原始数据失败!!!"));
        return false;
    }

    // 获取图像宽度和高度
    int ImgWidth = ImageWrapper->GetWidth();
    int ImgHeight = ImageWrapper->GetHeight();

    // 创建一个临时的 UTexture2D 对象
    Texture2D = UTexture2D::CreateTransient(ImgWidth, ImgHeight, PF_B8G8R8A8);

    // 将解码后的原始数据复制到纹理对象中
    FTexture2DMipMap& Mip = Texture2D->GetPlatformData()->Mips[0];
    void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
    FMemory::Memcpy(Data, RawData.GetData(), RawData.Num());
    Mip.BulkData.Unlock();

    // 更新纹理资源
    Texture2D->UpdateResource();

    return true;
}

bool MyUtil::FindMatchingFile(FString& WildcardPath, FString& OutFoundPath)
{
    // 兼容多种图片扩展名
    TArray<FString> FileExtensions = { TEXT("png"), TEXT("jpg"), TEXT("jpeg") };

    WIN32_FIND_DATA FindFileData;
    for (const FString& Extension : FileExtensions)
    {
        FString CurrentWildcardPath = WildcardPath + TEXT("*.") + Extension;

        HANDLE hFind = FindFirstFile(*CurrentWildcardPath, &FindFileData);

        if (hFind != INVALID_HANDLE_VALUE)
        {
            OutFoundPath = FPaths::GetPath(WildcardPath) / FindFileData.cFileName;
            FindClose(hFind);
            return true;
        }
    }

    return false;
}

FLinearColor MyUtil::OpenColorPickerDialog()
{
#if PLATFORM_WINDOWS

    // 设置默认颜色为 sRGB = 81D8D0FF (蒂芙尼蓝)
    FLinearColor DefaultColor = FLinearColor::FromSRGBColor(FColor(0x81, 0xD8, 0xD0, 0xFF));
    CHOOSECOLORA cc;
    static COLORREF acrCustClr[16]; // array of custom colors
    static DWORD rgbCurrent;        // initial color selection

    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = GetActiveWindow();
    cc.lpCustColors = (LPDWORD)acrCustClr;
    cc.rgbResult = rgbCurrent;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;

    if (ChooseColorA(&cc) == TRUE)
    {
        rgbCurrent = cc.rgbResult;
        FColor PickedColor(GetRValue(rgbCurrent), GetGValue(rgbCurrent), GetBValue(rgbCurrent), 255);
        return FLinearColor(PickedColor);
    }

    return DefaultColor;
#else
    // 在非 Windows 平台上返回默认颜色
    return FLinearColor::FromSRGBColor(FColor(0x81, 0xD8, 0xD0, 0xFF));
#endif
}

FString MyUtil::SelDirPath()
{
    FString DirPath;

    // Initialize COM library
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        IFileOpenDialog* pFileOpen;

        // Create the FileOpenDialog object
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr))
        {
            // Set options to select folders
            DWORD dwOptions;
            pFileOpen->GetOptions(&dwOptions);
            pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);

            // Show the Open dialog box
            hr = pFileOpen->Show(NULL);

            // Get the file name from the dialog box
            if (SUCCEEDED(hr))
            {
                IShellItem* pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    // Convert the wchar_t* to FString
                    if (SUCCEEDED(hr))
                    {
                        DirPath = FString(pszFilePath);
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }
        CoUninitialize();
    }
    return DirPath;
}

FString MyUtil::SelFilePath()
{
    FString FilePath;

    // Initialize COM library
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        IFileOpenDialog* pFileOpen;

        // Create the FileOpenDialog object
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr))
        {
            // Set options to select files (default, so no need to change options)
            DWORD dwOptions;
            pFileOpen->GetOptions(&dwOptions);
            pFileOpen->SetOptions(dwOptions & ~FOS_PICKFOLDERS);  // Ensure FOS_PICKFOLDERS is not set

            // Set the file types to display (jpg, jpeg, png)
            COMDLG_FILTERSPEC rgSpec[] =
            {
                { L"Image Files", L"*.jpg;*.jpeg;*.png" },
            };
            pFileOpen->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);
            pFileOpen->SetFileTypeIndex(1);  // Select the filter index (1-based index)

            // Show the Open dialog box
            hr = pFileOpen->Show(NULL);

            // Get the file name from the dialog box
            if (SUCCEEDED(hr))
            {
                IShellItem* pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    // Convert the wchar_t* to FString
                    if (SUCCEEDED(hr))
                    {
                        FilePath = FString(pszFilePath);
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }
        CoUninitialize();
    }
    return FilePath;
}



int32 MyUtil::GetImgNumFromPath(const FString& Path, const TArray<FString>& ImgExt)
{
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    int32 ImageCount = 0;

    // 遍历指定目录下的文件
    TArray<FString> FileNames;
    PlatformFile.FindFiles(FileNames, *Path, nullptr);

    for (const FString& FileName : FileNames)
    {
        // 获取文件扩展名
        FString FileExtension = FPaths::GetExtension(FileName);

        // 检查是否为指定类型的图片
        for (const FString& Extension : ImgExt)
        {
            if (FileExtension.Equals(Extension, ESearchCase::IgnoreCase))
            {
                ImageCount++;
                break;
            }
        }
    }

    return ImageCount;
}

int32 MyUtil::GetMinValue(std::initializer_list<int32> values)
{
    return *std::min_element(values.begin(), values.end());
}

bool MyUtil::ShowVirKeyboard()
{
    // 获取当前前台窗口句柄
    HWND hWnd = GetForegroundWindow();
    if (hWnd == nullptr)
    {
        MessageBox(NULL, TEXT("无法获取前台窗口句柄"), TEXT("错误"), MB_OK | MB_ICONWARNING);
        return false;
    }

    // 启动触摸键盘程序
    SHELLEXECUTEINFO sei = { sizeof(sei) };
    sei.cbSize = sizeof(sei);
    sei.lpVerb = TEXT("runas"); // 请求使用管理员权限运行
    sei.lpFile = TEXT("C:\\Windows\\System32\\osk.exe"); // 使用 osk.exe 作为替代方案
    sei.nShow = SW_SHOW;

    if (!ShellExecuteEx(&sei))
    {
        DWORD error = GetLastError();
        if (error == ERROR_CANCELLED)
        {
            MessageBox(NULL, TEXT("用户取消了管理员权限请求"), TEXT("错误"), MB_OK | MB_ICONWARNING);
        }
        else
        {
            MessageBox(NULL, TEXT("无法启动触摸键盘程序"), TEXT("错误"), MB_OK | MB_ICONWARNING);
        }
        return false;
    }

    // 等待触摸键盘程序启动并获取其窗口句柄
    Sleep(1000); // 等待一秒钟以确保触摸键盘窗口已经创建
    HWND hKeyboardWnd = FindWindowW(TEXT("OSKMainClass"), NULL);
    if (hKeyboardWnd != nullptr)
    {
        // 将触摸键盘窗口置于前台
        SetForegroundWindow(hKeyboardWnd);
    }

    return true;
}

//传入路径然后返回路径 下的所有图片名称到数组
TArray<FString> MyUtil::GetImageFiles(const FString& Directory)
{
    TArray<FString> ArrFiles;
    IFileManager& FileManager = IFileManager::Get();

    // 添加多种图片格式的搜索规则
    TArray<FString> ArrImageExt = { TEXT("*.png"), TEXT("*.jpg"), TEXT("*.jpeg") };

    for (const FString& ImageExt : ArrImageExt)
    {
        FString Path = Directory / ImageExt;
        FileManager.FindFiles(ArrFiles, *Path, true, false);
    }

    // 拼接完整的文件路径
    for (FString& File : ArrFiles)
    {
        File = Directory / File;
    }

    return ArrFiles;

}

int32 MyUtil::ExtractNumFromStr(const FString& InputString)
{
    FString NumberString;

    // 遍历字符串，提取数字部分
    for (TCHAR Char : InputString)
    {
        if (FChar::IsDigit(Char))
        {
            NumberString.AppendChar(Char);
        }
        else
        {
            break;
        }
    }

    // 将提取的数字字符串转换为 int32 类型
    int32 Result = FCString::Atoi(*NumberString);
    return Result;
}

