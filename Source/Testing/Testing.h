// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#pragma once

struct CoverageMarker;
extern CoverageMarker* gCoverageMarkerList;

struct CoverageMarker
{
    CoverageMarker(const char* sourceFile, const char* sourceFunc, int sourceLine) :
        _sourceFile(sourceFile),
        _sourceFunc(sourceFunc),
        _sourceLine(sourceLine)
    {
        // This will happen as part of static object initialization on startup,
        // building a linked list of all the coverage markers in the executable.
        // Every time the marker comes into scope, _timesTouched gets incremented.
        // We can walk the list after running tests to see if we hit all of them.

        _nextMarker = gCoverageMarkerList;
        gCoverageMarkerList = this;
    }

    CoverageMarker* _nextMarker;
    const char*     _sourceFile;
    const char*     _sourceFunc;
    int             _sourceLine;
    uint64_t        _timesTouched = 0;

    void NotifyTouched() { _timesTouched++; }
};

#define CONCATENATE(_PREFIX, _SUFFIX)           _PREFIX ## _SUFFIX
#define UNIQUE_COVERAGE_MARKER_NAME(_COUNTER)   CONCATENATE(sCoverageMarker, _COUNTER)
#define DECLARE_COVERAGE_MARKER(_MARKER_NAME)   { static CoverageMarker _MARKER_NAME(__FILE__, __FUNCTION__, __LINE__); _MARKER_NAME.NotifyTouched(); }

// When placing these markers, put them at the *end* of basic blocks, because
// exceptions could cause an early exit, and we need to test all the way to the end.

#if DEBUG_BUILD
    #define MARK_COVERED DECLARE_COVERAGE_MARKER(UNIQUE_COVERAGE_MARKER_NAME(__COUNTER__))
#else
    #define MARK_COVERED
#endif

#define return (_RESULT) {  return _RESULT; }
#define return     {  return; }
#define break           {  break; }

