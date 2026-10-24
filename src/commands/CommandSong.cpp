#include "CommandSong.hpp"
#include "client/Minecraft.h"
#include "client/sound/MusicEngine.h"
#include <sstream>

CommandSong::CommandSong() : Command("song") {
    // Populate m_syntax with song enum list if available
    std::vector<std::string> songList;
    songList.push_back("aria_math_hardstyle");
    songList.push_back("featherfall");
    songList.push_back("komorebi");
    songList.push_back("Precipice");

    m_syntax.push_back(CommandArgument("name", CommandArgType::Enum, true, songList));
}

std::string CommandSong::execute(Minecraft& mc, Player& player, const std::vector<std::string>& args) {
    if (!mc.musicEngine) {
        return SECTION "cMusic engine not available";
    }

    if (args.empty()) {
        std::vector<std::string> trackNames = mc.musicEngine->getTrackNamesWithoutExtension();
        std::ostringstream ss;
        for (size_t i = 0; i < trackNames.size(); ++i) {
            ss << trackNames[i];
            if (i + 1 < trackNames.size()) {
                ss << ", ";
            }
        }
        return ss.str();
    }

    std::string trackName = args[0];
    bool success = mc.musicEngine->playTrack(trackName);

    if (!success) {
        return SECTION "cUnknown song: " + trackName;
    }

    return "";
}

std::string CommandSong::help(Minecraft& mc) {
    return "Usage: /song <name>";
}
