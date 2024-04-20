#include "main.h"
#include "container.h"
#include "file.h"
#include "connection.h"
#include "pathfinder.h"

#include <random>

StringView GPSComponent::componentName() const {
  return "OMP GPS Plugin";
}

SemanticVersion GPSComponent::componentVersion() const {
  

  return SemanticVersion(0, 0, 1);
}

void GPSComponent::onLoad(ICore *c) {

  core_ = c;

  getCore() = c;
  get() = this;

  setAmxLookups(c);

}

void GPSComponent::onInit(IComponentList *components) {

  pawn_component_ = components->queryComponent<IPawnComponent>();
  if (!pawn_component_) 
  {

    StringView name = componentName();
    core_->logLn(LogLevel::Error,"Error loading component %.*s: Pawn component not loaded", name.length(), name.data());

    TimePoint lastCheck = Time::now();
    return;
  }

  core_->getEventDispatcher().addEventHandler(this);
  setAmxFunctions(pawn_component_->getAmxFunctions());
  setAmxLookups(components);

  pawn_component_->getEventDispatcher().addEventHandler(this);

  core_->logLn(LogLevel::Message, "\n\n"
    "    | OMP-GPS-Plugin | open.mp | %s"
    "\n"
    "    |--------------------------------------------"
    "\n"
    "    | Author: kristo, port: EquiNoxAdv"
    "\n"
    "    | Compiled: %s at %s"
    "\n"
    "    |--------------------------------------------------------------"
    "\n\n",&__DATE__[7], __DATE__, __TIME__);

  
    auto line_count = 0, node_count = 0, connection_count = 0;

    if (!File::LoadNodes("GPS.dat", line_count, node_count, connection_count))
    {
        core_->logLn(LogLevel::Message, "[GPS plugin]: Failed to open \"GPS.dat\"!");
    }
    else
    {
        core_->logLn(LogLevel::Message, "[GPS plugin]: Read %i lines, loaded %i nodes and %i connections.", line_count, node_count, connection_count);
    }
}


void GPSComponent::onReady() {

}

void GPSComponent::onAmxLoad(IPawnScript &script) {
    
    pawn_natives::AmxLoad(script.GetAMX());
};

void GPSComponent::onAmxUnload(IPawnScript &script) { 

};


void GPSComponent::onTick(Microseconds elapsed, TimePoint now) {


}

void GPSComponent::onFree(IComponent *component) {

  if (component == pawn_component_ || component == this) {
    
    if (pawn_component_) {

        core_->getEventDispatcher().removeEventHandler(this);
        pawn_component_->getEventDispatcher().removeEventHandler(this);

        setAmxFunctions();
        setAmxLookups();
    }

    pawn_component_ = nullptr;
  }
}

void GPSComponent::reset() {}

void GPSComponent::free() {
  delete this;
}

ICore *&GPSComponent::getCore() {
  static ICore *core{};

  return core;
}

GPSComponent*& GPSComponent::get() {
    static GPSComponent* component{};

    return component;
}



//Natives

SCRIPT_API(CreateMapNode, int(float X, float Y, float Z, int* nodeID))
{

    Container::LockExclusive();

    const auto id = Container::Nodes::AllocateID();

    Container::Nodes::Add(id, X, Y, Z);
    Container::UnlockExclusive();

    *nodeID = id;

    return GPS_ERROR_NONE;;
}

SCRIPT_API(DestroyMapNode, int(int nodeID))
{
    const auto id = nodeID;
    const auto node = Container::Nodes::Find(id);

    if (node == nullptr)
    {
        return GPS_ERROR_INVALID_NODE;
    }

    Container::LockExclusive();
    Container::Nodes::SetForDeletion(node);
    Container::UnlockExclusive();

    return GPS_ERROR_NONE;
}

SCRIPT_API(IsValidMapNode, bool(int nodeID))
{
    const auto id = nodeID;

    return Container::Nodes::Find(id) != nullptr;

}

SCRIPT_API(GetMapNodePos, int(int nodeID, float* X, float* Y, float* Z))
{
    const auto id = nodeID;
    const auto node = Container::Nodes::Find(id);

    if (node == nullptr)
    {
        return GPS_ERROR_INVALID_NODE;
    }

    *X = node->getX();
    *Y = node->getY();
    *Z = node->getZ();

    return GPS_ERROR_NONE;
}

SCRIPT_API(CreateConnection, int(int source, int target, int* connectionid))
{

    const auto source_id = source, target_id = target;

    Container::LockExclusive();
    const auto id = Container::Connections::Add(source_id, target_id);
    Container::UnlockExclusive();

    if (id == -1)
    {
        return GPS_ERROR_INVALID_NODE;
    }
    
    *connectionid = id;

    return GPS_ERROR_NONE;
}

SCRIPT_API(DestroyConnection, int(int connectionid))
{
    const auto id = connectionid;
    const auto connection = Container::Connections::Find(id);

    if (connection == nullptr)
    {
        return GPS_ERROR_INVALID_CONNECTION;
    }

    Container::LockExclusive();
    Container::Connections::Delete(connection);
    Container::UnlockExclusive();

    return GPS_ERROR_NONE;
}

