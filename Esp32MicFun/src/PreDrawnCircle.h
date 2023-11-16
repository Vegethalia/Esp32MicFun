#pragma once

#include <cstdint>
#include <vector>

// template <uint16_t TOTAL_LEDS>
class PreDrawmCircle {
private:
    std::vector<std::vector<uint8_t>> _TheCircle;

public:
    PreDrawmCircle(uint8_t theRadius)
    {
        int diameter = (theRadius * 2) + 1;
        _TheCircle.resize(diameter);
        for (int i = 0; i < diameter; i++) {
            _TheCircle[i].resize(diameter);
            for (int j = 0; j < diameter; j++) {
                _TheCircle[i][j] = 0;
            }
            // memset(_TheCircle[i].data(), 0, sizeof(uint8_t) * diameter);
        }

        DrawCircle();
    }

    uint8_t GetRadius()
    {
        return _TheCircle.size() / 2;
    }
    /// @brief The total width of the circle, should be (GetRadius()*2)+1
    /// @return
    uint8_t GetDiameter() { return _TheCircle.size(); }
    /// @brief Returns a pointer to the buffer with the circle drawn. It is a pointer to a buffer of size GetDiameter()*GetDiameter()
    /// @return
    const std::vector<std::vector<uint8_t>>* GetCircleBuffer() { return &_TheCircle; }

protected:
    /// @brief Given a bidimensional array of size pDataArray[radius*2+1][radius*2+1], draws a circle of diameter radius*2+1 centered in (radius,radius)
    /// @param pDataArray the array. Assumed to be have at least a size of (((radius * 2) + 1) * ((radius * 2) + 1))
    void DrawCircle()
    {
        uint8_t radius = GetRadius();
        // memset(pDataArray, 0, sizeof(bool) * (((radius * 2) + 1) * ((radius * 2) + 1)));

        // https://www.geeksforgeeks.org/mid-point-circle-drawing-algorithm/
        int p = 1 - (int)radius;
        int x = (int)radius, y = 0;
        while (x >= y) {
            SetPixelInAllOctants(x, y);
            y++;
            if (p <= 0) {
                p = p + 2 * y + 1;
            } else // Mid-point is outside the perimeter
            {
                x--;
                p = p + 2 * y - 2 * x + 1;
            }

            // All the perimeter points have already been printed
            if (x < y)
                break;
        }
    }

private:
    void SetPixelInAllOctants(int x, int y)
    {
        uint8_t radius = GetRadius();
        uint8_t w = (radius * 2) + 1;

        _TheCircle[x + radius][y + radius] = 1;
        _TheCircle[-x + radius][y + radius] = 1;
        _TheCircle[x + radius][-y + radius] = 1;
        _TheCircle[-x + radius][-y + radius] = 1;

        if (x != y) {
            _TheCircle[y + radius][x + radius] = 1;
            _TheCircle[-y + radius][x + radius] = 1;
            _TheCircle[y + radius][-x + radius] = 1;
            _TheCircle[-y + radius][-x + radius] = 1;
        }
    }
};

// template <uint8_t CIRCLE_RADIUS, uint16_t TOTAL_LEDS>
// class PreDrawmCircle : public BasicCircle<TOTAL_LEDS> {
// public:
//     PreDrawmCircle()
//         : BasicCircle<TOTAL_LEDS>()
//     {
//         //_TheCircle.
//         this->DrawCircle();
//     }
//     // returns the radius  of this circle
//     virtual uint8_t GetRadius() override { return CIRCLE_RADIUS; }
//     virtual uint8_t GetDiameter() override { return (CIRCLE_RADIUS * 2) + 1; }

//     /// @brief Returns a pointer to the buffer with the circle drawn. It is a pointer to a buffer of size GetDiameter()*GetDiameter()
//     /// @return
//     //    virtual const bool* GetBuffer() override { return _TheCircle; }

// protected:
//     // std::vector<std::vector<bool>> _TheCircle;
//     //  bool _TheCircle[((CIRCLE_RADIUS * 2) + 1)][((CIRCLE_RADIUS * 2) + 1)];
// };
