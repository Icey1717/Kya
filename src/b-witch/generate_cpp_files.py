import re

class_names = [
    "CActorBlazer"
]

for class_name in class_names:
    # Extract the part after "CActor"
    actor_name = "Actor" + class_name[6:]
    actor_name_formatted = re.sub(r'(?<!^)(?=[A-Z])', '_', actor_name).upper()
    
    # Create a file based on actor_name
    with open(f"{actor_name}.cpp", "w") as file:
        file.write(f'#include "{actor_name}.h"\n\n')
        file.flush()
    
    print(f'"/{actor_name}.cpp"')

    # Create a file based on actor_name
    with open(f"{actor_name}.h", "w") as file:
        file.write(f'#ifndef {actor_name_formatted}_H\n#define {actor_name_formatted}_H\n\n#include "Types.h"\n#include "Actor.h"\n\n')
        file.write(f'class C{actor_name} : public CActor ')
        file.write("{\npublic:\n")
        file.write(f'\tC{actor_name}()')
        file.write("{\n\t\tIMPLEMENTATION_GUARD_LOG()\n\t}\n};\n\n")
        file.write(f"#endif //{actor_name_formatted}_H")
        file.flush()

    print(f'"/{actor_name}.h"')