/* Copyright (c) 2017 Pieter Wuille
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>
#include <vector>
#include <string>

namespace bech32
{

/** Encode a Bech32 string. Returns the empty string in case of failure. */
std::string encode(const std::string& hrp, const std::vector<uint8_t>& values);

/** Decode a Bech32 string. Returns (hrp, data). Empty hrp means failure. */
std::pair<std::string, std::vector<uint8_t> > decode(const std::string& str);



// 增加 convertbits  2020-04-15  yqq

typedef std::vector<uint8_t> data;

/** Convert from one power-of-2 number base to another. */
template<int frombits, int tobits, bool pad>
bool convertbits(data& out, const data& in) {
	int acc = 0;
	int bits = 0;
	const int maxv = (1 << tobits) - 1;
	const int max_acc = (1 << (frombits + tobits - 1)) - 1;
	for (size_t i = 0; i < in.size(); ++i) {
		int value = in[i];
		acc = ((acc << frombits) | value) & max_acc;
		bits += frombits;
		while (bits >= tobits) {
			bits -= tobits;
			out.push_back((acc >> bits) & maxv);
		}
	}
	if (pad) {
		if (bits) out.push_back((acc << (tobits - bits)) & maxv);
	}
	else if (bits >= frombits || ((acc << (tobits - bits)) & maxv)) {
		return false;
	}
	return true;
}

}
