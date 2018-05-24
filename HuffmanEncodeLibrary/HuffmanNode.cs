using System.Collections.Generic;
using System.Text;
using BitStreams;

namespace HuffmanEncodeLibrary {
    public class HuffmanNode {
        public HuffmanNode[] Childs;
        public List<Bit> Code;
        public int Value;

        public override string ToString() {
            var stringBuilder = new StringBuilder();

            foreach (var bit in Code) {
                stringBuilder.Append(bit.AsInt());
            }

            return stringBuilder.Append(" ").Append((char) Value).ToString();
        }
    }
}