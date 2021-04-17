#include <cstdio>
#include <string>
#include <cstring>

#include <switch.h>
#include <borealis.hpp>

#include "miiport.hpp"


Result init() {
    Result res;
    res = setsysInitialize();
    if(R_FAILED(res)) return res;
    res = miiInitialize(MiiServiceType_System);
    if(R_FAILED(res)) return res;
    return 0;
}

void deinit() {
    miiExit();
    setsysExit();
}

class FocusList : public brls::List {
    private:
        bool allowFocus = false;
    public:
        FocusList(bool focus) {
            allowFocus = focus;
        }
        void setAllowFocus(bool focus) {
            allowFocus = focus;
        }
        brls::View* getDefaultFocus() override {
            if(allowFocus) {
                return this->getContentView();
            }
            else {
                return nullptr;
            }
        }
};

const std::string TITLE = "MiiPort";

// todo: add button to open official mii menu, compare with swkbdShow in libnx and nn::mii::ShowMiiEdit in sdk
int main(int argc, char* argv[]) {
    brls::Logger::setLogLevel(brls::LogLevel::INFO);

    brls::Style custom_style = brls::Style::horizon();
    custom_style.Sidebar.width = 280;
    custom_style.Sidebar.marginLeft = 55;
    custom_style.Header.height = 25;
    custom_style.Header.fontSize = 25;
    custom_style.Header.rectangleWidth = 9;
    custom_style.List.Item.height = 65;
    custom_style.List.Item.heightWithSubLabel = 74;
    custom_style.List.spacing = 45;

    if (R_FAILED(init()) || !brls::Application::init(TITLE, custom_style, brls::Theme::horizon()))
    {
        brls::Logger::error("Unable to init Borealis application");
        return EXIT_FAILURE;
    }

    brls::TabFrame* rootFrame = new brls::TabFrame();
    rootFrame->setTitle(TITLE);
    rootFrame->setIcon(BOREALIS_ASSET("icon/MiiPort.png"));

    FocusList* aboutList = new FocusList(false);

    aboutList->addView(new brls::Header("About", false));
    aboutList->addView(new brls::Label(brls::LabelStyle::REGULAR, 
    "A tool to import and export Miis in a variety of formats.\n"
    "Supports importing the NFIF, charinfo, coredata and storedata formats.\n"
    "Exports full DBs in NFIF and individual characters in charinfo."
    , true));
    aboutList->addView(new brls::Header("How to use", false));
    aboutList->addView(new brls::Label(brls::LabelStyle::REGULAR, 
    "Place Mii files in \"sd:/MiiPort/miis/\".\n"
    "Give files a file extension that corresponds to their format i.e. \".charinfo\".\n"
    "Currently exports to \"sd:/MiiPort/miis/exportedDB.NFIF\" and \"sd:/MiiPort/miis/[name].charinfo\" or \"sd:/MiiPort/miis/[Mii ID].charinfo\" if the name can not be used. This will overwrite an existing file.\n"
    "For cordata files, a Mii ID can be specified in hexadecimal in the file name, otherwise a random one will be used.\n"
    "For example \"7C118DA34ADB46CB8FFC083BD00DC111.coredata\"\n"
    , true));

    const fs::path import_path = "/MiiPort/miis";

    FocusList* fileList = new FocusList(true);

    fs::create_directories(import_path);
    // iterator does not give unicode paths at all
    for(auto& entry: fs::directory_iterator(import_path)) {
        brls::ListItem* fileItem = new brls::ListItem(entry.path().filename());
        fileItem->getClickEvent()->subscribe([path{std::move(entry.path())}](brls::View* view) {
            Result res = importMiiFile(path);
            importNotify(res);
        });
        fileList->addView(fileItem);
    }
    if(fileList->getViewsCount() == 0){
        fileList->setAllowFocus(false);
        std::stringstream ss;
        ss << "No mii files.\nAdd files to " << import_path;
        fileList->addView(new brls::Label(brls::LabelStyle::REGULAR, ss.str(), true));
    }
    
    brls::List* exportList = new brls::List();
    brls::ListItem* exportItem = new brls::ListItem("Export Mii database as NFIF");
    exportItem->getClickEvent()->subscribe([import_path](brls::View* view) {
        fs::path path = import_path / "exportedDB.NFIF";
        Result res = miiDbExportToFile(path.c_str());
        if(R_FAILED(res)) {
            std::stringstream ss;
            ss << "Export error: 0x" << std::hex << res;
            brls::Application::notify(ss.str());
        }
        else {
            brls::Application::notify("Exported!");
        }
    });
    exportItem->setTextSize(28);
    exportList->addView(exportItem);
    brls::Label *note = new brls::Label(brls::LabelStyle::REGULAR, "Export individual Miis as charinfo", false);
    exportList->addView(note);

    {
        Result res;
        int count;
        const int max_miis = 100;
        charInfo miis[max_miis];
        res = getCharInfos(miis, max_miis, &count);
        if(R_FAILED(res)) {
            notifyError(res);
        }
        else {
            for(int i = 0; i < count; i++) {
                std::u16string utf16_name = miis[i].nickname;
                std::string utf8_name = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(utf16_name);
                fs::path export_path;
                // special characters seem poorly supported in paths. 
                // If the name uses any, use create ID for file name instead.
                // compare lengths to check for special characters.
                if(utf16_name.length() == utf8_name.length()) {
                    export_path = import_path / utf8_name += ".charinfo";
                }
                else {
                    export_path = import_path / getHexStr(&miis[i].create_id) += ".charinfo";
                }
                // todo: face icon for each Mii?
                brls::ListItem* miiItem = new brls::ListItem(utf8_name, "",getHexStr(&miis[i].create_id));
                miiItem->getClickEvent()->subscribe(
                [export_path{std::move(export_path)}, mii{std::move(miis[i])}]
                (brls::View* view) {
                    // todo: ask before replacing file?
                    writeToFile(export_path.c_str(), &mii);
                    brls::Application::notify("Exported!");
                });
                exportList->addView(miiItem);
            }
        }
    }


    rootFrame->addTab("Import", fileList);
    rootFrame->addTab("Export", exportList);
    rootFrame->addSeparator();
    rootFrame->addTab("About", aboutList);
    
    brls::Application::pushView(rootFrame);

    while (brls::Application::mainLoop()){};

    // Exit
    deinit();
    return EXIT_SUCCESS;
}