import QtQuick 1.1
import com.nokia.meego 1.0
import "Logic.js" as Game

Page {
    id: mainPage

    Component.onCompleted: {
        Game.createRegions();
    }

    function findRegion(column, row) {
        return Game.findRegion(column, row);
    }

    function changeColorPlayer(player, number) {
        Game.changeColorPlayer(player, number);
    }

    Rectangle {
        id: background

        anchors.fill: parent

    }
}
