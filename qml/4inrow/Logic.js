var _MAX_REGIONS_HORIZONTAL = 10;
var _MAX_REGIONS_VERTICAL = 8;
var _REGION_SIZE = 48;
var _REGION_PADDING = 2;
var _REGIONS_NUM = _MAX_REGIONS_HORIZONTAL * _MAX_REGIONS_VERTICAL;

var array_regions = new Array(_REGIONS_NUM);
var array_indicator = 0;
var component_region;

function createRegions() {
    var counter = 0;
    for (var j = 0; j < _MAX_REGIONS_VERTICAL; j++) {
        for (var i = 0; i < _MAX_REGIONS_HORIZONTAL; i++) {
            createRegion(i, j);
        }
    }
}

function createRegion(column, row) {
    if (component_region == null)
        component_region = Qt.createComponent("RegionItem.qml");

    if (component_region.status == Component.Ready) {
        var dynamicObject = component_region.createObject(background);
        if (dynamicObject == null) {
            console.log("error creating region");
            console.log(component_region.errorString());
            return false;
        }
        dynamicObject.x = 480 - (column * _REGION_SIZE + column * _REGION_PADDING);
        dynamicObject.y = 384 - (row * _REGION_SIZE + row * _REGION_PADDING);
        dynamicObject.visible = true;
        dynamicObject.row = row;
        dynamicObject.column = column;
        dynamicObject.text = array_indicator;

        array_regions[array_indicator] = dynamicObject;
        array_indicator++;
    } else {
        console.log("error loading region component");
        console.log(component_region.errorString());
        return false;
    }
    return true;
}

function findRegion(column, row) {
    for (var i = 0; i < _REGIONS_NUM; i++) {
        if (array_regions[i] != null)
            if (array_regions[i].row == row &&
                    array_regions[i].column == column) {
                console.log("were looking for column ", column, " and row ", row);
                console.log("received column ", array_regions[i].column, " and row ", array_regions[i].row);
                return i;
            }
    }
}

function changeColorPlayer(player, number) {
    if (array_regions[number] != null) {
        if (player == 0)
            array_regions[number].color = "blue";
        else
            array_regions[number].color = "red";
    }
}

function parseData(column, row) {
    console.log('column ', column, ' row ', row);
}
