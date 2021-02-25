# MTLReadPixels

For some reason [getBytes:bytesPerRow:fromRegion:mipmapLevel:](https://developer.apple.com/documentation/metal/mtltexture/1515751-getbytes?language=objc) for drawable's texture is very slow on M1, so copy drawable's texture by the compute shader and then get the bytes.