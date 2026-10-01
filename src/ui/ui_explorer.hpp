#pragma once

#include <switch.h>

#include "render.hpp"
#include "ui/ui_common.hpp"

namespace sw::ui {

class Explorer: public Widget {
    public:
        Explorer(Renderer &renderer, Context &context);
        virtual ~Explorer();

        virtual bool update_state(PadState &pad, HidTouchScreenState &touch) override;

        virtual void render() override;

        static constexpr inline std::string_view path_from_entry_name(std::string_view name) {
            return name.substr(name.find("##")+2);
        }

        static constexpr inline std::string_view filename_from_entry_name(std::string_view name) {
            return name.substr(0, name.find("##"));
        }

    public:
        Context &context;

        Renderer::Texture file_texture, folder_texture,
            recent_texture, sd_texture, usb_texture, network_texture;

        bool is_focused = false;
        bool allow_file_deletion = false;

        fs::Path path;
        fs::Path selection;

        std::vector<fs::Node> entries;
        std::size_t cur_focused_entry = -1;

        bool is_initial_scan     = true;
        bool need_directory_scan = true;
        bool want_focus_reset    = false;

    private:
        fs::Path pending_delete_path;
        std::shared_ptr<fs::Filesystem> pending_delete_fs;
        std::shared_ptr<fs::Filesystem> pending_history_fs;
        int delete_error = 0;
        bool deleted_file = false;
        int history_error = 0;
        bool cleared_history = false;
};

} // namespace sw::ui
