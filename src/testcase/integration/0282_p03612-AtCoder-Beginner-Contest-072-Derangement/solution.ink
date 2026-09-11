// Translated from solution.cpp.

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  read(N);
  var S: dynamic = 0;
  var seq: dynamic = 0;
  var p: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      read(p);
      if ((i != p))
      {
        S += ((seq / 2) + (seq % 2));
        seq = 0;
      }
      if ((i == p))
      {
        seq += 1;
      }
      i += 1;
    }
  }
  S += ((seq / 2) + (seq % 2));
  write(S, "\n");
  return 0;
}
