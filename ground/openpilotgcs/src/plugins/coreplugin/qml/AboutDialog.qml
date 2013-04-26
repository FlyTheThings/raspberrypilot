 /****************************************************************************
 **
 ** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
 ** All rights reserved.
 ** Contact: Nokia Corporation (qt-info@nokia.com)
 **
 ** This file is part of the examples of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:BSD$
 ** You may use this file under the terms of the BSD license as follows:
 **
 ** "Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions are
 ** met:
 **   * Redistributions of source code must retain the above copyright
 **     notice, this list of conditions and the following disclaimer.
 **   * Redistributions in binary form must reproduce the above copyright
 **     notice, this list of conditions and the following disclaimer in
 **     the documentation and/or other materials provided with the
 **     distribution.
 **   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
 **     the names of its contributors may be used to endorse or promote
 **     products derived from this software without specific prior written
 **     permission.
 **
 ** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 ** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 ** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 ** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 ** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 ** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 ** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 ** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 ** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

 import QtQuick 1.1
 
 
 // This is a tabbed pane element. Add a nested Rectangle to add a tab.
 TabWidget {
    // Define AuthorsModel as type
    property AuthorsModel authors: AuthorsModel {}

    id: tabs
    width: 640; height: 480
    // This  tab is for the GCS version information
    Rectangle {
        property string title: "OpenPilot GCS"
        anchors.fill: parent
        color: "#e3e3e3"

        Rectangle {
            anchors.fill: parent; anchors.margins: 20
            color: "#e3e3e3"
            Image {
                source: "../images/openpilot_logo_128.png"
                x: 0; y: 0; z: 100
                fillMode: Image.PreserveAspectFit
            }
            Flickable  {
                anchors.fill: parent
                anchors.centerIn: parent
               Text {
                   id: versionLabel
                   x: 156; y: 0
                   width: 430; height: 379
                   horizontalAlignment: Qt.AlignLeft
                   font.pixelSize: 12
                   wrapMode: Text.WordWrap
                // @var version exposed in authorsdialog.cpp
                   text: version
               }
            }
        }
    }

    //  This tab is for the authors/contributors/credits
    Rectangle {
        property string title: "Authors"
        anchors.fill: parent; color: "#e3e3e3"
        Rectangle {
            anchors.fill: parent; anchors.margins: 20
            color: "#e3e3e3"
            Text {
                id: description
                text: "<h4>These people have been key contributors to the OpenPilot project. Without the work of the people in this list, OpenPilot would not be what it is today.</h4><p>This list is sorted alphabetically by name</p>"
                width: 600
                wrapMode: Text.WordWrap

            }
            ListView {
                id: authorsView
                y: description.y + description.height + 20
                width: parent.width; height: parent.height - description.height - 20
                spacing: 3
                model: authors
                delegate: Text {
                    text: name
                }
                clip: true
            }
            ScrollDecorator {
                flickableItem: authorsView
            }
        }
    }
}
