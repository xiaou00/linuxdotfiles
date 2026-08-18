pragma Singleton
import Quickshell
import QtQuick
import qs.services

Singleton {
    function scaledWidth(ratio) {
        return Compositor.screenW * ratio / Compositor.screenScale
    }

    function scaledHeight(ratio) {
        return Compositor.screenH * ratio / Compositor.screenScale
    }
}
