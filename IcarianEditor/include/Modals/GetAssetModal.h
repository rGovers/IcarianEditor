#pragma once

#include "Modals/Modal.h"

#include <filesystem>
#include <vector>

class AssetLibrary;

class GetAssetModalData
{
private:

protected:

public:
    GetAssetModalData() {}
    virtual ~GetAssetModalData() { }

    virtual void Confirm(const std::filesystem::path& a_path) = 0;
    virtual void Cancel() { }
};

class GetAssetModal : public Modal
{
private:
    GetAssetModalData*                 m_data;

    uint32_t                           m_curIndex;
    std::vector<std::filesystem::path> m_paths;

protected:

public:
    GetAssetModal(char* const* a_extensions, uint32_t a_extensionCount, AssetLibrary* a_assetLibrary, GetAssetModalData* a_data);
    virtual ~GetAssetModal();

    virtual bool Update();
};
