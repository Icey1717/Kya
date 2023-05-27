#ifndef TEXT_ICON_DICTIONARY
#define TEXT_ICON_DICTIONARY

#include "Types.h"

struct TextIconDictionaryEntry {
	char key[8];
	void* pMessageData;
	int index;
};

class edCTextResourcePool {
public:
	edCTextResourcePool();
	bool Init(int maxEntries);
	bool AddTextEntry(char* key, char* value);

	struct TextIconDictionaryEntry* pEntries;
	int maxEntries;
	int currentEntries;
};

extern edCTextResourcePool g_TextIconDictionary;

#endif //TEXT_ICON_DICTIONARY
