string utf16ToUtf8(const u16string& utf16Str) {
	if (utf16Str.empty()) {
		return {};
	}
	constexpr char16_t marker = 0xFEFF;
	const char16_t* source = utf16Str.data();
	size_t remainingLength = utf16Str.length();
	if (source[0] == marker) {
		source++;
		remainingLength--;
	}
	string utf8Str;
	utf8Str.reserve(remainingLength * 3);
	for (size_t i = 0; i < remainingLength; ++i) {
		const char16_t codeUnit = source[i];
		if (codeUnit < 0x80) {
			utf8Str.push_back(static_cast<char>(codeUnit));
			continue;
		}
		if (codeUnit < 0x800) {
			utf8Str.push_back(static_cast<char>((codeUnit >> 6) | 0xC0));
            utf8Str.push_back(static_cast<char>((codeUnit & 0x3F) | 0x80));
			continue;
		}
		if (codeUnit >= 0xD800 && codeUnit <= 0xDBFF && i + 1 < remainingLength) {
			const char16_t highSurrogate = codeUnit;
			const char16_t lowSurrogate = source[++i];
			const uint32_t codePoint = 0x10000 + ((highSurrogate - 0xD800) << 10) + (lowSurrogate - 0xDC00);           
			utf8Str.push_back(static_cast<char>((codePoint >> 18) | 0xF0));
			utf8Str.push_back(static_cast<char>(((codePoint >> 12) & 0x3F) | 0x80));
			utf8Str.push_back(static_cast<char>(((codePoint >> 6) & 0x3F) | 0x80));
			utf8Str.push_back(static_cast<char>((codePoint & 0x3F) | 0x80));
			continue;
		}
		utf8Str.push_back(static_cast<char>((codeUnit >> 12) | 0xE0));
		utf8Str.push_back(static_cast<char>(((codeUnit >> 6) & 0x3F) | 0x80));
		utf8Str.push_back(static_cast<char>((codeUnit & 0x3F) | 0x80));
	}
	return utf8Str;
}

struct monoString {
	uintptr_t classPointer;
	uintptr_t synchronizationData;
	int stringLength;
	char16_t buffer[128];
	string asUtf8() {
		if (stringLength <= 0 || stringLength > 128) {
			return {};
		}
		return utf16ToUtf8(u16string(buffer, stringLength));
	}
};
