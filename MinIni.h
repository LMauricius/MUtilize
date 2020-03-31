/*
Made by Mauricius

Part of my MUtilize repo: https://github.com/LegendaryMauricius/MUtilize
*/

#pragma once
#ifndef _MININI_H
#define _MININI_H

#include <string>
#include <map>
#include <istream>
#include <fstream>
#include <sstream>
#include <stdexcept>

/*
A simple class for ini files.
The template allows you to specify the type used as a string, as well as the string, input, output and file stream types.
Default is std::string.
*/
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

	struct FileError : public std::runtime_error {
		FormatException() {}
		FormatException(const std::string& what) : std::runtime_error(what) {}
	};

	struct FormatException : public std::runtime_error {
		FormatException() {}
		FormatException(const std::string& what, size_t line) :
			std::runtime_error(what),
			line(line)
		{}

		size_t line;
	};

	std::map<String, std::map<String, String> > dataMap;// dataMap[section][key] = value

	MinIni(): mAutoSync(false) {}

	/*
	@param filename The name of the file to open and read
	@param autosync If enabled, the file will automatically be synced to this MinIni's content before being closed
	*/
	MinIni(String filename, bool autosync) {
		open(filename, autosync);
	}

	~MinIni() {
		close();
	}

	/*
	The name of the file that's linked to this MinIni.
	Note that the filename will be linked only if it's specified in the call to open() or in the constructor.
	Using read() to read a file will not link the file.
	*/
	String filename() const {
		return mFilename;
	}

	// If enabled, the linked file will automatically be synced to this MinIni's content before being closed or destroyed
	bool autoSyncEnabled() const {
		return mAutoSync;
	}

	void setFilename(String filename) {
		mFilename = filename;
	}

	void enableAutoSync(bool enable = true) {
		mAutoSync = enable;
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

	// Returns whether a value exists with the key in the section sect
	bool exists(String sect, String key) const {
		auto it = dataMap.find(sect);
		return (it != dataMap.end() && it->second.find(key) != it->second.end());
	}

	/*
	Reads the content from the stream, adding it to the already existing content.
	The stream needs to be formatted as an ini file.If not, a FormatException will be thrown, unless ignoreErrors argument is enabled.
	In that case the improper line will be skipped, and the rest of the file will be read normally.
	Note that specifying a FileStream as the input stream won't link the file, i.e. the filename won't be changed.
	*/
	void readMore(InputStream& is, bool ignoreErrors = false) {
		String sect;
		String ln;
		size_t line = 0;
		while (std::getline(is, ln)) {
			line++;
			size_t commentPos;

			ln.erase(0, ln.find_first_not_of(L" \t"));
			ln.erase(ln.find_last_not_of(L" \t") + 1);
			if ((commentPos = ln.find_first_of(L"#")) != String::npos) {
				ln.erase(commentPos + 1);
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
					if (eqpos != String::npos) {
						if (!ignoreErrors)
							throw FormatException((std::stringstream() <<
								"Wrong ini file format at line " << line << "!"
								).str(), line);
						continue;
					}

					String key = ln.substr(0, eqpos);
					key.erase(key.find_last_not_of(L" \t") + 1);
					String val = ln.substr(eqpos + 1, String::npos);
					val.erase(0, val.find_first_not_of(L" \t"));

					dataMap[sect][key] = val;
				}
			}
		}
	}

	// Clears the content and reads it from the stream using readMore().
	void read(InputStream& is, bool ignoreErrors = false) {
		dataMap.clear();
		readMore(is, ignoreErrors);
	}

	// Writes the content to the output stream, formatted as an ini file
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
	Reads the file and links it to this MinIni
	@param filename The name of the file to open
	@param autosync If enabled, the file will automatically be synced to this MinIni's content before being closed
	*/
	void open(String filename, bool autosync, bool ignoreErrors = false) {
		mFilename = filename;
		mAutoSync = autosync;

		FStream file;
		file.open(filename, std::ios::in);

		if (file.good()) {
			read(file, ignoreErrors);
			file.close();
		}
	}

	// Writes the content to the linked file. Throws FileError if the file can't be opened or isn't linked.
	void sync() const {
		FileStream file;
		file.open(mFilename, std::ios::out);

		if (file.good()) {
			write(file);
			file.close();
		}
		else {
			throw FileError(
				mFilename.length()? (std::stringstream() << "Can't open ini file \"" << mFilename << "\"!").str() : "No linked file specified to be synced to this MinIni!",
				line);
		}
	}

	// Resets the MinIni and syncs the linked file before closing if autoSync is enabled.
	void close() {
		if (mFilename.length() && mAutoSync) {
			sync();
		}
		dataMap.clear();
		mFilename = String();
		mAutoSync = false;
	}
};

// widechar version of the default MinIni template
using WMinIni = MinIni<std::wstring>;

#endif