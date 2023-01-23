static CRGB __imgArlet[32][16] = { { { 181, 190, 178 }, { 218, 221, 213 }, { 243, 244, 239 }, { 195, 194, 182 }, { 190, 181, 167 }, { 220, 221, 203 }, { 221, 223, 204 }, { 194, 189, 168 }, { 215, 212, 192 }, { 192, 185, 164 }, { 210, 201, 184 }, { 211, 206, 187 }, { 192, 180, 165 }, { 146, 179, 142 }, { 117, 178, 140 }, { 153, 149, 134 } },
    { { 169, 176, 160 }, { 219, 222, 214 }, { 247, 249, 243 }, { 222, 219, 206 }, { 170, 157, 141 }, { 198, 192, 172 }, { 177, 170, 148 }, { 160, 153, 132 }, { 196, 191, 170 }, { 196, 187, 166 }, { 210, 198, 183 }, { 167, 154, 141 }, { 137, 117, 107 }, { 109, 116, 95 }, { 138, 149, 125 }, { 170, 145, 134 } },
    { { 149, 145, 130 }, { 209, 205, 197 }, { 232, 227, 219 }, { 196, 186, 172 }, { 155, 137, 122 }, { 153, 136, 116 }, { 155, 137, 115 }, { 160, 148, 127 }, { 182, 169, 148 }, { 176, 161, 145 }, { 147, 130, 119 }, { 139, 118, 110 }, { 121, 101, 93 }, { 161, 147, 129 }, { 221, 210, 185 }, { 149, 131, 120 } },
    { { 147, 129, 118 }, { 149, 133, 123 }, { 140, 123, 111 }, { 118, 98, 84 }, { 112, 88, 77 }, { 124, 99, 83 }, { 113, 90, 73 }, { 149, 131, 111 }, { 164, 144, 123 }, { 150, 128, 116 }, { 154, 135, 126 }, { 131, 111, 105 }, { 117, 105, 91 }, { 202, 195, 165 }, { 166, 159, 145 }, { 131, 117, 120 } },
    { { 111, 87, 79 }, { 100, 79, 70 }, { 87, 69, 60 }, { 92, 71, 62 }, { 92, 68, 60 }, { 89, 66, 56 }, { 94, 70, 59 }, { 128, 99, 81 }, { 133, 106, 88 }, { 114, 92, 83 }, { 124, 106, 101 }, { 104, 92, 85 }, { 188, 183, 159 }, { 182, 178, 154 }, { 128, 130, 131 }, { 99, 97, 114 } },
    { { 99, 75, 67 }, { 79, 56, 48 }, { 83, 64, 53 }, { 84, 64, 55 }, { 79, 60, 52 }, { 73, 53, 44 }, { 88, 64, 53 }, { 117, 85, 67 }, { 153, 119, 101 }, { 118, 96, 87 }, { 133, 121, 114 }, { 146, 171, 170 }, { 156, 177, 171 }, { 183, 179, 163 }, { 85, 91, 110 }, { 67, 73, 102 } },
    { { 108, 83, 72 }, { 98, 74, 64 }, { 88, 67, 56 }, { 75, 56, 48 }, { 73, 54, 49 }, { 75, 53, 47 }, { 97, 69, 58 }, { 113, 77, 59 }, { 132, 97, 77 }, { 138, 111, 99 }, { 152, 153, 136 }, { 75, 120, 132 }, { 86, 141, 153 }, { 127, 140, 141 }, { 52, 62, 86 }, { 54, 63, 90 } },
    { { 114, 89, 79 }, { 92, 68, 59 }, { 74, 55, 52 }, { 64, 46, 45 }, { 65, 45, 41 }, { 99, 76, 71 }, { 104, 74, 63 }, { 150, 112, 95 }, { 148, 116, 96 }, { 134, 104, 87 }, { 181, 178, 145 }, { 88, 125, 131 }, { 44, 85, 99 }, { 85, 123, 138 }, { 72, 103, 122 }, { 44, 55, 84 } },
    { { 102, 79, 72 }, { 85, 64, 58 }, { 66, 48, 46 }, { 61, 44, 41 }, { 66, 47, 45 }, { 93, 68, 61 }, { 136, 101, 87 }, { 174, 137, 119 }, { 179, 144, 127 }, { 184, 147, 130 }, { 176, 149, 127 }, { 139, 138, 136 }, { 87, 133, 149 }, { 71, 119, 136 }, { 82, 129, 145 }, { 72, 117, 132 } },
    { { 104, 81, 76 }, { 79, 58, 53 }, { 77, 56, 51 }, { 79, 58, 54 }, { 70, 50, 47 }, { 105, 75, 66 }, { 142, 106, 93 }, { 182, 147, 132 }, { 198, 163, 149 }, { 196, 160, 143 }, { 176, 136, 118 }, { 169, 132, 116 }, { 113, 119, 117 }, { 89, 152, 168 }, { 91, 154, 173 }, { 80, 136, 151 } },
    { { 103, 78, 72 }, { 86, 62, 57 }, { 83, 59, 55 }, { 81, 57, 52 }, { 108, 81, 73 }, { 113, 83, 75 }, { 119, 89, 83 }, { 186, 150, 140 }, { 215, 183, 172 }, { 201, 163, 147 }, { 169, 123, 106 }, { 161, 118, 98 }, { 153, 118, 102 }, { 79, 96, 101 }, { 91, 157, 172 }, { 77, 127, 145 } },
    { { 100, 73, 69 }, { 92, 68, 61 }, { 98, 71, 64 }, { 114, 85, 75 }, { 124, 92, 83 }, { 74, 51, 47 }, { 114, 91, 90 }, { 178, 144, 134 }, { 225, 196, 184 }, { 201, 159, 143 }, { 162, 111, 93 }, { 159, 113, 94 }, { 161, 116, 99 }, { 126, 98, 88 }, { 53, 86, 94 }, { 88, 149, 164 } },
    { { 112, 85, 78 }, { 103, 78, 68 }, { 106, 78, 69 }, { 100, 72, 65 }, { 92, 65, 58 }, { 63, 46, 43 }, { 106, 90, 90 }, { 171, 134, 128 }, { 224, 196, 185 }, { 197, 155, 140 }, { 146, 97, 83 }, { 140, 97, 84 }, { 156, 111, 95 }, { 158, 121, 105 }, { 66, 54, 54 }, { 62, 99, 108 } },
    { { 126, 99, 91 }, { 120, 93, 83 }, { 99, 69, 58 }, { 110, 79, 71 }, { 96, 64, 57 }, { 84, 59, 53 }, { 101, 83, 85 }, { 170, 134, 128 }, { 219, 187, 176 }, { 174, 132, 119 }, { 139, 97, 87 }, { 108, 70, 65 }, { 156, 111, 100 }, { 176, 132, 122 }, { 133, 105, 100 }, { 16, 26, 37 } },
    { { 137, 111, 105 }, { 132, 102, 94 }, { 124, 93, 80 }, { 121, 85, 74 }, { 119, 81, 72 }, { 110, 74, 65 }, { 122, 96, 96 }, { 178, 143, 134 }, { 195, 158, 147 }, { 165, 124, 112 }, { 173, 136, 126 }, { 114, 75, 71 }, { 159, 113, 108 }, { 174, 132, 119 }, { 185, 150, 141 }, { 66, 56, 58 } },
    { { 143, 118, 112 }, { 155, 124, 115 }, { 163, 129, 117 }, { 157, 117, 106 }, { 139, 98, 85 }, { 127, 88, 74 }, { 139, 103, 95 }, { 189, 154, 143 }, { 197, 160, 148 }, { 186, 149, 136 }, { 178, 139, 130 }, { 142, 97, 91 }, { 155, 104, 102 }, { 163, 118, 110 }, { 198, 160, 150 }, { 121, 98, 94 } },
    { { 164, 136, 129 }, { 182, 149, 139 }, { 177, 139, 127 }, { 166, 125, 112 }, { 143, 101, 90 }, { 134, 95, 82 }, { 156, 119, 107 }, { 210, 176, 164 }, { 218, 185, 173 }, { 179, 140, 127 }, { 161, 119, 110 }, { 147, 99, 94 }, { 149, 95, 93 }, { 157, 106, 101 }, { 198, 158, 150 }, { 141, 111, 104 } },
    { { 149, 120, 111 }, { 157, 124, 113 }, { 153, 119, 107 }, { 134, 99, 86 }, { 128, 90, 80 }, { 145, 106, 92 }, { 195, 160, 149 }, { 226, 196, 184 }, { 239, 209, 197 }, { 189, 148, 135 }, { 147, 103, 93 }, { 145, 99, 93 }, { 146, 93, 91 }, { 166, 116, 112 }, { 198, 160, 152 }, { 143, 112, 107 } },
    { { 162, 133, 122 }, { 151, 117, 102 }, { 157, 118, 104 }, { 165, 125, 111 }, { 157, 115, 100 }, { 150, 109, 93 }, { 186, 150, 139 }, { 224, 195, 182 }, { 238, 207, 194 }, { 194, 154, 140 }, { 155, 114, 104 }, { 145, 101, 95 }, { 159, 107, 105 }, { 168, 118, 114 }, { 203, 165, 157 }, { 144, 112, 108 } },
    { { 181, 152, 141 }, { 188, 152, 140 }, { 174, 135, 120 }, { 165, 126, 111 }, { 148, 107, 92 }, { 129, 88, 74 }, { 144, 110, 103 }, { 203, 170, 159 }, { 222, 187, 174 }, { 195, 157, 144 }, { 188, 150, 142 }, { 144, 98, 92 }, { 178, 127, 124 }, { 173, 129, 123 }, { 204, 165, 156 }, { 114, 93, 91 } },
    { { 190, 160, 149 }, { 177, 141, 129 }, { 180, 142, 125 }, { 166, 128, 111 }, { 131, 91, 79 }, { 111, 76, 67 }, { 118, 93, 94 }, { 177, 143, 134 }, { 206, 172, 158 }, { 181, 140, 128 }, { 198, 161, 154 }, { 114, 77, 74 }, { 174, 130, 128 }, { 184, 144, 136 }, { 177, 148, 142 }, { 38, 36, 44 } },
    { { 170, 140, 127 }, { 158, 122, 104 }, { 148, 109, 92 }, { 153, 113, 97 }, { 124, 84, 73 }, { 92, 64, 60 }, { 100, 83, 88 }, { 167, 134, 126 }, { 222, 191, 177 }, { 172, 131, 116 }, { 141, 96, 87 }, { 105, 67, 59 }, { 155, 111, 103 }, { 184, 144, 131 }, { 100, 86, 84 }, { 21, 30, 49 } },
    { { 189, 161, 147 }, { 168, 134, 117 }, { 143, 104, 85 }, { 131, 91, 76 }, { 133, 95, 82 }, { 91, 63, 58 }, { 111, 94, 96 }, { 175, 139, 130 }, { 229, 202, 192 }, { 204, 164, 149 }, { 148, 100, 85 }, { 141, 97, 84 }, { 156, 110, 95 }, { 138, 105, 94 }, { 44, 47, 64 }, { 23, 34, 57 } },
    { { 187, 159, 148 }, { 181, 149, 131 }, { 172, 135, 116 }, { 146, 108, 91 }, { 136, 100, 86 }, { 109, 78, 68 }, { 128, 102, 101 }, { 191, 157, 147 }, { 227, 200, 189 }, { 211, 173, 158 }, { 169, 121, 102 }, { 162, 116, 97 }, { 150, 111, 93 }, { 77, 64, 64 }, { 31, 46, 70 }, { 26, 42, 70 } },
    { { 126, 100, 90 }, { 148, 118, 104 }, { 160, 124, 106 }, { 158, 123, 104 }, { 126, 89, 73 }, { 135, 97, 81 }, { 159, 125, 114 }, { 198, 164, 150 }, { 213, 182, 166 }, { 207, 171, 155 }, { 170, 127, 108 }, { 164, 120, 103 }, { 120, 96, 90 }, { 32, 41, 65 }, { 30, 44, 70 }, { 33, 56, 84 } },
    { { 111, 85, 74 }, { 89, 62, 53 }, { 122, 92, 78 }, { 136, 103, 87 }, { 121, 85, 70 }, { 142, 104, 88 }, { 176, 138, 120 }, { 183, 149, 130 }, { 188, 153, 134 }, { 187, 153, 136 }, { 171, 137, 120 }, { 139, 114, 101 }, { 47, 49, 66 }, { 41, 56, 84 }, { 30, 43, 68 }, { 112, 154, 172 } },
    { { 125, 101, 89 }, { 102, 79, 66 }, { 86, 62, 52 }, { 117, 87, 74 }, { 99, 69, 54 }, { 102, 70, 55 }, { 118, 85, 69 }, { 121, 88, 71 }, { 130, 94, 78 }, { 134, 106, 91 }, { 123, 107, 93 }, { 109, 98, 92 }, { 90, 95, 115 }, { 43, 54, 81 }, { 35, 44, 71 }, { 38, 52, 80 } },
    { { 116, 97, 86 }, { 113, 92, 80 }, { 97, 75, 65 }, { 97, 73, 61 }, { 97, 73, 59 }, { 87, 60, 47 }, { 91, 64, 51 }, { 106, 79, 66 }, { 128, 104, 89 }, { 138, 117, 104 }, { 124, 110, 100 }, { 123, 117, 105 }, { 102, 104, 99 }, { 80, 86, 101 }, { 48, 54, 88 }, { 46, 52, 84 } },
    { { 112, 100, 96 }, { 115, 99, 92 }, { 105, 87, 78 }, { 100, 81, 70 }, { 87, 64, 55 }, { 89, 64, 52 }, { 97, 73, 61 }, { 111, 90, 75 }, { 133, 114, 98 }, { 113, 92, 79 }, { 121, 107, 95 }, { 124, 117, 106 }, { 113, 109, 103 }, { 117, 124, 121 }, { 70, 83, 89 }, { 54, 60, 88 } },
    { { 127, 120, 124 }, { 131, 123, 125 }, { 124, 112, 107 }, { 126, 114, 106 }, { 132, 118, 111 }, { 114, 96, 88 }, { 96, 74, 64 }, { 92, 69, 55 }, { 105, 82, 65 }, { 122, 103, 88 }, { 117, 105, 91 }, { 114, 101, 89 }, { 108, 102, 91 }, { 120, 127, 115 }, { 85, 102, 90 }, { 79, 92, 123 } },
    { { 131, 124, 129 }, { 133, 126, 130 }, { 136, 126, 127 }, { 128, 123, 119 }, { 116, 106, 101 }, { 135, 119, 113 }, { 147, 129, 120 }, { 135, 116, 103 }, { 126, 106, 88 }, { 133, 115, 95 }, { 122, 107, 90 }, { 118, 106, 90 }, { 94, 89, 75 }, { 113, 117, 101 }, { 93, 110, 93 }, { 90, 108, 132 } },
    { { 129, 122, 128 }, { 122, 115, 120 }, { 119, 112, 116 }, { 116, 113, 112 }, { 120, 112, 111 }, { 130, 120, 117 }, { 131, 120, 112 }, { 133, 119, 107 }, { 132, 114, 97 }, { 131, 113, 93 }, { 128, 111, 91 }, { 138, 121, 105 }, { 124, 122, 103 }, { 116, 119, 96 }, { 109, 121, 98 }, { 97, 113, 118 } } };

