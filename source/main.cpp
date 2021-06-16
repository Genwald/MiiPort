#include <cstdio>
#include <string>
#include <cstring>

#include <switch.h>
// needed to access scrollview private members in TopScrollList
// todo: don't do this. Investigate alternatives. Fork borealis? Maybe possible on newer version?
#define private protected
#include <borealis.hpp>
#undef private

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

// modify so that the list can be made unfocusable, useful if the list has no focusable children.
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

// modify so that the view is scrolled so that focused item is at the top instead of the middle of the screen.
class TopScrollList : public brls::List {
    bool updateScrolling(bool animated) {
        // Don't scroll if layout hasn't been called yet
        if (!this->ready || !this->contentView)
            return false;

        float contentHeight = (float)this->contentView->getHeight();

        // Ensure content is laid out too
        if (contentHeight == 0)
            return false;

        brls::View* focusedView = brls::Application::getCurrentFocus();
        // Edited here so that the focused element is at the top of the view
        float newScroll = -(this->scrollY * contentHeight) - ((float)(focusedView->getY() - 15) - (float)this->getY());

        // Bottom boundary
        if ((float)this->y + newScroll + contentHeight < (float)this->bottomY)
            newScroll = (float)this->height - contentHeight;

        // Top boundary
        if (newScroll > 0.0f)
            newScroll = 0.0f;

        // Apply 0.0f -> 1.0f scale
        newScroll = abs(newScroll) / contentHeight;

        //Start animation
        this->startScrolling(animated, newScroll);

        return true;
    }

    // override so that my updateScrolling gets used
    void onChildFocusGained(brls::View* child) override {
        if (!this->ready)
            return;

        if (child != this->contentView)
            return;

        // Start scrolling
        updateScrolling(true);

        brls::View::onChildFocusGained(child);
    }
};

// modify brls::Header so that they can be focused, but don't highlight
class FocusHeader : public brls::Header {
    // make focusable
    brls::View* getDefaultFocus() override {
        return this;
    }
    public:
        FocusHeader(std::string label, bool separator = true, std::string sublabel = "") 
            : Header(label, separator, sublabel)
        {}

        // no highlight
        void onFocusGained() override {
            this->focused = true;

            this->focusEvent.fire(this);

            if (this->hasParent())
                this->getParent()->onChildFocusGained(this);
        }
};

const std::string TITLE = "MiiPort";

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

    TopScrollList* aboutList = new TopScrollList();

    aboutList->addView(new FocusHeader("About", false));
    aboutList->addView(new brls::Label(brls::LabelStyle::REGULAR, 
    "A tool to import and export Miis in a variety of formats.\n"
    "Supports importing the NFIF, charinfo, coredata and storedata formats.\n"
    "Exports full DBs in NFIF and individual characters in charinfo."
    , true));

    aboutList->addView(new FocusHeader("How to use", false));
    aboutList->addView(new brls::Label(brls::LabelStyle::REGULAR, 
    "Place Mii files in \"sd:/MiiPort/miis/\".\n"
    "Give files a file extension that corresponds to their format i.e. \".charinfo\".\n"
    "Currently exports to \"sd:/MiiPort/miis/exportedDB.NFIF\" and \"sd:/MiiPort/miis/[name].charinfo\" or \"sd:/MiiPort/miis/[Mii ID].charinfo\" if the name can not be used. This will overwrite an existing file.\n"
    "For cordata files, a Mii ID can be specified in hexadecimal in the file name, otherwise a random one will be used.\n"
    "For example \"7C118DA34ADB46CB8FFC083BD00DC111.coredata\"\n"
    , true));

    aboutList->addView(new FocusHeader("QR key info", false));
    aboutList->addView(new brls::Label(brls::LabelStyle::REGULAR, 
    "In order to import Miis from a qr code, you must supply the Mii QR key. This is needed to decrypt the Mii data stored in Mii QR codes.\n\n"
    "You can find this on the internet by searching for \"Mii QR key\".\n"
    "This program looks for the key in hex in the file \"/MiiPort/qrkey.txt\".\n"
    "It will accept it in a variety of formats such as:\n"
    "\"[0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]\"\n"
    "or \"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\"\n"
    , true));

    const fs::path import_path = "/MiiPort/miis";

    FocusList* fileList = new FocusList(true);

    fs::create_directories(import_path);
    std::vector<fs::directory_entry> dirEntVec;
    // iterator does not give unicode paths at all
    std::copy(fs::directory_iterator(import_path), fs::directory_iterator(), std::back_inserter(dirEntVec));
    std::sort(dirEntVec.begin(), dirEntVec.end());
    for(fs::path path: dirEntVec) {
        brls::ListItem* fileItem = new brls::ListItem(path.filename());
        if(path.extension() == ".jpg") {
            fileItem->setThumbnail(path);
        }
        fileItem->getClickEvent()->subscribe([path{std::move(path)}](brls::View* view) {
            Result res = importMiiFile(path);
            errorNotify(res);
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
            errorNotify(res);
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
            errorNotify(res);
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
                [export_path{std::move(export_path)}, mii{miis[i]}]
                (brls::View* view) {
                    // todo: ask before replacing file?
                    writeToFile(export_path.c_str(), &mii);
                    brls::Application::notify("Exported!");
                });
                miiItem->registerAction("Show Mii QR", brls::Key::Y, [mii{miis[i]}, name{utf8_name}] {
                    ver3StoreData qr_data;
                    charInfoToVer3StoreData(&mii, &qr_data);
                    Result res = showQrPopup(&qr_data, name);
                    if(R_FAILED(res)) {
                        errorNotify(res);
                    }
                    return true;
                });
                exportList->addView(miiItem);
            }
        }
    }

    rootFrame->addTab("Import", fileList);
    rootFrame->addTab("Export", exportList);
    rootFrame->addSeparator();
    rootFrame->addTab("About", aboutList);
    rootFrame->registerAction("Show Mii applet", brls::Key::X, [] {
        miiLaShowMiiEdit(MiiSpecialKeyCode_Special);
        return true;
    });
    
    brls::Application::pushView(rootFrame);

    while (brls::Application::mainLoop()){};

    // Exit
    deinit();
    return EXIT_SUCCESS;
}