// Translated from solution.cpp.

func main() -> dynamic
{
  cin.tie(0)->sync_with_stdio(0);
  cin.exceptions(cin.failbit);
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  var tot: dynamic = 0;
  var sumPos: dynamic = 0;
  {
    var cpp_name: dynamic = 0;
    while ((cpp_name < (k)))
    {
      var pos: dynamic = cpp_uninitialized();
      var num: dynamic = cpp_uninitialized();
      read(pos, num);
      pos -= 1;
      sumPos += (pos * num);
      sumPos %= n;
      tot += num;
      cpp_name += 1;
    }
  }
  if ((tot > n))
  {
    write(-1, "\n");
  } else if ((tot < n))
  {
    write(1, "\n");
  } else
  {
    var expected: dynamic = ((((n - 1)) * n) / 2);
    if (((expected % n) == sumPos))
    {
      write(1, "\n");
    } else
    {
      write(-1, "\n");
    }
  }
}
