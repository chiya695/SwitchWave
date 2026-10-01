#include <dirent.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_nx.h>
#include <imgui_deko3d.h>

#include "utils.hpp"
#include "fs/fs_recent.hpp"
#include "i18n.hpp"

#include "ui/ui_explorer.hpp"

namespace sw::ui {

namespace {

extern "C" {
    u32 __nx_fsdev_direntry_cache_size = 64;
}

std::string_view utf8_skip_from_end(std::string_view sv, int skip) {
    auto *data = sv.data() + sv.length();
    for (int i = 0; (i < skip) && (data > sv.data()); ++i)
        while ((*--data & 0xc0) == 0x80);
    return sv.substr(uintptr_t(data - sv.data()));
}

} // namespace

Explorer::Explorer(Renderer &renderer, Context &context): Widget(renderer), context(context) {
    this->path = !this->context.cur_path.empty() ? this->context.cur_path : "sdmc:/";

    this->file_texture    = this->renderer.load_texture("romfs:/textures/file-64*64-bc4.bc",
        64, 64, DkImageFormat_R_BC4_Unorm, DkImageFlags_Usage2DEngine);
    this->folder_texture  = this->renderer.load_texture("romfs:/textures/folder-64*64-bc4.bc",
        64, 64, DkImageFormat_R_BC4_Unorm, DkImageFlags_Usage2DEngine);
    this->recent_texture  = this->renderer.load_texture("romfs:/textures/recent-64*64-bc4.bc",
        64, 64, DkImageFormat_R_BC4_Unorm, DkImageFlags_Usage2DEngine);
    this->sd_texture      = this->renderer.load_texture("romfs:/textures/sd-64*64-bc4.bc",
        64, 64, DkImageFormat_R_BC4_Unorm, DkImageFlags_Usage2DEngine);
    this->usb_texture     = this->renderer.load_texture("romfs:/textures/usb-64*64-bc4.bc",
        64, 64, DkImageFormat_R_BC4_Unorm, DkImageFlags_Usage2DEngine);
    this->network_texture = this->renderer.load_texture("romfs:/textures/network-64*64-bc4.bc",
        64, 64, DkImageFormat_R_BC4_Unorm, DkImageFlags_Usage2DEngine);
}

Explorer::~Explorer() {
    this->renderer.unregister_texture(this->file_texture);
    this->renderer.unregister_texture(this->folder_texture);
    this->renderer.unregister_texture(this->recent_texture);
    this->renderer.unregister_texture(this->sd_texture);
    this->renderer.unregister_texture(this->usb_texture);
    this->renderer.unregister_texture(this->network_texture);
}

bool Explorer::update_state(PadState &pad, HidTouchScreenState &touch) {
    if (this->need_directory_scan) {
        this->need_directory_scan = false;
        this->cur_focused_entry = -1;
        this->context.cur_path = this->path.base();

        auto *dir = opendir(this->path.c_str());
        if (dir) {
            SW_SCOPEGUARD([dir] { closedir(dir); });
            this->entries.clear();

            auto *reent    = __syscall_getreent();
            auto *devoptab = devoptab_list[dir->dirData->device];

            // Hack to support the recent filesystem: NAME_MAX is sufficient in theory,
            // but this fs returns full paths
            // PATH_MAX on devkitA64 is just 1024 but linux allows 4096
            // On top of that, reserve some space for the mountpoint
            std::string fname;
            fname.reserve(4096+1+0x20);

            struct stat st;
            while (true) {
                std::memset(fname.data(), '\0', fname.capacity());

                reent->deviceData = devoptab->deviceData;
                if (devoptab->dirnext_r(reent, dir->dirData, fname.data(), &st))
                    break;

                auto path = this->path / fname.c_str();

                // Strip "recent:/" from path
                if (this->context.cur_fs->type == fs::Filesystem::Type::Recent)
                    path = path.internal().substr(1);

                // In the recent filesystem multiple files might have the same name
                auto name = std::string(path.filename()) + "##" + path.base();

                if (S_ISDIR(st.st_mode))
                    this->entries.emplace_back(fs::Node{fs::Node::Type::Directory, std::move(name), 0, path.base()});
                else
                    this->entries.emplace_back(fs::Node{fs::Node::Type::File, std::move(name), std::size_t(st.st_size), path.base()});
            }

            if (this->context.cur_fs->type != fs::Filesystem::Type::Recent) {
                std::sort(this->entries.begin(), this->entries.end(), [](const fs::Node &lhs, const fs::Node &rhs) {
                    if (lhs.type != rhs.type)
                        return lhs.type < rhs.type;
                    return strcasecmp(lhs.name.c_str(), rhs.name.c_str()) < 0;
                });
            }

            this->want_focus_reset = !this->is_initial_scan;
            this->is_initial_scan  = false;
        } else {
            std::printf("Failed to open directory %s: %s (%d)\n", this->path.c_str(), std::strerror(errno), errno);
            this->context.set_error(errno);
        }
    }

    return true;
}

void Explorer::render() {
    {
        ImGui::PushItemWidth(this->screen_rel_width(0.15));
        SW_SCOPEGUARD([] { ImGui::PopItemWidth(); });

        if (ImGui::BeginCombo("##fscombo", this->context.cur_fs->name.data())) {
            SW_SCOPEGUARD([] { ImGui::EndCombo(); });

            for (auto &fs: this->context.filesystems) {
                Renderer::Texture *tex;
                switch (fs->type) {
                    using enum fs::Filesystem::Type;
                    default:
                    case Recent:
                        tex = &this->recent_texture;
                        break;
                    case Sdmc:
                        tex = &this->sd_texture;
                        break;
                    case Usb:
                        tex = &this->usb_texture;
                        break;
                    case Network:
                        tex = &this->network_texture;
                        break;
                }

                ImVec4 tint_col = (ImGui::nx::getCurrentTheme() == ColorSetId_Dark) ?
                    ImVec4(1, 1, 1, 1) : ImVec4(0, 0, 0, 1);

                ImGui::Image(ImGui::deko3d::makeTextureID(tex->handle, true),
                    ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize()), ImVec2(0, 0), ImVec2(1, 1), tint_col);

                ImGui::SameLine();
                if (ImGui::Selectable(fs->name.data(), this->context.cur_fs == fs)) {
                    this->context.cur_fs = fs;
                    this->need_directory_scan = true;

                    this->path = fs::Path(this->context.cur_fs->mount_name) + "/";
                }
            }
        }
    }

