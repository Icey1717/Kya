#ifndef TEXT_ICON_DICTIONARY
#define TEXT_ICON_DICTIONARY

#include "Types.h"

class edCTextStyle;
struct edDList_material;
struct edTextBitmap;

typedef void (*edResourceFunc)(edCTextStyle*);

class edCTextResourcePool {
public:

	struct edCTextResource {
		edCTextResource();
		ulong key;
		void* pMessageData;
		int index;
	};

	bool CallbackAdd(char* name, edResourceFunc pFunc);
	bool TextAdd(char* key, char* value);
	bool BitmapAdd(char* label, edTextBitmap* param_3);
	edTextBitmap* GetResourcePtr(ulong key, int index);

	edCTextResourcePool();
	bool Init(int maxEntries);

	edCTextResource* pEntries;
	int maxEntries;
	int currentEntries;
};

extern edCTextResourcePool edTextResources;

#endif //TEXT_ICON_DICTIONARY
