// Translated from solution.cpp.

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  var n: dynamic = cpp_uninitialized();
  read(n);
  var ans: dynamic = 0;
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      {
        var j: dynamic = (i + i);
        while ((j <= n))
        {
          ans += (j / i);
          j += i;
        }
      }
      i += 1;
    }
  }
  write((ans * 4), "\n");
  return 0;
}
