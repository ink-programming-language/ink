// Translated from solution.cpp.

var N: dynamic = cpp_uninitialized();

var R: dynamic = cpp_uninitialized();

var vals: dynamic = cpp_array((1 << 18));

var sum: dynamic = 0;

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  write(fixed, setprecision(18));
  read(N, R);
  {
    var i: dynamic = 0;
    while ((i < ((1 << N))))
    {
      read(vals[i]);
      sum += vals[i];
      i += 1;
    }
  }
  write((sum / ((1 << N))), cpp_char("\n"));
  {
    var i: dynamic = 0;
    while ((i < R))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      read(x, y);
      sum += (y - vals[x]);
      vals[x] = y;
      write((sum / ((1 << N))), cpp_char("\n"));
      i += 1;
    }
  }
  return 0;
}
