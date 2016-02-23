#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <locale>
#include <cstdint>
#include <codecvt>
#include "arithmetic_convert.hpp"
#include "input.hpp"
struct info {
	std::uint32_t cid;
	std::uint32_t foid;
	std::wstring filename;
};
template<typename char_type1, typename char_type2>
void download(const std::basic_string<char_type1>& url, const std::basic_string<char_type2>& out_filename, const char* cokie, char_cvt::char_enc type)
{
	const char* const header_base = R"(--header "Host: cybozulive.com" --header "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:44.0) Gecko/20100101 Firefox/44.0" --header "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" --header "Accept-Language: ja,en-US;q=0.7,en;q=0.3" --header "Connection: keep-alive")";
	const std::string cmd = std::string("curl") + ' ' + header_base + R"( --header "Cookie: )" + cokie + R"(" -L ")" + char_cvt::to_string(url, type) + R"(" -o ")" + char_cvt::to_string(out_filename, type) + "\"";
	std::cout << "downloading " << char_cvt::to_string(out_filename, type) << std::endl;
	std::system(cmd.c_str());//exec
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
}
template<typename char_type>
std::vector<std::basic_string<char_type>> list(std::basic_string<char_type> const & text, std::basic_regex<char_type> const & re)
{
	std::vector<std::basic_string<char_type>> result;
	std::match_results<typename std::basic_string<char_type>::const_iterator> m; // match_results
	std::regex_search(text, m, re);
	for (auto && elem : m) {// elemはsub_match
		result.push_back(elem.str());
	}
	return result;
}
template<typename char_type>
std::wstring extract_filename(const std::basic_string<char_type>& filename)
{
	std::wregex reg(LR"(<span class="fileName">(.+)</span>)");
	std::wifstream file(filename);
	file.imbue(std::locale(std::locale(), new std::codecvt_utf8_utf16<wchar_t>));
	for (std::wstring buf; std::getline(file, buf);) {
		const auto r = list(buf, reg);
		if (r.empty()) continue;
		return r[1];
	}
	return{};
}
bool extract_cid_foid(std::vector<info>& re, const std::string& filename, const wchar_t* group_id, const char* cokie, char_cvt::char_enc type)
{
	using std::uint32_t;
	bool re_flag = false;
	std::wregex reg(LR"(gwCabinet/view\?cid=(\d+)&amp;coffset=(\d+)&amp;currentFolderId=(\d+))");
	std::wifstream file(filename);
	file.imbue(std::locale(std::locale(), new std::codecvt_utf8_utf16<wchar_t>));
	for (std::wstring buf; std::getline(file, buf);) {
		const auto r = list(buf, reg);
		if (r.empty()) {
			if (std::wstring::npos != buf.find(L"末尾へ")) {
				if (std::wstring::npos == buf.find(L"disable")) re_flag = true;
			}
			continue;
		}
		info tmp;
		tmp.cid = atithmetic_cvt::from_str<uint32_t>(r[1]);
		tmp.foid = atithmetic_cvt::from_str<uint32_t>(r[3]);
		const auto url = std::wstring(L"https://cybozulive.com/") + group_id + L"/gwCabinet/view?cid=" + r[1] + L"&coffset=" + r[2] + L"&currentFolderId=" + r[3];
		const auto fname = L"fid" + r[3] + L"coffset" + r[2] + L"cid" + r[1] + L".html";
		download(url, fname, cokie, type);
		tmp.filename = extract_filename(fname);
		re.push_back(std::move(tmp));
	}
	return re_flag;
}
std::vector<std::pair<std::uint32_t, std::wstring>> extract_foid_fname(const char* filename)
{
	std::vector<std::pair<std::uint32_t, std::wstring>> re;
	std::wregex reg(LR"(gwCabinet/list\?currentFolderId=(\d+).*" class="iconLink categoryLink" title="(.+).*")");
	std::wifstream file(filename);
	file.imbue(std::locale(std::locale(), new std::codecvt_utf8_utf16<wchar_t>));
	for (std::wstring buf; std::getline(file, buf);) {
		const auto r = list(buf, reg);
		if (r.empty()) continue;
		re.emplace_back(atithmetic_cvt::from_str<uint32_t>(r[1]), r[2]);
	}
	return re;
}
std::string make_filelist_utl(const std::wstring& group_id, uint32_t folder_id, uint32_t coffset, char_cvt::char_enc type)
{
	return "https://cybozulive.com/" + char_cvt::to_string(group_id, type) + "/gwCabinet/ajax/listAjax?currentFolderId=" + std::to_string(folder_id) + "&csort=2&csortOrder=0&coffset=" + std::to_string(coffset) + "&dummy=1456150280879";
}
std::vector<std::string> split(const std::string &str, char delim)
{
	std::vector<std::string> res;
	std::size_t current = 0, found;
	while ((found = str.find_first_of(delim, current)) != std::string::npos) {
		res.emplace_back(str, current, found - current);
		current = found + 1;
	}
	res.emplace_back(str, current, str.size() - current);
	return res;
}
std::vector<int> select_dl_folder(const std::vector<std::pair<std::uint32_t, std::wstring>>& foid_fname, char_cvt::char_enc type)
{
	std::vector<int> re;
	int n = 0;
	for (auto&& f : foid_fname) {
		std::cout << n << " : " << char_cvt::to_string(f.second, type) << std::endl;
		++n;
	}
	std::cout << "select download folder (0-" << n << ") ex.)0, 3, 5" << std::endl;
	std::string buf;
	//input
	std::cin >> buf;
	//std::getline(std::cin, buf);
	const auto tmp = split(buf, ',');
	std::vector<int> dl_num;
	dl_num.reserve(tmp.size());
	for (const auto& s : tmp) dl_num.push_back(std::stoi(s));
	//make unique
	std::sort(dl_num.begin(), dl_num.end());
	dl_num.erase(std::unique(dl_num.begin(), dl_num.end()), dl_num.end());
	//append
	re.reserve(dl_num.size());
	for (auto& d : dl_num) re.push_back(d);
	return re;
}
int main(int argc, char* argv[])
{
	if (argc != 2) return -1;
	std::wcout.imbue(std::locale());
	const wchar_t* const group_id = L"";
	const char* const cokie = "";
	//input char encode type
	const auto type = static_cast<char_cvt::char_enc>(input("char encode (1=UTF-8, 2=Shift-JIS)", 2, 1));
	//load folder list
	const auto foid_fname = extract_foid_fname(argv[1]);
	std::vector<int> dl_folder = select_dl_folder(foid_fname, type);
	std::unordered_map<std::wstring, std::vector<info>> map;
	for (auto f : dl_folder) {
		const std::string new_folder_name = std::to_string(foid_fname[f].first);
		std::system(("mkdir " + new_folder_name).c_str());
		std::system(("cd " + new_folder_name).c_str());
		bool download_next = true;
		std::vector<info> tmp;
		for (uint32_t i = 0; download_next; i += 20) {
			std::string filename = std::to_string(i) + ".html";
			download(make_filelist_utl(group_id, foid_fname[f].first, i, type), filename, cokie, type);
			download_next = extract_cid_foid(tmp, filename, group_id, cokie, type);//append
		}
		map[foid_fname[f].second] = std::move(tmp);
		std::system("cd ../");
	}
	std::cout << "writing list...";
	std::ofstream out("cid_list.txt");
	for (auto& m : map) {
		out << char_cvt::wstring2string(m.first, char_cvt::char_enc::utf8) << std::endl;
		for (auto info : m.second) {
			out 
				<< "folder_id:" << info.foid 
				<< " cid:" << info.cid 
				<< " filename:" << char_cvt::wstring2string(info.filename, char_cvt::char_enc::utf8) 
				<< std::endl;
		}
	}
	std::cout << "done." << std::endl;
	return 0;
}
