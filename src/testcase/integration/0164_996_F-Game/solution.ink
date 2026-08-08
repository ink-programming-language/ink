// Translated from solution.cpp.

var N: dynamic;

var R: dynamic;

var vals = cpp_array((1 << 18));

var sum = 0;

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  write(fixed, setprecision(18));
  read(N, R);
  {
    var i = 0;
    while ((i < ((1 << N))))
    {
      read(vals[i]);
      sum += vals[i];
      i += 1;
    }
  }
  write((sum / ((1 << N))), cpp_char("\n"));
  {
    var i = 0;
    while ((i < R))
    {
      var x: dynamic;
      var y: dynamic;
      read(x, y);
      sum += (y - vals[x]);
      vals[x] = y;
      write((sum / ((1 << N))), cpp_char("\n"));
      i += 1;
    }
  }
  return 0;
}
