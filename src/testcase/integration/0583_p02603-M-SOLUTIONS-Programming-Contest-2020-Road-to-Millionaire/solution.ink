// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  read(N);
  var yen: dynamic = 1000;
  var now: dynamic = cpp_uninitialized();
  read(now);
  {
    var i: dynamic = 0;
    while ((i < (N - 1)))
    {
      var nxt: dynamic = cpp_uninitialized();
      read(nxt);
      yen += max(0, ((yen / now) * ((nxt - now))));
      now = nxt;
      i += 1;
    }
  }
  write(yen, "\n");
}
