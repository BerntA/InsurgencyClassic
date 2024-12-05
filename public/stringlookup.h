/* deathz0rz
These are convenience macro's to convert from constant names
to constant values. Example:

[constant.h]

#define CONE_15_DEGREES		0.9659258f
#define CONE_45_DEGREES		0.707f
#define CONE_90_DEGREES		0

DECLARE_STRING_LOOKUP_CONSTANTS(float,floats)

//you don't have to do this, but it saves typing :)
#define CFL(k) STRING_LOOKUP(floats,k)
//CFL = Cone FLoat

[constant.cpp]
DEFINE_STRING_LOOKUP_CONSTANTS(float,floats)
	ADD_LOOKUP(CONE_15_DEGREES)
	ADD_LOOKUP(CONE_45_DEGREES)
	ADD_LOOKUP(CONE_90_DEGREES)
END_STRING_LOOKUP_CONSTANTS()

(...)
	m_flCone=CFL(pKV->GetString("cone"));
(...)
*/

#include <utlstringmap.h>
#include <utlvector.h>

#ifndef STRING_LOOKUP_H
#define STRING_LOOKUP_H

template<typename T> class CStringLookupConstantTable
{
protected:
	typedef struct m_stringconstant_s {
		const char*	pszKey;
		T			tValue;
	} m_stringconstant_t;

	CUtlStringMap<T*> m_StringMap;

public:
	inline bool FindKey(const char *pszKey, T *ptValue)
	{
		if (m_StringMap.Defined(pszKey))
		{
			if (ptValue)
				*ptValue=*m_StringMap[pszKey];
			return true;
		}
		else
		{
			return false;
		}
	};

	CUtlVector<T>* FromString(const char* pszStr)
	{  
		static CUtlVector<T> LocalArray;  
		static char szNoSep[]="qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM1234567890_";  
		char* pszCur=NULL;  
		char* pszIt;  
		char szTmp[256];
		T tTmp;

		LocalArray.RemoveAll();  

		for(pszIt=(char*)pszStr;*pszIt;pszIt++)  
		{  
			if (pszCur)  
			{  
				if (!strchr(szNoSep,*pszIt))  
				{  
					Q_strncpy(szTmp,pszCur,min((int)(pszIt-pszCur)+1,256)); 
					if (FindKey(szTmp,&tTmp))
						LocalArray.AddToTail(tTmp);  
					pszCur=NULL;  
				}  
			}  
			else  
			{  
				if (strchr(szNoSep,*pszIt))  
				{  
					pszCur=pszIt;  
				}  
			}  
		}  
		if (pszCur)  
		{  
			Q_strncpy(szTmp,pszCur,min((int)(pszIt-pszCur)+1,256));  
			if (FindKey(szTmp,&tTmp))
				LocalArray.AddToTail(tTmp);
		}  
		return &LocalArray;  
	}
};

//SLTP means String Lookup Table Private
#define DECLARE_STRING_LOOKUP_CONSTANTS(type,identifier) \
	namespace SLTP__##identifier \
	{ \
		class CStringLookupConstantTable__##identifier : public CStringLookupConstantTable<type> \
		{ \
		public: \
			inline CUtlVector<type>* FromString(const char* pszStr) { return CStringLookupConstantTable<type>::FromString(pszStr); }; \
			CStringLookupConstantTable__##identifier(); \
			~CStringLookupConstantTable__##identifier() {}; \
		}; \
	} \
	\
	extern SLTP__##identifier::CStringLookupConstantTable__##identifier g_StringLookupConstantTable__##identifier;

#define STRING_LOOKUP(identifier,key,var) \
	g_StringLookupConstantTable__##identifier.FindKey(key,&var)

#define STRING_EXISTS(identifier,key) \
	g_StringLookupConstantTable__##identifier.FindKey(key,NULL)

#define CONVERT_STRING(identifier,str) \
	g_StringLookupConstantTable__##identifier.FromString(str);

#define DEFINE_STRING_LOOKUP_CONSTANTS(type,identifier) \
	SLTP__##identifier::CStringLookupConstantTable__##identifier g_StringLookupConstantTable__##identifier; \
	namespace SLTP__##identifier \
	{ \
		CStringLookupConstantTable__##identifier::CStringLookupConstantTable__##identifier() \
		{ \
			static m_stringconstant_t aConstants[] = {

#define ADD_LOOKUP(macro) \
				#macro, macro, \

#define END_STRING_LOOKUP_CONSTANTS() \
			}; \
			for (int i=0;i<sizeof(aConstants)/sizeof(m_stringconstant_t);i++) \
			{ \
				m_StringMap[aConstants[i].pszKey]=&aConstants[i].tValue; \
			} \
		}; \
	};

#endif //!STRING_LOOKUP_H