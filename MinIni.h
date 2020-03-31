#ifndef _MININI_H
#define _MININI_H

#include <string>
#include <map>
#include <istream>
#include <fstream>
#include <sstream>

template<
	class _StringT = std::string,
	class _SStreamT = std::basic_stringstream<_StringT::value_type>,
	class _IStreamT = std::basic_istream<_StringT::value_type>,
	class _OStreamT = std::basic_ostream<_StringT::value_type>,
	class _FStreamT = std::basic_fstream<_StringT::value_type> >
class MinIni
{
private:
	String mFilename;
	bool mAutoSync;

public:
	using String		= _StringT;
	using StringStream	= _SStreamT;
	using InputStream	= _IStreamT;
	using OutputStream	= _OStreamT;
	using FileStream	= _FStreamT;

	std::map<String, std::map<String, String> > dataMap;// dataMap[section][key] = value

	MinIni() {}

	/*
	@param filename The name of the file to open
	@param autosync If enabled, the file will automatically be synced to this MinIni's content before being closed
	*/
	MinIni(String filename, bool autosync) {
		open(filename, autosync);
	}

	~MinIni() {
		close();
	}

	String filename() const {
		return mFilename;
	}

	bool autoSyncEnabled() const {
		return mAutoSync;
	}

	// Returns the String value if it exists. If not, inserts the default value (def) and returns it
	String getStr(String sect, String key, String def) {
		auto& keyvalmap = dataMap[sect];

		auto it = keyvalmap.find(key);
		if (it == keyvalmap.end()) {
			keyvalmap.insert(std::make_pair(key, def));
			return def;
		}
		else {
			return it->second;
		}
	}

	// Sets the value to the String
	void setStr(String sect, String key, String val) {
		dataMap[sect][key] = val;
	}

	// Returns the value if it exists. If not, inserts the default value (def) and returns it. val must be streamable to and from a StringStream.
	template<class _T>
	T get(String sect, String key, _T def = _T()) {
		StringStream ss;
		auto& keyvalmap = dataMap[sect];

		auto it = keyvalmap.find(key);
		if (it == keyvalmap.end()) {
			ss << def;
			keyvalmap.insert(std::make_pair(key, ss.str()));
			return def;
		}
		else {
			ss << it->second;
			ss >> ret;
			return ret;
		}
	}

	// Sets the value to val. val must be streamable to a StringStream.
	template<class _T>
	void set(String sect, String key, _T val) {
		StringStream ss;
		ss << val;
		setStr(sect, key, ss.str());
	}

	bool exists(String sect, String key) const {
		auto it = dataMap.find(sect);
		return (it != dataMap.end() && it->second.find(key) != it->second.end());
	}

	void read(InputStream& is) {
		dataMap.clear();

		String sect;
		String ln;
		while (std::getline(is, ln)) {
			ln.erase(0, ln.find_first_not_of(L" \t"));
			ln.erase(ln.find_last_not_of(L" \t") + 1);
			if (ln.find_first_of(L"#") != String::npos) {
				ln.erase(ln.find_first_of(L"#") + 1);
			}

			if (ln.length()) {
				if (ln[0] == '[') {
					sect = ln.substr(1, ln.find_first_of(']') - 1);
					sect.erase(0, sect.find_first_not_of(L" \t"));
					sect.erase(sect.find_last_not_of(L" \t") + 1);
					dataMap[sect];
				}
				else {
					size_t eqpos = ln.find_first_of('=');
					String key = ln.substr(0, eqpos);
					key.erase(key.find_last_not_of(L" \t") + 1);
					String val = ln.substr(eqpos + 1, String::npos);
					val.erase(0, val.find_first_not_of(L" \t"));

					dataMap[sect][key] = val;
				}
			}
		}
	}

	void write(OutputStream& is) const {
		for (auto& sect : dataMap) {
			if (sect.first != L"") {
				file << "[" << sect.first << "]\n";
			}
			for (auto& keyval : sect.second) {
				file << keyval.first << " = " << keyval.second << "\n";
			}
		}
	}

	/*
	@param filename The name of the file to open
	@param autosync If enabled, the file will automatically be synced to this MinIni's content before being closed
	*/
	void open(String filename, bool autosync) {
		mFilename = filename;
		mAutoSync = autosync;

		FStream file;
		file.open(filename, std::ios::in);

		if (file.good()) {
			read(file);
			file.close();
		}
	}

	void sync() const {
		FileStream file;
		file.open(mFilename, std::ios::out);

		if (file.good()) {
			write(file);
			file.close();
		}
	}

	void close() {
		if (mFilename.length() && mAutoSync) {
			sync();
		}
		inistruct.clear();
		mFilename = String();
	}
};

#endif