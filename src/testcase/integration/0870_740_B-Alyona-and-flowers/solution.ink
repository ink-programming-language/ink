// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  for (var i: dynamic in val)
  {
    read(i);
  }
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var res: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      read(a, b);
      var t: dynamic = 0;
      {
        var j: dynamic = (a - 1);
        while ((j < b))
        {
          t += val[j];
          j += 1;
        }
      }
      if ((t > 0))
      {
        res += t;
      }
      i += 1;
    }
  }
  write(res, "\n");
  return 0;
}
