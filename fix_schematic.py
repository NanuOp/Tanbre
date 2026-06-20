import json

def fix_schematic(file_path):
    with open(file_path, 'r') as f:
        data = json.load(f)
        
    shapes = data['schematics'][0]['dataStr']['shape']
    new_shapes = []
    
    # 1. Coordinates of NRF/MK1 pins and capacitors to check for deletions
    nrf_pins = [(270, -600), (310, -600), (270, -590), (310, -590), (270, -580), (310, -580), (270, -570), (310, -570)]
    mk1_pins = [(385, -720), (475, -720), (385, -710), (475, -710), (385, -700), (475, -700), (385, -690), (475, -690)]
    all_pins = nrf_pins + mk1_pins
    
    # We want to remove:
    # - Any wire (W~) that touches any of these pins.
    # - Any junction (J~) that falls inside the bounding box where the old NRF/cap wires were (X between 200 and 360, Y between -700 and -550).
    # - The LIB shape for MK1 (which is at (430, -705)).
    
    deleted_wires_count = 0
    deleted_junctions_count = 0
    deleted_components_count = 0
    
    for s in shapes:
        # Check components
        if s.startswith('LIB~'):
            parts = s.split('#@$')
            lib_info = parts[0].split('~')
            # Check if this is MK1 at (430, -705)
            if 'MK1' in s or (abs(float(lib_info[1]) - 430) < 5 and abs(float(lib_info[2]) - (-705)) < 5):
                deleted_components_count += 1
                continue
                
        # Check wires
        if s.startswith('W~'):
            coords = s.split('~')[1].split(' ')
            match = False
            for i in range(0, len(coords), 2):
                wx, wy = float(coords[i]), float(coords[i+1])
                for px, py in all_pins:
                    if abs(wx - px) < 0.5 and abs(wy - py) < 0.5:
                        match = True
                        break
            if match:
                deleted_wires_count += 1
                continue
                
        # Check junctions
        if s.startswith('J~'):
            j_parts = s.split('~')
            jx, jy = float(j_parts[1]), float(j_parts[2])
            if 200 <= jx <= 360 and -700 <= jy <= -550:
                deleted_junctions_count += 1
                continue
                
        new_shapes.append(s)
        
    print(f"Removed {deleted_components_count} MK1 components, {deleted_wires_count} wires, and {deleted_junctions_count} junctions.")
    
    # 2. Add our new wires
    new_wires = [
        # Pin 1 (GND): (270, -600) -> XIAO GND: (575, -480)
        "W~270 -600 240 -600 240 -690 600 -690 600 -480 575 -480~#008800~1~0~none~nrf_gnd_main~0",
        # C1 Pin 1 (GND): (275, -650) -> Main GND wire
        "W~275 -650 240 -650~#008800~1~0~none~nrf_c1_gnd~0",
        # C2 Pin 1 (GND): (270, -685) -> Main GND wire
        "W~270 -685 240 -685~#008800~1~0~none~nrf_c2_gnd~0",
        
        # Pin 2 (VCC): (310, -600) -> XIAO 3V3: (575, -470)
        "W~310 -600 340 -600 340 -665 590 -665 590 -470 575 -470~#008800~1~0~none~nrf_vcc_main~0",
        # Connect to C2 Pin 2: (310, -685) -> (340, -685) -> (340, -600)
        "W~310 -685 340 -685 340 -600~#008800~1~0~none~nrf_c2_vcc~0",
        # C1 Pin 2 (VCC): (315, -650) -> (340, -650)
        "W~315 -650 340 -650~#008800~1~0~none~nrf_c1_vcc~0",
        
        # Pin 3 (CE): (270, -590) -> XIAO Pin 7 (D7): (575, -430)
        "W~270 -590 230 -590 230 -430 575 -430~#008800~1~0~none~nrf_ce~0",
        
        # Pin 4 (CSN): (310, -590) -> XIAO Pin 2 (D2): (435, -470)
        "W~310 -590 330 -590 330 -470 435 -470~#008800~1~0~none~nrf_csn~0",
        
        # Pin 5 (MOSI): (270, -580) -> XIAO Pin 10 (D10): (575, -460)
        "W~270 -580 220 -580 220 -460 575 -460~#008800~1~0~none~nrf_mosi~0",
        
        # Pin 6 (SCK): (310, -580) -> XIAO Pin 8 (D8): (575, -440)
        "W~310 -580 325 -580 325 -440 575 -440~#008800~1~0~none~nrf_sck~0",
        
        # Pin 7 (MISO): (270, -570) -> XIAO Pin 9 (D9): (575, -450)
        "W~270 -570 210 -570 210 -450 575 -450~#008800~1~0~none~nrf_miso~0"
    ]
    
    # 3. Add our new junctions
    new_junctions = [
        "J~240~-650",
        "J~240~-685",
        "J~340~-650",
        "J~340~-685",
        # Connect new wire bends to existing net junctions if needed (EasyEDA handles simple overlap but junctions are cleaner)
        "J~575~-480",
        "J~575~-470",
        "J~575~-460",
        "J~575~-450",
        "J~575~-440",
        "J~575~-430",
        "J~435~-470"
    ]
    
    new_shapes.extend(new_wires)
    new_shapes.extend(new_junctions)
    
    data['schematics'][0]['dataStr']['shape'] = new_shapes
    
    return data

if __name__ == '__main__':
    # Fix the schematic in workspace
    data = fix_schematic('/home/shivprasad/Desktop/Tanbre/SCH_Tanbre_v2_2026-05-30(4).json')
    
    # Save back to workspace
    with open('/home/shivprasad/Desktop/Tanbre/SCH_Tanbre_v2_2026-05-30(4).json', 'w') as f:
        json.dump(data, f, indent=4)
        
    # Also overwrite the download file so user can easily import it!
    with open('/home/shivprasad/Downloads/SCH_Tanbre_v2_2026-05-30 (4).json', 'w') as f:
        json.dump(data, f, indent=4)
        
    print("Schematic successfully fixed and written to workspace and Downloads!")
