// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#pragma once

// CoverageTracker is a template so that it isn't expanded until after parsing 
// the whole file, so that it can use CoverageMarker without declaration errors.

template<typename MARKER>
class CoverageTracker
{
    int _markersCovered = 0;
    int _markersTotal   = 0;
    float _coverageRate = 0;

public:
    static MARKER*& GetCoverageMarkerListRef()
    {
        // This ensures that _coverageMarkerList is initialized before markers start
        // chaining themselves onto it at startup.

        static MARKER* sCoverageMarkerList = NULL;
        return sCoverageMarkerList;
    }

    void RefreshCoverage()
    {
        _markersCovered = 0;
        _markersTotal   = 0;

        MARKER* marker = GetCoverageMarkerListRef();
        while (marker)
        {
            _markersTotal++;
            if (marker->_timesTouched > 0)
                _markersCovered++;

            marker = marker->_nextMarker;
        }

        _coverageRate = (_markersTotal > 0)? (_markersCovered * 1.0f / _markersTotal) : 0;
    }
};

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

        CoverageMarker*& globalMarkerList = CoverageTracker<CoverageMarker>::GetCoverageMarkerListRef();
        _nextMarker = globalMarkerList;
        globalMarkerList = this;
    }

    CoverageMarker* _nextMarker;
    const char*     _sourceFile;
    const char*     _sourceFunc;
    int             _sourceLine;
    uint64_t        _timesTouched = 0;

    void NotifyTouched() { _timesTouched++; }
};

extern CoverageTracker<CoverageMarker> gCoverageTracker;

#define CONCATENATE(_PREFIX, _SUFFIX)           _PREFIX ## _SUFFIX
#define UNIQUE_COVERAGE_MARKER_NAME(_COUNTER)   CONCATENATE(sCoverageMarker, _COUNTER)
#define DECLARE_COVERAGE_MARKER(_MARKER_NAME)   { extern CoverageMarker _MARKER_NAME(__FILE__, __FUNCTION__, __LINE__); _MARKER_NAME.NotifyTouched(); }

// When placing these markers, put them at the *end* of basic blocks, because
// exceptions could cause an early exit, and we need to test all the way to the end.

#if DEBUG_BUILD
    #define ASSERT_COVERAGE DECLARE_COVERAGE_MARKER(UNIQUE_COVERAGE_MARKER_NAME(__COUNTER__))
#else
    #define ASSERT_COVERAGE
#endif

#define RETURN_ASSERT_COVERAGE(_RESULT) { ASSERT_COVERAGE; return _RESULT; }
#define VOID_RETURN_ASSERT_COVERAGE     { ASSERT_COVERAGE; return; }
#define BREAK_ASSERT_COVERAGE           { ASSERT_COVERAGE; break; }


