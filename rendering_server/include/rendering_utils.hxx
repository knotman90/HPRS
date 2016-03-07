#ifndef RENDERING_UTILS_H
#define RENDERING_UTILS_H

#include <bprinter/table_printer.h>
#include <iostream>

namespace HPRS {
inline void printStats(const int printPeriod) {
  sbprinter::TablePrinter tp(&std::cout);

  tp.AddColumn("TIMING", 20);
  tp.AddColumn("TIME(s)", 10);
  float time;
  int   frames;
  icetGetIntegerv(ICET_FRAME_COUNT, &frames);

  if (frames % printPeriod == 0) {
    tp.PrintHeader();
    icetGetFloatv(ICET_RENDER_TIME,       &time);
    tp << "RENDER TIME" << time;

    icetGetFloatv(ICET_BUFFER_READ_TIME,  &time);
    tp << "BUFFER_READ_TIME" << time;

    icetGetFloatv(ICET_BUFFER_WRITE_TIME, &time);
    tp << "BUFFER_WRITE_TIME" << time;

    icetGetFloatv(ICET_COMPRESS_TIME,     &time);
    tp << "COMPRESS_TIME" << time;

    icetGetFloatv(ICET_BLEND_TIME,        &time);
    tp << "BUFFER_BLEND_TIME" << time;

    icetGetFloatv(ICET_COLLECT_TIME,      &time);
    tp << "COLLECT_TIME" << time;

    icetGetFloatv(ICET_COMPOSITE_TIME,    &time);
    tp << "COMPOSITE_TIME" << time;

    icetGetFloatv(ICET_TOTAL_DRAW_TIME,   &time);
    tp << "TOTAL_DRAW_TIME" << time;


    tp << "FRAME_COUNT" << frames;
    tp.PrintFooter();
  }
}
} // namespace HPRS

#endif // RENDERING_UTILS_H
