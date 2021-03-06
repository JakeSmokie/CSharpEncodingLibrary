﻿using System;
using System.Collections.Generic;
using System.Linq;

namespace EncodeLibrary.Huffman {
    /// <summary>
    /// Creates table in specific format:
    /// 
    ///     ^ - node, l - left child, r - right child
    ///     { ^, ^l, ^r, ^ll, ^lr, ^rl, ^rr, ... }
    /// 
    ///     0 - non-leaf node whose left and right childs are non-leaf nodes
    /// 
    ///     1 - non-leaf node whose left child is leaf node
    ///     * - some byte to decode [exactly _left_ child leaf node]
    /// 
    ///     2 - non-leaf node whose right child is leaf node
    ///     * - some byte to decode [exactly _right_ child leaf node]
    /// 
    ///     3 - non-leaf node whose left and right childs are leaf nodes
    ///     * - some byte to decode [exactly _left_ child leaf node]
    ///     * - some byte to decode [exactly _right_ child leaf node]
    ///
    /// Example:
    ///
    ///     This sequence:
    ///     a = { 2, 1, q, 3, w, e, r } where `q, w, e, r` - some bytes
    ///
    ///     Will be interpreted as:
    ///     a[0]     ^
    ///             / \
    ///     a[1]   ^   r a[6]
    ///           / \
    ///     a[2] q   ^   a[3]
    ///             / \
    ///     a[4]   w   e a[5]
    /// 
    /// </summary>
    public class HuffmanTreeBuilder {
        private class Node {
            public double Amount;
            public byte? Value;
            public Node[] Childs;

            public override string ToString() {
                return $"{nameof(Amount)}: {Amount}, {nameof(Value)}: {(char?) Value}";
            }
        }

        /// <summary>
        /// Creates table from symbol frequencies
        /// </summary>
        public byte[] CreateTable(double[] frequencies, bool reverseCodes = false) {
            if (frequencies.Length != 0x100) {
                throw new HuffmanTreeBuilderException("Each frequency for byte should be expressed");
            }

            var nodes = MakeNodesFromFrequencies(frequencies);
            BuildTree(nodes, reverseCodes);
            var table = CreateTable(nodes.Last());

            return table;
        }

        private ICollection<Node> MakeNodesFromFrequencies(IEnumerable<double> frequencies) {
            return frequencies.Select((t, i) => new Node {
                    Value = (byte) i,
                    Amount = t
                })
                .ToList();
        }

        /// <summary>
        /// Creates table from some dataset
        /// </summary>
        public byte[] CreateTable(byte[] data, bool reverseCodes = false) {
            var nodes = CountSymbols(data);

            if (nodes.Count == 1) {
                throw new HuffmanTreeBuilderException("Alphabet contained 1 symbol. Cannot build tree.");
            }

            BuildTree(nodes, reverseCodes);
            var table = CreateTable(nodes.Last());

            return table;
        }

        private static List<Node> CountSymbols(IEnumerable<byte> data) {
            return data.GroupBy(b => b)
                .Select(symbol => new Node {
                    Value = symbol.Key,
                    Amount = symbol.Count()
                })
                .ToList();
        }

        private static void BuildTree(ICollection<Node> nodes, bool reverseCodes) {
            for (var i = 0; i < nodes.Count - 1; i += 2) {
                var leastAmountNodes = nodes.OrderBy(n => n.Amount).Skip(i).Take(2).ToArray();

                nodes.Add(new Node {
                    Childs = reverseCodes ? 
                        leastAmountNodes.Reverse().ToArray() :
                        leastAmountNodes,
                    Amount = leastAmountNodes.Sum(n => n.Amount)
                });
            }
        }

        private static byte[] CreateTable(Node node) {
            var table = new List<byte>();
            TraverseTree(node, table);

            return table.ToArray();
        }

        private static void TraverseTree(Node node, ICollection<byte> table) {
            if (node.Value != null) {
                table.Add((byte) node.Value);
                return;
            }

            table.Add((byte) ((node.Childs[0].Value is null ? 0 : 1) | (node.Childs[1].Value is null ? 0 : 2)));

            for (var i = 0; i < 2; i++) {
                if (node.Childs[i] != null)
                    TraverseTree(node.Childs[i], table);
            }
        }
    }
}