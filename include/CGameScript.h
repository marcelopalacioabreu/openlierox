/////////////////////////////////////////
//
//              OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Game script class
// Created 7/2/02
// Jason Boettcher


#ifndef __CGAMESCRIPT_H__
#define __CGAMESCRIPT_H__

#include <map>
#include <set>
#include "Sounds.h"
#include "GfxPrimitives.h"
#include "CProjectile.h"
#include "Version.h"
#include "Color.h"
#include "StaticAssert.h"
#include "StringUtils.h"

static const Version GS_MinLxVersion[] = {
	Version(), // for GS_VERSION == 7
	OLXBetaVersion(9), // for GS_VERSION == 8
};

#define		GS_LX56_VERSION	7
#define		GS_FIRST_SUPPORTED_VERSION	GS_LX56_VERSION
#define		GS_MINLXVERSION(ver)	GS_MinLxVersion[ver - GS_FIRST_SUPPORTED_VERSION]
// current most recent version
#define		GS_VERSION		8

// Error codes
#define		GSE_OK			1
// TODO: what is this for?
#define		GSE_MEM			0
#define		GSE_VERSION		-1
#define		GSE_FILE		-2
#define		GSE_BAD			-3



// Header structure
// WARNING: never change this!
// it's used in CGameScript.cpp and it represents
// the original file format
struct gs_header_t {
	gs_header_t() { ID[0] = 0; Version = 0; ModName[0] = 0; }
	char	ID[18];
	Uint32	Version;
	char	ModName[64];
};



// Worm structure
// WARNING: never change this!
// it's used in CGameScript.cpp and it represents
// the original file format
struct gs_worm_t {
	float	AngleSpeed;

	float	GroundSpeed;
	float	AirSpeed;
	float	Gravity;
	float	JumpForce;
	float	AirFriction;
	float	GroundFriction;

};


struct weapon_t;
struct proj_t;




class CGameScript {
public:
	// Constructor
	CGameScript() {
		loaded = false;
		NumWeapons = 0;
		Weapons = NULL;
        pModLog = NULL;

		RopeLength = 150;
		RestLength = 20;
		Strength = 0.5f;
	}

	~CGameScript() {
		Shutdown();
	}

private:
	// Attributes

	std::string		sDirectory;


	// Header
	bool loaded;
	gs_header_t	Header;


	// Weapons
	int			NumWeapons;
	weapon_t	*Weapons;

	// Worm
	gs_worm_t	Worm;

	typedef std::map<int, proj_t*> Projectiles;
	typedef std::map<std::string, int, stringcaseless> ProjFileMap;
	ProjFileMap projFileIndexes;
	Projectiles projectiles;
	std::set<proj_t*> savedProjs;
	
	// Ninja Rope
	int			RopeLength;
	int			RestLength;
	float		Strength;

    // Mod log file
    FILE        *pModLog;
	
	std::vector< SmartPointer<SDL_Surface> > CachedImages;	// To safely delete the vars, along with CGameScript.
	std::vector< SmartPointer<SoundSample> > CachedSamples;	// To safely delete the vars, along with CGameScript.

private:

	void		Shutdown();
	void		ShutdownProjectile(proj_t *prj);

public:
	// Methods

	int			Load(const std::string& dir);
	int			Save(const std::string& filename);
	bool		isLoaded() const { return loaded; }
	
private:
	proj_t		*LoadProjectile(FILE *fp);
	bool		SaveProjectile(proj_t *proj, FILE *fp);
    void        writeString(const std::string& szString, FILE *fp);
    std::string readString(FILE *fp);

public:
	size_t		GetMemorySize();
	std::string	getError(int code);

	const weapon_t	*FindWeapon(const std::string& name);
    bool        weaponExists(const std::string& szName);

	static bool	CheckFile(const std::string& dir, std::string& name, bool abs_filename = false);

    void        modLog(const std::string& text);

#ifndef _CONSOLE
	SDL_Surface * LoadGSImage(const std::string& dir, const std::string& filename);
	SoundSample * LoadGSSample(const std::string& dir, const std::string& filename);
#endif

	const gs_header_t	*GetHeader()				{ return &Header; }
	static bool	isCompatibleWith(int scriptVer, const Version& ver) {
		if(scriptVer < GS_FIRST_SUPPORTED_VERSION) return false;
		if(scriptVer > GS_VERSION) return false; // or actually no idea
		return GS_MINLXVERSION(scriptVer) <= ver;		
	}
	bool		isCompatibleWith(const Version& ver) const { return isCompatibleWith(Header.Version, ver); }
	std::string modName() const { return Header.ModName; }
	std::string directory() const { return sDirectory; }
	
	int			GetNumWeapons()				{ return NumWeapons; }
	const weapon_t	*GetWeapons()				{ return Weapons; }

private:
	void	initNewWeapons(int num);
	void	SetNumWeapons(int _w)			{ NumWeapons = _w; }
	void	SetWeapons(weapon_t *_w)		{ Weapons = _w; }

	
	// Ninja Rope settings
	void	SetRopeLength(int _l)			{ RopeLength = _l; }
	void	SetRestLength(int _l)			{ RestLength = _l; }
	void	SetStrength(float _s)			{ Strength = _s; }

public:
	
	
	int		getRopeLength()				{ return RopeLength; }
	int		getRestLength()				{ return RestLength; }
	float	getStrength()				{ return Strength; }

	const gs_worm_t	*getWorm()	const	{ return &Worm; }

	int		getProjectileCount() const	{ return projectiles.size(); }
	
	bool		Compile(const std::string& dir);
	
private:
	bool	CompileWeapon(const std::string& dir, const std::string& weapon, int id);
	void	CompileBeam(const std::string& file, weapon_t *Weap);
	proj_t  *CompileProjectile(const std::string& dir, const std::string& pfile);
	bool	CompileExtra(const std::string& dir);
	bool	CompileJetpack(const std::string& file, weapon_t *Weap);
};


#endif  //  __CGAMESCRIPT_H__
