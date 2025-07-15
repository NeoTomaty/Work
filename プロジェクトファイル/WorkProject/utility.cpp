#include	<filesystem>
#include	<string>
#include	<Windows.h>
#include	<assimp/scene.h>
#include	<SimpleMath.h>

namespace utility {
    // std::string 用のディレクトリ取得関数
    std::filesystem::path get_directory(const std::string& path) {
        return std::filesystem::path(path).parent_path();
    }

    // std::u8string 用のディレクトリ取得関数
    std::filesystem::path get_directory(const std::u8string& path) {
        return std::filesystem::path(path).parent_path();
    }

    // std::wstring 用のディレクトリ取得関数
    std::filesystem::path get_directory(const std::wstring& path) {
        return std::filesystem::path(path).parent_path();
    }

	// ワイド文字(utf16)をｓ－ｊｉｓに
	std::string wide_to_multi_winapi(std::wstring const& src)
	{
		auto const dest_size = ::WideCharToMultiByte(
			CP_ACP,
			0U,
			src.data(),
			-1,
			nullptr,
			0,
			nullptr,
			nullptr);
		std::vector<char> dest(dest_size, '\0');
		if (::WideCharToMultiByte(
			CP_ACP,
			0U,
			src.data(),
			-1,
			dest.data(),
			static_cast<int>(dest.size()),
			nullptr,
			nullptr) == 0) {
			throw std::system_error{ static_cast<int>(::GetLastError()), std::system_category() };
		}
		dest.resize(std::char_traits<char>::length(dest.data()));
		dest.shrink_to_fit();
		return std::string(dest.begin(), dest.end());
	}

	// utf-8をワイド文字(utf-16)に
	std::wstring utf8_to_wide_winapi(std::string const& src)
	{
		auto const dest_size = ::MultiByteToWideChar(
			CP_UTF8,			 // ソース側がUTF-8
			0U,
			src.data(),
			-1,
			nullptr,
			0U);
		std::vector<wchar_t> dest(dest_size, L'\0');
		if (::MultiByteToWideChar(CP_UTF8, 0U, src.data(), -1, dest.data(), static_cast<int>(dest.size())) == 0) {
			throw std::system_error{ static_cast<int>(::GetLastError()), std::system_category() };
		}
		dest.resize(std::char_traits<wchar_t>::length(dest.data()));
		dest.shrink_to_fit();
		return std::wstring(dest.begin(), dest.end());
	}

	// utf8をS-JISに
	std::string utf8_to_multi_winapi(std::string const& src)
	{
		auto const wide = utf8_to_wide_winapi(src);
		return wide_to_multi_winapi(wide);
	}


	DirectX::SimpleMath::Matrix aiMtxToDxMtx(aiMatrix4x4 asimpmtx) 
	{
		DirectX::SimpleMath::Matrix dxmtx;

		dxmtx._11 = asimpmtx.a1; dxmtx._12 = asimpmtx.b1; dxmtx._13 = asimpmtx.c1; dxmtx._14 = asimpmtx.d1;
		dxmtx._21 = asimpmtx.a2; dxmtx._22 = asimpmtx.b2; dxmtx._23 = asimpmtx.c2; dxmtx._24 = asimpmtx.d2;
		dxmtx._31 = asimpmtx.a3; dxmtx._32 = asimpmtx.b3; dxmtx._33 = asimpmtx.c3; dxmtx._34 = asimpmtx.d3;
		dxmtx._41 = asimpmtx.a4; dxmtx._42 = asimpmtx.b4; dxmtx._43 = asimpmtx.c4; dxmtx._44 = asimpmtx.d4;
		return dxmtx;
	}
}

