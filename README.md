# OPEN-MP GPS Plugin forked from SA-MP GPS Plugin

[![sampctl](https://img.shields.io/badge/sampctl-samp--gps--plugin-2f2f2f.svg?style=for-the-badge)](https://github.com/kristoisberg/samp-gps-plugin)

**Notice:** This repository is not being actively maintained anymore. If anyone wishes to continue the development of the project, please create a fork of the repository and release future versions there.

This plugin offers a way of accessing and manipulating the data of San Andreas map nodes and finding paths between them. It is intended to be a modern and straightforward replacement for RouteConnector. The plugin uses a simple implementation of the A* algorithm for pathfinding. Finding a path from the top-leftmost node on the map to the bottom-rightmost node that consists of 684 nodes takes just a few milliseconds.


### Advantages over RouteConnector

* **Safer API** - Unlike RouteConnector, this plugins does not give you an array of nodes as the result of pathfinding. Instead of that, it gives you the ID of the found path that can be used later on. Each function (except `IsValidMapNode`, `IsValidPath` and `GetHighestMapNodeID`) returns an error code and the real result of them is passed by reference.
* **Compatibility** - RouteConnector has a compatibility issue with some part of YSI that makes it call a wrong public function instead of the actual `GPS_WhenRouteIsCalculated` callback. This plugin lets you call a custom callback and pass arguments to it. In addition to that, RouteConnector uses Intel Threading Building Blocks for threading that has caused numerous compatibility (and PEBCAK) issues on Linux servers. This plugin uses `std::thread` for threading and does not have any dependencies. This plugin is also compatible with PawnPlus and supports asynchronous pathfinding out of box.
* **Performance** - It works.


## Installation

Include in your code and begin using the library:

```pawn
#include <GPS>
```


## API


### Functions

`CreateMapNode(Float:x, Float:y, Float:z, &MapNode:nodeid)`

* Adds a node to the map and passes the ID of it to `nodeid`.

`DestroyMapNode(MapNode:nodeid)`

* If the specified map node is valid, returns `GPS_ERROR_NONE` and tries to destroy it, otherwise returns `GPS_ERROR_INVALID_NODE`. If the node is not a part of any path, it gets destroyed immediately, otherwise it will be destroyed once all paths containing it are destroyed, until that it will be excluded from pathfinding and several other features.

`bool:IsValidMapNode(MapNode:nodeid)`

* Returns if the map node with the specified ID is valid.

`GetMapNodePos(MapNode:nodeid, &Float:x, &Float:y, &Float:z)`

* If the specified map node is valid, returns `GPS_ERROR_NONE` and passes the position of it to `x`, `y` and `z`, otherwise returns `GPS_ERROR_INVALID_NODE`.

`CreateConnection(MapNode:source, MapNode:target, &Connection:connectionid)`

* If both specified map nodes are valid, returns `GPS_ERROR_NONE`, creates a connection from `source` to `target` and passes the ID of it to `connectionid`, otherwise returns `GPS_ERROR_INVALID_NODE`. **Note:** Connections are not double-sided may need to be added in both directions separately.

`DestroyConnection(Connection:connectionid)`

* If the specified connection is valid, returns `GPS_ERROR_NONE` and destroys it, otherwise returns `GPS_ERROR_INVALID_CONNECTION`.

`GetConnectionSource(Connection:connectionid, &MapNode:nodeid)`

* If the specified connection is valid, returns `GPS_ERROR_NONE` and passes the ID of the source node of it to `nodeid`, otherwise returns `GPS_ERROR_INVALID_CONNECTION`.

`GetConnectionTarget(Connection:connectionid, &MapNode:nodeid)`

* If the specified connection is valid, returns `GPS_ERROR_NONE` and passes the ID of the target node of it to `nodeid`, otherwise returns `GPS_ERROR_INVALID_CONNECTION`.

`GetMapNodeConnectionCount(MapNode:nodeid, &count)`

* If the specified map node is valid, returns `GPS_ERROR_NONE` and passes the amount of its connections to `count`, otherwise returns `GPS_ERROR_INVALID_NODE`. If `count` is larger than 2, the node is an intersection.

`GetMapNodeConnection(MapNode:nodeid, index, &Connection:connectionid)`

* If the specified map node is valid and it has a connection with the specified index, returns `GPS_ERROR_NONE` and passes the ID of the connection to `connectionid`, otherwise returns `GPS_ERROR_INVALID_NODE` or `GPS_ERROR_INVALID_CONNECTION` depending on the error.

`GetConnectionBetweenMapNodes(MapNode:source, MapNode:target, &Connection:connectionid)`

* If both specified map nodes are valid, tries to find a connection from `source` to `target`. If a connection is found, returns `GPS_ERROR_NONE` and passes the ID of the connection to `connectionid`, otherwise returns `GPS_ERROR_INVALID_CONNECTION`. If either of the specified map nodes is invalid, returns `GPS_ERROR_INVALID_NODE`.

`GetDistanceBetweenMapNodes(MapNode:first, MapNode:second, &Float:distance)`

* If both of the specified map nodes are valid, returns `GPS_ERROR_NONE` and passes the distance between them to `distance`, otherwise returns `GPS_ERROR_INVALID_NODE`.

`GetAngleBetweenMapNodes(MapNode:first, MapNode:second, &Float:angle)`

* If both of the specified map nodes are valid, returns `GPS_ERROR_NONE` and passes the angle between them to `angle`, otherwise returns `GPS_ERROR_INVALID_NODE`.

`GetMapNodeDistanceFromPoint(MapNode:nodeid, Float:x, Float:y, Float:z, &Float:distance)`

* If the specified map node is valid, returns `GPS_ERROR_NONE` and passes the distance of the map node from the specified position to `distance`, otherwise returns `GPS_ERROR_INVALID_NODE`.

`GetMapNodeAngleFromPoint(MapNode:nodeid, Float:x, Float:y, &Float:angle)`

* If the specified map node is valid, returns `GPS_ERROR_NONE` and passes the angle of the map node from the specified position to `angle`, otherwise returns `GPS_ERROR_INVALID_NODE`.

`GetClosestMapNodeToPoint(Float:x, Float:y, Float:z, &MapNode:nodeid, MapNode:ignorednode = INVALID_MAP_NODE_ID)`

* Passes the ID of the closest map node to the specified position to `nodeid`. If `ignorednode` is specified and it is the closest node to the position, it is ignored and the ID of the next closest node is passed to `nodeid` instead. Returns `GPS_ERROR_INVALID_NODE` if no nodes exist, otherwise returns `GPS_ERROR_NONE`.

`GetHighestMapNodeID()`

* Returns the ID of the map node with the highest ID. Could be used for iteration purposes.

`GetRandomMapNode(&MapNode:nodeid)`

* Passes the ID of a random map node, found using Mersenne Twister, to `nodeid`. Returns `GPS_ERROR_INVALID_NODE` if no map nodes exist, otherwise returns `GPS_ERROR_NONE`.

`SaveMapNodesToFile(const filename[])`

* Saves all existing nodes and their connections to a file with the specified name.

`FindPath(MapNode:source, MapNode:target, &Path:pathid)`

* If both of the specified map nodes are valid, returns `GPS_ERROR_NONE` and tries to find a path from `source` to `target` and pass its ID to `pathid`, otherwise returns `GPS_ERROR_INVALID_NODE`. If pathfinding fails, returns `GPS_ERROR_INVALID_PATH`.

`FindPathThreaded(MapNode:source, MapNode:target, const callback[], const format[] = "", {Float, _}:...)`

* If both of the specified map nodes are valid, returns `GPS_ERROR_NONE` and tries to find a path from `source` to `target`. After pathfinding is finished, calls the specified callback and passes the path ID (could be `INVALID_PATH_ID` if pathfinding fails) and the specified arguments to it.

`Task:FindPathAsync(MapNode:source, MapNode:target)`

* Pauses the current function and continues it after it is finished. Throws an AMX error if pathfinding fails for any reason. Only available if PawnPlus is included before GPS. Usage explained below.

`bool:IsValidPath(Path:pathid)`

* Returns if the path with the specified ID is valid.

`GetPathSize(Path:pathid, &size)`

* If the specified path is valid, returns `GPS_ERROR_NONE` and passes the amount of nodes in it to `size`, otherwise returns `GPS_ERROR_INVALID_PATH`.

`GetPathNode(Path:pathid, index, &MapNode:nodeid)`

* If the specified path is valid and the index contains a node, returns `GPS_ERROR_NONE` and passes the ID of the node at that index to `nodeid`, otherwise returns `GPS_ERROR_INVALID_PATH` or `GPS_ERROR_INVALID_NODE` depending on the error.

`GetPathNodeIndex(Path:pathid, MapNode:nodeid, &index)`

* If the specified path is valid and the specified map node is a part of the path, returns `GPS_ERROR_NONE` and passes the index of the map node to `index`, otherwise returns `GPS_ERROR_INVALID_PATH` or `GPS_ERROR_INVALID_NODE` depending on the error.

`GetPathLength(Path:pathid, &Float:length)`

* If the specified path is valid, returns `GPS_ERROR_NONE` and passes the length of the path in metres to `length`, otherwise returns `GPS_ERROR_INVALID_PATH`.

`DestroyPath(Path:pathid)`

* If the specified path is valid, returns `GPS_ERROR_NONE` and destroys the path, otherwise returns `GPS_ERROR_INVALID_PATH`.


### Error codes

* `GPS_ERROR_NONE` - The function was executed successfully.
* `GPS_ERROR_INVALID_PARAMS` - An invalid amount of arguments was passed to the function. Should never happen without the PAWN compiler noticing it unless the versions of the plugin and include are different.
* `GPS_ERROR_INVALID_PATH` - An invalid path ID as passed to the function or threaded pathfinding was not successful.
* `GPS_ERROR_INVALID_NODE` - An invalid map node ID/index was passed to the function or `GetClosestMapNodeToPoint` or `GetRandomMapNode` failed because no map nodes exist.
* `GPS_ERROR_INVALID_CONNECTION` - An invalid connection ID/index was passed to the function.
* `GPS_ERROR_INTERNAL` - An internal error happened - threaded pathfinding failed because dispatching a thread failed.


## Examples


```pawn
CMD:pathtols(playerid, params[]) {

    new Float:x, Float:y, Float:z, MapNode:start;
    GetPlayerPos(playerid, x, y, z);

    if (GetClosestMapNodeToPoint(x, y, z, start) != GPS_ERROR_NONE) {
        return SendClientMessage(playerid, COLOR_RED, "Finding a node near you failed, GPS.dat was not loaded.");
    }

    new MapNode:target;

    if (GetClosestMapNodeToPoint(1258.7352, -2036.7100, 59.4561, target)) { // this is also valid since the value of GPS_ERROR_NONE is 0.
        return SendClientMessage(playerid, COLOR_RED, "Finding a node near LSPD failed, GPS.dat was not loaded.");
    }

    new Path:_path = Path:-1;

    if (FindPath(start, target, _path)) {
        return SendClientMessage(playerid, COLOR_RED, "Pathfinding failed for some reason, you should store this error code and print it out since there are multiple ways it could fail.");
    }

    SendClientMessage(playerid, COLOR_WHITE, "Finding the path...");

    if (!IsValidPath(_path)) {
        return SendClientMessage(playerid, COLOR_RED, "Pathfinding failed!");
    }

    new string[128], size, Float:length;
    GetPathSize(_path, size);
    GetPathLength(_path, length);

    format(string, sizeof(string), "Found a path Amount of nodes: %i, length: %fm.", size, length);
    SCM(playerid, -1, string);

    new MapNode:nodeid;

    for (new index; index < size; index++) {

        GetPathNode(_path, index, nodeid);
        GetMapNodePos(nodeid, x, y, z);

        CreateDynamicPickup(1318, 1, x, y, z);

        printf("%f %f %f", x, y, z;
    }

    DestroyPath(_path);
    return 1;
}
```


## For the moment I have not implemented FindPathThreaded because I was lazy.

## Testing

To test, simply compile and paste:


## Credits

* kristo - Creator of the plugin.
* Gamer_Z - Creator of the original RouteConnector plugin which helped me understand the structure of GPS.dat and influenced this plugin a lot, the author of the original `GPS.dat`.
* NaS - Author of the fixed `GPS.dat` distributed with the plugin.
* Southclaws, IllidanS4, Hual - Helped me with the plugin in major ways (there were other helpful people as well, I appreciate it all).
