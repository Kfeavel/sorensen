# Sorensen Compression
The world's most space efficient compression algorithm!

With Sorensen Compression, all files, no matter the size, can be compressed to only **24 bytes**.
You read that right. Only **24 bytes** to compress any file! By recording minimal information about the
original file, Sorensen Compression is able to save you precious storage space.

## How it works

What is a file? Nothing more than a pile of bits. By recording how many bits the original file contained
(the file size, `N`), how many of those bits were set to `1`, and a hash, we're able to recreate any file of any
size using only this information by walking every permutation of `1`s and `0`s for `N` bits.

## Tradeoffs

Everyone knows that space is far more precious than time. The universe has about 1.7 × 10<sup>106</sup> years left.
As such, it seems like a fair tradeoff that all of the space and information saved during compression is offset by
how much time it takes to decompress a zipped file.

If a more precise measure of how long Sorensen Compression takes is desired, the Big-O notation for the decompression
algorithm is \
**O( (2n)! / (n!)^2 )**.
