class ICustomizeHelper
{
public:
	//Always possible
	virtual const char* GetModelName(int iTeam, int iClass) = 0;
	virtual void LoadModel(int iTeam, int iClass) = 0;
	
	//Only possible when a model is loaded
	virtual void SaveModel() = 0;
	virtual void CloseModel() = 0;
	
	virtual int GetModelIndex() = 0;
	
	//Indexes:
	// -2 - Primary weapon.
	// -1 - Face/skin
	// 0 etc - Bodygroups
	virtual int GetCustomizeCount() = 0;
	// Attachment for the model with GetModelIndex()
	virtual const char* GetCustomizeAttachment(int iCust) = 0;
	virtual const char* GetCustomizeImage(int iCust) = 0;
	virtual const char* GetCustomizeName(int iCust) = 0;
	
	virtual int GetCustomizeOptionCount(int iCust) = 0;
	virtual const char* GetCustomizeOptionName(int iCust, int iOption) = 0;
	virtual const char* GetCustomizeOptionDesc(int iCust, int iOption) = 0;
	
	virtual int GetSelectedCustomizeOption(int iCust) = 0;
	virtual void SetSelectedCustomizeOption(int iCust, int iOption) = 0;
	
	virtual int GetSelectedRealWeaponID() = 0;
	virtual int GetSelectedRealSkin() = 0;
	virtual int GetSelectedRealBody() = 0;
};