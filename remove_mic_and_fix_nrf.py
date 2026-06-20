import json

def process_schematic(file_path):
    with open(file_path, 'r') as f:
        data = json.load(f)
        
    shapes = data['schematics'][0]['dataStr']['shape']
    new_shapes = []
    
    # 1. Coordinates of MK1 pins, capacitors, and MIC pins to check for wire deletions
    mk1_pins = [(235, -600), (325, -600), (235, -590), (325, -590), (235, -580), (325, -580), (235, -570), (325, -570)]
    c_pins = [(275, -650), (315, -650), (270, -685), (310, -685)]
    mic_pins = [(75, -345), (75, -335), (75, -325)]
    
    all_target_pins = mk1_pins + c_pins + mic_pins
    
    deleted_components = 0
    deleted_wires = 0
    deleted_junctions = 0
    
    for s in shapes:
        # Check component definitions (LIB)
        if s.startswith('LIB~'):
            parts = s.split('#@$')
            lib_info = parts[0].split('~')
            lx, ly = float(lib_info[1]), float(lib_info[2])
            
            # Check if this is the ANALOG MIC component at (90, -330)
            if 'ANALOG MIC' in s or (abs(lx - 90) < 5 and abs(ly - (-330)) < 5):
                deleted_components += 1
                continue
                
        # Check wires (W)
        if s.startswith('W~'):
            coords = s.split('~')[1].split(' ')
            match = False
            for i in range(0, len(coords), 2):
                wx, wy = float(coords[i]), float(coords[i+1])
                for px, py in all_target_pins:
                    if abs(wx - px) < 0.5 and abs(wy - py) < 0.5:
                        match = True
                        break
            if match:
                deleted_wires += 1
                continue
                
        # Check junctions (J)
        if s.startswith('J~'):
            j_parts = s.split('~')
            jx, jy = float(j_parts[1]), float(j_parts[2])
            # Bounding box 1: NRF & Caps (X in [150, 370], Y in [-700, -550])
            if 150 <= jx <= 370 and -700 <= jy <= -550:
                deleted_junctions += 1
                continue
            # Bounding box 2: MIC (X in [0, 160], Y in [-420, -300])
            if 0 <= jx <= 160 and -420 <= jy <= -300:
                deleted_junctions += 1
                continue
                
        new_shapes.append(s)
        
    print(f"Removed {deleted_components} MIC components, {deleted_wires} wires, and {deleted_junctions} junctions.")
    
    # 2. Add our new wires for MK1 (nRF24 module) and C1/C2 capacitors
    new_wires = [
        # Pin 1 (GND): (235, -600) -> XIAO GND: (575, -480)
        "W~235 -600 200 -600 200 -690 600 -690 600 -480 575 -480~#008800~1~0~none~mk1_gnd_main~0",
        # C1 Pin 1 (GND): (275, -650) -> Main GND wire
        "W~275 -650 200 -650~#008800~1~0~none~mk1_c1_gnd~0",
        # C2 Pin 1 (GND): (270, -685) -> Main GND wire
        "W~270 -685 200 -685~#008800~1~0~none~mk1_c2_gnd~0",
        
        # Pin 2 (VCC): (325, -600) -> XIAO 3V3: (575, -470)
        "W~325 -600 345 -600 345 -665 590 -665 590 -470 575 -470~#008800~1~0~none~mk1_vcc_main~0",
        # Connect to C2 Pin 2: (310, -685) -> (345, -685) -> (345, -600)
        "W~310 -685 345 -685 345 -600~#008800~1~0~none~mk1_c2_vcc~0",
        # C1 Pin 2 (VCC): (315, -650) -> (345, -650)
        "W~315 -650 345 -650~#008800~1~0~none~mk1_c1_vcc~0",
        
        # Pin 3 (CE): (235, -590) -> XIAO Pin 7 (D7): (575, -430)
        "W~235 -590 190 -590 190 -430 575 -430~#008800~1~0~none~mk1_ce~0",
        
        # Pin 4 (CSN): (325, -590) -> XIAO Pin 2 (D2): (435, -470)
        "W~325 -590 335 -590 335 -475 435 -475 435 -470~#008800~1~0~none~mk1_csn~0",
        
        # Pin 5 (MOSI on symbol, maps to SCK on physical Pad 5): (235, -580) -> XIAO Pin 8 (D8): (575, -440)
        "W~235 -580 180 -580 180 -440 575 -440~#008800~1~0~none~mk1_sck~0",
        
        # Pin 6 (SCK on symbol, maps to MOSI on physical Pad 6): (325, -580) -> XIAO Pin 10 (D10): (575, -460)
        "W~325 -580 330 -580 330 -460 575 -460~#008800~1~0~none~mk1_mosi~0",
        
        # Pin 7 (IRQ on symbol, maps to MISO on physical Pad 7): (235, -570) -> XIAO Pin 9 (D9): (575, -450)
        "W~235 -570 170 -570 170 -450 575 -450~#008800~1~0~none~mk1_miso~0"
        # Pin 8 (MISO on symbol, maps to IRQ on physical Pad 8): (325, -570) -> Leave unconnected!
    ]
    
    # 3. Add our new junctions
    new_junctions = [
        "J~200~-650",
        "J~200~-685",
        "J~345~-650",
        "J~345~-685",
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
    data = process_schematic('/home/shivprasad/Desktop/Tanbre/SCH_Tanbre_v2_2026-05-30_new.json')
    
    # Save to workspace as default SCH_Tanbre_v2_2026-05-30.json
    with open('/home/shivprasad/Desktop/Tanbre/SCH_Tanbre_v2_2026-05-30.json', 'w') as f:
        json.dump(data, f, indent=4)
        
    # Also overwrite the download file so user can easily import it!
    with open('/home/shivprasad/Downloads/SCH_Tanbre_v2_2026-05-30.json', 'w') as f:
        json.dump(data, f, indent=4)
        
    print("Successfully removed MIC and fixed NRF wiring in both workspace and Downloads!")