static CRGB __imgArletErola[33][16] = {
    { { 51, 34, 26 }, { 46, 29, 21 }, { 41, 27, 18 }, { 34, 21, 14 }, { 32, 18, 14 }, { 35, 20, 15 }, { 46, 26, 18 }, { 59, 34, 19 }, { 72, 47, 32 }, { 60, 44, 37 }, { 44, 51, 44 }, { 37, 69, 76 }, { 42, 77, 84 }, { 65, 72, 71 }, { 18, 25, 42 }, { 19, 25, 43 } },
    { { 54, 36, 29 }, { 43, 27, 20 }, { 30, 17, 15 }, { 24, 12, 11 }, { 27, 14, 11 }, { 46, 30, 26 }, { 59, 37, 28 }, { 86, 59, 46 }, { 86, 62, 48 }, { 82, 60, 48 }, { 93, 86, 66 }, { 49, 68, 71 }, { 17, 49, 59 }, { 29, 56, 67 }, { 31, 62, 74 }, { 17, 33, 46 } },
    { { 52, 36, 31 }, { 36, 21, 18 }, { 33, 19, 16 }, { 33, 19, 16 }, { 29, 15, 13 }, { 50, 29, 22 }, { 75, 50, 40 }, { 99, 74, 62 }, { 108, 82, 71 }, { 107, 81, 68 }, { 96, 69, 55 }, { 87, 65, 55 }, { 50, 64, 67 }, { 29, 79, 91 }, { 28, 74, 86 }, { 28, 70, 80 } },
    { { 50, 32, 28 }, { 41, 24, 20 }, { 39, 22, 18 }, { 44, 25, 20 }, { 59, 38, 32 }, { 52, 33, 27 }, { 60, 41, 37 }, { 101, 76, 68 }, { 121, 99, 91 }, { 111, 83, 71 }, { 91, 57, 43 }, { 86, 55, 40 }, { 83, 56, 44 }, { 42, 45, 47 }, { 28, 76, 86 }, { 30, 68, 79 } },
    { { 54, 35, 30 }, { 49, 31, 24 }, { 52, 32, 26 }, { 54, 34, 28 }, { 50, 30, 25 }, { 27, 13, 10 }, { 56, 43, 42 }, { 94, 69, 63 }, { 125, 105, 96 }, { 109, 79, 68 }, { 82, 46, 34 }, { 80, 48, 37 }, { 85, 53, 41 }, { 78, 55, 44 }, { 21, 22, 24 }, { 30, 63, 70 } },
    { { 68, 49, 44 }, { 62, 42, 35 }, { 53, 32, 24 }, { 54, 32, 26 }, { 48, 25, 19 }, { 43, 24, 18 }, { 54, 40, 42 }, { 94, 68, 63 }, { 120, 97, 88 }, { 93, 63, 54 }, { 78, 49, 42 }, { 54, 27, 24 }, { 85, 53, 46 }, { 96, 66, 58 }, { 73, 54, 50 }, { 13, 3, 9 } },
    { { 69, 52, 48 }, { 76, 54, 48 }, { 77, 54, 46 }, { 77, 50, 41 }, { 70, 41, 32 }, { 62, 35, 25 }, { 68, 46, 42 }, { 100, 76, 68 }, { 106, 79, 70 }, { 98, 70, 61 }, { 97, 70, 63 }, { 70, 39, 35 }, { 85, 51, 48 }, { 92, 60, 53 }, { 107, 81, 74 }, { 51, 37, 35 } },
    { { 88, 68, 63 }, { 99, 75, 67 }, { 97, 70, 61 }, { 91, 62, 52 }, { 75, 45, 37 }, { 69, 41, 31 }, { 87, 61, 52 }, { 117, 93, 84 }, { 121, 99, 90 }, { 99, 71, 62 }, { 87, 57, 51 }, { 78, 44, 41 }, { 81, 43, 41 }, { 85, 50, 46 }, { 110, 82, 76 }, { 72, 50, 45 } },
    { { 79, 59, 52 }, { 77, 53, 44 }, { 78, 53, 44 }, { 75, 50, 40 }, { 77, 48, 38 }, { 82, 53, 41 }, { 107, 82, 74 }, { 125, 106, 96 }, { 127, 114, 104 }, { 106, 77, 67 }, { 80, 50, 43 }, { 77, 46, 42 }, { 82, 45, 44 }, { 91, 55, 52 }, { 112, 85, 79 }, { 74, 51, 48 } },
    { { 98, 77, 69 }, { 101, 75, 66 }, { 95, 68, 56 }, { 89, 61, 50 }, { 80, 50, 39 }, { 69, 40, 29 }, { 79, 55, 49 }, { 113, 90, 82 }, { 123, 100, 90 }, { 106, 80, 71 }, { 102, 76, 69 }, { 76, 44, 40 }, { 96, 60, 58 }, { 94, 63, 58 }, { 113, 85, 79 }, { 57, 42, 39 } },
    { { 100, 78, 70 }, { 94, 68, 58 }, { 92, 64, 52 }, { 89, 61, 49 }, { 67, 39, 30 }, { 52, 29, 23 }, { 56, 40, 42 }, { 94, 70, 64 }, { 118, 94, 84 }, { 93, 63, 54 }, { 98, 70, 65 }, { 53, 28, 25 }, { 92, 62, 59 }, { 103, 75, 67 }, { 78, 62, 58 }, { 6, 3, 13 } },
    { { 100, 80, 70 }, { 88, 65, 52 }, { 77, 49, 34 }, { 70, 42, 30 }, { 68, 41, 31 }, { 45, 26, 22 }, { 55, 42, 44 }, { 94, 70, 63 }, { 127, 107, 99 }, { 110, 81, 70 }, { 75, 42, 31 }, { 71, 41, 31 }, { 83, 50, 39 }, { 78, 54, 46 }, { 18, 17, 26 }, { 2, 7, 21 } },
    { { 93, 74, 66 }, { 95, 72, 60 }, { 93, 66, 52 }, { 80, 54, 41 }, { 69, 44, 33 }, { 60, 37, 28 }, { 72, 52, 49 }, { 107, 82, 75 }, { 125, 105, 96 }, { 116, 90, 78 }, { 92, 59, 44 }, { 88, 55, 41 }, { 75, 50, 39 }, { 27, 21, 25 }, { 3, 13, 30 }, { 6, 13, 33 } },
    { { 55, 36, 28 }, { 53, 34, 26 }, { 70, 47, 35 }, { 78, 54, 40 }, { 63, 38, 26 }, { 75, 48, 35 }, { 95, 68, 56 }, { 104, 79, 66 }, { 108, 84, 70 }, { 107, 82, 70 }, { 92, 66, 53 }, { 76, 54, 44 }, { 33, 26, 31 }, { 7, 18, 37 }, { 2, 12, 29 }, { 45, 71, 82 } },
    { { 65, 49, 39 }, { 52, 35, 25 }, { 42, 26, 19 }, { 57, 37, 27 }, { 49, 28, 16 }, { 48, 26, 14 }, { 58, 35, 24 }, { 59, 37, 24 }, { 68, 45, 33 }, { 72, 53, 42 }, { 52, 41, 32 }, { 43, 35, 31 }, { 41, 45, 55 }, { 16, 22, 39 }, { 7, 14, 34 }, { 16, 22, 39 } },
    { { 39, 30, 25 }, { 56, 43, 36 }, { 49, 35, 28 }, { 45, 30, 22 }, { 43, 27, 18 }, { 42, 24, 14 }, { 46, 28, 19 }, { 57, 40, 30 }, { 72, 43, 34 }, { 63, 47, 39 }, { 63, 45, 35 }, { 52, 35, 25 }, { 41, 35, 31 }, { 28, 32, 38 }, { 21, 29, 42 }, { 20, 22, 42 } },
    { { 1, 1, 1 }, { 19, 16, 15 }, { 22, 17, 14 }, { 16, 12, 9 }, { 63, 53, 48 }, { 60, 47, 41 }, { 54, 40, 33 }, { 46, 31, 21 }, { 48, 35, 24 }, { 32, 22, 16 }, { 23, 18, 15 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 21, 29, 26 }, { 30, 40, 62 } },
    { { 0, 0, 0 }, { 23, 20, 18 }, { 82, 72, 61 }, { 98, 84, 68 }, { 85, 69, 55 }, { 76, 60, 48 }, { 83, 65, 49 }, { 91, 70, 52 }, { 87, 68, 49 }, { 61, 45, 31 }, { 23, 16, 10 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 10, 15, 11 }, { 46, 57, 69 } },
    { { 35, 30, 27 }, { 92, 77, 63 }, { 107, 91, 75 }, { 106, 90, 73 }, { 101, 84, 67 }, { 91, 73, 56 }, { 80, 62, 45 }, { 89, 67, 49 }, { 92, 70, 48 }, { 95, 71, 48 }, { 95, 73, 53 }, { 76, 60, 46 }, { 7, 4, 2 }, { 0, 0, 0 }, { 38, 36, 35 }, { 99, 93, 92 } },
    { { 98, 82, 67 }, { 108, 91, 74 }, { 113, 97, 79 }, { 104, 87, 67 }, { 82, 66, 49 }, { 77, 57, 43 }, { 92, 71, 55 }, { 103, 83, 68 }, { 112, 92, 77 }, { 103, 81, 63 }, { 96, 73, 52 }, { 94, 71, 50 }, { 64, 48, 34 }, { 3, 1, 0 }, { 95, 81, 68 }, { 103, 82, 66 } },
    { { 105, 87, 71 }, { 89, 75, 61 }, { 74, 60, 50 }, { 50, 36, 27 }, { 41, 29, 23 }, { 62, 47, 39 }, { 56, 41, 33 }, { 77, 61, 53 }, { 98, 80, 68 }, { 102, 83, 70 }, { 103, 81, 65 }, { 98, 75, 56 }, { 93, 71, 54 }, { 85, 66, 52 }, { 101, 79, 63 }, { 88, 66, 51 } },
    { { 79, 65, 54 }, { 72, 60, 50 }, { 52, 40, 34 }, { 37, 27, 22 }, { 55, 42, 34 }, { 54, 42, 36 }, { 38, 32, 34 }, { 55, 44, 42 }, { 83, 64, 56 }, { 90, 71, 60 }, { 86, 65, 54 }, { 86, 64, 51 }, { 91, 68, 54 }, { 94, 71, 56 }, { 94, 71, 55 }, { 89, 66, 51 } },
    { { 65, 54, 46 }, { 51, 43, 38 }, { 53, 42, 36 }, { 58, 45, 38 }, { 67, 53, 44 }, { 51, 38, 31 }, { 44, 36, 38 }, { 56, 43, 41 }, { 80, 63, 53 }, { 81, 63, 52 }, { 78, 59, 49 }, { 72, 52, 43 }, { 76, 56, 46 }, { 86, 64, 51 }, { 75, 54, 42 }, { 64, 44, 35 } },
    { { 70, 60, 51 }, { 63, 52, 45 }, { 56, 44, 38 }, { 57, 44, 38 }, { 55, 41, 34 }, { 65, 49, 39 }, { 59, 45, 37 }, { 73, 57, 49 }, { 96, 78, 67 }, { 77, 58, 48 }, { 83, 64, 55 }, { 49, 34, 31 }, { 75, 55, 48 }, { 84, 64, 53 }, { 81, 61, 51 }, { 58, 40, 33 } },
    { { 53, 44, 40 }, { 43, 36, 33 }, { 44, 35, 30 }, { 37, 27, 23 }, { 53, 39, 33 }, { 71, 55, 45 }, { 75, 60, 49 }, { 83, 67, 56 }, { 90, 71, 62 }, { 69, 49, 43 }, { 77, 56, 49 }, { 49, 36, 37 }, { 69, 46, 42 }, { 77, 58, 48 }, { 76, 56, 48 }, { 51, 34, 29 } },
    { { 38, 34, 31 }, { 35, 29, 27 }, { 34, 27, 24 }, { 34, 25, 23 }, { 46, 34, 29 }, { 62, 47, 40 }, { 63, 49, 42 }, { 73, 56, 48 }, { 79, 61, 52 }, { 65, 47, 42 }, { 73, 53, 47 }, { 46, 34, 38 }, { 67, 41, 38 }, { 78, 59, 52 }, { 72, 52, 45 }, { 47, 31, 28 } },
    { { 47, 41, 37 }, { 44, 36, 32 }, { 42, 33, 29 }, { 42, 31, 27 }, { 47, 34, 30 }, { 51, 38, 33 }, { 46, 38, 39 }, { 64, 50, 47 }, { 78, 62, 55 }, { 74, 56, 47 }, { 74, 56, 49 }, { 39, 27, 27 }, { 67, 47, 43 }, { 75, 58, 50 }, { 67, 49, 42 }, { 43, 28, 27 } },
    { { 53, 47, 42 }, { 45, 39, 35 }, { 49, 39, 33 }, { 50, 36, 29 }, { 55, 41, 35 }, { 57, 45, 40 }, { 27, 28, 34 }, { 61, 48, 46 }, { 82, 66, 59 }, { 82, 63, 54 }, { 75, 57, 49 }, { 64, 46, 40 }, { 69, 52, 47 }, { 68, 52, 45 }, { 52, 36, 33 }, { 42, 28, 27 } },
    { { 61, 56, 52 }, { 52, 47, 43 }, { 46, 38, 34 }, { 46, 35, 30 }, { 43, 31, 26 }, { 61, 48, 44 }, { 41, 33, 34 }, { 72, 57, 52 }, { 87, 70, 61 }, { 86, 67, 58 }, { 76, 57, 48 }, { 71, 51, 43 }, { 65, 47, 41 }, { 58, 42, 39 }, { 61, 55, 61 }, { 76, 75, 84 } },
    { { 56, 56, 55 }, { 56, 52, 48 }, { 52, 47, 43 }, { 49, 42, 38 }, { 37, 29, 26 }, { 45, 34, 31 }, { 61, 47, 41 }, { 76, 61, 53 }, { 83, 66, 55 }, { 78, 60, 50 }, { 69, 51, 42 }, { 61, 45, 38 }, { 57, 42, 37 }, { 77, 73, 80 }, { 79, 80, 92 }, { 78, 79, 92 } },
    { { 4, 4, 5 }, { 45, 46, 45 }, { 61, 59, 58 }, { 51, 46, 42 }, { 51, 43, 39 }, { 45, 36, 32 }, { 46, 36, 32 }, { 58, 46, 38 }, { 62, 48, 41 }, { 61, 46, 39 }, { 58, 44, 36 }, { 69, 62, 59 }, { 66, 59, 55 }, { 78, 75, 79 }, { 89, 92, 105 }, { 84, 87, 101 } },
    { { 0, 0, 0 }, { 0, 0, 0 }, { 24, 25, 25 }, { 56, 56, 56 }, { 57, 54, 53 }, { 61, 57, 55 }, { 52, 42, 39 }, { 41, 31, 28 }, { 45, 32, 28 }, { 50, 35, 28 }, { 59, 44, 35 }, { 57, 44, 37 }, { 52, 44, 38 }, { 43, 34, 31 }, { 71, 70, 75 }, { 89, 94, 107 } },
    { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 1 }, { 31, 33, 34 }, { 51, 52, 53 }, { 70, 67, 64 }, { 61, 51, 45 }, { 65, 54, 46 }, { 68, 57, 49 }, { 67, 59, 53 }, { 24, 22, 21 }, { 0, 0, 0 }, { 2, 2, 1 }, { 74, 76, 83 } }
};

