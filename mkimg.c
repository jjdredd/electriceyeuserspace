#include <jpeglib.h>
void mkimg(char *buff, int width, int height, FILE *streamio){
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  JSAMPROW row[1];

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  jpeg_stdio_dest(&cinfo, streamio);
  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;
  jpeg_set_defaults(&cinfo);
  jpeg_set_quality (&cinfo, 85, 1);
  jpeg_start_compress(&cinfo, 1);
  while (cinfo.next_scanline < cinfo.image_height){
    row[0] = (JSAMPROW) &buff[cinfo.next_scanline*width*3];
    jpeg_write_scanlines(&cinfo, row, 1);
  }
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
  return;
}

