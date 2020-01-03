// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#pragma once
extern void SanityCheck(Console* console);

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
        // Every time the marker comes into scope, _timesTouched is incremented.
        // We can walk the list after testing to see if we hit all of them.

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

#define CONCATENATE(_PREFIX, _SUFFIX)                   _PREFIX ## _SUFFIX
#define UNIQUE_COVERAGE_MARKER_NAME(_COUNTER)           CONCATENATE(sCoverageMarker, _COUNTER)
#define DECLARE_COVERAGE_MARKER(_MARKER_NAME, _DESC)    { static CoverageMarker _MARKER_NAME(__FILE__, __FUNCTION__, __LINE__); _MARKER_NAME.NotifyTouched(); }


#if DEBUG_BUILD
    #define TEST_COVERAGE DECLARE_COVERAGE_MARKER(UNIQUE_COVERAGE_MARKER_NAME(__COUNTER__))
#else
    #define TEST_COVERAGE
#endif


#define RETURN_WITH_COVERAGE(_RESULT) { TEST_COVERAGE; return _RESULT; }