static CRGB __imgArletErola2[33][16] = {
    { { 24, 15, 11 }, { 19, 11, 8 }, { 26, 16, 11 }, { 36, 22, 17 }, { 32, 18, 14 }, { 35, 20, 15 }, { 44, 25, 17 }, { 51, 29, 16 }, { 30, 19, 11 }, { 24, 16, 13 }, { 42, 49, 42 }, { 37, 69, 76 }, { 42, 77, 84 }, { 65, 72, 71 }, { 18, 25, 42 }, { 19, 25, 43 } },
    { { 23, 14, 11 }, { 30, 18, 13 }, { 39, 24, 19 }, { 39, 24, 18 }, { 36, 21, 16 }, { 46, 30, 26 }, { 59, 37, 28 }, { 86, 59, 46 }, { 86, 62, 48 }, { 82, 60, 48 }, { 93, 86, 66 }, { 49, 68, 71 }, { 17, 49, 59 }, { 29, 56, 67 }, { 31, 62, 74 }, { 17, 33, 46 } },
    { { 38, 25, 22 }, { 41, 25, 20 }, { 33, 19, 16 }, { 33, 19, 16 }, { 35, 20, 16 }, { 50, 29, 22 }, { 75, 50, 40 }, { 99, 74, 62 }, { 108, 82, 71 }, { 107, 81, 68 }, { 96, 69, 55 }, { 87, 65, 55 }, { 50, 64, 67 }, { 29, 79, 91 }, { 28, 74, 86 }, { 28, 70, 80 } },
    { { 50, 32, 28 }, { 41, 24, 20 }, { 39, 22, 18 }, { 44, 25, 20 }, { 57, 36, 30 }, { 52, 33, 27 }, { 60, 41, 37 }, { 101, 76, 68 }, { 121, 99, 91 }, { 111, 83, 71 }, { 91, 57, 43 }, { 86, 55, 40 }, { 83, 56, 44 }, { 42, 45, 47 }, { 28, 76, 86 }, { 30, 68, 79 } },
    { { 54, 35, 30 }, { 49, 31, 24 }, { 52, 32, 26 }, { 54, 34, 28 }, { 50, 30, 25 }, { 40, 22, 19 }, { 32, 29, 37 }, { 94, 68, 63 }, { 125, 105, 96 }, { 109, 79, 68 }, { 82, 46, 34 }, { 80, 48, 37 }, { 85, 53, 41 }, { 78, 55, 44 }, { 21, 22, 24 }, { 30, 63, 70 } },
    { { 68, 49, 44 }, { 62, 42, 35 }, { 53, 32, 24 }, { 54, 32, 26 }, { 48, 25, 19 }, { 42, 23, 20 }, { 29, 28, 36 }, { 91, 66, 62 }, { 120, 97, 88 }, { 93, 63, 54 }, { 78, 49, 42 }, { 54, 27, 24 }, { 85, 53, 46 }, { 96, 66, 58 }, { 73, 54, 50 }, { 13, 3, 9 } },
    { { 69, 52, 48 }, { 76, 54, 48 }, { 77, 54, 46 }, { 77, 50, 41 }, { 70, 41, 32 }, { 62, 35, 25 }, { 68, 46, 42 }, { 100, 76, 68 }, { 106, 79, 70 }, { 98, 70, 61 }, { 97, 70, 63 }, { 70, 39, 35 }, { 85, 51, 48 }, { 92, 60, 53 }, { 107, 81, 74 }, { 51, 37, 35 } },
    { { 88, 68, 63 }, { 99, 75, 67 }, { 97, 70, 61 }, { 91, 62, 52 }, { 75, 45, 37 }, { 69, 41, 31 }, { 87, 61, 52 }, { 117, 93, 84 }, { 121, 99, 90 }, { 99, 71, 62 }, { 87, 57, 51 }, { 78, 44, 41 }, { 81, 43, 41 }, { 85, 50, 46 }, { 110, 82, 76 }, { 72, 50, 45 } },
    { { 79, 59, 52 }, { 77, 53, 44 }, { 78, 53, 44 }, { 75, 50, 40 }, { 77, 48, 38 }, { 82, 53, 41 }, { 107, 82, 74 }, { 125, 106, 96 }, { 127, 114, 104 }, { 106, 77, 67 }, { 80, 50, 43 }, { 77, 46, 42 }, { 82, 45, 44 }, { 91, 55, 52 }, { 112, 85, 79 }, { 74, 51, 48 } },
    { { 98, 77, 69 }, { 101, 75, 66 }, { 95, 68, 56 }, { 89, 61, 50 }, { 80, 50, 39 }, { 69, 40, 29 }, { 79, 55, 49 }, { 113, 90, 82 }, { 123, 100, 90 }, { 106, 80, 71 }, { 102, 76, 69 }, { 76, 44, 40 }, { 96, 60, 58 }, { 94, 63, 58 }, { 113, 85, 79 }, { 57, 42, 39 } },
    { { 100, 78, 70 }, { 94, 68, 58 }, { 92, 64, 52 }, { 89, 61, 49 }, { 67, 39, 30 }, { 52, 29, 23 }, { 40, 31, 39 }, { 90, 67, 62 }, { 118, 94, 84 }, { 93, 63, 54 }, { 98, 70, 65 }, { 53, 28, 25 }, { 92, 62, 59 }, { 103, 75, 67 }, { 78, 62, 58 }, { 6, 3, 13 } },
    { { 100, 80, 70 }, { 88, 65, 52 }, { 77, 49, 34 }, { 70, 42, 30 }, { 68, 41, 31 }, { 45, 26, 22 }, { 29, 28, 38 }, { 94, 70, 63 }, { 127, 107, 99 }, { 110, 81, 70 }, { 75, 42, 31 }, { 71, 41, 31 }, { 83, 50, 39 }, { 78, 54, 46 }, { 18, 17, 26 }, { 2, 7, 21 } },
    { { 93, 74, 66 }, { 95, 72, 60 }, { 93, 66, 52 }, { 80, 54, 41 }, { 69, 44, 33 }, { 60, 37, 28 }, { 72, 52, 49 }, { 107, 82, 75 }, { 125, 105, 96 }, { 116, 90, 78 }, { 92, 59, 44 }, { 88, 55, 41 }, { 75, 50, 39 }, { 27, 21, 25 }, { 3, 13, 30 }, { 6, 13, 33 } },
    { { 55, 36, 28 }, { 53, 34, 26 }, { 70, 47, 35 }, { 78, 54, 40 }, { 63, 38, 26 }, { 75, 48, 35 }, { 95, 68, 56 }, { 104, 79, 66 }, { 108, 84, 70 }, { 107, 82, 70 }, { 92, 66, 53 }, { 76, 54, 44 }, { 33, 26, 31 }, { 7, 18, 37 }, { 2, 12, 29 }, { 45, 71, 82 } },
    { { 9, 5, 4 }, { 31, 21, 14 }, { 40, 25, 18 }, { 57, 37, 27 }, { 49, 28, 16 }, { 48, 26, 14 }, { 58, 35, 24 }, { 59, 37, 24 }, { 68, 45, 33 }, { 72, 53, 42 }, { 52, 41, 32 }, { 37, 30, 26 }, { 39, 43, 53 }, { 16, 22, 39 }, { 7, 14, 34 }, { 16, 22, 39 } },
    { { 15, 11, 8 }, { 14, 10, 8 }, { 24, 16, 13 }, { 45, 30, 22 }, { 37, 23, 16 }, { 42, 24, 14 }, { 46, 28, 19 }, { 54, 39, 29 }, { 69, 42, 33 }, { 41, 30, 24 }, { 17, 11, 8 }, { 17, 10, 6 }, { 36, 31, 27 }, { 28, 32, 38 }, { 21, 29, 42 }, { 20, 22, 42 } },
    { { 1, 1, 1 }, { 8, 6, 6 }, { 16, 13, 10 }, { 16, 12, 9 }, { 33, 27, 25 }, { 43, 33, 29 }, { 54, 40, 33 }, { 38, 25, 17 }, { 40, 29, 19 }, { 29, 19, 14 }, { 9, 7, 5 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 21, 29, 26 }, { 30, 40, 62 } },
    { { 0, 0, 0 }, { 23, 20, 18 }, { 82, 72, 61 }, { 98, 84, 68 }, { 85, 69, 55 }, { 76, 60, 48 }, { 83, 65, 49 }, { 91, 70, 52 }, { 87, 68, 49 }, { 61, 45, 31 }, { 23, 16, 10 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 10, 15, 11 }, { 46, 57, 69 } },
    { { 35, 30, 27 }, { 92, 77, 63 }, { 107, 91, 75 }, { 106, 90, 73 }, { 101, 84, 67 }, { 91, 73, 56 }, { 80, 62, 45 }, { 89, 67, 49 }, { 92, 70, 48 }, { 95, 71, 48 }, { 95, 73, 53 }, { 76, 60, 46 }, { 7, 4, 2 }, { 0, 0, 0 }, { 38, 36, 35 }, { 99, 93, 92 } },
    { { 98, 82, 67 }, { 108, 91, 74 }, { 113, 97, 79 }, { 104, 87, 67 }, { 82, 66, 49 }, { 77, 57, 43 }, { 92, 71, 55 }, { 103, 83, 68 }, { 112, 92, 77 }, { 103, 81, 63 }, { 96, 73, 52 }, { 94, 71, 50 }, { 64, 48, 34 }, { 3, 1, 0 }, { 95, 81, 68 }, { 103, 82, 66 } },
    { { 105, 87, 71 }, { 89, 75, 61 }, { 74, 60, 50 }, { 50, 36, 27 }, { 41, 29, 23 }, { 62, 47, 39 }, { 56, 41, 33 }, { 77, 61, 53 }, { 98, 80, 68 }, { 102, 83, 70 }, { 103, 81, 65 }, { 98, 75, 56 }, { 93, 71, 54 }, { 85, 66, 52 }, { 101, 79, 63 }, { 88, 66, 51 } },
    { { 79, 65, 54 }, { 72, 60, 50 }, { 52, 40, 34 }, { 37, 27, 22 }, { 55, 42, 34 }, { 54, 42, 36 }, { 36, 31, 34 }, { 55, 44, 42 }, { 83, 64, 56 }, { 90, 71, 60 }, { 86, 65, 54 }, { 86, 64, 51 }, { 91, 68, 54 }, { 94, 71, 56 }, { 94, 71, 55 }, { 89, 66, 51 } },
    { { 65, 54, 46 }, { 51, 43, 38 }, { 53, 42, 36 }, { 58, 45, 38 }, { 67, 53, 44 }, { 51, 38, 31 }, { 25, 26, 36 }, { 56, 43, 41 }, { 80, 63, 53 }, { 81, 63, 52 }, { 78, 59, 49 }, { 72, 52, 43 }, { 76, 56, 46 }, { 86, 64, 51 }, { 75, 54, 42 }, { 64, 44, 35 } },
    { { 70, 60, 51 }, { 63, 52, 45 }, { 56, 44, 38 }, { 57, 44, 38 }, { 55, 41, 34 }, { 65, 49, 39 }, { 59, 45, 37 }, { 73, 57, 49 }, { 96, 78, 67 }, { 77, 58, 48 }, { 83, 64, 55 }, { 49, 34, 31 }, { 75, 55, 48 }, { 84, 64, 53 }, { 81, 61, 51 }, { 58, 40, 33 } },
    { { 53, 44, 40 }, { 43, 36, 33 }, { 44, 35, 30 }, { 37, 27, 23 }, { 53, 39, 33 }, { 71, 55, 45 }, { 75, 60, 49 }, { 83, 67, 56 }, { 90, 71, 62 }, { 69, 49, 43 }, { 77, 56, 49 }, { 49, 36, 37 }, { 69, 46, 42 }, { 77, 58, 48 }, { 76, 56, 48 }, { 51, 34, 29 } },
    { { 38, 34, 31 }, { 35, 29, 27 }, { 34, 27, 24 }, { 34, 25, 23 }, { 46, 34, 29 }, { 62, 47, 40 }, { 63, 49, 42 }, { 73, 56, 48 }, { 79, 61, 52 }, { 65, 47, 42 }, { 73, 53, 47 }, { 46, 34, 38 }, { 67, 41, 38 }, { 78, 59, 52 }, { 72, 52, 45 }, { 47, 31, 28 } },
    { { 47, 41, 37 }, { 44, 36, 32 }, { 42, 33, 29 }, { 42, 31, 27 }, { 47, 34, 30 }, { 51, 38, 33 }, { 46, 38, 39 }, { 64, 50, 47 }, { 78, 62, 55 }, { 74, 56, 47 }, { 74, 56, 49 }, { 39, 27, 27 }, { 67, 47, 43 }, { 75, 58, 50 }, { 67, 49, 42 }, { 43, 28, 27 } },
    { { 53, 47, 42 }, { 45, 39, 35 }, { 49, 39, 33 }, { 50, 36, 29 }, { 55, 41, 35 }, { 55, 43, 40 }, { 22, 25, 35 }, { 61, 48, 46 }, { 82, 66, 59 }, { 82, 63, 54 }, { 75, 57, 49 }, { 64, 46, 40 }, { 69, 52, 47 }, { 68, 52, 45 }, { 52, 36, 33 }, { 42, 28, 27 } },
    { { 61, 56, 52 }, { 52, 47, 43 }, { 46, 38, 34 }, { 46, 35, 30 }, { 43, 31, 26 }, { 61, 48, 44 }, { 41, 33, 34 }, { 72, 57, 52 }, { 87, 70, 61 }, { 86, 67, 58 }, { 76, 57, 48 }, { 71, 51, 43 }, { 65, 47, 41 }, { 58, 42, 39 }, { 61, 55, 61 }, { 76, 75, 84 } },
    { { 56, 56, 55 }, { 56, 52, 48 }, { 52, 47, 43 }, { 49, 42, 38 }, { 37, 29, 26 }, { 45, 34, 31 }, { 61, 47, 41 }, { 76, 61, 53 }, { 83, 66, 55 }, { 78, 60, 50 }, { 69, 51, 42 }, { 61, 45, 38 }, { 57, 42, 37 }, { 77, 73, 80 }, { 79, 80, 92 }, { 78, 79, 92 } },
    { { 4, 4, 5 }, { 45, 46, 45 }, { 61, 59, 58 }, { 51, 46, 42 }, { 51, 43, 39 }, { 45, 36, 32 }, { 46, 36, 32 }, { 58, 46, 38 }, { 62, 48, 41 }, { 61, 46, 39 }, { 58, 44, 36 }, { 69, 62, 59 }, { 66, 59, 55 }, { 78, 75, 79 }, { 89, 92, 105 }, { 84, 87, 101 } },
    { { 0, 0, 0 }, { 0, 0, 0 }, { 24, 25, 25 }, { 56, 56, 56 }, { 57, 54, 53 }, { 61, 57, 55 }, { 52, 42, 39 }, { 41, 31, 28 }, { 45, 32, 28 }, { 50, 35, 28 }, { 59, 44, 35 }, { 57, 44, 37 }, { 52, 44, 38 }, { 43, 34, 31 }, { 71, 70, 75 }, { 89, 94, 107 } },
    { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 1 }, { 31, 33, 34 }, { 51, 52, 53 }, { 70, 67, 64 }, { 61, 51, 45 }, { 65, 54, 46 }, { 68, 57, 49 }, { 67, 59, 53 }, { 24, 22, 21 }, { 0, 0, 0 }, { 2, 2, 1 }, { 74, 76, 83 } }
};

