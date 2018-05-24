using System;
using System.Collections;

namespace HuffmanEncodeLibrary {
    public struct Symbol {
        public const int SymbolsAmount = 256;

        public BitArray Code;
        public byte Sym;
        public int Count;

        public override string ToString() {
            var val = new int[1];
            Code.CopyTo(val, 0);

            return $"{nameof(Code)}: {Convert.ToString(val[0], 2)}, {nameof(Sym)}: {(char) Sym}, {nameof(Count)}: {Count}";
        }
    }
}