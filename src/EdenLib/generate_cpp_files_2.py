import re

class_names = [
    "_edDevScanPsxPorts",
    "_edDevScanPorts",
    "_edDevInitRoot",
    "_edDevInitLibs",
    "_edDevFindPsxDevice",
    "_edDevDualShock2",
    "_edDevAttachPsxPort",
    "edDevSetRouteRates",
    "edDevSetRepeatRate",
    "edDevSetRepeatDelay",
    "edDevSetRates",
    "edDevReadUpdate",
    "edDevInit",
    "edDevGetRouteType",
    "edDevGetRouteReleased",
    "edDevGetRoutePressed",
    "edDevGetRouteClick",
    "edDevGetRouteAnalog",
    "edDevGetNumPorts",
    "edDevGetNumChans",
    "edDevGetDevType",
    "edDevGetDevConected",
    "edDevGetChanType",
    "edDevGetChanPressed",
    "edDevGetChanClick",
    "edDevAddRouteList",
    "edDevAddRoute"
]

for class_name in class_names:
    # Extract the part after "CActor"
    actor_name_formatted = re.sub(r'(?<!^)(?=[A-Z])', '_', class_name).upper()
    
    # Create a file based on class_name
    with open(f"{class_name}.cpp", "w") as file:
        file.write(f'#include "{class_name}.h"\n\n')
        file.flush()
    
    print(f'"/{class_name}.cpp"')

    # Create a file based on class_name
    with open(f"{class_name}.h", "w") as file:
        file.write(f'#ifndef {actor_name_formatted}_H\n#define {actor_name_formatted}_H\n\n#include "Types.h"\n#include "Actor.h"\n\n')
        file.write(f"#endif //{actor_name_formatted}_H")
        file.flush()

    print(f'"/{class_name}.h"')