static CRGB __imgMontseJordi[33][16] = {
    { { 0, 0, 0 }, { 0, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0 }, { 48, 43, 40 }, { 81, 69, 63 }, { 75, 57, 54 }, { 75, 56, 53 }, { 73, 54, 50 }, { 60, 36, 33 }, { 71, 36, 31 }, { 81, 42, 36 }, { 86, 47, 38 }, { 85, 44, 33 }, { 81, 38, 27 }, { 81, 37, 28 } },
    { { 0, 0, 0 }, { 0, 0, 0 }, { 24, 21, 20 }, { 68, 60, 56 }, { 79, 67, 64 }, { 42, 27, 27 }, { 54, 38, 39 }, { 50, 30, 30 }, { 60, 35, 32 }, { 75, 43, 40 }, { 86, 49, 47 }, { 84, 46, 44 }, { 79, 35, 32 }, { 79, 35, 28 }, { 79, 35, 26 }, { 79, 37, 26 } },
    { { 0, 0, 0 }, { 0, 0, 0 }, { 78, 73, 71 }, { 90, 79, 75 }, { 67, 46, 43 }, { 84, 59, 56 }, { 97, 73, 71 }, { 81, 62, 63 }, { 105, 80, 78 }, { 105, 71, 66 }, { 97, 58, 52 }, { 93, 55, 51 }, { 87, 46, 45 }, { 75, 27, 26 }, { 76, 30, 24 }, { 83, 37, 28 } },
    { { 0, 0, 0 }, { 44, 40, 38 }, { 91, 79, 77 }, { 67, 53, 51 }, { 63, 39, 37 }, { 82, 57, 55 }, { 111, 93, 95 }, { 120, 99, 98 }, { 113, 88, 84 }, { 102, 66, 58 }, { 96, 58, 49 }, { 100, 62, 58 }, { 102, 64, 66 }, { 85, 35, 35 }, { 77, 27, 24 }, { 85, 37, 29 } },
    { { 0, 0, 0 }, { 86, 75, 72 }, { 74, 58, 55 }, { 47, 32, 34 }, { 78, 57, 57 }, { 121, 108, 113 }, { 127, 113, 116 }, { 126, 105, 104 }, { 112, 80, 75 }, { 87, 47, 35 }, { 88, 50, 40 }, { 103, 64, 64 }, { 106, 66, 68 }, { 97, 49, 45 }, { 83, 28, 26 }, { 92, 27, 28 } },
    { { 49, 43, 40 }, { 87, 70, 67 }, { 67, 47, 45 }, { 70, 49, 50 }, { 118, 103, 108 }, { 127, 114, 117 }, { 125, 103, 104 }, { 123, 95, 93 }, { 107, 67, 57 }, { 86, 44, 36 }, { 76, 42, 41 }, { 105, 69, 71 }, { 101, 60, 56 }, { 103, 58, 52 }, { 87, 33, 30 }, { 64, 4, 15 } },
    { { 86, 74, 71 }, { 82, 62, 60 }, { 63, 44, 46 }, { 102, 82, 87 }, { 126, 114, 122 }, { 112, 88, 82 }, { 117, 88, 83 }, { 121, 85, 79 }, { 107, 63, 53 }, { 96, 53, 47 }, { 101, 67, 65 }, { 102, 70, 69 }, { 96, 50, 45 }, { 102, 56, 49 }, { 88, 36, 32 }, { 0, 0, 0 } },
    { { 86, 72, 68 }, { 73, 53, 52 }, { 69, 53, 55 }, { 123, 110, 118 }, { 119, 104, 110 }, { 96, 69, 62 }, { 91, 64, 59 }, { 109, 72, 62 }, { 104, 61, 51 }, { 99, 57, 52 }, { 104, 66, 63 }, { 95, 62, 52 }, { 95, 49, 46 }, { 94, 46, 39 }, { 79, 33, 31 }, { 0, 0, 0 } },
    { { 80, 65, 61 }, { 62, 44, 43 }, { 95, 79, 82 }, { 126, 107, 113 }, { 100, 80, 80 }, { 101, 73, 65 }, { 61, 39, 34 }, { 96, 60, 50 }, { 109, 70, 65 }, { 110, 76, 80 }, { 91, 51, 45 }, { 93, 50, 43 }, { 87, 47, 40 }, { 87, 40, 33 }, { 66, 26, 24 }, { 1, 0, 0 } },
    { { 74, 55, 52 }, { 50, 32, 31 }, { 107, 84, 86 }, { 122, 96, 97 }, { 96, 72, 72 }, { 91, 59, 49 }, { 72, 41, 34 }, { 90, 55, 47 }, { 116, 84, 87 }, { 122, 91, 96 }, { 99, 56, 54 }, { 86, 39, 32 }, { 73, 31, 25 }, { 87, 41, 32 }, { 79, 23, 26 }, { 55, 8, 15 } },
    { { 61, 43, 40 }, { 46, 28, 28 }, { 109, 82, 80 }, { 119, 90, 88 }, { 114, 83, 82 }, { 65, 40, 36 }, { 84, 47, 37 }, { 98, 65, 58 }, { 120, 91, 93 }, { 117, 82, 82 }, { 96, 53, 47 }, { 89, 40, 33 }, { 83, 38, 27 }, { 86, 40, 31 }, { 57, 6, 12 }, { 62, 8, 16 } },
    { { 54, 36, 35 }, { 41, 22, 22 }, { 104, 73, 70 }, { 120, 86, 83 }, { 120, 85, 81 }, { 98, 63, 58 }, { 96, 58, 51 }, { 117, 80, 74 }, { 105, 71, 64 }, { 96, 56, 47 }, { 95, 48, 41 }, { 91, 39, 33 }, { 87, 37, 28 }, { 67, 27, 23 }, { 37, 0, 3 }, { 46, 2, 6 } },
    { { 49, 32, 32 }, { 33, 14, 15 }, { 102, 66, 63 }, { 117, 80, 77 }, { 118, 80, 74 }, { 114, 75, 66 }, { 111, 71, 61 }, { 100, 58, 49 }, { 73, 35, 27 }, { 85, 45, 37 }, { 109, 64, 55 }, { 105, 53, 46 }, { 79, 34, 30 }, { 28, 7, 7 }, { 35, 3, 4 }, { 40, 7, 8 } },
    { { 53, 32, 31 }, { 31, 13, 15 }, { 90, 60, 55 }, { 114, 78, 74 }, { 116, 78, 70 }, { 115, 74, 64 }, { 112, 70, 60 }, { 86, 47, 40 }, { 65, 32, 26 }, { 59, 27, 23 }, { 97, 55, 45 }, { 94, 48, 42 }, { 47, 20, 18 }, { 31, 5, 5 }, { 29, 4, 4 }, { 12, 1, 1 } },
    { { 61, 40, 38 }, { 44, 23, 22 }, { 64, 41, 38 }, { 108, 72, 68 }, { 115, 74, 65 }, { 114, 72, 61 }, { 111, 68, 59 }, { 77, 44, 39 }, { 72, 36, 29 }, { 60, 28, 23 }, { 77, 40, 30 }, { 47, 21, 18 }, { 1, 0, 0 }, { 32, 11, 8 }, { 41, 15, 13 }, { 14, 0, 0 } },
    { { 61, 46, 43 }, { 58, 38, 36 }, { 59, 40, 39 }, { 76, 47, 44 }, { 106, 65, 55 }, { 110, 66, 54 }, { 108, 61, 51 }, { 95, 55, 47 }, { 62, 31, 27 }, { 64, 29, 25 }, { 26, 8, 6 }, { 15, 3, 3 }, { 7, 4, 4 }, { 1, 0, 0 }, { 14, 5, 4 }, { 24, 7, 6 } },
    { { 36, 28, 26 }, { 69, 52, 52 }, { 56, 35, 33 }, { 68, 39, 37 }, { 90, 50, 42 }, { 105, 60, 48 }, { 99, 53, 42 }, { 90, 45, 36 }, { 54, 25, 22 }, { 15, 2, 2 }, { 19, 1, 2 }, { 9, 1, 1 }, { 21, 6, 6 }, { 1, 0, 0 }, { 0, 0, 0 }, { 5, 1, 1 } },
    { { 0, 0, 0 }, { 33, 24, 22 }, { 32, 20, 19 }, { 49, 29, 29 }, { 50, 22, 20 }, { 69, 33, 26 }, { 67, 30, 23 }, { 40, 14, 11 }, { 17, 0, 1 }, { 17, 3, 4 }, { 10, 0, 1 }, { 11, 2, 2 }, { 2, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
    { { 0, 0, 0 }, { 2, 1, 1 }, { 48, 46, 52 }, { 49, 41, 46 }, { 51, 29, 30 }, { 34, 9, 10 }, { 31, 6, 5 }, { 36, 8, 7 }, { 30, 4, 5 }, { 32, 8, 10 }, { 54, 17, 18 }, { 34, 8, 9 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
    { { 0, 0, 0 }, { 34, 34, 37 }, { 80, 83, 94 }, { 72, 65, 71 }, { 82, 55, 55 }, { 83, 48, 44 }, { 79, 41, 35 }, { 71, 36, 30 }, { 62, 33, 26 }, { 62, 25, 20 }, { 57, 19, 15 }, { 61, 22, 22 }, { 12, 5, 5 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
    { { 0, 0, 0 }, { 58, 58, 64 }, { 82, 86, 99 }, { 92, 84, 91 }, { 104, 76, 76 }, { 110, 74, 75 }, { 108, 70, 67 }, { 77, 47, 42 }, { 79, 44, 36 }, { 56, 23, 20 }, { 84, 44, 37 }, { 77, 33, 30 }, { 50, 18, 17 }, { 17, 8, 9 }, { 1, 1, 1 }, { 9, 19, 33 } },
    { { 0, 0, 0 }, { 68, 72, 79 }, { 96, 99, 107 }, { 110, 95, 101 }, { 116, 88, 87 }, { 120, 89, 93 }, { 118, 84, 83 }, { 89, 53, 47 }, { 69, 32, 27 }, { 78, 41, 37 }, { 94, 53, 49 }, { 103, 52, 49 }, { 79, 34, 30 }, { 51, 23, 23 }, { 18, 31, 47 }, { 39, 71, 116 } },
    { { 0, 0, 0 }, { 60, 63, 70 }, { 106, 108, 114 }, { 120, 104, 104 }, { 123, 93, 92 }, { 124, 95, 98 }, { 117, 83, 80 }, { 99, 61, 54 }, { 81, 44, 38 }, { 86, 46, 42 }, { 102, 56, 55 }, { 100, 51, 47 }, { 73, 37, 32 }, { 59, 27, 25 }, { 55, 57, 85 }, { 30, 63, 113 } },
    { { 0, 0, 0 }, { 53, 55, 61 }, { 115, 115, 118 }, { 124, 102, 98 }, { 125, 105, 102 }, { 126, 103, 101 }, { 99, 71, 64 }, { 110, 76, 66 }, { 117, 89, 90 }, { 104, 71, 72 }, { 93, 54, 53 }, { 64, 30, 28 }, { 49, 26, 24 }, { 54, 27, 24 }, { 49, 32, 42 }, { 33, 57, 105 } },
    { { 0, 0, 0 }, { 61, 65, 72 }, { 108, 107, 114 }, { 117, 93, 88 }, { 125, 103, 98 }, { 118, 97, 95 }, { 78, 50, 40 }, { 93, 59, 49 }, { 119, 85, 82 }, { 123, 90, 95 }, { 111, 72, 68 }, { 47, 20, 20 }, { 64, 27, 25 }, { 73, 41, 38 }, { 65, 43, 44 }, { 28, 39, 79 } },
    { { 0, 0, 0 }, { 35, 38, 42 }, { 100, 101, 108 }, { 112, 89, 85 }, { 116, 89, 83 }, { 95, 75, 74 }, { 81, 54, 44 }, { 80, 47, 40 }, { 120, 83, 82 }, { 120, 85, 86 }, { 101, 66, 59 }, { 71, 34, 32 }, { 85, 40, 41 }, { 76, 41, 38 }, { 79, 57, 59 }, { 59, 57, 83 } },
    { { 0, 0, 0 }, { 0, 0, 0 }, { 76, 81, 87 }, { 108, 94, 96 }, { 111, 88, 84 }, { 86, 67, 63 }, { 86, 60, 51 }, { 100, 67, 59 }, { 122, 84, 85 }, { 109, 69, 65 }, { 69, 44, 43 }, { 94, 52, 52 }, { 92, 54, 53 }, { 82, 51, 50 }, { 91, 68, 69 }, { 78, 65, 73 } },
    { { 0, 0, 0 }, { 0, 0, 0 }, { 31, 35, 38 }, { 91, 94, 101 }, { 108, 99, 97 }, { 95, 76, 72 }, { 94, 66, 56 }, { 116, 86, 84 }, { 125, 84, 86 }, { 95, 55, 50 }, { 85, 60, 59 }, { 103, 65, 62 }, { 93, 61, 55 }, { 98, 68, 68 }, { 99, 78, 78 }, { 77, 63, 63 } },
    { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 44, 47, 51 }, { 97, 98, 99 }, { 107, 93, 90 }, { 117, 92, 83 }, { 125, 98, 91 }, { 124, 88, 84 }, { 89, 54, 49 }, { 76, 48, 43 }, { 112, 77, 71 }, { 106, 75, 70 }, { 103, 80, 80 }, { 88, 71, 68 }, { 61, 47, 47 } },
    { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 51, 52, 53 }, { 98, 92, 94 }, { 114, 94, 90 }, { 126, 100, 89 }, { 124, 96, 88 }, { 98, 66, 56 }, { 66, 41, 35 }, { 96, 68, 63 }, { 100, 79, 78 }, { 115, 97, 97 }, { 98, 87, 84 }, { 70, 51, 50 } },
    { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 67, 41, 34 }, { 109, 79, 70 }, { 107, 87, 81 }, { 124, 107, 99 }, { 99, 76, 67 }, { 90, 68, 60 }, { 78, 59, 54 }, { 96, 83, 81 }, { 107, 100, 100 }, { 94, 84, 83 }, { 82, 56, 48 } },
    { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 31, 19, 15 }, { 78, 60, 56 }, { 99, 90, 88 }, { 111, 102, 99 }, { 102, 88, 82 }, { 88, 75, 69 }, { 82, 70, 68 }, { 90, 73, 68 }, { 90, 63, 53 }, { 86, 57, 46 } },
    { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 1, 1, 1 }, { 23, 23, 23 }, { 83, 74, 72 }, { 101, 85, 82 }, { 100, 78, 69 }, { 98, 70, 57 }, { 93, 63, 50 }, { 86, 58, 46 } }
};

struct ParametricCurve {
    uint16_t initialPoints[64];
    float xCoord[252];
    float yCoord[252];
};

// ParametricCurve __2to3PiHalf = {
//     //{ 0, 25, 50, 75, 100, 125, 150, 175, 200, 225 },
//     { 0, 31, 63, 94, 126, 157, 189, 220 },
//     //{ 0, 43, 85, 127, 169, 211 },
//     { 0, 0.045, 0.18, 0.403, 0.715, 1.112, 1.593, 2.155, 2.795, 3.509, 4.293, 5.143, 6.054, 7.021, 8.039, 9.101, 10.202, 11.336, 12.496, 13.676, 14.868, 16.067, 17.266, 18.457, 19.635, 20.793, 21.923, 23.02, 24.078, 25.09, 26.051, 26.955, 27.798, 28.575, 29.281, 29.912, 30.465, 30.937, 31.325, 31.626, 31.84, 31.965, 31.999, 31.944, 31.8, 31.566, 31.245, 30.838, 30.348, 29.777, 29.129, 28.407, 27.615, 26.758, 25.84, 24.867, 23.844, 22.777, 21.672, 20.535, 19.373, 18.191, 16.998, 15.798, 14.6, 13.41, 12.234, 11.079, 9.952, 8.859, 7.807, 6.8, 5.845, 4.947, 4.111, 3.342, 2.645, 2.022, 1.478, 1.015, 0.637, 0.345, 0.142, 0.027, 0.002, 0.067, 0.222, 0.466, 0.796, 1.213, 1.712, 2.292, 2.948, 3.679, 4.478, 5.342, 6.266, 7.245, 8.273, 9.345, 10.454, 11.594, 12.759, 13.942, 15.137, 16.336, 17.534, 18.723, 19.897, 21.049, 22.172, 23.261, 24.309, 25.31, 26.259, 27.15, 27.978, 28.739, 29.429, 30.043, 30.578, 31.031, 31.4, 31.682, 31.876, 31.98, 31.995, 31.92, 31.755, 31.502, 31.161, 30.736, 30.227, 29.639, 28.973, 28.235, 27.428, 26.557, 25.627, 24.642, 23.609, 22.533, 21.42, 20.277, 19.109, 17.925, 16.729, 15.529, 14.332, 13.145, 11.973, 10.824, 9.704, 8.62, 7.577, 6.581, 5.638, 4.754, 3.933, 3.18, 2.498, 1.893, 1.367, 0.923, 0.564, 0.292, 0.108, 0.014, 0.009, 0.094, 0.269, 0.532, 0.882, 1.317, 1.835, 2.432, 3.106, 3.852, 4.666, 5.544, 6.481, 7.472, 8.51, 9.59, 10.707, 11.853, 13.023, 14.209, 15.405, 16.605, 17.802, 18.988, 20.157, 21.303, 22.419, 23.499, 24.537, 25.527, 26.464, 27.341, 28.155, 28.9, 29.573, 30.17, 30.687, 31.121, 31.471, 31.733, 31.907, 31.991, 31.986, 31.891, 31.706, 31.433, 31.073, 30.629, 30.102, 29.496, 28.814, 28.06, 27.238, 26.353, 25.41, 24.414, 23.371, 22.286, 21.166, 20.017, 18.845, 17.657, 16.46, 15.26, 14.065, 12.88, 11.713, 10.57, 9.458, 8.382, 7.349, 6.365, 5.435, 4.564, 3.758, 3.02, 2.356, 1.768, 1.26, 0.835, 0.496, 0.243, 0.079, 0.005 },
//     { 8, 8.375, 8.749, 9.121, 9.49, 9.856, 10.216, 10.572, 10.921, 11.262, 11.596, 11.92, 12.235, 12.539, 12.832, 13.112, 13.38, 13.635, 13.875, 14.101, 14.311, 14.506, 14.684, 14.846, 14.99, 15.117, 15.227, 15.318, 15.391, 15.445, 15.481, 15.498, 15.497, 15.476, 15.437, 15.38, 15.304, 15.21, 15.097, 14.967, 14.82, 14.655, 14.474, 14.277, 14.064, 13.836, 13.593, 13.336, 13.066, 12.783, 12.489, 12.183, 11.866, 11.54, 11.205, 10.862, 10.512, 10.156, 9.794, 9.428, 9.058, 8.686, 8.312, 7.937, 7.562, 7.189, 6.817, 6.448, 6.083, 5.723, 5.369, 5.021, 4.681, 4.349, 4.026, 3.713, 3.411, 3.12, 2.842, 2.576, 2.324, 2.086, 1.863, 1.655, 1.463, 1.288, 1.129, 0.987, 0.863, 0.757, 0.669, 0.599, 0.547, 0.515, 0.501, 0.505, 0.529, 0.571, 0.632, 0.711, 0.808, 0.923, 1.056, 1.207, 1.374, 1.558, 1.758, 1.974, 2.204, 2.449, 2.708, 2.981, 3.266, 3.562, 3.87, 4.188, 4.515, 4.852, 5.196, 5.547, 5.904, 6.267, 6.634, 7.004, 7.377, 7.751, 8.126, 8.501, 8.874, 9.245, 9.613, 9.977, 10.337, 10.69, 11.036, 11.375, 11.706, 12.027, 12.338, 12.639, 12.927, 13.204, 13.467, 13.717, 13.953, 14.173, 14.378, 14.568, 14.74, 14.896, 15.035, 15.156, 15.259, 15.345, 15.411, 15.459, 15.489, 15.5, 15.492, 15.465, 15.42, 15.356, 15.274, 15.174, 15.055, 14.92, 14.766, 14.596, 14.409, 14.207, 13.989, 13.756, 13.508, 13.247, 12.972, 12.685, 12.387, 12.077, 11.758, 11.429, 11.091, 10.745, 10.393, 10.035, 9.672, 9.304, 8.933, 8.56, 8.186, 7.811, 7.436, 7.063, 6.693, 6.325, 5.962, 5.604, 5.251, 4.906, 4.568, 4.239, 3.92, 3.61, 3.312, 3.025, 2.751, 2.49, 2.242, 2.009, 1.791, 1.589, 1.402, 1.232, 1.079, 0.943, 0.825, 0.725, 0.643, 0.579, 0.534, 0.508, 5.00, 0.511, 0.541, 0.589, 0.656, 0.741, 0.845, 0.966, 1.105, 1.261, 1.434, 1.623, 1.829, 2.05, 2.285, 2.535, 2.799, 3.075, 3.364, 3.664, 3.976, 4.297, 4.628, 4.967, 5.313, 5.667, 6.026, 6.39, 6.758, 7.129, 7.503, 7.877 }
// };
