// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import "Logic.js" as Game

Rectangle {
    id: region
    width: 48
    height: 48
    color: "green"

    property int row: -1;
    property int column: -1;
    property string text: "n/a";
    signal clicked(int row, int column);

    Text {
        anchors.centerIn: region
        text: region.text;
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            var z = Engine.makeMove(0, column);
            console.log('Z ', z);
            var f = findRegion(Engine.getLastAIcolumn(), Engine.getLastAIrow());
            console.log('found ', f);
            console.log('lastColumn ', Engine.getLastAIcolumn(), ' last row ', Engine.getLastAIrow());

            region.color = "blue";
            changeColorPlayer(1, f);
        }
    }
}
