#include "test-app.h"
#include "utils/BaseUtil.h"

#include "wingui/WinGui.h"
#include "wingui/Layout.h"
#include "wingui/Window.h"
#include "wingui/ListBoxCtrl.h"
#include "wingui/ButtonCtrl.h"
#include "wingui/wingui2.h"

#include "utils/Log.h"

// in TestTab.cpp
extern int TestTab(HINSTANCE hInstance, int nCmdShow);
// in TestLayout.cpp
extern int TestLayout(HINSTANCE hInstance, int nCmdShow);
// in TestLice.cpp
extern int TestLice(HINSTANCE hInstance, int nCmdShow);

HINSTANCE gHinst = nullptr;

static void LaunchTabs() {
    TestTab(gHinst, SW_SHOW);
}

static void LaunchLayout() {
    TestLayout(gHinst, SW_SHOW);
}

/*
static void LaunchLice() {
    TestLice(gHinst, SW_SHOW);
}
*/

static ILayout* CreateMainLayout(HWND hwnd) {
    auto* vbox = new VBox();

    vbox->alignMain = MainAxisAlign::MainCenter;
    vbox->alignCross = CrossAxisAlign::CrossCenter;

    {
        auto b = CreateButton(hwnd, "Tabs test", LaunchTabs);
        vbox->AddChild(b);
    }

    {
        auto b = CreateButton(hwnd, "Layout test", LaunchLayout);
        vbox->AddChild(b);
    }

    /*
    {
        auto b = CreateButton(hwnd, "Lice test", LaunchLice);
        vbox->AddChild(b);
    }
    */
    auto padding = new Padding(vbox, DefaultInsets());
    return padding;
}

void TestApp(HINSTANCE hInstance) {
    gHinst = hInstance;

    // return TestDirectDraw(hInstance, nCmdShow);
    // return TestTab(hInstance, nCmdShow);
    // return TestLayout(hInstance, nCmdShow);

    auto w = new Window();
    w->backgroundColor = MkColor((u8)0xae, (u8)0xae, (u8)0xae);
    w->SetTitle("this is a title");
    w->initialPos = {100, 100};
    w->initialSize = {480, 640};
    bool ok = w->Create(0);
    CrashIf(!ok);

    auto l = CreateMainLayout(w->hwnd);
    w->onSize = [&](SizeEvent* args) {
        HWND hwnd = args->hwnd;
        int dx = args->dx;
        int dy = args->dy;
        if (dx == 0 || dy == 0) {
            return;
        }
        LayoutToSize(l, {dx, dy});
        InvalidateRect(hwnd, nullptr, false);
    };

    // important to call this after hooking up onSize to ensure
    // first layout is triggered
    w->SetIsVisible(true);

    auto res = RunMessageLoop(nullptr, w->hwnd);
    delete w;
    return;
}

using namespace wg;

struct CommandPaletteWnd : Wnd {
    ~CommandPaletteWnd() override {
        delete mainLayout;
    }
    Button* btn = nullptr;
    Edit* editQuery = nullptr;
    ListBox *listBoxResults = nullptr;

    LayoutBase* mainLayout = nullptr;

    bool Create();

    void OnDestroy() override;
    void QueryChanged();
    void SelectionChanged();
    void ButtonClicked();
};

void CommandPaletteWnd::QueryChanged() {
    logf("query changed\n");
}

void CommandPaletteWnd::SelectionChanged() {
    logf("selection changed\n");
}

void CommandPaletteWnd::ButtonClicked() {
    logf("button clicked\n");
}

void CommandPaletteWnd::OnDestroy() {
    ::PostQuitMessage(0);
}

bool CommandPaletteWnd::Create() {
    {
        CreateCustomArgs args;
        args.title = L"Command Palette";
        args.visible = false;
        CreateCustom(args);
    }
    if (!hwnd) {
        return false;
    }

    auto vbox = new VBox();
    vbox->alignMain = MainAxisAlign::MainStart;
    vbox->alignCross = CrossAxisAlign::Stretch;

    {
        auto c = new Edit();
        EditCreateArgs args;
        args.parent = hwnd;
        args.isMultiLine = false;
        args.withBorder = true;
        args.cueText = "a cue text";
        HWND ok = c->Create(args);
        CrashIf(!ok);
        c->maxDx = 150;
        c->onTextChanged = std::bind(&CommandPaletteWnd::QueryChanged, this);
        editQuery = c;
        vbox->AddChild(c);
    }

    {
        auto c = new ListBox();
        c->idealSizeLines = 32;
        c->SetInsetsPt(4, 0);
        auto wnd = c->Create(hwnd);
        CrashIf(!wnd);

        auto m = new ListBoxModelStrings();
        m->strings.Append("Hello");
        m->strings.Append("My friend");
        c->SetModel(m);
        c->onSelectionChanged = std::bind(&CommandPaletteWnd::SelectionChanged, this);
        listBoxResults = c;
        vbox->AddChild(c, 1);
    }
    {
        auto c = new Button();
        auto wnd = c->Create(hwnd);
        CrashIf(!wnd);
        c->SetText(L"A button");
        c->onClicked = std::bind(&CommandPaletteWnd::ButtonClicked, this);
        btn = c;
        vbox->AddChild(c);
    }

    auto padding = new Padding(vbox, DpiScaledInsets(hwnd, 4, 8));
    mainLayout = padding;

    LayoutAndSizeToContent(mainLayout, 520, 720, hwnd);
    SetIsVisible(true);
    ::SetFocus(editQuery->hwnd);
    return true;
}

void TestWingui() {
    auto w = new CommandPaletteWnd();
    bool ok = w->Create();
    CrashIf(!ok);
    auto res = RunMessageLoop(nullptr, w->hwnd);
    delete w;
    return;
}