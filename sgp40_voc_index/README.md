# SGP40 VOC Index Driver Bundle

This driver allows to combine Sensirion's SGP40 and SHTC3 sensors to provide a
humidity compensated sensor signal. It further embeds a VOC algorithm that
post-processes the raw sensor output into the resulting VOC Index.

## Volatile Organic Compound (VOC) Index

The VOC Index represents an air quality value on a scale from 0 to 500 where a
lower value represents cleaner air.

## Folder content

 * `sensirion_voc_algorithm.c` / `sensirion_voc_algorithm.h` - Provides the
   algorithm to post-process the raw sensor output into the VOC Index.
 * `sgp40_voc_index.c` / `sgp40_voc_index.h` -  Combines the sgp40 and shtc1
   drivers with the VOC algorithm.
 * `sgp40_voc_index_example_usage.c` - Shows how to use the resulting bundle.