SCRIPT_API(GetConnectionSource, int(int connectionid, int* nodeid))
{
 
    const auto id = connectionid;
    const auto connection = Container::Connections::Find(id);

    if (connection == nullptr)
    {
        return GPS_ERROR_INVALID_CONNECTION;
    }

    *nodeid = connection->getSource()->getID();

    return GPS_ERROR_NONE;
}

SCRIPT_API(GetConnectionTarget, int(int connectionid, int* nodeid))
{

    const auto id = connectionid;
    const auto connection = Container::Connections::Find(id);

    if (connection == nullptr)
    {
        return GPS_ERROR_INVALID_CONNECTION;
    }

    *nodeid = connection->getTarget()->getID();

    return GPS_ERROR_NONE;
}

SCRIPT_API(GetMapNodeConnectionCount, int(int nodeid, int* count))
{

    const auto id = nodeid;
    const auto node = Container::Nodes::Find(id);

    if (node == nullptr)
    {
        return GPS_ERROR_INVALID_NODE;
    }

    *count = node->getConnections().size();

    return GPS_ERROR_NONE;
}

SCRIPT_API(GetMapNodeConnection, int(int nodeid, int _index, int* connectionid))
{

    const auto id = nodeid;
    const auto node = Container::Nodes::Find(id);

    if (node == nullptr)
    {
        return GPS_ERROR_INVALID_NODE;
    }

    const auto index = _index;
    const auto connections = node->getConnections();

    if (connections.size() <= index)
    {
        return GPS_ERROR_INVALID_CONNECTION;
    }

    *connectionid = connections.at(index)->getId();

    return GPS_ERROR_NONE;
}

SCRIPT_API(GetConnectionBetweenMapNodes, int(int _source, int _target, int* connectionid))
{

    const auto source_id = _source;
    const auto source = Container::Nodes::Find(source_id);

    if (source == nullptr)
    {
        return GPS_ERROR_INVALID_NODE;
    }

    const auto target_id = _target;
    const auto target = Container::Nodes::Find(target_id);

    if (target == nullptr)
    {
        return GPS_ERROR_INVALID_NODE;
    }

    for (const auto connection : source->getConnections())
    {
        if (connection->getTarget() == target)
        {
          
            *connectionid = connection->getId();
            return GPS_ERROR_NONE;
        }
    }

    return GPS_ERROR_INVALID_NODE;
}

SCRIPT_API(GetDistanceBetweenMapNodes, int(int _first, int _second, float* _distance))
{

    const auto first_id = _first;
    const auto first = Container::Nodes::Find(first_id);

    if (first == nullptr)
    {
        return GPS_ERROR_INVALID_NODE;
    }

    const auto second_id = _second;
    const auto second = Container::Nodes::Find(second_id);

    if (second == nullptr)
    {
        return GPS_ERROR_INVALID_NODE;
    }

    auto distance = first->getDistanceFromNode(second);

    *_distance = distance;

    return GPS_ERROR_INVALID_NODE;
}

SCRIPT_API(GetAngleBetweenMapNodes, int(int _first, int _second, float* _angle))
{

    const auto first_id = _first;
    const auto first = Container::Nodes::Find(first_id);

    if (first == nullptr)
    {
        return GPS_ERROR_INVALID_NODE;
    }

    const auto second_id = _second;
    const auto second = Container::Nodes::Find(second_id);

    if (second == nullptr)
    {
        return GPS_ERROR_INVALID_NODE;
    }

    auto angle = first->getAngleFromNode(second);
    *_angle = angle;

    return GPS_ERROR_NONE;
}

SCRIPT_API(GetMapNodeDistanceFromPoint, int(int nodeid, float x, float y, float z, float* _distance))
{

    const auto id = nodeid;
    const auto node = Container::Nodes::Find(id);

    if (node == nullptr)
    {
        return GPS_ERROR_INVALID_NODE;
    }

    auto distance = node->getDistanceFromPoint(x, y, z);
    *_distance = distance;

    return GPS_ERROR_NONE;
}

SCRIPT_API(GetMapNodeAngleFromPoint, int(int nodeid, float x, float y, float* _angle))
{

    const auto id = nodeid;
    const auto node = Container::Nodes::Find(id);

    if (node == nullptr)
    {
        return GPS_ERROR_INVALID_NODE;
    }

    auto angle = node->getAngleFromPoint(x, y);
    *_angle = angle;

    return GPS_ERROR_NONE;
}

