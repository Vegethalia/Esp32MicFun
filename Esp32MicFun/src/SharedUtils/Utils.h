#pragma once
#include <math.h>
#include <string>

#define EARTH_RADIUS_METERS 6372795.0f // 6371000

class Utils {
public:
    template <typename... Args>
    static std::string string_format(const std::string& format, Args... args)
    {
        size_t size = snprintf(nullptr, 0, format.c_str(), args...) + (size_t)1; // Extra space for '\0'
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), size, format.c_str(), args...);
        return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
    }

    // Returns the distance (in meters) between two GPS coordinates. Uses the ‘haversine’ formula.
    // Based on https://www.movable-type.co.uk/scripts/latlong.html by Chris Veness
    static float DistanceBetween2Points(float Lat1, float Lon1, float Lat2, float Lon2)
    {
        float dLat = radians(Lat2 - Lat1);
        float dLon = radians(Lon2 - Lon1);

        float a = sin(dLat / 2.0f) * sin(dLat / 2.0f) + cos(radians(Lat1)) * cos(radians(Lat2)) * sin(dLon / 2.0f) * sin(dLon / 2.0f);

        float d = 2.0f * atan2(sqrt(a), sqrt(1.0f - a));

        return d * EARTH_RADIUS_METERS;
    }

    // Same as above, but faster and less precise (47.78m vs 61.95m for instance). Uses the Equirectangular approximation formula.
    // If performance is an issue and precission is not important...
    // Based on https://www.movable-type.co.uk/scripts/latlong.html by Chris Veness
    static float FastDistanceBetween2Points(float Lat1, float Lon1, float Lat2, float Lon2)
    {
        //		const x = (λ2 - λ1) * Math.cos((φ1 + φ2) / 2);
        //		const y = (φ2 - φ1);
        //		const d = Math.sqrt(x * x + y * y) * R;

        float dLat = radians(Lat2 - Lat1);
        float dLatAdd = radians(Lon2 + Lon1);
        float dLon = radians(Lon2 - Lon1);

        float x = dLon * cos(dLatAdd / 2);
        float d = sqrt(x * x + dLat * dLat) * EARTH_RADIUS_METERS;

        return d;
    }

    // Splits the input strim into 2 values, delimited by the delimiters specified in pDelims.
    // Only returns the first 2 values
    // ex:  SplitStringFirst2Values("mivar=red=kaka", &pstr1, &pstr2, "=")
    //      will return pstr1="mivar" pstr2="red"
    // The method returns true if two values were successfully extracted.
    static bool SplitString2Values(char* pInputStr, char** pStr1, char** pStr2, const char* pDelims)
    {
        char* ptr = strtok(pInputStr, pDelims);
        if (ptr) {
            *pStr1 = ptr;
            ptr = strtok(nullptr, pDelims);
            if (ptr) {
                *pStr2 = ptr;
                return true;
            }
        }
        return false;
    }
};
