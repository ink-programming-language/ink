// Translated from solution.cpp.

var mod: dynamic = ((1e+9) + 7);

var size: dynamic = 1e+5;

func solve() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  s += ("d0l0l0l0l0l0l0l0l0l0l0l0l0l0l0l0l0l0l0l0l0l0l0l0l0l0l0l0l0l0l0l0l0l0lrr" + "rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrru");
  s += ("ddddddd1l1l1l1l1l1l1l1l1l1l1l1l1l1l1l1l1l1l1l1l1l1l1l1l1l1l1l1l1l1l1l1l" + "1l1lrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrruuuuuuu");
  {
    var i: dynamic = 0;
    while ((i < 32))
    {
      s += ("dd0d0d0d0d0uuuuuuuudstdddddddddustustuuuuuudstddddddddustuuuuuu0dl1ru" + "uu1" + "dddddstdstdstdstuuuuuudd0d0d0d0d0uuuuuuduudstddstdddddddustustuuuul1r" + "0u0" + "duu0dddddstdstdstdstuuuuuuudstdd1dddddstuuuuuul");
      i += 1;
    }
  }
  s += "ld";
  write(s);
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var beg: dynamic = clock();
  solve();
  var end: dynamic = clock();
  fprintf(stderr, "%.3f sec\n", (cpp_double((end - beg)) / CLOCKS_PER_SEC));
  return 0;
}
