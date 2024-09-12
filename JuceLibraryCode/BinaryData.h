/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   magic_xml;
    const int            magic_xmlSize = 2046;

    extern const char*   WAVEFORM_v01_2_png;
    const int            WAVEFORM_v01_2_pngSize = 31822;

    extern const char*   piano_7922564_png;
    const int            piano_7922564_pngSize = 20867;

    extern const char*   LatoMedium_ttf;
    const int            LatoMedium_ttfSize = 663564;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 4;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
