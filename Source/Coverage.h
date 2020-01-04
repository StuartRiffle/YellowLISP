// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#pragma once
#if DEBUG_BUILD

    // This is hacky and fragile! It's just to use as rought guide while debugging,
    // and has only been tested in MSVC so far. It can not generate a list of
    // missed coverage markers, and it only works in DEBUG_BUILD.
    //
    // To figure out what coverage is missing, look at gCoverageMarker in the
    // debugger, and see what functions are being hit around the gaps.
    //
    // For this to work, the compiler macro __COUNTER__ has to be unique over the
    // entire codebase, but by default it resets for every compilation unit. So
    // the code has to be built as one bulk/unity/jumbo build!

    #define MAX_COVERAGE_MARKERS (2000) // Arbitrary; just increase it if you hit the assert

    struct CoverageMarker;

    // CoverageTracker is a template so that it isn't expanded until after parsing 
    // the whole file, so that it can use CoverageMarker without declaration errors.

    template<typename MARKER>
    class CoverageTracker
    {
        int _markersCovered = 0;
        float _coverageRate = 0;

    public:
        static int& GetMarkerRangeRef()
        {
            static int sMaxRange = 0;
            return sMaxRange;
        }

        static int& GetMarkerRangeOverageRef()
        {
            static int sRangeOverage = 0;
            return sRangeOverage;
        }

        void RefreshCoverage()
        {
            int highestCounterRaw = GetMarkerRangeRef();
            assert(highestCounterRaw < MAX_COVERAGE_MARKERS);

            // Correct for spurious calls to highestCounter

            int highestCounter = highestCounterRaw - GetMarkerRangeOverageRef();

            _markersCovered = 0;

            extern MARKER gCoverageMarker[MAX_COVERAGE_MARKERS];

            for (int i = 0; i < highestCounter; i++)
            {
                if (gCoverageMarker[i]._sourceFile == NULL)
                    continue;

                if (gCoverageMarker[i]._timesTouched > 0)
                    _markersCovered++;
            }

            _coverageRate = (highestCounter > 0)? (_markersCovered * 1.0f / highestCounter) : 0;
        }
    };

    struct CoverageMarker
    {
        const char*     _sourceFile = NULL;
        const char*     _sourceFunc = NULL;
        int             _sourceLine = 0;
        uint64_t        _timesTouched = 0;
    };

    typedef CoverageTracker<CoverageMarker> CoverageTrackerType; 
    extern CoverageTrackerType gCoverageTracker;

    struct CoverageMarkerRangeFinder
    {
        CoverageMarkerRangeFinder(int counterHere)
        {
            int& highestCounterSeen = CoverageTrackerType::GetMarkerRangeRef();
            if (counterHere > highestCounterSeen)
                highestCounterSeen = counterHere;

            // Sampling __COUNTER__ incremented it, so track the number of range finders
            // so that CoverageTracker can correct for that.

            int rangeFindersConstructed = CoverageTrackerType::GetMarkerRangeOverageRef();
            rangeFindersConstructed++;
        }
    };

    // When placing these markers, put them at the *end* of basic blocks, because
    // exceptions could cause an early exit, and we need to test all the way to the end.

    #define ASSERT_COVERAGE \
    { \
        extern CoverageMarker gCoverageMarker[MAX_COVERAGE_MARKERS]; \
        const int indexLocal = __COUNTER__; \
        assert(indexLocal < MAX_COVERAGE_MARKERS); \
        gCoverageMarker[indexLocal]._sourceFile = __FILE__; \
        gCoverageMarker[indexLocal]._sourceFunc = __FUNCTION__; \
        gCoverageMarker[indexLocal]._sourceLine = __LINE__; \
        gCoverageMarker[indexLocal]._timesTouched++; \
    }


    #define DECLARE_UNIQUE_COVERAGE_RANGE_FINDER(_INDEX) \
        static volatile CoverageMarkerRangeFinder coverageMarkerRangeFinder##_INDEX(_INDEX);

#else
    // RELEASE_BUILD
    
    #define ASSERT_COVERAGE
    #define DECLARE_UNIQUE_COVERAGE_RANGE_FINDER(_INDEX)
#endif

#define RETURN_ASSERT_COVERAGE(_RESULT) { ASSERT_COVERAGE; return _RESULT; }
#define VOID_RETURN_ASSERT_COVERAGE     { ASSERT_COVERAGE; return; }
#define BREAK_ASSERT_COVERAGE           { ASSERT_COVERAGE; break; }

#define UPDATE_COVERAGE_MARKER_RANGE_FOR_REAL(_INDEX) DECLARE_UNIQUE_COVERAGE_RANGE_FINDER(_INDEX)
#define UPDATE_COVERAGE_MARKER_RANGE UPDATE_COVERAGE_MARKER_RANGE_FOR_REAL(__COUNTER__)

