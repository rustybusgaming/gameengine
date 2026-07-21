# Source Engine Support for Nexus Game Engine

This module adds support for Valve's Source engine file formats to the Nexus Engine, allowing you to import and convert assets from games like Half-Life 2, Counter-Strike, Portal, and other Source engine titles.

## Supported Source Engine Formats

| Format | Extension | Description | Support Status |
|--------|-----------|-------------|---------------|
| VMF    | .vmf      | Valve Map Format (Hammer Editor) | Basic Support |
| VMT    | .vmt      | Valve Material Type | Basic Support |
| VTF    | .vtf      | Valve Texture Format | Stub (Requires VTFLib) |
| MDL    | .mdl      | Model Format | Metadata Only |
| VVD    | .vvd      | Vertex Data | Not Implemented |
| PHY    | .phy      | Physics Model | Not Implemented |
| ANI    | .ani      | Animation | Not Implemented |
| PCF    | .pcf      | Precomputed Lighting | Not Implemented |
| BSP    | .bsp      | Binary Space Partition (compiled maps) | Stub |
| VCD    | .vcd      | Choreography Scenes | Not Implemented |
| VCO    | .vco      | Compiled Objects | Not Implemented |
| VPK    | .vpk      | Valve Package (archive) | Stub |
| VDF    | .vdf      | Valve Data Format | Not Implemented |

## Usage

### Converting Individual Source Files

```
NexusAssetConverter source_file.ext output_file.ext [options]
```

Example:
```
NexusAssetConverter map.vmf converted_map.scene --quality high
NexusAssetConverter material.vmt converted_material.mat
NexusAssetConverter texture.vtf converted_texture.dds
```

### Converting an Entire Source Game Directory

```
NexusAssetConverter source_game_path output_directory --source-game
```

Example:
```
NexusAssetConverter "C:\Program Files (x86)\Steam\steamapps\common\Half-Life 2" converted_hl2 --source-game
```

### Options

- `--quality <high|medium|low>`: Sets the conversion quality
- `--compress`: Enables compression of output files
- `--optimize`: Optimizes assets for Nexus Engine (enabled by default)
- `--source-game`: Treats the input as a Source engine game directory

## Technical Details

### Source Engine Map Conversion (VMF)

VMF files are text-based and use a format similar to the Valve Data Format (VDF). The converter parses:

- World geometry (brushes/solids)
- Entities (props, lights, triggers, etc.)
- Materials and textures
- Basic entity properties

The output is a Nexus scene file that preserves the structure and properties of the original map.

### Source Engine Material Conversion (VMT)

VMT files define materials in the Source engine. The converter maps:

- Source shader types to appropriate Nexus shaders
- Texture paths (albedo/diffuse, normal maps, etc.)
- Material properties like opacity, specular levels, etc.

### Source Engine Model Conversion (MDL)

MDL files are complex binary formats that require specialized parsing. Currently, only basic metadata extraction is implemented, with full conversion requiring additional development.

## Future Improvements

- Complete VTF texture conversion using VTFLib
- Full MDL model conversion support
- BSP decompilation and conversion
- VPK package extraction
- Physics model conversion
- Particle effect conversion
- Sound conversion

## Dependencies

- For VTF conversion: VTFLib
- For MDL conversion: Source SDK parsing components
- For BSP conversion: Source SDK parsing components

## References

- [Valve Developer Community - VMF Format](https://developer.valvesoftware.com/wiki/Valve_Map_Format)
- [Valve Developer Community - VMT Format](https://developer.valvesoftware.com/wiki/Material)
- [Valve Developer Community - MDL Format](https://developer.valvesoftware.com/wiki/MDL)
- [Valve Developer Community - VTF Format](https://developer.valvesoftware.com/wiki/VTF)
- [Valve Developer Community - BSP Format](https://developer.valvesoftware.com/wiki/BSP)