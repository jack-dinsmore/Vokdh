#include "view.h"
#include "allViews.h"

bool View::createDeviceIndependentResources(HINSTANCE hInst) {
    HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(writeFactory), reinterpret_cast<IUnknown**>(&writeFactory));
    if (FAILED(hr)) {
        postMessage(MESSAGE_TYPE::M_TERMINATE, "Direct write factory creation failed.");
        return false;
    }

    return extraCreateDeviceIndependentResources(hInst);
}

bool View::createDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) {
    styleMap = styleMap.summon();
    bgColor = (D2D1::ColorF)styleMap["colors-translation"]["background"];
    renderTarget->CreateSolidColorBrush((D2D1::ColorF)styleMap["colors-translation"]["foreground"], &foreBrush);
    renderTarget->CreateSolidColorBrush((D2D1::ColorF)styleMap["colors-translation"]["background"], &backBrush);
    renderTarget->CreateSolidColorBrush((D2D1::ColorF)styleMap["colors-translation"]["english"], &englishBrush);
    renderTarget->CreateSolidColorBrush((D2D1::ColorF)styleMap["colors-translation"]["tobair"], &tobairBrush);
    renderTarget->CreateSolidColorBrush((D2D1::ColorF)styleMap["colors-translation"]["dark-background"], &darkBGBrush);
    return extraCreateDeviceDependentResources(renderTarget);
}

void View::discardDeviceDependentResources() {
    SafeRelease(&darkBGBrush);
    SafeRelease(&foreBrush);
    SafeRelease(&englishBrush);
    SafeRelease(&tobairBrush);
    SafeRelease(&backBrush);
    extraDiscardDeviceDependentResources();
}

void View::draw(ID2D1HwndRenderTarget* renderTarget) {
    if (stageResize) {
        resize(renderTarget->GetSize().width, renderTarget->GetSize().height);
        stageResize = false;
    }
    extraDraw(renderTarget);
}




ViewHandler::ViewHandler(TextTree& textTree) {
    translationView = new TranslationView(textTree);
    helpView = new HelpView(textTree);
    dictionaryView = new DictionaryView(textTree);
    grammarView = new GrammarView(textTree);
    prepositionsView = new PrepositionsView(textTree);
    shortWordsView = new ShortWordsView(textTree);
    textView = new TextView(textTree);
    switchTo(VIEW_TYPE::TRANSLATION);
}

ViewHandler::~ViewHandler() {
    delete translationView;
    delete helpView;
}

void ViewHandler::createDeviceIndependentResources(HINSTANCE hInst) {
    translationView->createDeviceIndependentResources(hInst);
    helpView->createDeviceIndependentResources(hInst);
    dictionaryView->createDeviceIndependentResources(hInst);
    grammarView->createDeviceIndependentResources(hInst);
    prepositionsView->createDeviceIndependentResources(hInst);
    shortWordsView->createDeviceIndependentResources(hInst);
    textView->createDeviceIndependentResources(hInst);
}

View* ViewHandler::view() {
    switch (viewType) {
    case VIEW_TYPE::TRANSLATION:
        return translationView;
    case VIEW_TYPE::HELP:
        return helpView;
    case VIEW_TYPE::DICTIONARY:
        return dictionaryView;
    case VIEW_TYPE::GRAMMAR:
        return grammarView;
    case VIEW_TYPE::PREPOSITIONS:
        return prepositionsView;
    case VIEW_TYPE::SHORT_WORDS:
        return shortWordsView;
    case VIEW_TYPE::TEXT:
        return textView;
    }
    return nullptr;
}

bool ViewHandler::handleControlKeyPress(int key) {
    switch (key) {
    case 'D':
        switchTo(VIEW_TYPE::DICTIONARY);
        // Dictionary
        return true;
    case 'T':
        switchTo(VIEW_TYPE::TEXT);
        // Dictionary
        return true;
    case 'G':
        switchTo(VIEW_TYPE::GRAMMAR);
        // Grammar page
        return true;
    case 'P':
        // Preposition page
        switchTo(VIEW_TYPE::PREPOSITIONS);
        return true;
    case 'W':
        // Short words page
        switchTo(VIEW_TYPE::SHORT_WORDS);
        return true;
    case 'H':
        switchTo(VIEW_TYPE::HELP);
        return true;
    }
    return view()->handleControlKeyPress(key);
}

bool ViewHandler::handleKeyPress(int key) {
    if (view()->handleKeyPress(key)) {
        return true;
    }
    if (key == VK_ESCAPE) {
        if (viewType != VIEW_TYPE::TRANSLATION) {
            switchTo(VIEW_TYPE::TRANSLATION);
            return true;
        }
    }
    return false;
}

void ViewHandler::switchTo(VIEW_TYPE vt) {
    int width = view()->screenWidth;
    int height = view()->screenHeight;
    if (viewType == vt && viewType != VIEW_TYPE::TRANSLATION) {
        switchTo(VIEW_TYPE::TRANSLATION);
        return;
    }
    if (viewType == VIEW_TYPE::TRANSLATION) {
        // Draw grayed-out thing.
        drawGrayOut = true;
    }
    viewType = vt;
    view()->resize(width, height);
    view()->handleReopen();
}

void ViewHandler::draw(ID2D1HwndRenderTarget* rt) {
    if (drawGrayOut) {
        ID2D1SolidColorBrush* grayoutBrush;
        rt->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 0.5), &grayoutBrush);
        rt->FillRectangle({ 0, 0, (FLOAT)view()->screenWidth, (FLOAT)view()->screenHeight }, grayoutBrush);
        SafeRelease(&grayoutBrush);
        drawGrayOut = false;
    }

    view()->draw(rt);
}