SCRIPT_API(GetClosestMapNodeToPoint, int(float x, float y, float z, int* nodeid, int ignorednode))
{

    const auto ignored = ignorednode;
    auto result = INVALID_NODE_ID, id = INVALID_NODE_ID;
    auto distance = std::numeric_limits<float>::infinity(), temp = 0.0f;

    for (const auto node : Container::Nodes::GetAll())
    {
        if (node.second->isSetForDeletion())
        {
            continue;
        }

        id = node.second->getID();

        if (ignored == id)
        {
            continue;
        }

        temp = node.second->getDistanceFromPoint(x, y, z);

        if (temp < distance)
        {
            result = id;
            distance = temp;
        }
    }

    *nodeid = result;

    return GPS_ERROR_NONE;
}

SCRIPT_API(GetHighestMapNodeID, int(bool _param))
{
    return Container::Nodes::GetHighestID();
}

SCRIPT_API(GetRandomMapNode, int(int* nodeid))
{

    auto nodes = Container::Nodes::GetAll();

    if (nodes.empty())
    {
        return GPS_ERROR_INVALID_NODE;
    }

    std::mt19937 mersenne{ std::random_device{}() };

    std::vector<std::pair<int, Node*>> result;
    std::sample(nodes.begin(), nodes.end(), std::back_inserter(result), 1, mersenne);

    *nodeid = result.at(0).second->getID();

    return GPS_ERROR_NONE;
}

SCRIPT_API(SaveMapNodesToFile, int(const std::string& filename))
{

    if (File::SaveNodes(filename))
    {
        return GPS_ERROR_NONE;
    }

    return GPS_ERROR_INTERNAL;
}

SCRIPT_API(FindPath, int(int source, int _target, int* pathid))
{
    const auto start_id = source;
    const auto start = Container::Nodes::Find(start_id);

    if (start == nullptr)
    {
        return GPS_ERROR_INVALID_NODE;
    }

    const auto target_id = _target;
    const auto target = Container::Nodes::Find(target_id);

    if (target == nullptr)
    {
        return GPS_ERROR_INVALID_NODE;
    }

    const auto result = Pathfinder::FindPath(start, target);

    if (result == -1)
    {   
        return GPS_ERROR_INVALID_PATH;
    }

    *pathid = result;

    return GPS_ERROR_NONE;
}

SCRIPT_API(IsValidPath, bool(int pathid))
{

    const auto id = pathid;

    Container::LockShared();
    const auto path = Container::Paths::Find(id);
    Container::UnlockShared();

    return path != nullptr;
}

SCRIPT_API(GetPathSize, int(int pathid, int* size))
{
    const auto id = pathid;

    Container::LockShared();
    const auto path = Container::Paths::Find(id);
    Container::UnlockShared();

    if (path == nullptr)
    {
        return GPS_ERROR_INVALID_PATH;
    }
    
    *size = path->getNodes().size();

    return GPS_ERROR_NONE;
}

SCRIPT_API(GetPathLength, int(int pathid, float* size))
{
   
    const auto id = pathid;

    Container::LockShared();
    const auto path = Container::Paths::Find(id);
    Container::UnlockShared();

    if (path == nullptr)
    {
        return GPS_ERROR_INVALID_PATH;
    }

    auto result = path->getLength();

    *size = result;

    return GPS_ERROR_NONE;
}

SCRIPT_API(GetPathNode, int(int pathid, int _index, int* nodeid))
{

    const auto id = pathid;

    Container::LockShared();
    const auto path = Container::Paths::Find(id);
    Container::UnlockShared();

    if (path == nullptr)
    {
        return GPS_ERROR_INVALID_PATH;
    }

    const auto index = _index;
    auto nodes = path->getNodes();

    if (index < 0 || index >= nodes.size())
    {
        return GPS_ERROR_INVALID_NODE;
    }

    *nodeid = nodes.at(index)->getID();

    return GPS_ERROR_NONE;
}

SCRIPT_API(GetPathNodeIndex, int(int pathid, int nodeid, int* _index))
{

    const auto id = pathid;

    Container::LockShared();
    const auto path = Container::Paths::Find(id);
    Container::UnlockShared();

    if (path == nullptr)
    {
        return GPS_ERROR_INVALID_PATH;
    }

    const auto node_id = nodeid;
    const auto node = Container::Nodes::Find(node_id);

    if (node == nullptr)
    {
        return GPS_ERROR_INVALID_NODE;
    }

    const auto index = node->getPositionInPath(path);

    if (index == -1)
    {
        return GPS_ERROR_INVALID_NODE;
    }

    *_index = index;

    return GPS_ERROR_NONE;
}

SCRIPT_API(DestroyPath, int(int pathid))
{
    const auto id = pathid;

    Container::LockShared();
    const auto path = Container::Paths::Find(id);
    Container::UnlockShared();

    if (path == nullptr)
    {
        return GPS_ERROR_INVALID_PATH;
    }

    Container::LockExclusive();
    Container::Paths::Delete(path);
    Container::UnlockExclusive();

    return GPS_ERROR_NONE;
}

SCRIPT_API(FindPathThreaded, int(cell* params))
{
   //to lazy to finish this shit

    return GPS_ERROR_NONE;
}

COMPONENT_ENTRY_POINT() { return new GPSComponent(); }
