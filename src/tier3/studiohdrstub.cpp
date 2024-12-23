//====== Copyright � 1996-2004, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================
#include "studio.h"
#include "datacache/imdlcache.h"
#include "datamodel/dmelementfactoryhelper.h"
#include "istudiorender.h"
#include "bone_setup.h"
#include "tier3/tier3.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// FIXME: This trashy glue code is really not acceptable. Figure out a way of making it unnecessary.
//-----------------------------------------------------------------------------
const studiohdr_t* studiohdr_t::FindModel(void** cache, char const* pModelName) const
{
    MDLHandle_t handle = g_pMDLCache->FindMDL(pModelName);
    *cache = reinterpret_cast<void*>(static_cast<uintptr_t>(handle));
    return g_pMDLCache->GetStudioHdr(handle);
}

virtualmodel_t* studiohdr_t::GetVirtualModel(void) const
{
    return g_pMDLCache->GetVirtualModel(static_cast<MDLHandle_t>(reinterpret_cast<uintptr_t>(virtualModel)));
}

byte* studiohdr_t::GetAnimBlock(int i) const
{
    return g_pMDLCache->GetAnimBlock(static_cast<MDLHandle_t>(reinterpret_cast<uintptr_t>(virtualModel)), i);
}

int studiohdr_t::GetAutoplayList(unsigned short** pOut) const
{
    return g_pMDLCache->GetAutoplayList(static_cast<MDLHandle_t>(reinterpret_cast<uintptr_t>(virtualModel)), pOut);
}

const studiohdr_t* virtualgroup_t::GetStudioHdr(void) const
{
    return g_pMDLCache->GetStudioHdr(static_cast<MDLHandle_t>(reinterpret_cast<uintptr_t>(cache)));
}
