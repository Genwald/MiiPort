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

int main(int argc, char* argv[]) {
    brls::Logger::setLogLevel(brls::LogLevel::INFO);

    // todo: handle init errors
    init();

    brls::Style custom_style = brls::Style::horizon();
    custom_style.Sidebar.width = 280;
    custom_style.Sidebar.marginLeft = 55;

    if (!brls::Application::init(TITLE, custom_style, brls::Theme::horizon()))
    {
        brls::Logger::error("Unable to init Borealis application");
        return EXIT_FAILURE;
    }

    brls::TabFrame* rootFrame = new brls::TabFrame();
    rootFrame->setTitle(TITLE);
    rootFrame->setIcon(BOREALIS_ASSET("icon/MiiPort.png"));

    FocusList* aboutList = new FocusList(false);

    // todo: use headers or not? Change style maybe? margins?
    aboutList->addView(new brls::Header("About", false));
    aboutList->addView(new brls::Label(brls::LabelStyle::REGULAR, 
    "A tool to import Miis in a variety of formats.\nSupports the NFIF, charinfo, and coredata formats.\nExports only in NFIF."
    , true));
    aboutList->addView(new brls::Header("How to use", false));
    aboutList->addView(new brls::Label(brls::LabelStyle::REGULAR, 
    "Place Mii files in \"sd:/MiiPort/miis/\".\nGive files a file extension that corresponds to their format i.e. \".charinfo\".\nA Mii ID can be specified in hexadecimal in the file name of a coredata file, otherwise a random one will be used.\n For example \"7C118DA34ADB46CB8FFC083BD00DC111.coredata\"\nCurrently exports to \"sd:/MiiPort/miis/exportedDB.NFIF\" and will overwrite an existing file."
    , true));


    const fs::path import_path = "/MiiPort/miis";

    FocusList* fileList = new FocusList(true);

    fs::create_directories(import_path);
    for(auto& entry: fs::directory_iterator(import_path)) {
        brls::ListItem* fileItem = new brls::ListItem(entry.path().filename());
        fileItem->getClickEvent()->subscribe([entry](brls::View* view) {
            //todo: give human readable error for "database full" instead of code
            Result res = importMiiFile(entry.path());
            if (res == 0xFFFFFFFF) {
                brls::Application::notify("File extension not recognized");
            }
            else if(R_FAILED(res)) {
                std::stringstream ss;
                ss << "Import error: 0x" << std::hex << res;
                brls::Application::notify(ss.str());
            }
            else {
                brls::Application::notify("Imported!");
            }
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
    brls::ListItem* exportItem = new brls::ListItem("Export mii database as NFIF");
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
    exportList->addView(exportItem);

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