    bool want_explore_backward = this->is_focused && this->pending_delete_path.empty() && ImGui::IsKeyPressed(ImGuiKey_GamepadDpadLeft),
        want_explore_forward   = this->is_focused && this->pending_delete_path.empty() && ImGui::IsKeyPressed(ImGuiKey_GamepadDpadRight);

    std::string_view path = this->path.internal();

    auto suffix = utf8_skip_from_end(path, 43);
    auto buttonstr = (suffix.size() < path.size() ? "..." : "") + std::string(suffix);

    {
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0, 0.5));
        SW_SCOPEGUARD([] { ImGui::PopStyleVar(); });

        ImGui::SameLine();
        want_explore_backward |= ImGui::Button(buttonstr.c_str(), ImVec2(-1, 0));
    }

    auto reserved_height = (this->allow_file_deletion ? 3 : 1) *
        (ImGui::GetStyle().ItemSpacing.y + ImGui::GetTextLineHeightWithSpacing());

    if (ImGui::BeginListBox("##fsentries", ImVec2(-1, -reserved_height))) {
        SW_SCOPEGUARD([] { ImGui::EndListBox(); });

        this->is_focused = ImGui::IsWindowFocused();

        ImVec4 tint_col = (ImGui::nx::getCurrentTheme() == ColorSetId_Dark) ?
            ImVec4(1, 1, 1, 1) : ImVec4(0, 0, 0, 1);

        ImGuiListClipper clipper;
        clipper.Begin(this->entries.size());

        while (clipper.Step()) {
            for (auto i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                auto &entry = this->entries[i];
                ImGui::Image(ImGui::deko3d::makeTextureID((entry.type == fs::Node::Type::File) ?
                        this->file_texture.handle : this->folder_texture.handle, true),
                    ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize()), ImVec2(0, 0), ImVec2(1, 1), tint_col);
                ImGui::SameLine();

                bool selected = ImGui::Selectable(entry.name.c_str());
                want_explore_forward |= selected;
                auto is_item_focused = ImGui::IsItemFocused();

                if (is_item_focused || selected)
                    this->cur_focused_entry = i;
            }
        }

        if (want_explore_backward) {
            if (!this->path.is_root())
                this->path = this->path.parent();
            this->need_directory_scan = true;
            this->cur_focused_entry = -1;
        } else if (want_explore_forward && this->cur_focused_entry < this->entries.size()) {
            auto &entry = this->entries[this->cur_focused_entry];
            switch (entry.type) {
                case fs::Node::Type::Directory:
                    this->path = entry.path;
                    this->need_directory_scan = true;
                    break;
                case fs::Node::Type::File:
                    this->selection = entry.path;
                    this->context.cur_file = entry.path;
                    break;
            }
        }

        if (this->want_focus_reset && !this->entries.empty()) {
            auto &entry = this->entries.front();
            ImGui::SetNavWindow(ImGui::GetCurrentWindow());
            ImGui::SetNavID(ImGui::GetID(entry.name.c_str()), ImGuiNavLayer_Main, 0, ImRect());
            this->want_focus_reset = false;
        }
    }

    bool is_recent = this->context.cur_fs && this->context.cur_fs->type == fs::Filesystem::Type::Recent;
    if (!this->allow_file_deletion) {
        ImGui::TextUnformatted(i18n::tr("Navigate with \ue0ea"));
        return;
    }

    bool can_delete = this->allow_file_deletion && this->context.cur_fs && this->context.cur_fs->supports_file_deletion() &&
        this->cur_focused_entry < this->entries.size() &&
        this->entries[this->cur_focused_entry].type == fs::Node::Type::File;
    ImGui::BeginDisabled(!can_delete);
    bool want_delete = ImGui::Button(i18n::label("Delete file"));
    ImGui::EndDisabled();
    if (is_recent) {
        ImGui::SameLine();
        if (ImGui::Button(i18n::label("Clear recent playback"))) {
            this->pending_history_fs = this->context.cur_fs;
            this->history_error = 0;
            this->cleared_history = false;
            ImGui::OpenPopup(i18n::label("Confirm history clearing"));
        }
    }
    want_delete |= can_delete && this->is_focused && ImGui::IsKeyPressed(ImGuiKey_GamepadFaceLeft);
    if (want_delete && can_delete && this->pending_delete_path.empty() && !this->need_directory_scan) {
        this->pending_delete_path = this->entries[this->cur_focused_entry].path;
        this->pending_delete_fs = this->context.cur_fs;
        this->delete_error = 0;
        this->deleted_file = false;
        ImGui::OpenPopup(i18n::label("Confirm deletion"));
    }

    if (ImGui::BeginPopupModal(i18n::label("Confirm deletion"), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        SW_SCOPEGUARD([] { ImGui::EndPopup(); });
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + this->screen_rel_width(0.6));
        SW_SCOPEGUARD([] { ImGui::PopTextWrapPos(); });
        ImGui::TextWrapped("%s", i18n::tr("This permanently deletes one file from its storage or server. There is no recycle bin."));
        ImGui::TextWrapped("%s", this->pending_delete_path.c_str());
        ImGui::Separator();
        if (ImGui::Button(i18n::label("Cancel"))) {
            this->pending_delete_path.clear();
            this->pending_delete_fs.reset();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button(i18n::label("Delete permanently"))) {
            auto *mounted = this->context.get_filesystem(this->pending_delete_path);
            if (!this->pending_delete_fs || mounted != this->pending_delete_fs.get())
                this->delete_error = ENOTCONN;
            else if (!this->context.player_is_idle && this->context.cur_file == this->pending_delete_path.base())
                this->delete_error = EBUSY;
            else
                this->delete_error = this->pending_delete_fs->delete_file(this->pending_delete_path);
            this->deleted_file = !this->delete_error;
            if (this->deleted_file) {
                this->need_directory_scan = true;
                this->cur_focused_entry = -1;
                this->selection.clear();
                if (this->context.cur_file == this->pending_delete_path.base())
                    this->context.cur_file.clear();
            }
            this->pending_delete_path.clear();
            this->pending_delete_fs.reset();
            ImGui::CloseCurrentPopup();
        }
    }

    if (this->delete_error && !is_recent) {
        const char *message = std::strerror(this->delete_error);
        switch (this->delete_error) {
            case EACCES:
            case EPERM:
            case EROFS: message = i18n::tr("Deletion is not permitted by the server or the filesystem."); break;
            case EBUSY: message = i18n::tr("The file is in use. Stop playback before deleting it."); break;
            case EINVAL: message = i18n::tr("Only regular files can be deleted. Directories and links are not deleted."); break;
            case ENOENT: message = i18n::tr("The file no longer exists."); break;
            case ENOTCONN: message = i18n::tr("The server is disconnected or unavailable."); break;
            case ENOTSUP: message = i18n::tr("This filesystem does not support file deletion."); break;
        }
        ImGui::TextWrapped(i18n::tr("Delete failed: %s (%d)"), message, this->delete_error);
    } else if (this->deleted_file && !is_recent) {
        ImGui::TextUnformatted(i18n::tr("File deleted."));
    } else if (is_recent) {
        ImGui::TextUnformatted(i18n::tr("Navigate with \ue0ea"));
    } else {
        ImGui::TextUnformatted(i18n::tr("Navigate with \ue0ea; press \ue002 to delete a file"));
    }

    if (ImGui::BeginPopupModal(i18n::label("Confirm history clearing"), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        SW_SCOPEGUARD([] { ImGui::EndPopup(); });
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + this->screen_rel_width(0.6));
        SW_SCOPEGUARD([] { ImGui::PopTextWrapPos(); });
        ImGui::TextWrapped("%s", i18n::tr("This clears saved recent playback paths only. Media files and playback positions are not deleted."));
        if (ImGui::Button(i18n::label("Cancel"))) {
            this->pending_history_fs.reset();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button(i18n::label("Clear list"))) {
            auto recent = this->pending_history_fs;
            if (!recent || this->context.get_filesystem(std::string_view(recent->mount_name)) != recent.get())
                this->history_error = ENOTCONN;
            else
                this->history_error = static_cast<fs::RecentFs *>(recent.get())->clear_and_save();
            if (!this->history_error) {
                this->cleared_history = true;
                this->need_directory_scan = true;
                this->cur_focused_entry = -1;
                this->selection.clear();
            }
            this->pending_history_fs.reset();
            ImGui::CloseCurrentPopup();
        }
    }
    if (is_recent && this->history_error)
        ImGui::TextWrapped(i18n::tr("History clearing failed: %s (%d)"), std::strerror(this->history_error), this->history_error);
    else if (is_recent && this->cleared_history)
        ImGui::TextUnformatted(i18n::tr("Recent playback cleared."));
}

} // namespace sw::ui
