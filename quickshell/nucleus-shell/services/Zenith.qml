// Zenith.properties
pragma Singleton
import QtQuick
import Quickshell
import Quickshell.Io
import qs.config
import qs.modules.functions

Singleton {
    // current state and shit
    property string currentChat: "default"
    property string currentModel: "gpt-4o-mini"
    property string pendingInput: ""
    property bool loading: zenithProcess.running

    // signals (needed for ui loading)
    signal chatsListed(string text)
    signal chatLoaded(string text)
    signal aiReply(string text)

    // process to load data and talk to zenith

    Timer {
        interval: 1000
        repeat: true 
        running: true 
        onTriggered: listChatsProcess.running = true;
    }

    Process {
        id: listChatsProcess
        command: ["ls", FileUtils.trimFileProtocol(Directories.config) + "/zenith/chats"]
        running: true

        stdout: StdioCollector {
            onStreamFinished: chatsListed(text)
        }
    }

    Process {
        id: chatLoadProcess

        stdout: StdioCollector {
            onStreamFinished: chatLoaded(text)
        }
    }

    Process {
        id: zenithProcess

        command: [
            "zenith",
            "--api", Config.runtime.misc.intelligence.apiKey,
            "--chat", currentChat,
            "-a",
            "--model", currentModel,
            pendingInput
        ]

        stdout: StdioCollector {
            onStreamFinished: {
                if (text.trim() !== "")
                    aiReply(text.trim())
            }
        }
    }

    // api shit

    function loadChat(chatName) {
        chatLoadProcess.command = [
            "cat",
            FileUtils.trimFileProtocol(Directories.config)
                + "/zenith/chats/" + chatName + ".txt"
        ]
        chatLoadProcess.running = true
    }

    function send() {
        zenithProcess.running = true
    }
}
