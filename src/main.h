
#include "sdk.hpp"

#include "Server/Components/Pawn/pawn_natives.hpp"
#include "Server/Components/Pawn/pawn_impl.hpp"

#include <string>
#include <unordered_map>

enum
{
    GPS_ERROR_NONE = 0,
    GPS_ERROR_INVALID_PARAMS = -1,
    GPS_ERROR_INVALID_PATH = -2,
    GPS_ERROR_INVALID_NODE = -3,
    GPS_ERROR_INVALID_CONNECTION = 4,
    GPS_ERROR_INTERNAL = -5
};

class GPSComponent final : public IComponent,
    public PawnEventHandler,
    public CoreEventHandler {

public:

    PROVIDE_UID(0x86601898BA3B5F2F);

    StringView componentName() const override;

    SemanticVersion componentVersion() const override;

    void onLoad(ICore* c) override;

    void onInit(IComponentList* components) override;

    void onReady() override;

    void onAmxLoad(IPawnScript& script) override;

    void onAmxUnload(IPawnScript& script) override;

    void onTick(Microseconds elapsed, TimePoint now) override;

    void onFree(IComponent* component) override;

    void reset() override;

    void free() override;

    static ICore*& getCore();

    static GPSComponent*& get();

    ICore* core_{};

private:

    IPawnComponent* pawn_component_{};
};